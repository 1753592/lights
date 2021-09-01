#include "VCloudNode.h"
#include <osgUtil/CullVisitor>
#include <osg/MatrixTransform>
#include <osg/BufferTemplate>
#include <osg/BufferIndexBinding>
#include <osg/DispatchCompute>
#include <osg/PatchParameter>
#include <osg/ShapeDrawable>

#include <random>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <osgViewer/Imgui/imgui.h>

const int grasssz = 512;

VCloudNode::VCloudNode()
{
	_box = new osg::Box({ 0, 0, 0 }, 40, 40, 5);
	auto node = new osg::ShapeDrawable(_box);
	addChild(node);

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
	auto pg = new osg::Program;
	pg->addShader(new osg::Shader(osg::Shader::VERTEX, ReadFile("cloud.vert")));
	pg->addShader(new osg::Shader(osg::Shader::FRAGMENT, ReadFile("cloud.frag")));
	ss->setAttribute(pg);
}


void VCloudNode::traverse(NodeVisitor& nv)
{
	if (nv.getVisitorType() == nv.CULL_VISITOR) {
		auto cull = nv.asCullVisitor();
		Matrix matrix = *(cull->getProjectionMatrix());
		matrix.preMult(*cull->getModelViewMatrix());
		matrix = matrix.inverse(matrix);
		auto ss = getOrCreateStateSet();
		ss->getOrCreateUniform("invModelViewProjectMatrix", osg::Uniform::DOUBLE_MAT4)->set(matrix);
	} else if (nv.getVisitorType() == nv.UPDATE_VISITOR) {
	}

	Group::traverse(nv);
}

BoundingSphere VCloudNode::computeBound() const
{
	return BoundingSphere(_box->getCenter(), _box->getHalfLengths().length());
}
