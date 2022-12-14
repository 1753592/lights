#include "testNode.h"

#include <format>
//#include <boost/filesystem.hpp>
#include <filesystem>
#include <fstream>

#include <osg/Geometry>
#include <osgDB/WriteFile>
#include <osgUtil/CullVisitor>

#include "AreaTex.h"

const std::string vertShader = R"(

#version 330 compatibility

out vec4 clr;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	clr = gl_Color;
}

)";

const std::string fragShader = R"(

#version 330 compatibility

in vec4 clr;

void main()
{
	vec4 clr_tmp = clr;
	clr_tmp.w = 0.213 * clr.r + 0.715 * clr.g + 0.072 * clr.b;
	gl_FragColor = clr_tmp;
}

)";

const std::string svShader = R"(

#version 330 compatibility

out vec4 clr;

void main()
{
	gl_Position = gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	clr = gl_Color;
}

)";

const std::string edgeShader = R"(

#version 330 compatibility

in vec4 clr;

//layout(location = 0) out vec4 color;
//layout(location = 1) out vec2 edge;

uniform vec4 pix_size;
uniform sampler2D clr_tex;

const float threshold = 0.05f;

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec2 des = pix_size.xy;
	float c = texture(clr_tex, uv).w;	
	float l = abs(texture(clr_tex, uv - vec2(des.x, 0)).w - c);
	float l2 = abs(texture(clr_tex, uv - vec2(des.x * 2, 0)).w - c);
	float r = abs(texture(clr_tex, uv + vec2(des.x, 0)).w - c);
	
	float t = abs(texture(clr_tex, uv + vec2(0, des.y)).w - c);
	float t2 = abs(texture(clr_tex, uv + vec2(0, des.y * 2)).w - c);
	float b = abs(texture(clr_tex, uv - vec2(0, des.y)).w - c);

	float cmax = max(max(l, r), max(b, t));
	bool el = l > threshold;
	el = el && l > (max(cmax, l2) * 0.5);

	bool et = t > threshold;
	et = et && t > (max(cmax, t2) * 0.5);
	gl_FragColor = vec4(el ? 1: 0, et ? 1: 0, 0, 1);
}

)";

constexpr char bfShader[] = R"(

#version 450 compatibility

uniform vec4 pix_size;
uniform vec4 SMAA_RT_METRICS;

uniform sampler2D edge_tex;
uniform sampler2D area_tex;

const float rounding_factor = 0.25;
const int max_step = 16;

#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / vec2(160, 560))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
#define SMAA_CORNER_ROUNDING 25
#define SMAA_CORNER_ROUNDING_NORM ((SMAA_CORNER_ROUNDING) / 100.0)

#define SMAA_GLSL_4
#define SMAA_PRESET_LOW

//#define CALCULATE_FACTOR 1
//#define ROUNDING_FACTOR 1

float search_xleft(vec2 uv)
{
  int i = 0;
  float e = 0;
  uv -= vec2(1.5 * pix_size.x, 0);
  for(i = 0; i < max_step; i++)
  {
    e = texture(edge_tex, uv).g; 
    if(e < 0.9) break;
    uv -= vec2(pix_size.x * 2, 0);
  }
  return min(2 * (i + e), 2 * max_step);
}

float search_xright(vec2 uv)
{
  int i = 0;
  float e = 0;
  uv += vec2(1.5 * pix_size.x, 0);
  for(i = 0; i < max_step; i++)
  {
    e = texture(edge_tex, uv).g; 
    if(e < 0.9) break;
    uv += vec2(pix_size.x * 2, 0);
  }
  return min(2 * (i + e), 2 * max_step);
}

float search_yup(vec2 uv)
{
  int i = 0;
  float e = 0;
  uv += vec2(0, 1.5 * pix_size.y);
  for(i = 0; i < max_step; i++)
  {
    e = texture(edge_tex, uv).r; 
    if(e < 0.9) break;
    uv += vec2(0, pix_size.y * 2);
  }
  return min(2 * (i + e), 2 * max_step);
}

