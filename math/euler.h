#pragma once

class Vector;

class EAngle
{
public:
	EAngle()
	{
		p = y = r = 0;
	}

	EAngle(float pitch, float yaw, float roll)
	{
		p = pitch;
		y = yaw;
		r = roll;
	}

public:
	Vector ToVector() const;

	void Normalize();

public:
	float p;
	float y;
	float r;
};
