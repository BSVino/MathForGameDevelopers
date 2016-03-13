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
			m_lengths[k] = Integrate(k, 1);
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

	float ArcLengthIntegrand(int spline, float t)
	{
		float tt = t*t;

		vec3 dv = m_coeffs[spline][1] + 2 * m_coeffs[spline][2] * t + 3 * m_coeffs[spline][3] * tt;
		float xx = dv.x*dv.x;
		float yy = dv.y*dv.y;
		float zz = dv.z*dv.z;

		return sqrt(xx + yy + zz);
	}

	// Composite Simpson's Rule, Burden & Faires - Numerical Analysis 9th, algorithm 4.1
	float Integrate(int spline, float t)
	{
		int n = 16;
		float h = t / n;
		float XI0 = ArcLengthIntegrand(spline, t);
		float XI1 = 0;
		float XI2 = 0;

		for (int i = 0; i < n; i++)
		{
			float X = i*h;
			if (i % 2 == 0)
				XI2 += ArcLengthIntegrand(spline, X);
			else
				XI1 += ArcLengthIntegrand(spline, X);
		}

		float XI = h * (XI0 + 2 * XI2 + 4 * XI1) * (1.0f / 3);
		return XI;
	}

	vec3 ConstVelocitySplineAtTime(float t)
	{
		int spline = 0;
		while (t > m_lengths[spline])
		{
			t -= m_lengths[spline];
			spline += 1;
		}

		float s = t / m_lengths[spline]; // Here's our initial guess.

		// Do some Newton-Rhapsons.
		s = s - (Integrate(spline, s) - t) / ArcLengthIntegrand(spline, s);
		s = s - (Integrate(spline, s) - t) / ArcLengthIntegrand(spline, s);
		s = s - (Integrate(spline, s) - t) / ArcLengthIntegrand(spline, s);
		s = s - (Integrate(spline, s) - t) / ArcLengthIntegrand(spline, s);
		s = s - (Integrate(spline, s) - t) / ArcLengthIntegrand(spline, s);
		s = s - (Integrate(spline, s) - t) / ArcLengthIntegrand(spline, s);

		return SplineAtTime(spline + s);
	}
};

CubicSpline g_spline;
