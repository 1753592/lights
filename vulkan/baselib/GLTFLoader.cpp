#include "GLTFLoader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include "MeshInstance.h"
#include "MeshPrimitive.h"
#include "tmath.h"

#include <set>

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

GLTFLoader::GLTFLoader(std::shared_ptr<VulkanDevice> &dev) : _dev(dev)
{
  _m = std::make_shared<tinygltf::Model>();
}

GLTFLoader::~GLTFLoader()
{
}

std::shared_ptr<MeshInstance> GLTFLoader::load_file(const std::string& file)
{
  tinygltf::TinyGLTF gltf;
  std::string err, warn;
  if (!gltf.LoadASCIIFromFile(_m.get(), &err, &warn, file))
    return nullptr;
  //_m->scenes;

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

  auto meshInst = std::make_shared<MeshInstance>(_dev);
  meshInst->set_m(tg::rotate<float>(M_PI_2, tg::vec3(1, 0, 0)));

  for(auto &mesh : _m->meshes) {
    for(auto &pri: mesh.primitives){
      auto mesh_pri = create_primitive(&pri);
      meshInst->add_primitive(mesh_pri);
    }
  }
  return meshInst;
}

std::shared_ptr<MeshPrimitive>
GLTFLoader::create_primitive(const tinygltf::Primitive *pri)
{
  auto mesh_pri = std::make_shared<MeshPrimitive>(_dev);
  mesh_pri->_input_attr.reserve(pri->attributes.size());
  mesh_pri->_input_bind.reserve(pri->attributes.size());

  for (auto &attr : pri->attributes) {
    VkVertexInputBindingDescription vkinput;
    VkVertexInputAttributeDescription vkattr;
    auto &acc = _m->accessors[attr.second];
    auto &bufview = _m->bufferViews[acc.bufferView];
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

    vkattr.format = attr_format(&acc);
    vkattr.offset = 0;
    vkinput.binding = vkattr.binding;
    vkinput.stride = acc.ByteStride(_m->bufferViews[acc.bufferView]);
    vkinput.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    mesh_pri->_input_attr.emplace_back(vkattr);
    mesh_pri->_input_bind.emplace_back(vkinput);

    auto &buf = _m->buffers[bufview.buffer];
    mesh_pri->add_vertex_buf(buf.data.data() + bufview.byteOffset, bufview.byteLength);
  }
  if (pri->mode == TINYGLTF_MODE_LINE_LOOP)
    mesh_pri->_mode = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
  else if (pri->mode == TINYGLTF_MODE_TRIANGLES)
    mesh_pri->_mode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  else
    return nullptr;

  if (pri->indices >= 0) {
    auto &idx_acc = _m->accessors[pri->indices];
    auto ty = idx_acc.componentType;
    if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
      mesh_pri->_index_type = VK_INDEX_TYPE_UINT16;
    else if (ty == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
      mesh_pri->_index_type = VK_INDEX_TYPE_UINT32;
    else
      return nullptr;

    auto &bufview = _m->bufferViews[idx_acc.bufferView];
    auto &buf = _m->buffers[bufview.buffer];
    mesh_pri->set_index_buf(buf.data.data() + bufview.byteOffset, bufview.byteLength);
    mesh_pri->_index_count = idx_acc.count;
  }

  return mesh_pri; 
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
