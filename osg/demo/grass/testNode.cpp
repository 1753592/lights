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


class GrassNode : public osg::Drawable {
public:
	GrassNode()
		: _vao(0)
	{
		setCullingActive(false);
		auto ss = getOrCreateStateSet();
		ss->setMode(GL_CULL_FACE, 0);
	}

	~GrassNode()
	{

	}

	void drawImplementation(RenderInfo& renderInfo) const
	{
		auto contextId = renderInfo.getContextID();
		auto ext = renderInfo.getState()->get<GLExtensions>();

		if (_vao == 0) {
			ext->glGenVertexArrays(1, &_vao);
			ext->glBindVertexArray(_vao);
			ext->glBindBuffer(GL_ARRAY_BUFFER, _ssbo->getGLBufferObject(contextId)->getGLObjectID());
			ext->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Blade), reinterpret_cast<void*>(offsetof(Blade, v0)));
			ext->glEnableVertexAttribArray(0);

			ext->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Blade), reinterpret_cast<void*>(offsetof(Blade, v1)));
			ext->glEnableVertexAttribArray(1);

			ext->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Blade), reinterpret_cast<void*>(offsetof(Blade, v2)));
			ext->glEnableVertexAttribArray(2);

			ext->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Blade), reinterpret_cast<void*>(offsetof(Blade, up)));
			ext->glEnableVertexAttribArray(3);

			renderInfo.getState()->bindDrawIndirectBufferObject(_dibo->getGLBufferObject(contextId));
		}

		renderInfo.getState()->apply(getStateSet());
		ext->glBindVertexArray(_vao);
		ext->glDrawArraysIndirect(GL_PATCHES, reinterpret_cast<void*>(0));
		//ext->glDrawArraysIndirect(GL_POINTS, reinterpret_cast<void*>(0));
	}

	mutable GLuint _vao;

	osg::ref_ptr<osg::ShaderStorageBufferObject> _ssbo;
	osg::ref_ptr<osg::DrawIndirectBufferObject> _dibo;
};

const int sz = 200;

