#pragma once

#include "tvec.h"

struct MVP {
  tg::mat4 model;
  tg::mat4 prj;
  tg::mat4 view;
  tg::vec4 eye;
};
