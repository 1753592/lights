#include "testNode.h"
#include <osgUtil/CullVisitor>
#include <osg/MatrixTransform>
#include <osg/BufferTemplate>
#include <osg/BufferIndexBinding>
#include <osg/DispatchCompute>
#include <osg/PatchParameter>

#include <random>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <osgViewer/Imgui/imgui.h>

const int grasssz = 512;

TestNode::TestNode()
{
	setCullingActive(false);
	setNumChildrenRequiringUpdateTraversal(1);

	auto ReadFile = [](const std::string& fileName) {
		std::string dir = __FILE__;
		auto path = std::filesystem::path(dir).parent_path();
		path.append(fileName);
		std::ifstream fs(path.string(), std::ios::in);
		std::istreambuf_iterator<char> iter(fs), end;
		std::string shaderSource(iter, end);
		return shaderSource;
	};

	{
		auto vertexs = new osg::Vec3Array;
		vertexs->push_back({-1, 0, -1});
		vertexs->push_back({ 1, 0, -1});
		vertexs->push_back({ 1, 0,  1});
		vertexs->push_back({-1, 0, -1});
		vertexs->push_back({ 1, 0,  1});
		vertexs->push_back({-1, 0,  1});
		auto clrs = new osg::Vec3Array;
		clrs->push_back({ 0, 0.4, 0 });
		auto geom = new osg::Geometry;
		geom->setVertexArray(vertexs);
		geom->setColorArray(clrs, osg::Array::BIND_OVERALL);
		geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 6));
		addChild(geom);

		auto prg = new osg::Program;
		prg->addShader(new osg::Shader(osg::Shader::VERTEX, ReadFile("noise.vert")));
		prg->addShader(new osg::Shader(osg::Shader::FRAGMENT, ReadFile("noise.frag")));
		auto ss = geom->getOrCreateStateSet();
		ss->setAttribute(prg);
	}
}


void TestNode::traverse(NodeVisitor& nv)
{
	if (nv.getVisitorType() == nv.CULL_VISITOR) {
		auto cv = nv.asCullStack();
		auto vp = cv->getViewport();
		int width = vp->width();
		int height = vp->height();
		auto ss = getOrCreateStateSet();
		ss->getOrCreateUniform("screenSize", osg::Uniform::INT_VEC2)->set(width, height);
		//ss->getOrCreateUniform("time", osg::Uniform::FLOAT)->set(float(nv.getFrameStamp()->getReferenceTime()));
		ss->getOrCreateUniform("time", osg::Uniform::FLOAT)->set(1.f);
	}
	else if (nv.getVisitorType() == nv.UPDATE_VISITOR) 		{
		ImGui::Begin("xxx");
		//ImGui::SliderFloat("wind_mag", &m_wind_mag, 0.1f, 10.0f);
		//ImGui::SliderFloat("wind_length", &m_wind_length, 0.1f, 10.0f);
		//ImGui::SliderFloat("wind_period", &m_wind_period, 0.1f, 10.0f);
		char ch[512];
		ImGui::InputText("wtf", ch, 512);
		ImGui::End();
	}

	Group::traverse(nv);
}
