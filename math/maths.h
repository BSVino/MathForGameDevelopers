#pragma once

// https://www.youtube.com/watch?v=qJq7I2DLGzI
float Approach(float flGoal, float flCurrent, float dt)
{
	float flDifference = flGoal - flCurrent;

	if (flDifference > dt)
		return flCurrent + dt;
	if (flDifference < -dt)
		return flCurrent - dt;

	return flGoal;
}

// http://youtu.be/K3TLNQOxU0o
float Remap(float x, float t1, float t2, float s1, float s2)
{
	// "Yellow" is a "normalized" value between 0 and 1
	float yellow = (x - t1)/(t2 - t1);

	// "Green" is the result!
	float green = yellow*(s2 - s1) + s1;

	return green;
}
