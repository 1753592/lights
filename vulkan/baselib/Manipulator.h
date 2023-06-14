#pragma once

#include "tmath.h"

class Manipulator{
public:
  Manipulator();
  ~Manipulator();

  void home(const tg::vec3 &eye, const tg::vec3 pos, const tg::vec3 up);

  void rotate(int x, int y);

  void translate(int x, int y);

  void zoom(float in);

  const tg::vec3 & eye() const { return _eye; }

  tg::mat4 view_matrix();

private:

  tg::vec3 _eye;
  tg::vec3 _pos;
  tg::vec3 _up;
};