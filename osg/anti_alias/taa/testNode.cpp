#include "testNode.h"

#include <osg/Vec2>
#include <osg/Geometry>
#include <osg/Depth>
#include <osgUtil/CullVisitor>

#include <Windows.h>

const std::string vShader = R"(

#version 430 compatibility

out float dep;
out vec4 clr;

uniform mat4 taa_proj;

layout(binding = 0) uniform CameraBufferObject{
	mat4 pre_matrix;
	mat4 cur_matrix;
} camera_buffer;

struct VelocityStep{
	vec4 pre_pos;
	vec4 cur_pos;
};

out VelocityStep vel_out;

void main()
{
	gl_Position = taa_proj * gl_ModelViewMatrix * gl_Vertex;
	vec4 vtmp = gl_ModelViewProjectionMatrix * gl_Vertex; 
	dep = vtmp.z / vtmp.w;

	clr = gl_Color;

	vel_out.pre_pos = camera_buffer.pre_matrix * gl_Vertex;
	vel_out.cur_pos = camera_buffer.cur_matrix * gl_Vertex;	
}

)";

const std::string fShader = R"(

#version 430 compatibility

in vec4 clr;

layout(location = 0) out vec4 color;
layout(location = 1) out vec2 velocity;

struct VelocityStep{
	vec4 pre_pos;
	vec4 cur_pos;
};

in VelocityStep vel_out;


void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	//float r = texture(dep_tex, uv).r;
	//gl_FragColor = vec4(vec3(r), 1);
	color = clr;
	vec2 cur_pos = (vel_out.cur_pos.xy / vel_out.cur_pos.w) * 0.5 + 0.5;
	vec2 pre_pos = (vel_out.pre_pos.xy / vel_out.pre_pos.w) * 0.5 + 0.5;
	//velocity = cur_pos;
	velocity = cur_pos - pre_pos;
}

)";

const std::string vertShader = R"(

#version 330 compatibility

out vec2 uv;

void main()
{
	gl_Position = gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	uv = gl_MultiTexCoord0.xy;
}

)";

const std::string tavShader = R"(

#version 330 compatibility

out vec2 uv;
out vec4 clr;

void main()
{
	gl_Position = gl_Vertex;
	clr = gl_Color;
	gl_FrontColor = gl_Color;
	uv = gl_MultiTexCoord0.xy;
}

)";

const std::string taaShader = R"(

#version 330 compatibility

in vec2 uv;
in vec4 clr;

uniform bool init_clr;
uniform vec2 taa_jitter;

uniform sampler2D prev_tex;
uniform sampler2D curr_tex;

void main()
{

	vec3 his_clr = texture(prev_tex, uv).rgb;
	vec2 cur_uv = uv + taa_jitter;
	vec3 cur_clr = texture(curr_tex, cur_uv).rgb;
	
	//gl_FragColor = clr;
	vec3 des_clr = mix(his_clr, cur_clr, 0.05); 
	gl_FragColor = vec4(des_clr, 1);
}

)";


const std::string fragShader = R"(

#version 330 compatibility

uniform sampler2D clr_tex;
uniform sampler2D dep_tex;
uniform sampler2D vel_tex;

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	//float r = texture(dep_tex, uv).r;
	//gl_FragColor = vec4(vec3(r), 1);
	//gl_FragColor = vec4(texture(clr_tex, uv).rgb, 1);
	gl_FragColor = vec4(texture(vel_tex, uv).rg, 0, 1);
}

)";

