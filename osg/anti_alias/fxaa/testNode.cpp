#include "testNode.h"

#include <format>
//#include <boost/filesystem.hpp>
#include <filesystem>
#include <fstream>

#include <osg/Geometry>
#include <osgUtil/CullVisitor>


const std::string vertShader = R"(

#version 330 compatibility

out vec4 clr;

void main()
{
	gl_Position = gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	clr = gl_Color;
}

)";

const std::string fragShader = R"(

#version 330 compatibility

in vec4 clr;

void main()
{
	vec4 clr_tmp = clr;
	clr_tmp.w = 0.213 * clr.r + 0.715 * clr.g + 0.072 * clr.b;
	gl_FragColor = clr_tmp;
}

)";

constexpr char fxaaShader[] = R"(

#version 330 compatibility

uniform sampler2D clr_tex;
uniform sampler2D dep_tex;

{0}

void main()
{{
	vec2 uv = gl_TexCoord[0].xy;
	gl_FragColor = texture(clr_tex, uv);
}}

)";

std::string readFile(const std::string &file)
{
	std::string ret;
	auto f = fopen(file.c_str(), "rt");
	if (!f) return ret;
	fseek(f, 0, SEEK_END);
	uint64_t len = ftell(f);
	fseek(f, 0, SEEK_SET);
	ret.resize(len);
	fread(&ret[0], 1, len, f);
	fclose(f);
	return ret;
}

TestNode::TestNode()
{
	setCullingActive(false);
	init();
}

void TestNode::init()
{
	_quad = createTexturedQuadGeometry(Vec3(0, -0.5, 0), Vec3(0.5, 0.5, 0), Vec3(-0.5, 0.5, 0));
	auto clr = new Vec3Array;
	clr->push_back(Vec3(1, 0, 0));
	clr->push_back(Vec3(0, 1, 0));
	clr->push_back(Vec3(0, 0, 1));
	clr->push_back(Vec3(1, 1, 0));
	_quad->setColorArray(clr, Array::BIND_PER_VERTEX);
	auto ss = _quad->getOrCreateStateSet();
	auto prg = new osg::Program;
	prg->addShader(new osg::Shader(Shader::VERTEX, vertShader));
	prg->addShader(new osg::Shader(Shader::FRAGMENT, fragShader));
	ss->setAttribute(prg);
	_quad->setCullingActive(false);

	_cam = new Camera;
	_cam->setReferenceFrame(Transform::ABSOLUTE_RF);
	_cam->addChild(_quad);
	_cam->setRenderOrder(Camera::PRE_RENDER);
	_clrTex = new osg::Texture2D;
	_clrTex->setInternalFormat(GL_RGBA);
	_clrTex->setFilter(Texture::MIN_FILTER, Texture::NEAREST);
	_clrTex->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
	_depTex = new osg::Texture2D;
	_depTex->setInternalFormat(GL_DEPTH_COMPONENT);
	_depTex->setFilter(Texture::MIN_FILTER, Texture::NEAREST);
	_depTex->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
	_cam->attach(Camera::COLOR_BUFFER, _clrTex);
	_cam->attach(Camera::DEPTH_BUFFER, _depTex);
	_cam->setRenderTargetImplementation(_cam->FRAME_BUFFER_OBJECT);

	_ssquad = createTexturedQuadGeometry(Vec3(-1, -1, 0), Vec3(2, 0, 0), Vec3(0, 2, 0));
	ss = _ssquad->getOrCreateStateSet();
	ss->setTextureAttributeAndModes(0, _clrTex);
	ss->setTextureAttributeAndModes(1, _depTex);
	ss->getOrCreateUniform("clr_tex", Uniform::SAMPLER_2D)->set(0);
	ss->getOrCreateUniform("dep_tex", Uniform::SAMPLER_2D)->set(1);
	_ssquad->setCullingActive(false);
	{
		auto prg = new osg::Program;
		prg->addShader(new osg::Shader(Shader::VERTEX, vertShader));

		auto file_path = std::filesystem::path(__FILE__);
		auto dir = file_path.parent_path();
		auto fxstr = readFile(dir.string() + "../glsl/fxaa.glsl");
		auto shader = std::format(fxaaShader, fxstr);
		prg->addShader(new osg::Shader(Shader::FRAGMENT, shader));
		ss->setAttribute(prg);
	}
	addChild(_ssquad);
}

void TestNode::traverse(NodeVisitor& nv)
{
	osg::Group::traverse(nv);

	auto cv = nv.asCullVisitor();

	if (cv)
	{
		auto vp = cv->getViewport();
		if (_clrTex->getTextureWidth() != vp->width() || _clrTex->getTextureHeight() != vp->height())
		{
			_clrTex->setTextureSize(vp->width(), vp->height());
			_clrTex->dirtyTextureObject();
			_depTex->setTextureSize(vp->width(), vp->height());
			_depTex->dirtyTextureObject();
			_cam->dirtyAttachmentMap();
		}
		_cam->accept(*cv);
	}
}
