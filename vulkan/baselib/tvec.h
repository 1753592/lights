#ifndef __TVEC_INC__
#define __TVEC_INC__

#define NOMINMAX 1

#include <cmath>

namespace tg
{
//template <typename T, const int w, const int h> class matNM;
//template <typename T, const int len> class vecN;
//template <typename T> class Tquaternion;

#ifndef _USE_MATH_DEFINES
#define M_E        2.71828182845904523536   // e
#define M_LOG2E    1.44269504088896340736   // log2(e)
#define M_LOG10E   0.434294481903251827651  // log10(e)
#define M_LN2      0.693147180559945309417  // ln(2)
#define M_LN10     2.30258509299404568402   // ln(10)
#define M_PI       3.14159265358979323846   // pi
#define M_PI_2     1.57079632679489661923   // pi/2
#define M_PI_4     0.785398163397448309616  // pi/4
#define M_1_PI     0.318309886183790671538  // 1/pi
#define M_2_PI     0.636619772367581343076  // 2/pi
#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
#endif

constexpr float feps = 1e-6;
constexpr double deps = 1e-12;

template <typename T>
inline T degrees(T angleInRadians)
{
	return angleInRadians * static_cast<T>(180.0 / M_PI);
}

template <typename T>
inline T radians(T angleInDegrees)
{
	return angleInDegrees * static_cast<T>(M_PI / 180.0);
}

template <typename T, int len>
class vecN {
public:
  using ele_type = T;
  using self_type = vecN<T, len>;

  // Default constructor does nothing, just like built-in types
  inline vecN()
  {
    // Uninitialized variable
    assign(0);
  }

  // Copy constructor
  inline vecN(const vecN& that) { assign(that); }

  // Construction from scalar
  inline vecN(T s)
  {
    for (int n = 0; n < len; n++) {
      data_[n] = s;
    }
  }

  template <typename U>
  vecN(const vecN<U, len>& that)
  {
    for (int n = 0; n < len; n++) {
      data_[n] = that[n];
    }
  }

  template<typename U>
  inline void reset(U *ptr)
  {
    for(int n = 0; n < len; n++) {
      data_[n] = ptr[n];
    }
  }

  // Assignment operator
  inline vecN<T, len>& operator=(const vecN& that)
  {
    assign(that);
    return *this;
  }

  inline vecN<T, len>& operator=(const T& that)
  {
    for (int n = 0; n < len; n++)
      data_[n] = that;

    return *this;
  }

  template<typename U, const int m>
  inline vecN<T, len>& operator=(const vecN<U, m> &that)
  {
    constexpr int sz = len < m ? len : m;
    for (int i = 0; i < sz; i++)
      data_[i] = that[i];
    return *this;
  }

  inline vecN operator+(const vecN& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < len; n++)
      result.data_[n] = data_[n] + that.data_[n];
    return result;
  }

  inline vecN& operator+=(const vecN& that) { return (*this = *this + that); }

  inline vecN operator-() const
  {
    self_type result;
    int n;
    for (n = 0; n < len; n++)
      result.data_[n] = -data_[n];
    return result;
  }

  inline vecN operator-(const vecN& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < len; n++)
      result.data_[n] = data_[n] - that.data_[n];
    return result;
  }

  inline vecN& operator-=(const vecN& that) { return (*this = *this - that); }

  inline vecN operator*(const vecN& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < len; n++)
      result.data_[n] = data_[n] * that.data_[n];
    return result;
  }

  inline vecN& operator*=(const vecN& that) { return (*this = *this * that); }

  inline vecN operator*(const T& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < len; n++)
      result.data_[n] = data_[n] * that;
    return result;
  }

  inline vecN& operator*=(const T& that)
  {
    assign(*this * that);

    return *this;
  }

  inline vecN operator/(const vecN& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < len; n++)
      result.data_[n] = data_[n] / that.data_[n];
    return result;
  }

  inline vecN& operator/=(const vecN& that)
  {
    assign(*this / that);

    return *this;
  }

  inline vecN operator/(const T& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < len; n++)
      result.data_[n] = data_[n] / that;
    return result;
  }

  inline vecN& operator/=(const T& that)
  {
    assign(*this / that);
    return *this;
  }

  inline T& operator[](int n) { return data_[n]; }
  inline const T& operator[](int n) const { return data_[n]; }

  inline static int size(void) { return len; }

  inline operator const T*() const { return &data_[0]; }

