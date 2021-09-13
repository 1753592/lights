#pragma once

#include <osg/Drawable>
#include <osg/Geometry>
#include <osg/BufferIndexBinding>


using namespace osg;

namespace osg {
class Texture2D;
}

class TestNode : public Group {
public:
	TestNode();

	void traverse(NodeVisitor&);

	//void drawImplementation(RenderInfo& /*renderInfo*/) const;
	//BoundingSphere computeBound() const;
	osg::Texture2D* createTexture(int, int);
private:
	osg::ref_ptr<osg::Camera> _cam;

	osg::ref_ptr<osg::Texture2D> _colorTex;
	osg::ref_ptr<osg::Texture2D> _depthTex;
	osg::ref_ptr<osg::Texture2D> _normalTex;


};