float search_ydn(vec2 uv)
{
  int i = 0;
  float e = 0;
  uv -= vec2(0, 1.5 * pix_size.y);
  for(i = 0; i < max_step; i++)
  {
    e = texture(edge_tex, uv).r; 
    if(e < 0.9) break;
    uv -= vec2(0, pix_size.y * 2);
  }
  return min(2 * (i + e), 2 * max_step);
}

#ifdef CALCULATE_FACTOR

bvec2 mode_of_single(float value)
{
  bvec2 res = bvec2(false);
  if(value > 0.875)
    res = bvec2(true, true);
  else if(value > 0.5)
    res.y = true;
  else if(value > 0.125)
    res.x = true;
  return res;
}

bvec4 mode_of_pair(float v1, float v2)
{
  bvec4 ret;
  ret.xy = mode_of_single(v1);
  ret.zw = mode_of_single(v2);
  return ret;
}

float l_n_shape(float d, float m)
{
  float s = 0;
  float l = d * 0.5;
  if(l > (m + 0.5))
    s = (l - m) * 0.5 / l;
  else if(l > (m - 0.5))
  {
    float a = l - m + 0.5;
    float s = a * a * 0.25 / l;
  }
  return s;
}

float l_l_s_shape(float d1, float d2)
{
  float d = d1 + d2;
  float s1 = l_n_shape(d, d1);
  float s2 = l_n_shape(d, d2);
  return s1 + s2;
}

float l_l_d_shape(float d1, float d2)
{
  float d = d1 + d2;
  float s1 = l_n_shape(d, d1);
  float s2 = l_n_shape(d, d2);
  return s1 - s2;
}

float cal_area(vec2 d, bvec2 left, bvec2 right)
{
  float result = 0;
  if(left.x && left.y){
    if(right.x && !right.y)
      result = -l_l_d_shape(d.x + 0.5, d.y + 0.5);
    else if(!right.x && right.y)
      result = l_l_d_shape(d.x + 0.5, d.y + 0.5);
  }
  else if(left.x && !left.y){
    if(right.y)
      result = l_l_d_shape(d.x + 0.5, d.y + 0.5);
    else if(!right.x)
      result = l_n_shape(d.y + d.x + 1, d.x + 0.5);
    else 
      result = l_l_s_shape(d.x + 0.5, d.y + 0.5);
  }
  else if(!left.x && left.y){
    if(right.x)
      result = -l_l_d_shape(d.x + 0.5, d.y + 0.5);
    else if(!right.y)
      result = -l_n_shape(d.y + d.x + 1, d.x + 0.5);
    else 
      result = -l_l_s_shape(d.x + 0.5, d.y + 0.5);
  }
  else{
    if(right.x && !right.y)
      result = l_n_shape(d.x + d.y + 1, d.y + 0.5);
    else if(!right.x && right.y)
      result = -l_n_shape(d.x + d.y + 1, d.y + 0.5);
  }

  return result;
}

#else

vec2 smaa_area(vec2 dist, float e1, float e2)
{
  vec2 texcoord = vec2(SMAA_AREATEX_MAX_DISTANCE) * round(4.0 * vec2(e1, e2)) + dist;
  texcoord = SMAA_AREATEX_PIXEL_SIZE * (texcoord + 0.5);
  //texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;
  //texcoord.y = 1 - texcoord.y;
  //if(texcoord.x < 0 || texcoord.x > 1)
  //  return vec2(0, 1);
  //return texcoord;
  return textureLod(area_tex, texcoord, 0).rg;
}

vec2 hor_corner_pattern(vec2 w, vec4 texcoord, vec2 d)
{
  vec2 lr = step(d.xy, d.yx); 
  vec2 rd = (1 - SMAA_CORNER_ROUNDING_NORM) * lr;
  rd /= lr.x + lr.y;
  vec2 factor = vec2(1, 1);
  factor.x -= rd.x * textureLodOffset(edge_tex, texcoord.xy, 0, ivec2(0, -1)).r;
  factor.x -= rd.y * textureLodOffset(edge_tex, texcoord.zw, 0, ivec2(0, -1)).r;
  factor.y -= rd.x * textureLodOffset(edge_tex, texcoord.xy, 0, ivec2(0,  2)).r;
  factor.y -= rd.y * textureLodOffset(edge_tex, texcoord.zw, 0, ivec2(0,  2)).r;
  w *= clamp(factor, 0, 1);
  return w;
}

