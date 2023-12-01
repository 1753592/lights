#include "tvec.h"
#include "tmath.h"
#include <filesystem>

using tg::vec3;
using tg::mat4;

auto cal_psm_matrix(const tg::vec3 &light_dir, const tg::vec3 &cam_eye, const tg::vec3 &cam_center, float zn, float zf, const tg::boundingbox &psc)
{
  struct Ret {
    tg::mat4 mm;
    tg::mat4 mp;
  };

  tg::vec3 lt = tg::normalize(light_dir);
  auto rt = tg::cross(cam_center - cam_eye, lt);
  rt = tg::normalize(rt);
  auto ft = tg::cross(lt, rt);

  tg::vec3 center;
  tg::boundingbox bd = psc;
  float n = sqrt(zf * zn) - zn; 
  float dis = 0, f = 0;
  {
    bd.expand(cam_eye);
    center = psc.center();
    f = bd.radius() * 2 + n;
    dis = n + bd.radius();
  }

  tg::vec3 eye = center - ft * dis;
  auto mat = tg::lookat(eye, center, lt);
  return mat;
}

int main()
{
  tg::vec3 light_dir(1, 0, 1);
  tg::vec3 cam(0, 0, 10);
  tg::vec3 center(10, 0, 0);
  float n = 0.1, f = 10;
  tg::boundingbox bd(tg::vec3(5, 0, 0), tg::vec3(10, 0, 5));

  cal_psm_matrix(light_dir, cam, center, n, f, bd);
}