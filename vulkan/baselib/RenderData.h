#pragma once

#include "tvec.h"
#include "VulkanTexture.h"

struct MVP {
  tg::mat4 model;
  tg::mat4 prj;
  tg::mat4 view;
  tg::vec4 eye;
};

struct ParallelLight{
  tg::vec4 light_dir;
  tg::vec4 light_color;
};

struct PointLight{
  tg::vec4 light_pos;
  tg::vec4 light_color;
};

struct PBRBasic{
  float   ao;
  float   metallic;
  float   roughness;
  int     place;
  tg::vec4 albedo;
};

struct Material{
  bool          cull; 
  PBRBasic      pbr;
  std::shared_ptr<VulkanTexture> tex;
};

