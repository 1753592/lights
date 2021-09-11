#include "testNode.h"
#include <osgUtil/CullVisitor>
#include <osg/MatrixTransform>
#include <osg/BufferTemplate>
#include <osg/BufferIndexBinding>
#include <osg/DispatchCompute>
#include <osg/PatchParameter>
#include <osgDB/ReadFile>

#include <random>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <osgViewer/Imgui/imgui.h>

const int grasssz = 512;

TestNode::TestNode()
{
	auto node = osgDB::readNodeFile("D:/dev/lights/resources/sponza.gltf");
	addChild(node);
}


void TestNode::traverse(NodeVisitor& nv)
{
	if (nv.getVisitorType() == nv.CULL_VISITOR) {
		auto cv = nv.asCullStack();
		auto vp = cv->getViewport();
		int width = vp->width();
		int height = vp->height();
	} else if (nv.getVisitorType() == nv.UPDATE_VISITOR) {
		ImGui::Begin("xxx");
		ImGui::End();
	}

	Group::traverse(nv);
}