protected:
  T data_[len];

  inline void assign(const vecN& that) { memcpy(data_, that.data_, sizeof(T) * len); }
};

template<int len>
inline bool operator==(const vecN<float, len> &v1, const vecN<float, len> &v2)
{
	for (int i = 0; i < len; i++) {
		if (fabs(v1[i] - v2[i]) > feps)
			return false;
	}
	return true;
}

template<int len>
inline bool operator==(const vecN<double, len> &v1, const vecN<double, len> &v2)
{
	for (int i = 0; i < len; i++) {
		if (fabs(v1[i] - v2[i]) > deps)
			return false;
	}
	return true;
}

template <typename T>
class Tvec2 : public vecN<T, 2>{
public:
	typedef vecN<T, 2> base;
	typedef Tvec2<T> self_type;

	// Uninitialized variable
	inline Tvec2(){}
	// Copy constructor
	inline Tvec2(const self_type& v)
	{
		base::data_[0] = v[0];
		base::data_[1] = v[1];
	}

	inline self_type& operator=(const self_type &t)
	{
		base::data_[0] = t[0];
		base::data_[1] = t[1];
		return *this;
	}

	inline Tvec2(const base& v)
		: base(v)
	{}

	inline Tvec2(T x, T y)
	{
		base::data_[0] = x;
		base::data_[1] = y;
	}

	inline void operator=(const T& t)
	{
		base::data_[0] = t;
		base::data_[1] = t;
	}

	inline T& x() { return base::data_[0]; }
	inline T& y() { return base::data_[1]; }

	inline const T& x() const { return base::data_[0]; }
	inline const T& y() const { return base::data_[1]; }
};

template <typename T>
class Tvec3 : public vecN<T, 3> {
public:
  using base = vecN<T, 3>;
  using self_type = Tvec3<T>;

  // Uninitialized variable
  inline Tvec3() : base(0) {}

  inline Tvec3(T t) : base(t) {}

  // Copy constructor
  inline Tvec3(const self_type& v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
  }

  inline self_type& operator=(const self_type& v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    return *this;
  }

  inline Tvec3(const base& v) : base(v) {}

  // vec3(x, y, z);
  inline Tvec3(T x, T y, T z)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
  }

  // vec3(v, z);
  inline Tvec3(const Tvec2<T>& v, T z)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = z;
  }

  // vec3(x, v)
  inline Tvec3(T x, const Tvec2<T>& v)
  {
    base::data_[0] = x;
    base::data_[1] = v[0];
    base::data_[2] = v[1];
  }

  inline Tvec3(const vecN<T, 4>& v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
  }

  inline self_type operator=(const T& t)
  {
    base::data_[0] = t;
    base::data_[1] = t;
    base::data_[2] = t;
    return *this;
  }

  inline self_type operator=(const vecN<T, 4> &vec)
  {
    base::data_[0] = vec[0];
    base::data_[1] = vec[1];
    base::data_[2] = vec[2];
    return *this;
  }

  template <typename U>
  inline Tvec3(const Tvec3<U>& that) : base(that) { }

  template<typename U>
  inline Tvec3(const U* ptr) { reset(ptr); }

  template<typename U, int n>
  inline self_type operator=(vecN<U, n> vec)
  {
    base::operator=(vec); 
    return *this;
  }

  inline T& x() { return base::data_[0]; }
  inline T& y() { return base::data_[1]; }
  inline T& z() { return base::data_[2]; }

  inline const T& x() const { return base::data_[0]; }
  inline const T& y() const { return base::data_[1]; }
  inline const T& z() const { return base::data_[2]; }

  inline void set(const T& v) { set(v, v, v); }

  inline void set(const T& x, const T& y, const T& z)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
  }
};

template <typename T>
class Tvec4 : public vecN<T, 4> {
public:
  typedef vecN<T, 4> base;
  typedef Tvec4<T> self_type;

  // Uninitialized variable
  inline Tvec4() {}

  // Copy constructor
  inline Tvec4(const self_type& v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = v[3];
  }

  template <typename U>
  inline Tvec4(const Tvec4<U>& that) : base(that) {}

  template <typename U>
  inline Tvec4(const U* ptr) { reset(ptr); }

