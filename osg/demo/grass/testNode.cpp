#include "testNode.h"
#include <osgUtil/CullVisitor>
#include <osg/MatrixTransform>
#include <osg/BufferTemplate>
#include <osg/BufferIndexBinding>
#include <osg/DispatchCompute>

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
			ext->glBindBuffer(GL_ARRAY_BUFFER, _dibo->getGLBufferObject(contextId)->getGLObjectID());
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


TestNode::TestNode()
{
	setCullingActive(false);
	{
		std::random_device device;
		std::mt19937 gen(device());
		std::uniform_real_distribution<float> orientation_dis(0, osg::PI);
		std::uniform_real_distribution<float> height_dis(0.6f, 1.2f);
		std::uniform_real_distribution<float> dis(-1, 1);

		for (int i = -200; i < 200; ++i) {
			for (int j = -200; j < 200; ++j) {
				const auto x = static_cast<float>(i) / 10 - 1 + dis(gen) * 0.1f;
				const auto y = static_cast<float>(j) / 10 - 1 + dis(gen) * 0.1f;
				const auto blade_height = height_dis(gen);

				_blades.emplace_back(
					osg::Vec4(x, y, 0, orientation_dis(gen)),
					osg::Vec4(x, y, blade_height, blade_height),
					osg::Vec4(x, y, blade_height, 0.1f),
					osg::Vec4(0, 0, blade_height, 0.7f + dis(gen) * 0.3f));
			}
		}
	}

	auto cameraObj = new osg::UniformBufferObject;
	auto cameraBuffer = new osg::MatrixfArray(2);
	//cameraBuffer->setBufferObject(cameraObj);
	auto cameraBinding = new osg::UniformBufferBinding(0, cameraBuffer, 0, sizeof(Matrixf) * 2);
	_ubo = cameraBinding;

	auto sz = _blades.size() * sizeof(Blade);
	auto bladeBufferIn = new osg::BufferTemplate<std::vector<Blade>>;
	bladeBufferIn->setData(_blades);
	auto bladeInObj = new osg::ShaderStorageBufferObject;
	bladeInObj->setUsage(GL_DYNAMIC_COPY);
	bladeBufferIn->setBufferObject(bladeInObj);
	auto bladeInBinding = new osg::ShaderStorageBufferBinding(1, bladeBufferIn, 0, sz);

	auto bladeBufferOut = new osg::BufferTemplate<std::vector<Blade>>;
	bladeBufferOut->setData(_blades);
	auto bladeOutObj = new osg::ShaderStorageBufferObject;
	bladeOutObj->setUsage(GL_STREAM_DRAW);
	bladeBufferOut->setBufferObject(bladeOutObj);
	auto bladeOutBinding = new osg::ShaderStorageBufferBinding(2, bladeBufferOut, 0, sz);

	auto bladeBufferNum = new osg::BufferTemplate<BladesNum>;
	auto bladeNumObj = new osg::DrawIndirectBufferObject;
	bladeNumObj->setUsage(GL_DYNAMIC_DRAW);
	bladeBufferNum->setBufferObject(bladeNumObj);
	auto bladeNumBinding = new osg::ShaderStorageBufferBinding(3, bladeBufferNum, 0, sizeof(BladesNum));

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

		auto srcNode = new osg::DispatchCompute(_blades.size() / 32, 1, 1);
		srcNode->setDataVariance(osg::Object::DYNAMIC);
		auto ss = srcNode->getOrCreateStateSet();
		ss->setAttribute(program);
		ss->setAttribute(cameraBinding);
		ss->setAttribute(bladeInBinding);
		ss->setAttribute(bladeOutBinding);
		ss->setAttribute(bladeNumBinding);

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
		node->_ssbo = bladeOutObj;
		node->_dibo = bladeNumObj;
		addChild(node);

		//program->addBindUniformBlock("CameraBufferObject", 0);
	}
}

void TestNode::traverse(NodeVisitor& nv)
{
	Group::traverse(nv);
	if (nv.getVisitorType() == nv.CULL_VISITOR) 		{
		auto data = (MatrixfArray*)_ubo->getBufferData()->asArray();
		(*data)[0] = *nv.asCullVisitor()->getModelViewMatrix();
		(*data)[1] = *nv.asCullVisitor()->getProjectionMatrix();
		printf("");
	}
}

void TestNode::drawImplementation(RenderInfo& renderInfo) const
{
}

BoundingSphere TestNode::computeBound() const
{
	return BoundingSphere({ 0, 0, 0 }, 200);
}