vec2 ver_corner_pattern(vec2 w, vec4 texcoord, vec2 d)
{
  vec2 ud = step(d.xy, d.yx); 
  vec2 rd = (1 - SMAA_CORNER_ROUNDING_NORM) * ud;
  rd /= ud.x + ud.y;
  vec2 factor = vec2(1, 1);
  factor.x -= rd.x * textureLodOffset(edge_tex, texcoord.xy, 0, ivec2( 1, 0)).g;
  factor.x -= rd.y * textureLodOffset(edge_tex, texcoord.zw, 0, ivec2( 1, 0)).g;
  factor.y -= rd.x * textureLodOffset(edge_tex, texcoord.xy, 0, ivec2(-2, 0)).g;
  factor.y -= rd.y * textureLodOffset(edge_tex, texcoord.zw, 0, ivec2(-2, 0)).g;
  w *= clamp(factor, 0, 1);
  return w;
}

#endif

void main()
{
  vec2 uv = gl_TexCoord[0].xy;
  vec2 edge = texture(edge_tex, uv).rg;
  vec4 result = vec4(0);
  if(edge.g > 0){
    bvec2 l, r;
    float lt = search_xleft(uv);
    float rt = search_xright(uv);
    float ltv, rtv;
    #ifdef ROUNDING_FACTOR

      //float left1 = texture(edge_tex, uv + vec2(-lt, -1.25 * pix_size.y)).r;
      //float left2 = texture(edge_tex, uv + vec2(-lt, 0.75 * pix_size.y)).r;
      //l = mode_of_pair(left1, left2);
      //float right1 = texture(edge_tex, uv + vec2(rt + pix_size.x, -1.25 * pix_size.y)).r; 
      //float right2 = texture(edge_tex, uv + vec2(rt + pix_size.x, 0.75 * pix_size.y)).r; 
      //r = mode_of_pair(right1, right2);
    #else
      vec2 lfo = vec2(-lt * pix_size.x, 0.25 * pix_size.y);
      vec2 rto = vec2((rt + 1) * pix_size.x, 0.25 * pix_size.y);
      vec2 lfuv = uv + lfo, rtuv = uv + rto;
      ltv = texture(edge_tex, lfuv).r;
      rtv = texture(edge_tex, rtuv).r;
    #endif
    
    #ifdef CALCULATE_FACTOR
      l = mode_of_single(ltv);
      r = mode_of_single(rtv);
      float value = cal_area(vec2(lt, rt), l, r);
      result.xy = vec2(-value, value);
    #else
      vec2 sqrtd = sqrt(vec2(lt, rt));
      result.xy = smaa_area(sqrtd, ltv, rtv);
      result.xy = hor_corner_pattern(result.xy, vec4(lfuv, rtuv), vec2(lt, rt));
    #endif
  }
  if(edge.r > 0)
  {
    bvec2 u, d;
    float up = search_yup(uv);
    float dn = search_ydn(uv);
    #ifdef ROUNDING_FACTOR
    #else
      vec2 upo = vec2(-0.25 * pix_size.x, up * pix_size.y);
      vec2 dno = vec2(-0.25 * pix_size.x, -(dn + 1) * pix_size.y);
      vec2 upuv = uv + upo, dnuv = uv + dno;
      float upv = texture(edge_tex, upuv).g;
      float dnv = texture(edge_tex, dnuv).g;
    #endif

    #ifdef CALCULATE_FACTOR
      u = mode_of_single(upv);
      d = mode_of_single(dnv);
      float value = cal_area(vec2(up, dn), u, d);
      result.zw = vec2(-value, value);
    #else
      vec2 sqrtd = sqrt(round(vec2(up , dn)));
      result.zw = smaa_area(sqrtd, upv, dnv);
      result.zw = ver_corner_pattern(result.zw, vec4(upuv, dnuv), vec2(up, dn));
    #endif
  }
  gl_FragColor = result;
}

)";

