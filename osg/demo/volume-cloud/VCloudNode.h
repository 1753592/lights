#pragma once

#include <osg/Drawable>

using namespace osg;

namespace osg {
class Texture2D;
class ShapeDrawable;
}

class VCloudNode : public Group {
public:
	VCloudNode();

	void traverse(NodeVisitor&);

	//void drawImplementation(RenderInfo& /*renderInfo*/) const;
	BoundingSphere computeBound() const;

	void setDepthTexture(osg::Texture2D* tex);
private:

	osg::ref_ptr<osg::Box>				_box;
	osg::ref_ptr<osg::ShapeDrawable>	_boxDrawable;
};