  template <typename U, int n>
  inline self_type operator=(vecN<U, n> vec)
  {
    base::operator=(vec);
    return *this;
  }

  inline self_type& operator=(const self_type& v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = v[3];
    return *this;
  }

  // vec4(x, y, z, w);
  inline Tvec4(T x, T y, T z, T w)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
    base::data_[3] = w;
  }

  // vec4(v, z, w);
  inline Tvec4(const Tvec2<T>& v, T z, T w)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = z;
    base::data_[3] = w;
  }

  // vec4(x, v, w);
  inline Tvec4(T x, const Tvec2<T>& v, T w)
  {
    base::data_[0] = x;
    base::data_[1] = v[0];
    base::data_[2] = v[1];
    base::data_[3] = w;
  }

  // vec4(x, y, v);
  inline Tvec4(T x, T y, const Tvec2<T>& v)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = v[0];
    base::data_[3] = v[1];
  }

  // vec4(v1, v2);
  inline Tvec4(const Tvec2<T>& u, const Tvec2<T>& v)
  {
    base::data_[0] = u[0];
    base::data_[1] = u[1];
    base::data_[2] = v[0];
    base::data_[3] = v[1];
  }

  // vec4(v, w);
  inline Tvec4(const Tvec3<T>& v, T w)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = w;
  }

  // vec4(x, v);
  inline Tvec4(T x, const Tvec3<T>& v)
  {
    base::data_[0] = x;
    base::data_[1] = v[0];
    base::data_[2] = v[1];
    base::data_[3] = v[2];
  }

  inline Tvec4(const Tvec3<T>& v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = 0;
  }

  inline void operator=(const T& t)
  {
    base::data_[0] = t;
    base::data_[1] = t;
    base::data_[2] = t;
    base::data_[3] = t;
  }

  inline operator Tvec3<T>() { return Tvec3<T>(base::data_[0], base::data_[1], base::data_[2]); }

  inline T& x() { return base::data_[0]; }
  inline T& y() { return base::data_[1]; }
  inline T& z() { return base::data_[2]; }
  inline T& w() { return base::data_[3]; }

  inline const T& x() const { return base::data_[0]; }
  inline const T& y() const { return base::data_[1]; }
  inline const T& z() const { return base::data_[2]; }
  inline const T& w() const { return base::data_[3]; }

  inline void set(const T& x, const T& y, const T& z, const T& w)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
    base::data_[3] = w;
  }
};

// These types don't exist in GLSL and don't have full implementations
// (constructors and such). This is enough to get some template functions
// to compile correctly.
typedef vecN<float, 1> vec1;
typedef vecN<int, 1> vec1i;
typedef vecN<unsigned int, 1> vec1u;
typedef vecN<double, 1> vec1d;

typedef Tvec2<float> vec2;
typedef Tvec2<int> vec2i;
typedef Tvec2<unsigned int> vec2u;
typedef Tvec2<double> vec2d;

typedef Tvec3<float> vec3;
typedef Tvec3<int> vec3i;
typedef Tvec3<unsigned int> vec3u;
typedef Tvec3<double> vec3d;

typedef Tvec4<float> vec4;
typedef Tvec4<int> vec4i;
typedef Tvec4<unsigned int> vec4u;
typedef Tvec4<double> vec4d;

template <typename T, int n>
static inline const vecN<T, n> operator * (T x, const vecN<T, n>& v)
{
	return v * x;
}

template <typename T>
static inline const Tvec2<T> operator / (T x, const Tvec2<T>& v)
{
	return Tvec2<T>(x / v[0], x / v[1]);
}

template <typename T>
static inline const Tvec3<T> operator / (T x, const Tvec3<T>& v)
{
	return Tvec3<T>(x / v[0], x / v[1], x / v[2]);
}

template <typename T>
static inline const Tvec4<T> operator / (T x, const Tvec4<T>& v)
{
	return Tvec4<T>(x / v[0], x / v[1], x / v[2], x / v[3]);
}

template <typename T, int len>
static inline T dot(const vecN<T, len>& a, const vecN<T, len>& b)
{
	int n;
	T total = T(0);
	for(n = 0; n < len; n++){
		total += a[n] * b[n];
	}
	return total;
}

template <typename T>
static inline vecN<T, 3> cross(const vecN<T, 3>& a, const vecN<T, 3>& b)
{
	return Tvec3<T>(a[1] * b[2] - b[1] * a[2],
					a[2] * b[0] - b[2] * a[0],
					a[0] * b[1] - b[0] * a[1]);
}

