#ifndef __TMATH_INC__
#define __TMATH_INC__

#include "tvec.h"
#include <algorithm>

namespace tg
{

template <typename T>
struct random{
	operator T()
	{
		static unsigned int seed = 0x13371337;
		unsigned int res;
		unsigned int tmp;

		seed *= 16807;

		tmp = seed ^ (seed >> 4) ^ (seed << 15);

		res = (tmp >> 9) | 0x3F800000;

		return static_cast<T>(res);
	}
};

template<>
struct random<float>{
	operator float()
	{
		static unsigned int seed = 0x13371337;
		float res;
		unsigned int tmp;

		seed *= 16807;

		tmp = seed ^ (seed >> 4) ^ (seed << 15);

		*((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

		return (res - 1.0f);
	}
};

template<>
struct random<unsigned int>{
	operator unsigned int()
	{
		static unsigned int seed = 0x13371337;
		unsigned int res;
		unsigned int tmp;

		seed *= 16807;

		tmp = seed ^ (seed >> 4) ^ (seed << 15);

		res = (tmp >> 9) | 0x3F800000;

		return res;
	}
};

template<typename T>
struct EPSTraits {
	static constexpr T eps = feps;
};

template<>
struct EPSTraits<double> {
	static constexpr double eps = deps;
};

inline mat4 frustum(float left, float right, float bottom, float top, float n, float f)
{
	mat4 result(mat4::identity());

	if((right == left) ||
		(top == bottom) ||
	   (n == f) ||
	   (n < 0.0) ||
	   (f < 0.0))
		return result;

	result[0][0] = (2.0f * n) / (right - left);
	result[1][1] = (2.0f * n) / (top - bottom);

	result[2][0] = (right + left) / (right - left);
	result[2][1] = (top + bottom) / (top - bottom);
	result[2][2] = -(f + n) / (f - n);
	result[2][3] = -1.0f;

	result[3][2] = -(2.0f * f * n) / (f - n);
	result[3][3] = 0.0f;

	return result;
}

//aspect = width/height
template<typename T>
inline Tmat4<T> perspective(T fovy, T aspect, T n, T f)
{
  T q = 1.0f / tan(radians(0.5f * fovy));
  T A = q / aspect;
#ifdef DEPTH_REVERSE
  T B = n / (f - n);
  T C = n * f / (f - n);
#elif defined DEPTH_NEAR_ZERO 
  T B = f / (n - f);
  T C = n * f / (n - f);
#else
  T B = (n + f) / (n - f);
  T C = (2.0f * n * f) / (n - f);
#endif

  Tmat4<T> result;

  result[0] = Tvec4<T>(A, 0.0f, 0.0f, 0.0f);
  result[1] = Tvec4<T>(0.0f, q, 0.0f, 0.0f);
  result[2] = Tvec4<T>(0.0f, 0.0f, B, -1.0f);
  result[3] = Tvec4<T>(0.0f, 0.0f, C, 0.0f);

  return result;
}

inline mat4 ortho(float left, float right, float bottom, float top, float n, float f)
{
  return mat4(vec4(2.0f / (right - left), 0.0f, 0.0f, 0.0f), 
							vec4(0.0f, 2.0f / (top - bottom), 0.0f, 0.0f),
#ifdef DEPTH_REVERSE
              vec4(0.0f, 0.0f, 1.0f / (f - n), 0.0f), 
							vec4((left + right) / (left - right), (bottom + top) / (bottom - top), f / (f - n), 1.0f)
#elif defined DEPTH_NEAR_ZERO 
              vec4(0.0f, 0.0f, 1.0f / (n - f), 0.0f), 
							vec4((left + right) / (left - right), (bottom + top) / (bottom - top), n / (n - f), 1.0f)
#else
              vec4(0.0f, 0.0f, 2.0f / (n - f), 0.0f), 
							vec4((left + right) / (left - right), (bottom + top) / (bottom - top), (n + f) / (n - f), 1.0f)
#endif
  );
}

//reverse///////////////////////////////////////////////////////////////////////
inline void view_planes(const mat4& transmat, vec4& l, vec4& r, vec4& b, vec4& t, vec4& n, vec4& f)
{
	//mi represent ith row of transmat;
	// left = m4 + m1
	l = vec4(transmat[0][0] + transmat[0][3], transmat[1][0] + transmat[1][3], transmat[2][0] + transmat[2][3], transmat[3][0] + transmat[3][3]);
	// right = m4 - m1
	r = vec4(transmat[0][3] - transmat[0][0], transmat[1][3] - transmat[1][0], transmat[2][3] - transmat[2][0], transmat[3][3] - transmat[3][0]);
	// bottom = m2 + m4
	b = vec4(transmat[0][1] + transmat[0][3], transmat[1][1] + transmat[1][3], transmat[2][1] + transmat[2][3], transmat[3][1] + transmat[3][3]);
	// top = m4 - m2
	t = vec4(transmat[0][3] - transmat[0][1], transmat[1][3] - transmat[1][1], transmat[2][3] - transmat[2][1], transmat[3][3] - transmat[3][1]);
	// near = m3 + m4
	n = vec4(transmat[0][2] + transmat[0][3], transmat[1][2] + transmat[1][3], transmat[2][2] + transmat[2][3], transmat[3][2] + transmat[3][3]);
	// far = m4 - m3
	f = vec4(transmat[0][3] - transmat[0][2], transmat[1][3] - transmat[1][2], transmat[2][3] - transmat[2][2], transmat[3][3] - transmat[3][2]);
}

inline float sgn(float x)
{
  if (x > 0)
    return 1.f;
  else if (x < 0)
    return -1.f;
  return 0.f;
}

//frustum space clip
inline void near_clip(mat4& prjmat, const vec4& clip_plane)
{
	vec4 q;
	q[0] = (sgn(clip_plane[0]) + prjmat[2][0]) / prjmat[0][0];
	q[1] = (sgn(clip_plane[1]) + prjmat[2][1]) / prjmat[1][1];
	q[2] = -1;
	q[3] = (1 + prjmat[2][2]) / prjmat[3][2];

	q = clip_plane * (2.f / dot<float>(clip_plane, q));

	prjmat[0][2] = q[0];
	prjmat[1][2] = q[1];
	prjmat[2][2] = q[2] + 1;
	prjmat[3][2] = q[3];
}

template <typename T>
inline Tmat3<T> translate(T x, T y)
{
	return Tmat3<T>(Tvec3(1.f, 0.f, 0.f), Tvec3(0.f, 1.f, 0.f), Tvec3(x, y, 1.f));
}

template<typename T>
inline Tmat3<T> translate(const vecN<T, 2> &v)
{
	return translate(v[0], v[1]);
}

template <typename T>
inline Tmat4<T> translate(T x, T y, T z)
{
	return Tmat4<T>(Tvec4<T>(1.0f, 0.0f, 0.0f, 0.0f),
					Tvec4<T>(0.0f, 1.0f, 0.0f, 0.0f),
					Tvec4<T>(0.0f, 0.0f, 1.0f, 0.0f),
					Tvec4<T>(x, y, z, 1.0f));
}

template <typename T>
inline Tmat4<T> translate(const Tvec3<T>& v)
{
	return translate(v[0], v[1], v[2]);
}

template <typename T>
inline Tmat4<T> lookat(const Tvec3<T>& eye, const Tvec3<T>& center = Tvec3<T>(), const Tvec3<T>& up = Tvec3<T>(0, 0, 1))
{
	const Tvec3<T> f = normalize(center - eye);
	const Tvec3<T> s = normalize(cross(f, up));
	const Tvec3<T> u = normalize(cross(s, f));
	const Tmat4<T> M = Tmat4<T>(Tvec4<T>(s[0], u[0], -f[0], T(0)),
								Tvec4<T>(s[1], u[1], -f[1], T(0)),
								Tvec4<T>(s[2], u[2], -f[2], T(0)),
								Tvec4<T>(T(0), T(0), T(0), T(1)));

	return M * translate<T>(-eye);
}

template <typename T>
inline Tmat4<T> scale(T x, T y, T z)
{
	return Tmat4<T>(Tvec4<T>(x, 0.0f, 0.0f, 0.0f),
					Tvec4<T>(0.0f, y, 0.0f, 0.0f),
					Tvec4<T>(0.0f, 0.0f, z, 0.0f),
					Tvec4<T>(0.0f, 0.0f, 0.0f, 1.0f));
}

template <typename T>
inline Tmat4<T> scale(const Tvec3<T>& v)
{
	return scale(v[0], v[1], v[2]);
}

template <typename T>
inline Tmat4<T> scale(T x)
{
	return Tmat4<T>(Tvec4<T>(x, 0.0f, 0.0f, 0.0f),
					Tvec4<T>(0.0f, x, 0.0f, 0.0f),
					Tvec4<T>(0.0f, 0.0f, x, 0.0f),
					Tvec4<T>(0.0f, 0.0f, 0.0f, 1.0f));
}


template <typename T>
inline Tmat4<T> rotate(T rads, T x, T y, T z)
{
	Tmat4<T> result;

	const T x2 = x * x;
	const T y2 = y * y;
	const T z2 = z * z;
	const double c = cos(rads);
	const double s = sin(rads);
	const double omc = 1.0f - c;

	result[0] = Tvec4<T>(T(x2 * omc + c), T(y * x * omc + z * s), T(x * z * omc - y * s), T(0));
	result[1] = Tvec4<T>(T(x * y * omc - z * s), T(y2 * omc + c), T(y * z * omc + x * s), T(0));
	result[2] = Tvec4<T>(T(x * z * omc + y * s), T(y * z * omc - x * s), T(z2 * omc + c), T(0));
	result[3] = Tvec4<T>(T(0), T(0), T(0), T(1));

	return result;
}

template <typename T>
inline Tmat4<T> rotate(T angle, const vecN<T, 3>& v)
{
	return rotate<T>(angle, v[0], v[1], v[2]);
}

template <typename T>
inline Tmat4<T> rotate(T angle_x, T angle_y, T angle_z)
{
	return rotate(angle_z, 0.0f, 0.0f, 1.0f) *
		rotate(angle_y, 0.0f, 1.0f, 0.0f) *
		rotate(angle_x, 1.0f, 0.0f, 0.0f);
}

template <typename T, int N>
static inline vecN<T, N> min(const vecN<T, N>& x, const vecN<T, N>& y)
{
	vecN<T, N> t;
	int n;

	for(n = 0; n < N; n++){
		t[n] = std::min(x[n], y[n]);
	}

	return t;
}
template<typename T>
static inline T clamp(T t, T min = 0, T max = 1)
{
	return t > max ? max : t < min ? min : t;
}

template <typename T, const int N>
static inline vecN<T, N> max(const vecN<T, N>& x, const vecN<T, N>& y)
{
	vecN<T, N> t;
	int n;

	for(n = 0; n < N; n++){
		t[n] = std::max<T>(x[n], y[n]);
	}

	return t;
}

template <typename T, const int N>
static inline vecN<T, N> clamp(const vecN<T, N>& x, const vecN<T, N>& minVal, const vecN<T, N>& maxVal)
{
	return std::min<T>(std::max<T>(x, minVal), maxVal);
}

template <typename T, const int N>
static inline vecN<T, N> smoothstep(const vecN<T, N>& edge0, const vecN<T, N>& edge1, const vecN<T, N>& x)
{
	vecN<T, N> t;
	t = clamp((x - edge0) / (edge1 - edge0), vecN<T, N>(T(0)), vecN<T, N>(T(1)));
	return t * t * (vecN<T, N>(T(3)) - vecN<T, N>(T(2)) * t);
}

template <typename T, const int S>
static inline vecN<T, S> reflect(const vecN<T, S>& I, const vecN<T, S>& N)
{
	return I - 2 * dot(N, I) * N;
}

template <typename T, const int S>
static inline vecN<T, S> refract(const vecN<T, S>& I, const vecN<T, S>& N, T eta)
{
	T d = dot(N, I);
	T k = T(1) - eta * eta * (T(1) - d * d);
	if(k < 0.0){
		return vecN<T, N>(0);
	} else{
		return eta * I - (eta * d + sqrt(k)) * N;
	}
}

template <typename T, const int N, const int M>
static inline vecN<T, M> operator*(const vecN<T, N>& vec, const matNM<T, N, M>& mat)
{
	vecN<T, M> result(T(0));
	T sum;
	for(int m = 0; m < M; m++){
		sum = 0;
		for(int n = 0; n < N; n++){
			sum += vec[n] * mat[n][m];
		}
		result[m] = sum;
	}
	return result;
}

template <typename T, const int N, const int M>
static inline matNM<T, N, M> operator^(const matNM<T, N, M>& x, const matNM<T, N, M>& y)
{
  matNM<T, N, M> result;
  int i, j;

  for (j = 0; j < M; ++j) {
    for (i = 0; i < N; ++i) {
      result[i][j] = x[i][j] * y[i][j];
    }
  }

  return result;
}

template <typename T, const int N, const int M>
static inline vecN<T, M> operator*(const matNM<T, N, M>& mat, const vecN<T, N>& vec)
{
	vecN<T, M> result(T(0));
	T sum;
	for(int m = 0; m < M; m++){
		sum = 0;
		for(int n = 0; n < N; n++){
			sum += vec[n] * mat[n][m];
		}
		result[m] = sum;
	}
	return result;
}

template<typename T>
static inline Tvec3<T> operator*(const matNM<T, 4, 4>& mat, const Tvec3<T>& vec)
{
  Tvec4<T> tmp(vec, 1);
  vecN<T, 4> ret = operator*<T, 4, 4>(mat, tmp);
  return Tvec3<T>(ret[0] / ret[3], ret[1] / ret[3], ret[2] / ret[3]);
}

template <typename T, const int N>
static inline vecN<T, N> operator/(const T s, const vecN<T, N>& v)
{
	int n;
	vecN<T, N> result;

	for(n = 0; n < N; n++){
		result[n] = s / v[n];
	}

	return result;
}

template <typename T>
static inline T mix(const T& A, const T& B, typename T::ele_type t)
{
	return B + t * (B - A);
}

template <typename T>
static inline T mix(const T& A, const T& B, const T& t)
{
	return B + t * (B - A);
}

template <typename T, const int N>
static inline bool inverse(matNM<T, N, N> &des, const matNM<T, N, N>& ori)
{
	const int width = 2 * N;
	T mat[N][width];
	memset(&mat, 0, sizeof(T) * N * width);
	T* cidx[N];
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			mat[i][j] = ori[j][i];
		}
		mat[i][N + i] = 1;
		cidx[i] = (T*)&mat[i];
	}
	for (int i = 0; i < N; i++) {
		if (fabs(cidx[i][i]) < EPSTraits<T>::eps) {
			int k = i + 1;
			while (k < N) {
				if (fabs(cidx[k][i]) > EPSTraits<T>::eps)
					break;
				k++;
			}
			if (k == N)
				return false;
			else {
				T* tmp = cidx[i];
				cidx[i] = cidx[k];
				cidx[k] = tmp;
			}
		}
		T tmp = cidx[i][i];
		for (int j = 0; j < N; j++) {
			if (j == i)
				continue;
			T temp = cidx[j][i];
			for (int k = 0; k < width; k++) {
				cidx[j][k] = cidx[j][k] * tmp - temp * cidx[i][k];
			}
		}
	}
	for (int i = 0; i < N; i++) {
		for (int j = N; j < width; j++) {
			cidx[i][j] /= cidx[i][i];
		}
	}
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			des[j][i] = cidx[i][j + N];
		}
	}
	return true;
}

};

#endif /* __TMATH_H__ */
