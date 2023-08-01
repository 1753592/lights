#pragma once

#include "tvec.h"
#include "VulkanTexture.h"

struct MVP {
  tg::vec4 eye;
  tg::mat4 prj;
  tg::mat4 view;
};

struct Transform{
  tg::mat4 m;
};

struct ParallelLight{
  tg::vec4 light_dir;
  tg::vec4 light_color;
};

struct PointLight{
  tg::vec4 light_pos;
  tg::vec4 light_color;
};

struct alignas(64) PBRData {
  float   ao;
  float   metallic;
  float   roughness;
  int     place;
  tg::vec4 albedo;
};

struct Material{
  bool          cull; 
  PBRData       pbrdata;
  std::shared_ptr<VulkanTexture> albedo_tex;
};