TestNode::TestNode()
{
	setCullingActive(false);

	std::vector<Blade> blades;
	std::random_device device;
	std::mt19937 gen(device());
	std::uniform_real_distribution<float> orientation_dis(0, osg::PI);
	std::uniform_real_distribution<float> height_dis(0.6f, 1.2f);
	std::uniform_real_distribution<float> dis(-1, 1);

	for (int i = -sz; i < sz; ++i) {
		for (int j = -sz; j < sz; ++j) {
			const auto x = static_cast<float>(j) -1 + dis(gen) * 0.1f;
			const auto y = static_cast<float>(i) -1 + dis(gen) * 0.1f;
			const auto blade_height = height_dis(gen);

			blades.emplace_back(
				osg::Vec4(x, y, 0, orientation_dis(gen)),
				osg::Vec4(x, y, blade_height / 2.0, blade_height),
				osg::Vec4(x, y, blade_height, 0.2f),
				osg::Vec4(0, 0, 1, 0.7f + dis(gen) * 0.3f));
		}
	}

	auto cameraBuffer = new osg::MatrixfArray(2);
	auto cameraBinding = new osg::UniformBufferBinding(0, cameraBuffer, 0, sizeof(Matrixf) * 2);
	_ubo = cameraBinding;

	auto sz = blades.size() * sizeof(Blade);
	auto bladeBufferIn = new osg::BufferTemplate<std::vector<Blade>>;
	bladeBufferIn->setData(blades);
	auto bladeInObj = new osg::ShaderStorageBufferObject;
	bladeInObj->setUsage(GL_DYNAMIC_COPY);
	bladeBufferIn->setBufferObject(bladeInObj);
	auto bladeInBinding = new osg::ShaderStorageBufferBinding(1, bladeBufferIn, 0, sz);

	auto bladeBufferOut = new osg::BufferTemplate<std::vector<Blade>>;
	bladeBufferOut->setData(blades);
	auto bladeOutObj = new osg::ShaderStorageBufferObject;
	bladeOutObj->setUsage(GL_STREAM_DRAW);
	bladeBufferOut->setBufferObject(bladeOutObj);
	auto bladeOutBinding = new osg::ShaderStorageBufferBinding(2, bladeBufferOut, 0, sz);

	auto bladeBufferNum = new osg::BufferTemplate<BladesNum>;
	auto bladeNumObj = new osg::DrawIndirectBufferObject;
	bladeNumObj->setUsage(GL_DYNAMIC_DRAW);
	bladeBufferNum->setBufferObject(bladeNumObj);
	auto bladeNumBinding = new osg::ShaderStorageBufferBinding(3, bladeBufferNum, 0, sizeof(BladesNum));
	_nbo = bladeNumBinding;

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
		auto comShaderSource = ReadFile("grass.comp");
		auto program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::COMPUTE, comShaderSource));

		auto srcNode = new osg::DispatchCompute(blades.size() / 32, 1, 1);
		srcNode->setDataVariance(osg::Object::DYNAMIC);
		auto ss = srcNode->getOrCreateStateSet();
		ss->setAttribute(program);
		ss->setAttribute(cameraBinding);
		ss->setAttribute(bladeInBinding);
		ss->setAttribute(bladeOutBinding);
		ss->setAttribute(bladeNumBinding);
		_cmpNode = srcNode;

		addChild(srcNode);
	}

	{
		auto node = new GrassNode;
		auto ss = node->getStateSet();
		auto program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::VERTEX, ReadFile("grass.vert")));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, ReadFile("grass.frag")));
		program->addShader(new osg::Shader(osg::Shader::TESSCONTROL, ReadFile("grass.tesc")));
		program->addShader(new osg::Shader(osg::Shader::TESSEVALUATION, ReadFile("grass.tese")));
		ss->setAttribute(program);
		ss->setAttribute(cameraBinding);
		ss->setAttribute(new osg::PatchParameter(1));
		node->_ssbo = bladeOutObj;
		node->_dibo = bladeNumObj;
		addChild(node);

		//program->addBindUniformBlock("CameraBufferObject", 0);
	}
}

void TestNode::traverse(NodeVisitor& nv)
{
	//static int k = 0;
	//if (k > 10)
	//	getChild(1)->accept(nv);
	//else 		{
	//	k++;
	//	Group::traverse(nv);
	//}

	if (nv.getVisitorType() == nv.CULL_VISITOR) {
		auto data = (MatrixfArray*)_ubo->getBufferData()->asArray();
		(*data)[0] = *nv.asCullVisitor()->getModelViewMatrix();
		(*data)[1] = *nv.asCullVisitor()->getProjectionMatrix();
		_ubo->getBufferData()->dirty();

		_nbo->getBufferData()->dirty();

		auto ss = _cmpNode->getOrCreateStateSet();
		ss->getOrCreateUniform("wind_magnitude", osg::Uniform::FLOAT)->set(2.f);
		ss->getOrCreateUniform("wind_wave_length", osg::Uniform::FLOAT)->set(2.f);
		ss->getOrCreateUniform("wind_wave_period", osg::Uniform::FLOAT)->set(3.f);

		static float _prevTime = 0;
		float time = nv.getFrameStamp()->getReferenceTime();
		ss->getOrCreateUniform("current_time", osg::Uniform::FLOAT)->set(float(time));
		ss->getOrCreateUniform("delta_time", osg::Uniform::FLOAT)->set(time - _prevTime);
		_prevTime = time;
	}

	Group::traverse(nv);
}

void TestNode::drawImplementation(RenderInfo& renderInfo) const
{
}

BoundingSphere TestNode::computeBound() const
{
	return BoundingSphere({ 0, 0, 0 }, sz);
}