constexpr char blShader[] = R"(

#version 330 compatibility

uniform sampler2D clr_tex;
uniform sampler2D dep_tex;
uniform sampler2D blend_tex;
uniform sampler2D edge_tex;

uniform vec4 pix_size;

void main()
{
  vec2 uv = gl_TexCoord[0].xy;
  ivec2 st = ivec2(gl_FragCoord.xy);
  vec4 bl = texelFetch(blend_tex, st, 0);
  float br = texelFetch(blend_tex, st + ivec2(1, 0), 0).w;
  float bt = texelFetch(blend_tex, st + ivec2(0, -1), 0).y;
  //vec4 bl = texture(blend_tex, uv, 0);
  //float br = texture(blend_tex, uv + vec2(pix_size.x, 0)).w;
  //float bt = texture(blend_tex, uv + vec2(0, pix_size.y)).y;
  vec4 a = vec4(bl.x, bt, bl.z, br);
  vec4 w = a * a * a;
  float sum = w.x + w.y + w.z + w.w;
  if(sum > 1e-5){
    vec4 o = a * pix_size.yyxx;
    vec4 clr = vec4(0);
    clr = texture(clr_tex, uv + vec2(0,  o.x)) * w.x + clr;
    clr = texture(clr_tex, uv + vec2(0, -o.y)) * w.y + clr;
    clr = texture(clr_tex, uv + vec2(-o.z, 0)) * w.z + clr;
    clr = texture(clr_tex, uv + vec2( o.w, 0)) * w.w + clr;
    gl_FragColor = clr / sum;
    //gl_FragColor.w = 1;
    //gl_FragColor = vec4(0, sum, 0, 1);
    //gl_FragColor = vec4(0, 1, 0, 1);
  }
  else
    gl_FragColor = texture(clr_tex, uv);

  //gl_FragColor = texelFetch(edge_tex, st, 0);
  //gl_FragColor = texelFetch(blend_tex, st, 0);
  //gl_FragColor.w = 1;
  //vec2 edge = texelFetch(edge_tex, st, 0).rg;
  //edge = abs(bl.zw) * 3;
  //gl_FragColor = mix(clr, vec4(0, edge, 1), (edge.x + edge.y) * 0.5);
}

)";

std::string readFile(const std::string& file)
{
  std::string ret;
  auto f = fopen(file.c_str(), "rb");
  if (!f)
    return ret;
  fseek(f, 0, SEEK_END);
  uint64_t len = ftell(f);
  fseek(f, 0, SEEK_SET);
  ret.resize(len, 0);
  fread(&ret[0], 1, len, f);
  fclose(f);
  return ret;
}

TestNode::TestNode()
{
  setCullingActive(false);
  init();
}

