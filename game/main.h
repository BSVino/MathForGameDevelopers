#pragma once

float RandFloat(float min, float max)
{
	return ((float)mtrand()) * (max-min) /MTRAND_MAX + min;
}
