#pragma once

class Vector
{
public:
	Vector() {}
	Vector(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}
	Vector(const class Point& p);

	float Length() const;
	float LengthSqr() const;

	Vector operator-() const;

	Vector operator+(const Vector& v) const;
	Vector operator-(const Vector& v) const;

	Vector operator*(float s) const;
	Vector operator/(float s) const;

	Vector Normalized() const;
	void Normalize();

	float  Dot(const Vector& v) const;
	Vector Cross(const Vector& v) const;

public:
	union {
		struct {
			float x, y, z;
		};
		float v[3];
	};
};

class Point
{
public:
	Point() {}
	Point(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	Point(const Vector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Point operator+(const Vector& v) const;
	Point operator-(const Vector& v) const;

	float x, y, z;
};