const osg::Vec2 haltonSequence[] =
{
	{0.5f, 1.0f / 3},
	{0.25f, 2.0f / 3},
	{0.75f, 1.0f / 9},
	{0.125f, 4.0f / 9},
	{0.625f, 7.0f / 9},
	{0.375f, 2.0f / 9},
	{0.875f, 5.0f / 9},
	{0.0625f, 8.0f / 9}
};

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
	prg->addShader(new osg::Shader(Shader::VERTEX, vShader));
	prg->addShader(new osg::Shader(Shader::FRAGMENT, fShader));
	ss->setAttribute(prg);
	//_quad->setCullingActive(false);
	auto dep = new osg::Depth;
	dep->setWriteMask(true);
	ss->setAttribute(dep);
	_cameraBuffer = new osg::MatrixfArray(2);
	_cameraBufferBinding = new osg::UniformBufferBinding(0, _cameraBuffer, 0, sizeof(Matrixf) * 2);
	ss->setAttribute(_cameraBufferBinding);

	_cam = new Camera;
	_cam->setReferenceFrame(Transform::RELATIVE_RF);
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

	_velTex = new osg::Texture2D;
	_velTex->setInternalFormat(GL_RG16);
	_velTex->setSourceFormat(GL_RG);
	_velTex->setSourceType(GL_FLOAT);
	_velTex->setFilter(Texture::MIN_FILTER, Texture::NEAREST);

	_cam->attach(Camera::COLOR_BUFFER0, _clrTex);
	_cam->attach(Camera::DEPTH_BUFFER, _depTex);
	_cam->attach(Camera::COLOR_BUFFER1, _velTex);
	_cam->setRenderTargetImplementation(_cam->FRAME_BUFFER_OBJECT);

	_taaTex1 = new osg::Texture2D;
	_taaTex1->setInternalFormat(GL_RGBA);
	_taaTex1->setFilter(Texture::MIN_FILTER, Texture::NEAREST);
	_taaTex1->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
	_taaTex2 = osg::clone(_taaTex1.get(), osg::CopyOp::DEEP_COPY_ALL);

	_taaquad = createTexturedQuadGeometry(Vec3(-1, -1, 0), Vec3(2, 0, 0), Vec3(0, 2, 0));
	_taaquad->setCullingActive(false);
	ss = _taaquad->getOrCreateStateSet();
	ss->setTextureAttributeAndModes(0, _clrTex);
	ss->setTextureAttributeAndModes(1, _taaTex1);
	ss->setTextureAttributeAndModes(2, _taaTex2);
	ss->getOrCreateUniform("curr_texture", osg::Uniform::SAMPLER_2D)->set(0);
	{
		auto prg = new osg::Program;
		prg->addShader(new osg::Shader(Shader::VERTEX, tavShader));
		prg->addShader(new osg::Shader(Shader::FRAGMENT, taaShader));
		ss->setAttribute(prg);
	}

	_taaCam = new Camera;
	_taaCam->setClearMask(GL_COLOR_BUFFER_BIT);
	_taaCam->setImplicitBufferAttachmentMask(0);
	_taaCam->setReferenceFrame(Transform::ABSOLUTE_RF);
	_taaCam->setRenderOrder(Camera::PRE_RENDER, 1);
	_taaCam->setRenderTargetImplementation(_taaCam->FRAME_BUFFER_OBJECT);
	_taaCam->addChild(_taaquad);

	_ssquad = createTexturedQuadGeometry(Vec3(-1, -1, 0), Vec3(2, 0, 0), Vec3(0, 2, 0));
	//_ssquad->setCullingActive(false);
	ss = _ssquad->getOrCreateStateSet();
	ss->setTextureAttributeAndModes(0, _depTex);
	ss->setTextureAttributeAndModes(1, _taaTex1);
	ss->setTextureAttributeAndModes(2, _taaTex2);
	ss->setTextureAttributeAndModes(3, _velTex);
	ss->getOrCreateUniform("dep_tex", osg::Uniform::SAMPLER_2D)->set(0);
	ss->getOrCreateUniform("vel_tex", osg::Uniform::SAMPLER_2D)->set(3);
	{
		auto prg = new osg::Program;
		prg->addShader(new osg::Shader(Shader::VERTEX, vertShader));
		prg->addShader(new osg::Shader(Shader::FRAGMENT, fragShader));
		ss->setAttribute(prg);
	}


	addChild(_ssquad);
	//_quad->getBoundingBox();
	//_ssquad->getBound();
}

void TestNode::traverse(NodeVisitor& nv)
{
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

			_velTex->setTextureSize(vp->width(), vp->height());
			_velTex->dirtyTextureObject();

			_taaTex1->setTextureSize(vp->width(), vp->height());
			_taaTex1->dirtyTextureObject();
			_taaTex2->setTextureSize(vp->width(), vp->height());
			_taaTex2->dirtyTextureObject();

			_cam->setViewport(0, 0, vp->width(), vp->height());
			_cam->dirtyAttachmentMap();
		}

		int frameNum = cv->getFrameStamp()->getFrameNumber();
		int idx = frameNum % 8;
		auto &halt = haltonSequence[idx];
		osg::Vec2f jit((halt.x() - 0.5) / vp->width(), (halt.y() - 0.5) / vp->height());
		osg::Matrix proj = *cv->getProjectionMatrix();
		proj(2, 0) += jit.x() * 2;
		proj(2, 1) += jit.y() * 2;
		auto ss = _quad->getOrCreateStateSet();
		ss->getOrCreateUniform("taa_proj", osg::Uniform::FLOAT_MAT4)->set(proj);

		_preMatrix = _curMatrix;
		_curMatrix = *cv->getProjectionMatrix();
		_curMatrix.preMult(*cv->getModelViewMatrix());
		_cameraBuffer->at(0) = _preMatrix;
		_cameraBuffer->at(1) = _curMatrix;
		_cameraBuffer->dirty();

		if (_preMatrix != _curMatrix)
		{
			printf("");
		}

		_cam->accept(*cv);

		ss = _taaquad->getOrCreateStateSet();
		ss->getOrCreateUniform("taa_jitter", osg::Uniform::FLOAT_VEC2)->set(jit);
		if (frameNum % 2) {
			_taaCam->attach(Camera::COLOR_BUFFER, _taaTex1);
			ss->getOrCreateUniform("prev_tex", osg::Uniform::SAMPLER_2D)->set(2);
			ss = _ssquad->getStateSet();
			ss->getOrCreateUniform("clr_tex", osg::Uniform::SAMPLER_2D)->set(1);
		}
		else {
			_taaCam->attach(Camera::COLOR_BUFFER, _taaTex2);
			ss->getOrCreateUniform("prev_tex", osg::Uniform::SAMPLER_2D)->set(1);
			ss = _ssquad->getStateSet();
			ss->getOrCreateUniform("clr_tex", osg::Uniform::SAMPLER_2D)->set(2);
		}

		_taaCam->dirtyAttachmentMap();

		_taaCam->accept(*cv);

		//Sleep(100);
	}
	osg::Group::traverse(nv);
}
