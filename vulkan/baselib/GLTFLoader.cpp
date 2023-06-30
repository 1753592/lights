#include "GLTFLoader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include "MeshInstance.h"

struct MaterialUnit{
  float ao;
  float metallic;
  float roughness;
  tg::vec4 albedo;
};

struct Material{
  int bind;
  bool cull_back;
  MaterialUnit mu;

  bool operator < (const Material &other) { return bind < other.bind; }
};

GLTFLoader::GLTFLoader()
{
  _m = std::make_shared<tinygltf::Model>();
}

GLTFLoader::~GLTFLoader()
{
}

bool GLTFLoader::load_file(const std::string& file, VulkanDevice *dev)
{
  tinygltf::TinyGLTF gltf;
  std::string err, warn;
  if (!gltf.LoadASCIIFromFile(_m.get(), &err, &warn, file))
    return false;
  //_m->scenes;

  for(auto &buf : _m->buffers) {
    buf.data; 
  }

  std::vector<Material> materials;
  materials.resize(_m->materials.size());

  for (int i = 0; i < _m->materials.size(); i++) {
    Material &m = materials[i];
    m.mu.ao = 1;
    auto &material = _m->materials[i];
    auto &color = material.pbrMetallicRoughness.baseColorFactor;
    m.mu.albedo = tg::vec4(color[0], color[1], color[2], color[3]);
    m.mu.metallic = material.pbrMetallicRoughness.metallicFactor;
    m.mu.roughness = material.pbrMetallicRoughness.roughnessFactor;

    material.pbrMetallicRoughness.baseColorTexture.texCoord;
    int idx = material.pbrMetallicRoughness.baseColorTexture.index;

    if (idx == -1)
      continue;
    auto &tex = _m->textures[idx];
    auto &img = _m->images[tex.source];
    if (tinygltf::IsDataURI(img.uri) || img.image.size() > 0) {}
  }

  auto meshInst = std::make_shared<MeshInstance>(dev);

  for(auto buf : _m->buffers) {
    meshInst->add_buf(buf.data.data(), buf.data.size());
  }

  for(auto &mesh : _m->meshes) {
    for(auto &pri: mesh.primitives){
      create_primitive(&pri);
    }
  }
  _m->nodes;
  _m->accessors;
  _m->materials;
  _m->bufferViews;

  return true;
}

void GLTFLoader::create_primitive(const tinygltf::Primitive *pri)
{
  std::vector<uint32_t> buf_idxs;
  std::vector<VkVertexInputBindingDescription> input_des;
  std::vector<VkVertexInputAttributeDescription> attr_des;
  input_des.reserve(pri->attributes.size());
  attr_des.reserve(pri->attributes.size());
  for (auto &attr : pri->attributes) {
    VkVertexInputBindingDescription vkinput;
    VkVertexInputAttributeDescription vkattr;
    auto &acc = _m->accessors[attr.second];
    int bufidx = _m->bufferViews[acc.bufferView].buffer;
    vkattr.format = attr_format(&acc);
    vkattr.offset = acc.byteOffset;
    if (attr.first.compare("POSITION") == 0) {
      vkattr.location = 0;
      vkattr.binding = 0;
    } else if (attr.first.compare("NORMAL") == 0) {
      vkattr.location = 1;
      vkattr.binding = 1;
    } else if (attr.first.compare("TEXCOORD_0") == 0) {
      vkattr.location = 2;
      vkattr.binding = 2;
    } else if (attr.first.compare("TEXCOORD_1") == 1) {
      vkattr.location = 3;
      vkattr.binding = 3;
    } else
      continue;
    vkinput.binding = vkattr.binding;
    vkinput.stride = acc.ByteStride(_m->bufferViews[acc.bufferView]);
    vkinput.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    input_des.emplace_back(vkinput);
    attr_des.emplace_back(vkattr);
    buf_idxs.push_back(_m->bufferViews[acc.bufferView].buffer);
  }
  if (pri->mode == TINYGLTF_MODE_LINE_LOOP) {}

  if (pri->indices >= 0) {
    auto &idx_acc = _m->accessors[pri->indices];
    auto ty = idx_acc.componentType;
    if (ty < TINYGLTF_COMPONENT_TYPE_SHORT || ty > TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
      return;

  }
}

VkFormat GLTFLoader::attr_format(const tinygltf::Accessor *acc)
{
  switch (acc->componentType) {
    case TINYGLTF_COMPONENT_TYPE_FLOAT: {
      if (acc->type == TINYGLTF_TYPE_VEC2) {
        return VK_FORMAT_R32G32_SFLOAT;
      } else if (acc->type == TINYGLTF_TYPE_VEC3) {
        return VK_FORMAT_R32G32B32_SFLOAT;
      } else if (acc->type == TINYGLTF_TYPE_VEC4) {
        return VK_FORMAT_R32G32B32A32_SFLOAT;
      }
      break;
    }
    case TINYGLTF_COMPONENT_TYPE_INT: {
      if (acc->type == TINYGLTF_TYPE_VEC2) {
        return VK_FORMAT_R32G32_SINT;
      } else if (acc->type == TINYGLTF_TYPE_VEC3) {
        return VK_FORMAT_R32G32B32_SINT;
      } else if (acc->type == TINYGLTF_TYPE_VEC4) {
        return VK_FORMAT_R32G32B32A32_SINT;
      }
      break;
    }
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      if (acc->type == TINYGLTF_TYPE_VEC2) {
        return VK_FORMAT_R16G16_SINT;
      } else if (acc->type == TINYGLTF_TYPE_VEC3) {
        return VK_FORMAT_R16G16B16_SINT;
      } else if (acc->type == TINYGLTF_TYPE_VEC4) {
        return VK_FORMAT_R16G16B16A16_SINT;
      }
      break;
  }
}
