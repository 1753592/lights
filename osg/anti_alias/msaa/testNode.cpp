#include "testNode.h"

#include <osg/Geometry>
#include <osg/Depth>
#include <osgUtil/CullVisitor>

const std::string vertShader = R"(

#version 330 compatibility

out vec4 clr;

void main()
{
	gl_Position = gl_Vertex;
	gl_FrontColor = gl_Color;
	clr = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

)";

const std::string fShader = R"(

#version 330 compatibility

in vec4 clr;

void main()
{
	gl_FragColor = clr; 
}

)";

const std::string fragShader = R"(

#version 330 compatibility

uniform sampler2D clr_tex;
uniform sampler2D dep_tex;

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	//float r = texture(dep_tex, uv).r;
	//gl_FragColor = vec4(vec3(r), 1);
	gl_FragColor = texture(clr_tex, uv);
}

)";

#define DIRECT false

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
	//clr->push_back(Vec3(0, 1, 0));
	//clr->push_back(Vec3(0, 0, 1));
	//clr->push_back(Vec3(1, 1, 0));
	//_quad->setColorArray(clr, Array::BIND_PER_VERTEX);
	_quad->setColorArray(clr, Array::BIND_OVERALL);
	auto ss = _quad->getOrCreateStateSet();
	auto prg = new osg::Program;
	prg->addShader(new osg::Shader(Shader::VERTEX, vertShader));
	prg->addShader(new osg::Shader(Shader::FRAGMENT, fShader));
	ss->setAttribute(prg);
	_quad->setCullingActive(false);
	auto dep = new osg::Depth;
	dep->setWriteMask(true);
	ss->setAttribute(dep);

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
	//_depTex->setSourceFormat(GL_DEPTH_COMPONENT);
	//_depTex->setSourceType(GL_FLOAT);
	_depTex->setFilter(Texture::MIN_FILTER, Texture::NEAREST);
	_depTex->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
	_cam->attach(Camera::COLOR_BUFFER, _clrTex, 0, 0, false, 16, 16);
	_cam->attach(Camera::DEPTH_BUFFER, _depTex);
	_cam->setRenderTargetImplementation(_cam->FRAME_BUFFER_OBJECT);

	if (DIRECT)
	{
		addChild(_quad);
	}
	else {
		_ssquad = createTexturedQuadGeometry(Vec3(-1, -1, 0), Vec3(2, 0, 0), Vec3(0, 2, 0));
		ss = _ssquad->getOrCreateStateSet();
		ss->setTextureAttributeAndModes(0, _clrTex);
		ss->setTextureAttributeAndModes(1, _depTex);
		ss->getOrCreateUniform("clr_tex", osg::Uniform::SAMPLER_2D)->set(0);
		ss->getOrCreateUniform("dep_tex", osg::Uniform::SAMPLER_2D)->set(1);
		_ssquad->setCullingActive(false);
		{
			auto prg = new osg::Program;
			prg->addShader(new osg::Shader(Shader::VERTEX, vertShader));
			prg->addShader(new osg::Shader(Shader::FRAGMENT, fragShader));
			ss->setAttribute(prg);
		}
		addChild(_ssquad);
	}
}

void TestNode::traverse(NodeVisitor& nv)
{
	osg::Group::traverse(nv);

	if (DIRECT)
		return;

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