void TestNode::init()
{
  //_quad = createTexturedQuadGeometry(Vec3(0, -0.5, 0), Vec3(0.5, 0.5, 0), Vec3(-0.5, 0.5, 0));
  _quad = createTexturedQuadGeometry(Vec3(-0.5, -0.5, 0), Vec3(1, 0, 0), Vec3(0, 1, 0));
  auto clr = new Vec3Array;
  clr->push_back(Vec3(1, 0, 0));
  // clr->push_back(Vec3(0, 1, 0));
  // clr->push_back(Vec3(0, 0, 1));
  // clr->push_back(Vec3(1, 1, 0));
  _quad->setColorArray(clr, Array::BIND_OVERALL);
  auto ss = _quad->getOrCreateStateSet();
  auto prg = new osg::Program;
  prg->addShader(new osg::Shader(Shader::VERTEX, vertShader));
  prg->addShader(new osg::Shader(Shader::FRAGMENT, fragShader));
  ss->setAttribute(prg);
  _quad->setCullingActive(false);

  _cam = new Camera;
  _cam->addChild(_quad);
  _cam->setRenderOrder(Camera::PRE_RENDER);
  _cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  _cam->setClearColor(osg::Vec4(0, 0, 0, 0));
  _clrTex = new osg::Texture2D;
  _clrTex->setInternalFormat(GL_RGBA);
  _clrTex->setFilter(Texture::MIN_FILTER, Texture::LINEAR);
  _clrTex->setFilter(Texture::MAG_FILTER, Texture::LINEAR);
  _depTex = new osg::Texture2D;
  _depTex->setInternalFormat(GL_DEPTH_COMPONENT);
  _depTex->setFilter(Texture::MIN_FILTER, Texture::NEAREST);
  _depTex->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
  _cam->attach(Camera::COLOR_BUFFER, _clrTex);
  _cam->attach(Camera::DEPTH_BUFFER, _depTex);
  _cam->setRenderTargetImplementation(_cam->FRAME_BUFFER_OBJECT);

  _ssquad = createTexturedQuadGeometry(Vec3(-1, -1, 0), Vec3(2, 0, 0), Vec3(0, 2, 0));

  _edgeTex = new osg::Texture2D;
  _edgeTex->setInternalFormat(GL_RG);
  _edgeTex->setFilter(Texture::MIN_FILTER, Texture::LINEAR);
  _edgeTex->setFilter(Texture::MAG_FILTER, Texture::LINEAR);
  _edgePass = new Camera;
  _edgePass->addChild(_ssquad);
  _edgePass->setRenderOrder(_edgePass->PRE_RENDER, 1);
  _edgePass->setRenderTargetImplementation(_edgePass->FRAME_BUFFER_OBJECT);
  _edgePass->setImplicitBufferAttachmentMask(0);
  _edgePass->attach(_edgePass->COLOR_BUFFER, _edgeTex);
  _edgePass->setClearMask(GL_COLOR_BUFFER_BIT);
  _edgePass->setClearColor(Vec4(0, 0, 0, 0));
  {
	auto ss = _edgePass->getOrCreateStateSet();
    ss->setTextureAttributeAndModes(0, _clrTex, osg::StateAttribute::OVERRIDE);
	ss->addUniform(new osg::Uniform("clr_tex", 0), osg::StateAttribute::OVERRIDE);	

    auto prg = new osg::Program;
    prg->addShader(new osg::Shader(Shader::VERTEX, svShader));
    prg->addShader(new osg::Shader(Shader::FRAGMENT, edgeShader));
    ss->setAttribute(prg, osg::StateAttribute::OVERRIDE);
  }

  _blendTex = new osg::Texture2D;
  _blendTex->setInternalFormat(GL_RGBA8_SNORM);
  //_blendTex->setSourceFormat(GL_FLOAT);
  _blendTex->setFilter(Texture::MIN_FILTER, Texture::LINEAR);
  _blendTex->setFilter(Texture::MAG_FILTER, Texture::LINEAR);
  _blendFactor = new Camera;
  _blendFactor->setClearMask(GL_COLOR_BUFFER_BIT);
  _blendFactor->setClearColor(Vec4(0, 0, 0, 0));
  _blendFactor->addChild(_ssquad);
  _blendFactor->setRenderOrder(_blendFactor->PRE_RENDER, 2);
  _blendFactor->setRenderTargetImplementation(_blendFactor->FRAME_BUFFER_OBJECT);
  _blendFactor->setImplicitBufferAttachmentMask(0);
  _blendFactor->attach(_blendFactor->COLOR_BUFFER, _blendTex);

  auto img = new osg::Image;
  img->setImage(AREATEX_WIDTH, AREATEX_HEIGHT, 0, GL_RG, GL_RG, GL_UNSIGNED_BYTE, (unsigned char *)areaTexBytes, osg::Image::NO_DELETE);
  img->flipVertical();
  _areaTex = new osg::Texture2D(img);
  _areaTex->setResizeNonPowerOfTwoHint(false);
  _areaTex->setFilter(Texture::MIN_FILTER, Texture::LINEAR);
  _areaTex->setFilter(Texture::MAG_FILTER, Texture::LINEAR);

  {
    auto ss = _blendFactor->getOrCreateStateSet();
    ss->setTextureAttributeAndModes(0, _edgeTex, osg::StateAttribute::OVERRIDE);
	  ss->addUniform(new osg::Uniform("edge_tex", 0), osg::StateAttribute::OVERRIDE);	

    ss->setTextureAttributeAndModes(1, _areaTex, osg::StateAttribute::OVERRIDE);
	  ss->addUniform(new osg::Uniform("area_tex", 1), osg::StateAttribute::OVERRIDE);	

    auto prg = new osg::Program;
    prg->addShader(new osg::Shader(Shader::VERTEX, svShader));

    #if 0
    char bfShaderOut[70000] = {0};
    auto smaa = readFile(std::string(RS_DIR) + "/smaa.h");
    sprintf(bfShaderOut, bfShader, smaa.c_str());
    std::string bfsShader = bfShaderOut;
    #else
    std::string bfsShader = bfShader;
    #endif

    //std::ofstream fout("test.frag", std::ios::out);
    //fout << smaa; fout.close();
    //osgDB::writeImageFile(*_areaTex->getImage(), "area.png");

    prg->addShader(new osg::Shader(Shader::FRAGMENT, bfsShader));
    ss->setAttribute(prg, osg::StateAttribute::OVERRIDE);
  }

  ss = _ssquad->getOrCreateStateSet();
  ss->setTextureAttributeAndModes(0, _clrTex);
  ss->setTextureAttributeAndModes(1, _depTex);
  ss->setTextureAttributeAndModes(2, _blendTex);
  ss->setTextureAttributeAndModes(3, _edgeTex);
  ss->getOrCreateUniform("clr_tex", Uniform::SAMPLER_2D)->set(0);
  ss->getOrCreateUniform("dep_tex", Uniform::SAMPLER_2D)->set(1);
  ss->getOrCreateUniform("blend_tex", Uniform::SAMPLER_2D)->set(2);
  ss->getOrCreateUniform("edge_tex", Uniform::SAMPLER_2D)->set(3);
  _ssquad->setCullingActive(false);
  {
    auto prg = new osg::Program;
    prg->addShader(new osg::Shader(Shader::VERTEX, svShader));
    prg->addShader(new osg::Shader(Shader::FRAGMENT, blShader));
    ss->setAttribute(prg);
  }
  addChild(_ssquad);
}