template <typename T, int len>
static inline T pow(const vecN<T, len>& v, int num)
{
	T result(0);
	for(int i = 0; i < v.size(); i++){
		T res = v[i];
		for(int j = 1; j < num; j++)
			res *= v[i];
		result += res;
	}
	return result;
}

template <typename T, int len>
static inline T length(const vecN<T, len>& v)
{
	T result(0);

	for(int i = 0; i < v.size(); ++i){
		result += v[i] * v[i];
	}

	return (T)sqrt(result);
}

template <typename T, int len>
static inline T square(const vecN<T, len>& v)
{
	T result(0);

	for(int i = 0; i < v.size(); ++i){
		result += v[i] * v[i];
	}

	return result;
}

template <typename T, int len>
static inline vecN<T, len> normalize(const vecN<T, len>& v)
{
	return v / length(v);
}

template <typename T, int len>
static inline T distance(const vecN<T, len>& a, const vecN<T, len>& b)
{
	return length(b - a);
}

template <typename T, int len>
static inline T angle(const vecN<T, len>& a, const vecN<T, len>& b)
{
	return arccos(dot(a, b));
}

template <typename T, int len>
static inline vecN<T, len> abs(const vecN<T, len> &a)
{
	vecN<T, len> result;
	for(int i = 0; i < len; i++){
		result[i] = fabs(a[i]);
	}
	return result;
}

//Quaternion///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class Tquaternion {
  template <typename T> friend class Tmat3;
public:
  inline Tquaternion() {}

  inline Tquaternion(const Tquaternion& q) : s_(q.s_), v_(q.v_) {}

  inline Tquaternion(T s) : s_(s), v_(T(0)) {}

  inline Tquaternion(T s, const Tvec3<T>& v) : s_(s), v_(v) {}

  inline Tquaternion(const Tvec4<T>& v) : s_(v[3]), v_(v[0], v[1], v[2]) {}

  inline Tquaternion(T w, T x, T y, T z) : s_(w), v_(x, y, z) {}

  inline Tquaternion(const Tvec3<T>& v) : v_(v), s_(0) {}

  inline T& operator[](int n) { return a_[n]; }

  inline const T& operator[](int n) const { return a_[n]; }

  inline Tquaternion operator+(const Tquaternion& q) const { return quat(s_ + q.s_, v_ + q.v_); }

  inline Tquaternion& operator+=(const Tquaternion& q)
  {
    s_ += q.s_;
    v_ += q.v_;
    return *this;
  }

  inline Tquaternion operator-(const Tquaternion& q) const { return quat(s_ - q.s_, v_ - q.v_); }

  inline Tquaternion& operator-=(const Tquaternion& q)
  {
    s_ -= q.s_;
    v_ -= q.v_;

    return *this;
  }

  inline Tquaternion operator-() const { return Tquaternion(-s_, -v_); }

  inline Tquaternion operator*(const T s) const { return Tquaternion(a_[0] * s, a_[1] * s, a_[2] * s, a_[3] * s); }

  inline Tquaternion& operator*=(const T s)
  {
    s_ *= s;
    v_ *= s;
    return *this;
  }

  inline Tquaternion operator*(const Tquaternion& q) const
  {
    const T& w1 = a_[0];
    const T& x1 = a_[1];
    const T& y1 = a_[2];
    const T& z1 = a_[3];
    const T& w2 = q.a_[0];
    const T& x2 = q.a_[1];
    const T& y2 = q.a_[2];
    const T& z2 = q.a_[3];

    return Tquaternion(w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2, w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2, w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2,
                       w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2);
  }

  inline Tquaternion operator/(const T s) const { return Tquaternion(a_[0] / s, a_[1] / s, a_[2] / s, a_[3] / s); }

  inline Tquaternion& operator/=(const T t)
  {
    s_ /= t;
    v_ /= t;
    return *this;
  }

  inline operator Tvec4<T>&() { return *(Tvec4<T>*)&a_[0]; }

  inline operator const Tvec4<T>&() const { return *(const Tvec4<T>*)&a_[0]; }

  inline operator Tvec3<T>&() { return v_; }

  inline operator const Tvec3<T>&() const { return v_; }

  inline bool operator==(const Tquaternion& q) const { return (s_ == q.s_) && (v_ == q.v_); }

  inline bool operator!=(const Tquaternion& q) const { return (s_ != q.s_) || (v_ != q.v_); }

  static Tquaternion<T> rotate(const T& rad, const T& x, const T& y, const T& z) { return rotate(rad, Tvec3<T>(x, y, z)); }

  static Tquaternion<T> rotate(const T& rad, const Tvec3<T>& axis) { return Tquaternion<T>(cos(rad / T(2.0)), normalize(axis) * sin(rad / T(2.0))); }

  inline Tquaternion<T> conjugate() const { return Tquaternion<T>(s_, -v_); }

