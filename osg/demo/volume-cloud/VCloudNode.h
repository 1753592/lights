#pragma once

#include <osg/Drawable>
#include <osg/Geometry>
#include <osg/BufferIndexBinding>


using namespace osg;

class VCloudNode : public Group {
public:
	VCloudNode();

	void traverse(NodeVisitor&);

	//void drawImplementation(RenderInfo& /*renderInfo*/) const;
	BoundingSphere computeBound() const;
private:

	osg::ref_ptr<osg::Box>		_box;
};