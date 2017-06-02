// Adapted from Scratchapixel.com
#ifndef VEC3_H
#define VEC3_H

#include <math.h>
using namespace std;

template<typename T>
class Vec3 {
public:
	T x, y, z;

	//Constructors
	Vec3() : x(0), y(0), z(0) {};
	Vec3(T xx) : x(xx), y(xx), z(xx) {};
	Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {};

	////Operations
	Vec3<T> operator + (const Vec3 &v) const {
		return Vec3(x + v.x, y + v.y, z + v.z);
	}
	Vec3<T> operator - (const Vec3 &v) const {
		return Vec3(x - v.x, y - v.y, z - v.z);
	}
	Vec3<T> operator * (const T &r) const {
		return Vec3(x * r, y * r, z * r);
	}
	T dotProd(const Vec3<T> &v) {
		return x * v.x + y * v.y + z * v.z;
	}
	Vec3<T> crossProd(Vec3<T> &v) {
		return Vec3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}
	Vec3<T> normalize() {
		float len = length();
		if (len > 0) {
			return Vec3(x / len, y / len, z / len);
		}
		else
			return *this;
	}
	float length() {
		return sqrt(abs(x*x + y*y + z*z));
	}

		//Accessor via Square Brackets (v[0])
	const T& operator [] (uint8_t i) const { return (&x)[i]; }
	T& operator [] (uint8_t i) { return (&x)[i]; }

	//Print using cout
	friend std::ostream& operator << (std::ostream &s, const Vec3<T> &v) {
		return s << '(' << v.x << ' ' << v.y << ' ' << v.z << ')';
	}
};
#endif