private:
  union {
    struct {
      T s_;
      Tvec3<T> v_;
    };
    struct {
      T w_;
      T x_;
      T y_;
      T z_;
    };
    T a_[4];
  };
};

typedef Tquaternion<float> quat;
typedef Tquaternion<int> quati;
typedef Tquaternion<unsigned int> quatu;
typedef Tquaternion<double> quatd;

template <typename T>
static inline Tquaternion<T> operator*(T a, const Tquaternion<T>& b)
{
	return b * a;
}

template <typename T>
static inline Tquaternion<T> operator/(T a, const Tquaternion<T>& b)
{
	return Tquaternion<T>(a / b[0], a / b[1], a / b[2], a / b[3]);
}

template <typename T>
static inline Tquaternion<T> normalize(const Tquaternion<T>& q)
{
	return q / length(vecN<T, 4>(q));
}

template <typename T, const int w, const int h>
class matNM {
public:
  typedef class matNM<T, w, h> self_type;
  typedef class vecN<T, h> vector_type;

  // Default constructor does nothing, just like built-in types
  inline matNM()
  {
    // Uninitialized variable
  }

  // Copy constructor
  inline matNM(const matNM& that) { assign(that); }

  // Construction from element self_type
  // explicit to prevent assignment from T
  explicit inline matNM(T f)
  {
    for (int n = 0; n < w; n++) {
      data[n] = f;
    }
  }

  template <typename U>
  matNM(const matNM<U, w, h>& that)
  {
    for (int n = 0; n < w; n++) {
      data[n] = that[n];
    }
  }

  template <const int m, const int n>
  matNM(const matNM<T, m, n>& that)
  {
    constexpr int col = m < w ? m : w;
    constexpr int row = n < h ? n : h;
    for (int i = 0; i < col; i++)
      for (int j = 0; j < row; j++)
        data[i][j] = that[i][j];
  }

  // Construction from vector
  inline matNM(const vector_type& v)
  {
    for (int n = 0; n < w; n++) {
      data[n] = v;
    }
  }

  // Assignment operator
  inline matNM& operator=(const self_type& that)
  {
    assign(that);
    return *this;
  }

  template <typename U>
  matNM& operator=(const matNM<U, w, h>& that)
  {
    for (int n = 0; n < w; n++) {
      data[n] = that[n];
    }
    return *this;
  }

