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

public:
	float x, y, z;
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

	Point operator+(const Vector& v) const;
	Point operator-(const Vector& v) const;

	float x, y, z;
};