void TestNode::traverse(NodeVisitor& nv)
{
  osg::Group::traverse(nv);

  auto cv = nv.asCullVisitor();

  if (cv) {
    auto vp = cv->getViewport();
    if (_clrTex->getTextureWidth() != vp->width() || _clrTex->getTextureHeight() != vp->height()) {
      _clrTex->setTextureSize(vp->width(), vp->height());
      _clrTex->dirtyTextureObject();
      _depTex->setTextureSize(vp->width(), vp->height());
      _depTex->dirtyTextureObject();
      _edgeTex->setTextureSize(vp->width(), vp->height());
      _edgeTex->dirtyTextureObject();
      _blendTex->setTextureSize(vp->width(), vp->height());
      _blendTex->dirtyTextureObject();

      _cam->dirtyAttachmentMap();
      _edgePass->dirtyAttachmentMap();
      _blendFactor->dirtyAttachmentMap();

      auto ss = _ssquad->getOrCreateStateSet();
      ss->getOrCreateUniform("pix_size", osg::Uniform::FLOAT_VEC4)->set(osg::Vec4(1.0 / vp->width(), 1.0 / vp->height(), vp->width(), vp->height()));
      ss->getOrCreateUniform("SMAA_RT_METRICS", osg::Uniform::FLOAT_VEC4)->set(osg::Vec4(vp->width(), vp->height(), 1.0 / vp->width(), 1.0 / vp->height()));
    }
    _cam->accept(*cv);
	  _edgePass->accept(*cv);
    _blendFactor->accept(*cv);
  }
}