  inline matNM operator+(const self_type& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < w; n++)
      result.data[n] = data[n] + that.data[n];
    return result;
  }

  inline self_type& operator+=(const self_type& that) { return (*this = *this + that); }

  inline self_type operator-(const self_type& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < w; n++)
      result.data[n] = data[n] - that.data[n];
    return result;
  }

  inline self_type& operator-=(const self_type& that) { return (*this = *this - that); }

  inline self_type operator*(const T& that) const
  {
    self_type result;
    int n;
    for (n = 0; n < w; n++)
      result.data[n] = data[n] * that;
    return result;
  }

  inline self_type& operator*=(const T& that)
  {
    int n;
    for (n = 0; n < w; n++)
      data[n] = data[n] * that;
    return *this;
  }

  // Matrix multiply.
  // this * that not that * this
  inline self_type operator*(const self_type& that) const
  {
    self_type result(0);

    for (int j = 0; j < w; j++) {
      for (int i = 0; i < h; i++) {
        T sum(0);

        for (int n = 0; n < w; n++) {
          sum += data[n][i] * that[j][n];
        }

        result[j][i] = sum;
      }
    }

    return result;
  }

  // 	inline vector_type operator*(const vector_type_multiply &that) const
  // 	{
  // 		vector_type vec;
  // 		T sum(0);
  // 		for(int i = 0; i < h; i++){
  // 			sum = 0;
  // 			for(int j = 0; j < w; j++){
  // 				sum += data[j][i] * that[j];
  // 			}
  // 			vec[i] = sum;
  // 		}
  // 		return vec;
  // 	}

  inline self_type& operator*=(const self_type& that) { return (*this = *this * that); }

  inline vector_type& operator[](int n) { return data[n]; }
  inline const vector_type& operator[](int n) const { return data[n]; }
  inline operator T*() { return &data[0][0]; }
  inline operator const T*() const { return &data[0][0]; }

  inline matNM<T, h, w> transpose(void) const
  {
    matNM<T, h, w> result;
    int x, y;

    for (y = 0; y < w; y++) {
      for (x = 0; x < h; x++) {
        result[x][y] = data[y][x];
      }
    }

    return result;
  }

  static inline self_type identity()
  {
    self_type result(0);

    for (int i = 0; i < w; i++) {
      result[i][i] = 1;
    }

    return result;
  }

  static inline int width(void) { return w; }
  static inline int height(void) { return h; }

  template<typename U>
  inline void reset(const U* ele)
  {
    std::size_t off = 0;
    for(int i =0; i < w; i++) {
      data[i].reset(ele + off);
      off += sizeof(U) * h;
    }
  }

  inline void reset(const T* ele) { memcpy(data, ele, sizeof(T) * w * h); }

  inline bool reduce()
  {
    bool res = true;
    vecN<T, w> arr[h];
    vecN<T, w>* rarr[h];
    for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
        arr[i][j] = data[j][i];
      }
      rarr[i] = (arr + i);
    }
    for (int i = 0; i < h; i++) {
      for (;;) {
        vecN<T, w>& carr = *rarr[i];
        for (int k = 0; k < i; k++) {
          if (carr[k]) {
            carr /= carr[k];
            carr -= *rarr[k];
          }
        }
        if (carr[i] == 0) {
          int j = i + 1;
          for (; j < h; j++) {
            if (rarr[j][i]) {
              auto tmp = rarr[i];
              rarr[i] = rarr[j];
              rarr[j] = tmp;
              break;
            }
          }
          if (j == h) {
            res = false;
            break;
          }
        } else {
          carr /= carr[i];
          break;
        }
      }
    }
    return res;
  }

protected:
  // Column primary data (essentially, array of vectors)
  vecN<T, h> data[w];

  // Assignment function - called from assignment operator and copy constructor.
  inline void assign(const matNM& that)
  {
    int n;
    for (n = 0; n < w; n++)
      data[n] = that.data[n];
  }
};

/*
template <typename T, const int N>
class TmatN : public matNM<T,N,N>
{
public:
	typedef matNM<T,N,N> base;
	typedef TmatN<T,N> self_type;

	inline TmatN() {}
	inline TmatN(const self_type& that) : base(that) {}
	inline TmatN(float f) : base(f) {}
	inline TmatN(const vecN<T,4>& v) : base(v) {}

	inline self_type transpose(void)
	{
		self_type result;
		int x, y;

		for (y = 0; y < h; y++)
		{
			for (x = 0; x < h; x++)
			{
				result[x][y] = data[y][x];
			}
		}

		return result;
	}
};
*/

template <typename T>
class Tmat2 : public matNM<T, 2, 2> {
public:
  typedef matNM<T, 2, 2> base;
  typedef Tmat2<T> self_type;

  inline Tmat2() {}
  inline Tmat2(const self_type& that) : base(that) {}
  inline Tmat2(const base& that) : base(that) {}
  inline Tmat2(const vecN<T, 2>& v) : base(v) {}
  inline Tmat2(const vecN<T, 2>& v0, const vecN<T, 2>& v1)
  {
    base::data[0] = v0;
    base::data[1] = v1;
  }
};
typedef Tmat2<float> mat2;

template <typename T>
class Tmat3 : public matNM<T, 3, 3> {
public:
  typedef matNM<T, 3, 3> base;
  typedef Tmat3<T> self_type;

  inline Tmat3() {}
  inline Tmat3(const self_type& that) : base(that) {}
  inline Tmat3(const vecN<T, 3>& v) : base(v) {}
  inline Tmat3(const vecN<T, 3>& v0, const vecN<T, 3>& v1, const vecN<T, 3>& v2)
  {
    base::data[0] = v0;
    base::data[1] = v1;
    base::data[2] = v2;
  }

