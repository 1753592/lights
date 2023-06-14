#include "Manipulator.h"

#include <stdio.h>

using tg::vec3;
using tg::vec4;

Manipulator::Manipulator()
{
  home(vec3(0, -5, 0), vec3(0), vec3(0, 0, 1));
}

Manipulator::~Manipulator()
{
}

void Manipulator::home(const tg::vec3& eye, const tg::vec3 pos, const tg::vec3 up)
{
  _eye = eye;
  _pos = pos;
  _up = tg::cross(eye - pos, tg::cross(up, eye - pos));
  _up = tg::normalize(_up);
}

void Manipulator::rotate(int x, int y)
{
  y = 0;
  vec3 tmp = _eye - _pos;
  vec3 rt = tg::cross(_up, tmp);
  if (x) {
    float xrad = x / 100.f;
    auto q = tg::quat::rotate(-xrad, _up);
    auto xtmp = q * tg::quat(tmp) * q.conjugate();
    tmp = xtmp;
    rt = q * vec4(rt, 0) * q.conjugate();
  }

  if (y) {
    float yrad = y / 100.f;
    auto q = tg::quat::rotate(yrad, rt);
    auto ytmp = q * tg::quat(tmp) * q.conjugate(); 
    tmp = ytmp;
    _up = q * vec4(_up, 0) * q.conjugate();
  }

  _eye = _pos + tmp;
}

void Manipulator::translate(int x, int y)
{
    //osg::Vec3 tmp = _eye - _pos;
    //tmp.normalize();
    //osg::Vec3 rt = _up ^ tmp;
    //rt *= -event.motion.xrel * 0.1;
    //auto up = _up * event.motion.yrel * 0.1;
    //_eye += rt;
    //_eye += up;
    //_pos += rt;
    //_pos += up
}

void Manipulator::zoom(float in)
{
  auto tmp = _eye - _pos;
  if (in > 0)
    tmp *= 1.1;
  else
    tmp *= 0.9;
  auto len = tg::length(tmp);
  if (len < 1 || len > 1000)
    return;
  _eye = _pos + tmp;
}

tg::mat4 Manipulator::view_matrix()
{
  auto m = tg::lookat(_eye, _pos, _up);

  return m;
}
