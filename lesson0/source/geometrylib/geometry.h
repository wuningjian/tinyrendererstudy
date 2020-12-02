#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>

// 模板 vec2 有参构造 无参构造 + - *重载
template <class t> struct Vec2 {
	union {
		struct {t u, v;};
		struct {t x, y;};
		t raw[2];
	};
	Vec2() : u(0), v(0) {}
	Vec2(t _u, t _v) : u(_u),v(_v) {}
	inline Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(u+V.u, v+V.v); }
	inline Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(u-V.u, v-V.v); }
	inline Vec2<t> operator *(float f)          const { return Vec2<t>(u*f, v*f); }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
    union {
        struct{t x,y,z;};
        struct{t ivert,iuv,inormal;};
        t raw[3];
    };
    Vec3():x(0),y(0),z(0){}
    Vec3(t _x, t _y, t _z):x(_x),y(_y),z(_z){}
    inline Vec3<t> operator +(const Vec3<t> &V) const {return Vec3<t>(x+V.x, y+V.y, z+V.z); }
    inline Vec3<t> operator -(const Vec3<t> &V) const {return Vec3<t>(x-V.x, y-V.y, z-V.z); }
    inline t operator *(const Vec3<t> &V) const {return x*V.x + y*V.y + z*V.z; }
    inline Vec3<t> operator *(const float i) const {return Vec3<t>(x*f, y*f, z*f);}
    float norm() const{return std::sqrt(x*x+y*y+z*z);}
    Vec3<t> & normalize() {
        *this = (*this) * (1/norm());
        return *this;
    }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;

typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;
// 

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}

#endif