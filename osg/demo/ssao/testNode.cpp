#include "testNode.h"
#include <osgUtil/CullVisitor>
#include <osg/MatrixTransform>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgGA/EventVisitor>
#include <osgGA/GUIEventAdapter>
#include <osgUtil/CullVisitor>

#include <random>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <osgViewer/Imgui/imgui.h>

auto ReadFile = [](const std::string& fileName) {
	std::string dir = __FILE__;
	auto path = std::filesystem::path(dir).parent_path();
	path.append(fileName);
	if (!std::filesystem::exists(path)) {
		path = std::filesystem::path(dir).parent_path();
		path = path.parent_path();
		path.append("glsl");
		path.append(fileName);
	}
	std::ifstream fs(path.string(), std::ios::in);
	std::istreambuf_iterator<char> iter(fs), end;
	std::string shaderSource(iter, end);
	return shaderSource;
};


TestNode::TestNode()
{
	setNumChildrenRequiringUpdateTraversal(1);
	setNumChildrenRequiringEventTraversal(1);

	auto grp = new osg::Group;
	grp->addChild(new osg::ShapeDrawable(new osg::Box({-25,-25, 6 }, 10, 10, 10)));
	grp->addChild(new osg::ShapeDrawable(new osg::Box({ 25,-25, 6 }, 10, 10, 10)));
	grp->addChild(new osg::ShapeDrawable(new osg::Box({ 25, 25, 6 }, 10, 10, 10)));
	grp->addChild(new osg::ShapeDrawable(new osg::Box({-25, 25, 6 }, 10, 10, 10)));
	grp->addChild(new osg::ShapeDrawable(new osg::Box({ 0, 0, 0 }, 100, 100, 2)));

	auto ss = grp->getOrCreateStateSet();
	auto pg = new osg::Program;
	pg->addShader(new osg::Shader(osg::Shader::VERTEX, ReadFile("geom.vert")));
	pg->addShader(new osg::Shader(osg::Shader::FRAGMENT, ReadFile("geom.frag")));
	ss->setAttribute(pg, osg::StateAttribute::OVERRIDE);

	_cam = new osg::Camera;
	_cam->addChild(grp);

	int sz = 1024;
	_colorTex = createTexture(sz, sz);
	_normalTex = createTexture(sz, sz);
	_cam->attach(_cam->COLOR_BUFFER0, _colorTex);
	_cam->attach(_cam->COLOR_BUFFER1, _normalTex);
	_cam->setReferenceFrame(_cam->ABSOLUTE_RF);
	_cam->setViewport(0, 0, sz, sz);

	addChild(grp);
}


void TestNode::traverse(NodeVisitor& nv)
{
	if (nv.getVisitorType() == nv.CULL_VISITOR) {
		auto cv = nv.asCullVisitor();
		auto vp = cv->getViewport();
		int width = vp->width();
		int height = vp->height();
		_cam->accept(nv);
		_cam->setProjectionMatrix(*cv->getProjectionMatrix());
		_cam->setViewMatrix(*cv->getModelViewMatrix());
	} else if (nv.getVisitorType() == nv.UPDATE_VISITOR) {
		ImGui::Begin("xxx");
		ImGui::End();
	} else if (nv.getVisitorType() == nv.EVENT_VISITOR) {
		auto ev = nv.asEventVisitor();
		auto evs = ev->getEvents();
		auto ea = evs.begin()->get()->asGUIEventAdapter();
		if (ea->getEventType() == ea->RESIZE) {
		}
	}

	Group::traverse(nv);
}

osg::Texture2D*
TestNode::createTexture(int w, int h)
{
	auto tex = new osg::Texture2D;
	tex->setTextureSize(w, h);
	tex->setInternalFormat(GL_RGB16F);
	tex->setFilter(tex->MAG_FILTER, tex->LINEAR);
	tex->setNumMipmapLevels(0);
	return tex;
}
