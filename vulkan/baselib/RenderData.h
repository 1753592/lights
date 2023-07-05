#pragma once

#include "tvec.h"
#include "VulkanTexture.h"

struct MVP {
  tg::mat4 model;
  tg::mat4 prj;
  tg::mat4 view;
  tg::vec4 eye;
};

struct PrimitiveData{
  tg::mat4 transform;
};


struct PBRData{
  float ao;
  float metallic;
  float roughness;
  tg::vec4 albedo;
};

struct Material{
  bool      cull; 
  PBRData   pbr;
  std::shared_ptr<VulkanTexture> tex;
};