  Tmat3(const Tquaternion<T>& quat)
  {
    Tvec4<T> v(quat);
    // const T ww = v.w() * v.w();
    const T xx = v.x() * v.x();
    const T yy = v.y() * v.y();
    const T zz = v.z() * v.z();
    const T xy = v.x() * v.y();
    const T xz = v.x() * v.z();
    const T xw = v.x() * v.w();
    const T yz = v.y() * v.z();
    const T yw = v.y() * v.w();
    const T zw = v.z() * v.w();

    auto& m = base::data;

    m[0][0] = T(1) - T(2) * (yy + zz);
    m[0][1] = T(2) * (xy + zw);
    m[0][2] = T(2) * (xz - yw);

    m[1][0] = T(2) * (xy - zw);
    m[1][1] = T(1) - T(2) * (xx + zz);
    m[1][2] = T(2) * (yz + xw);

    m[2][0] = T(2) * (xz + yw);
    m[2][1] = T(2) * (yz - xw);
    m[2][2] = T(1) - T(2) * (xx + yy);
  }
};
typedef Tmat3<float> mat3;
typedef Tmat3<int> imat3;
typedef Tmat3<unsigned int> umat3;
typedef Tmat3<double> dmat3;

template <typename T>
class Tmat4 : public matNM<T, 4, 4> {
public:
  typedef matNM<T, 4, 4> base;
  typedef Tmat4<T> self_type;

  inline Tmat4() {}
  inline Tmat4(const self_type& that) : base(that) {}
  inline Tmat4(const base& that) : base(that) {}
  inline Tmat4(const vecN<T, 4>& v) : base(v) {}
  inline Tmat4(const vecN<T, 4>& v0, const vecN<T, 4>& v1, const vecN<T, 4>& v2, const vecN<T, 4>& v3)
  {
    base::data[0] = v0;
    base::data[1] = v1;
    base::data[2] = v2;
    base::data[3] = v3;
  }

  inline Tmat4(const Tmat3<T>& that) : base(that)
  {
    base::data[0][3] = T(0);
    base::data[1][3] = T(0);
    base::data[2][3] = T(0);

    base::data[3][0] = T(0);
    base::data[3][1] = T(0);
    base::data[3][2] = T(0);
    base::data[3][3] = T(1);
  }
};

typedef Tmat4<float> mat4;
typedef Tmat4<int> mat4i;
typedef Tmat4<unsigned int> mat4u;
typedef Tmat4<double> mat4d;

template <typename T, const int w, const int h>
Tquaternion<T> matquat(const matNM<T, w, h> &m)
{
  Tquaternion<T> q;
  T tq[4];
  tq[0] = 1 + m[0][0] + m[1][1] + m[2][2];
  tq[1] = 1 + m[0][0] - m[1][1] - m[2][2];
  tq[2] = 1 - m[0][0] + m[1][1] - m[2][2];
  tq[3] = 1 - m[0][0] - m[1][1] + m[2][2];
  int i = 0, j = 0;
  for (i = 1; i < 4; i++)
    j = (tq[i] > tq[j]) ? i : j;
  if (j == 0) {
    q.w_ = tq[0];
    q.x_ = m[1][2] - m[2][1];
    q.y_ = m[2][0] - m[0][2];
    q.z_ = m[0][1] - m[1][0];
  } else if (j == 1) {
    q.w_ = m[1][2] - m[2][1];
    q.x_ = tq[1];
    q.y_ = m[0][1] + m[1][0];
    q.z_ = m[2][0] + m[0][2];
  } else if (j == 2) {
    q.w_ = m[2][0] - m[0][2];
    q.x_ = m[0][1] + m[1][0];
    q.y_ = tq[2];
    q.z_ = m[1][2] + m[2][1];
  } else /* if (j==3) */
  {
    q.w_ = m[0][1] - m[1][0];
    q.x_ = m[2][0] + m[0][2];
    q.y_ = m[1][2] + m[2][1];
    q.z_ = tq[3];
  }

  T s = sqrt(0.25 / tq[j]);
  q.w_ *= s;
  q.x_ *= s;
  q.y_ *= s;
  q.z_ *= s;
  return q;
}

};  // namespace tg

#endif /* __TVEC_INC__ */
