#include "VCloudNode.h"
#include <osgUtil/CullVisitor>
#include <osg/MatrixTransform>
#include <osg/BufferTemplate>
#include <osg/BufferIndexBinding>
#include <osg/DispatchCompute>
#include <osg/PatchParameter>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BindImageTexture>

#include <random>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <osgViewer/Imgui/imgui.h>

auto ReadFile = [](const std::string& fileName) {
	std::string dir = __FILE__;
	auto path = std::filesystem::path(dir).parent_path();
	path.append(fileName);
	std::ifstream fs(path.string(), std::ios::in);
	std::istreambuf_iterator<char> iter(fs), end;
	std::string shaderSource(iter, end);
	return shaderSource;
};


VCloudNode::VCloudNode()
	: _needComputeNoise(true)
{
	_box = new osg::Box({ 0, 0, -10 }, 40, 40, 30);
	auto node = new osg::ShapeDrawable(_box);
	node->setColor({ 1, 0, 0, 1 });
	addChild(node);
	_boxDrawable = node;

	createNoise();

	auto ss = node->getOrCreateStateSet();
	ss->setMode(GL_BLEND, 1);
	ss->setMode(GL_CULL_FACE, 0);
	auto pg = new osg::Program;
	pg->addShader(new osg::Shader(osg::Shader::VERTEX, ReadFile("cloud.vert")));
	pg->addShader(new osg::Shader(osg::Shader::FRAGMENT, ReadFile("cloud.frag")));
	ss->setAttribute(pg);
	ss->getOrCreateUniform("boxPos", osg::Uniform::FLOAT_VEC4)->set(Vec4(_box->getCenter(), 0));
	ss->getOrCreateUniform("boxLength", osg::Uniform::FLOAT_VEC4)->set(Vec4(_box->getHalfLengths(), 0));

	{
		//auto node = new osg::ShapeDrawable(_box);
	}
}


void VCloudNode::traverse(NodeVisitor& nv)
{
	if (nv.getVisitorType() == nv.CULL_VISITOR) {
		auto cull = nv.asCullVisitor();
		Matrix matrix = *(cull->getProjectionMatrix());
		matrix.preMult(*cull->getModelViewMatrix());
		matrix = matrix.inverse(matrix);
		auto ss = getOrCreateStateSet();
		ss->getOrCreateUniform("invModelViewProjectMatrix", osg::Uniform::FLOAT_MAT4)->set(Matrixf(matrix));
		ss->getOrCreateUniform("camPos", osg::Uniform::FLOAT_VEC3)->set(cull->getViewPoint());
		auto vp = cull->getViewport();
		ss->getOrCreateUniform("screenSize", osg::Uniform::FLOAT_VEC4)->set(osg::Vec4(vp->width(), vp->height(), 0, 0));

		if (_needComputeNoise) {
			_noise->accept(nv);
			//_needComputeNoise = false;
		}

	} else if (nv.getVisitorType() == nv.UPDATE_VISITOR) {
	}

	Group::traverse(nv);
}

BoundingSphere VCloudNode::computeBound() const
{
	return BoundingSphere(_box->getCenter(), _box->getHalfLengths().length());
}

void VCloudNode::setDepthTexture(osg::Texture2D* tex)
{
	auto ss = _boxDrawable->getOrCreateStateSet();
	ss->setTextureAttribute(0, tex);
	ss->getOrCreateUniform("depTex", osg::Uniform::SAMPLER_2D)->set(0);
}

void VCloudNode::createNoise()
{
	_noiseTex = new osg::Texture3D;
	_noiseTex->setTextureSize(256, 256, 256);
	_noiseTex->setFilter(_noiseTex->MIN_FILTER, _noiseTex->LINEAR);
	_noiseTex->setFilter(_noiseTex->MAG_FILTER, _noiseTex->LINEAR);
	_noiseTex->setInternalFormat(GL_R32F);
	_noiseTex->setSourceFormat(GL_RED);
	_noiseTex->setSourceType(GL_FLOAT);

	auto bind = new osg::BindImageTexture(0, _noiseTex, osg::BindImageTexture::WRITE_ONLY, GL_R32F);

	_noise = new osg::DispatchCompute(16, 16, 128);
	_noise->setDataVariance(STATIC);
	auto ss = _noise->getOrCreateStateSet();
	auto pg = new osg::Program;
	pg->addShader(new osg::Shader(osg::Shader::COMPUTE, ReadFile("noise.comp")));
	ss->setAttributeAndModes(pg);
	ss->addUniform(new osg::Uniform("noiseTex", int(0)));
	ss->setTextureAttribute(0, _noiseTex);
	ss->setAttribute(bind);
}
