#include "VCloudNode.h"
#include <osgUtil/CullVisitor>
#include <osg/MatrixTransform>
#include <osg/BufferTemplate>
#include <osg/BufferIndexBinding>
#include <osg/DispatchCompute>
#include <osg/PatchParameter>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>

#include <random>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <osgViewer/Imgui/imgui.h>

VCloudNode::VCloudNode()
{
	_box = new osg::Box({ 0, 0, -10 }, 40, 40, 30);
	auto node = new osg::ShapeDrawable(_box);
	addChild(node);
	_boxDrawable = node;

	auto ReadFile = [](const std::string& fileName) {
		std::string dir = __FILE__;
		auto path = std::filesystem::path(dir).parent_path();
		path.append(fileName);
		std::ifstream fs(path.string(), std::ios::in);
		std::istreambuf_iterator<char> iter(fs), end;
		std::string shaderSource(iter, end);
		return shaderSource;
	};
	
	auto ss = node->getOrCreateStateSet();
	ss->setMode(GL_BLEND, 1);
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
