#pragma once

#define SPLINE_POINTS 16

struct CubicSpline
{
	vec3 m_points[SPLINE_POINTS];
	vec3 m_coeffs[SPLINE_POINTS-1][4];
	float m_lengths[SPLINE_POINTS - 1];

	// Spline construction, Burden & Faires - Numerical Analysis 9th, algorithm 3.4
	void InitializeSpline()
	{
		int n = SPLINE_POINTS - 1;

		vec3 a[SPLINE_POINTS];
		for (int i = 1; i <= n - 1; i++)
			a[i] = 3 * ((m_points[i + 1] - 2*m_points[i] + m_points[i - 1]));

		float l[SPLINE_POINTS];
		float mu[SPLINE_POINTS];
		vec3 z[SPLINE_POINTS];

		l[0] = l[n] = 1;
		mu[0] = 0;
		z[0] = z[n] = vec3();
		m_coeffs[n][2] = vec3();

		for (int i = 1; i <= n - 1; i++)
		{
			l[i] = 4 - mu[i-1];
			mu[i] = 1 / l[i];
			z[i] = (a[i] - z[i-1]) / l[i];
		}

		for (int i = 0; i < SPLINE_POINTS; i++)
			m_coeffs[i][0] = m_points[i];

		for (int j = n - 1; j >= 0; j--)
		{
			m_coeffs[j][2] = z[j] - mu[j] * m_coeffs[j + 1][2];
			m_coeffs[j][3] = (1.0f / 3.0f)*(m_coeffs[j + 1][2] - m_coeffs[j][2]);
			m_coeffs[j][1] = m_points[j + 1] - m_points[j] - m_coeffs[j][2] - m_coeffs[j][3];
		}

		for (int k = 0; k < SPLINE_POINTS - 1; k++)
			m_lengths[k] = SimpsonsRuleSingleSpline(k, 1);
	}

	vec3 SplineAtTime(float t)
	{
		if (t > SPLINE_POINTS)
			t = SPLINE_POINTS;

		if (t == SPLINE_POINTS)
			t = SPLINE_POINTS - 0.0000f;

		int spline = (int)t;
		float fractional = t - spline;

		spline = spline % (SPLINE_POINTS-1);

		float x = fractional;
		float xx = x*x;
		float xxx = x*xx;

		vec3 result = m_coeffs[spline][0] + m_coeffs[spline][1]*x + m_coeffs[spline][2]*xx + m_coeffs[spline][3]*xxx;
		return result;
	}

	float ArcLengthIntegrandSingleSpline(int spline, float t)
	{
		VAssert(t >= -1);
		VAssert(t <= 2);

		float tt = t*t;

		vec3 dv = m_coeffs[spline][1] + 2 * m_coeffs[spline][2] * t + 3 * m_coeffs[spline][3] * tt;
		float xx = dv.x*dv.x;
		float yy = dv.y*dv.y;
		float zz = dv.z*dv.z;

		return sqrt(xx + yy + zz);
	}

	float ArcLengthIntegrand(float t)
	{
		if (t < 0)
			return ArcLengthIntegrandSingleSpline(0, t);

		if (t >= SPLINE_POINTS-1)
			return ArcLengthIntegrandSingleSpline(SPLINE_POINTS-1, fmod(t, 1));

		return ArcLengthIntegrandSingleSpline((int)t, t - (int)t);
	}

	// Composite Simpson's Rule, Burden & Faires - Numerical Analysis 9th, algorithm 4.1
	float SimpsonsRuleSingleSpline(int spline, float t)
	{
		VAssert(t >= -1);
		VAssert(t <= 2);

		int n = 16;
		float h = t / n;
		float XI0 = ArcLengthIntegrandSingleSpline(spline, t);
		float XI1 = 0;
		float XI2 = 0;

		for (int i = 0; i < n; i += 2)
			XI2 += ArcLengthIntegrandSingleSpline(spline, i*h);

		for (int i = 1; i < n; i += 2)
			XI1 += ArcLengthIntegrandSingleSpline(spline, i*h);

		float XI = h * (XI0 + 2 * XI2 + 4 * XI1) * (1.0f / 3);
		return XI;
	}

	float SimpsonsRule(float t0, float t1)
	{
		float multiplier = 1;
		if (t0 > t1)
		{
			std::swap(t0, t1);
			multiplier = -1;
		}

		int first_spline = (int)t0;
		if (first_spline < 0)
			first_spline = 0;

		int last_spline = (int)t1;
		if (last_spline >= SPLINE_POINTS-1)
			last_spline = SPLINE_POINTS-2;

		if (first_spline == last_spline)
			return (SimpsonsRuleSingleSpline(last_spline, t1 - last_spline) - SimpsonsRuleSingleSpline(first_spline, t0 - first_spline)) * multiplier;

		float sum = m_lengths[first_spline] - SimpsonsRuleSingleSpline(first_spline, t0);

		for (int k = first_spline+1; k < last_spline; k++)
			sum += m_lengths[k];

		sum += SimpsonsRuleSingleSpline(last_spline, t1 - last_spline);

		return sum * multiplier;
	}

	vec3 ConstVelocitySplineAtTime(float t, float speed)
	{
		float total_length = GetTotalLength();
		float desired_distance = fmod(t * speed, total_length);

		float t_last = SPLINE_POINTS * desired_distance / total_length;
		float t_next = t_last;

		auto g = [this, desired_distance](float t) -> float {
			return SimpsonsRule(0, t) - desired_distance;
		};

		auto L = [this](float t) -> float {
			return ArcLengthIntegrand(t);
		};

		float t_max = SPLINE_POINTS;
		float t_min = -0.1f;

		while (t_max - t_min > 0.5f)
		{
			float t_mid = (t_max + t_min)/2;
			if (g(t_min) * g(t_mid) < 0)
				t_max = t_mid;
			else
				t_min = t_mid;
		}

		t_next = (t_max + t_min)/2;

		do {
			t_last = t_next;
			t_next = t_last - g(t_last) / L(t_last);
		} while(fabs(t_last - t_next) > 0.001f);

		// Because of root finding it may be slightly negative sometimes.
		VAssert(t_next >= -0.1f && t_next <= 999999);

		return SplineAtTime(t_next);
	}

	float GetTotalLength()
	{
		float sum = 0;

		for (int k = 0; k < VArraySize(m_lengths); k++)
			sum += m_lengths[k];

		return sum;
	}
};

CubicSpline g_spline;
