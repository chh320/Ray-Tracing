#ifndef VEC3_H
#define VEC3_H

#include <Vec4.h>
#include <MathUtils.h>

namespace GLSLPT 
{
	struct Vec3
	{
	public:
		Vec3();
		Vec3(float x, float y, float z);
		Vec3(const Vec4& b);

		Vec3 operator*(const Vec3& b) const;
		Vec3 operator+(const Vec3& b) const;
		Vec3 operator-(const Vec3& b) const;
		Vec3 operator*(float b) const;

		float operator[](int i) const;
		float& operator[](int i) ;

		static Vec3 Min(const Vec3& a, const Vec3& b);
		static Vec3 Max(const Vec3& a, const Vec3& b);
		static Vec3 Cross(const Vec3& a, const Vec3& b);
		static Vec3 Pow(const Vec3& a, float exp);
		static float Length(const Vec3& a);
		static float Distance(const Vec3& a, const Vec3& b);
		static float Dot(const Vec3& a, const Vec3& b);
		static Vec3 Clamp(const Vec3& a, const Vec3& min, const Vec3& max);
		static Vec3 Normalize(const Vec3& a);

		float x, y, z;
	};

	inline Vec3::Vec3() {
		x = y = z = 0;
	}

	inline Vec3::Vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	inline Vec3::Vec3(const Vec4& b) {
		x = b.x;
		y = b.y;
		z = b.z;
	}

	inline Vec3 Vec3::operator*(const Vec3& b) const {
		return Vec3(x * b.x, y * b.y, z * b.z);
	}

	inline Vec3 Vec3::operator+(const Vec3& b) const {
		return Vec3(x + b.x, y + b.y, z + b.z);
	}

	inline Vec3 Vec3::operator-(const Vec3& b) const {
		return Vec3(x - b.x, y - b.y, z - b.z);
	}

	inline Vec3 Vec3::operator*(float b) const {
		return Vec3(x * b, y * b, z * b);
	}

	inline float Vec3::operator[](int i) const{
		if (i == 0) {
			return x;
		}
		else if (i == 1) {
			return y;
		}
		else {
			return z;
		}
	}

	inline float& Vec3::operator[](int i) {
		if (i == 0) {
			return x;
		}
		else if (i == 1) {
			return y;
		}
		else {
			return z;
		}
	}

	inline Vec3 Vec3::Min(const Vec3& a, const Vec3& b) {
		Vec3 out;
		out.x = std::min(a.x, b.x);
		out.y = std::min(a.y, b.y);
		out.z = std::min(a.z, b.z);
		return out;
	}

	inline Vec3 Vec3::Max(const Vec3& a, const Vec3& b)
	{
		Vec3 out;
		out.x = std::max(a.x, b.x);
		out.y = std::max(a.y, b.y);
		out.z = std::max(a.z, b.z);
		return out;
	}

	inline Vec3 Vec3::Cross(const Vec3& a, const Vec3& b)
	{
		return Vec3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - b.z * a.x,
			a.x * b.y - a.y * b.x);
	};

	inline Vec3 Vec3::Pow(const Vec3& a, float exp)
	{
		return Vec3(
			powf(a.x, exp),
			powf(a.y, exp),
			powf(a.z, exp)
		);
	}

	inline float Vec3::Dot(const Vec3& a, const Vec3& b)
	{
		return (a.x * b.x + a.y * b.y + a.z * b.z);
	}

	inline float Vec3::Length(const Vec3& a) {
		return sqrtf(Dot(a, a));
	}

	inline float Vec3::Distance(const Vec3& a, const Vec3& b)
	{
		Vec3 t = a;
		return Length(t - b);
	}

	inline Vec3 Vec3::Clamp(const Vec3& a, const Vec3& min, const Vec3& max)
	{
		return Vec3(
			Math::Clamp(a.x, min.x, max.x),
			Math::Clamp(a.y, min.y, max.y),
			Math::Clamp(a.z, min.z, max.z)
		);
	}

	inline Vec3 Vec3::Normalize(const Vec3& a) {
		float length = Length(a);
		return a * (1.0 / length);
	}
}
#endif // !VEC3_H
