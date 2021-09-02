#pragma once

#include <osg/Drawable>

using namespace osg;

namespace osg {
	class Texture2D;
	class Texture3D;
	class ShapeDrawable;
	class DispatchCompute;
}

class VCloudNode : public Group {
public:
	VCloudNode();

	void traverse(NodeVisitor&);

	//void drawImplementation(RenderInfo& /*renderInfo*/) const;
	BoundingSphere computeBound() const;

	void setDepthTexture(osg::Texture2D* tex);

	bool needComputeNoise() { return _needComputeNoise; }
private:

	void createNoise();

	osg::ref_ptr<osg::Box>				_box;
	osg::ref_ptr<osg::ShapeDrawable>	_boxDrawable;

	osg::ref_ptr<osg::DispatchCompute>	_noise;
	osg::ref_ptr<osg::Texture3D>		_noiseTex;

	bool		_needComputeNoise;
};