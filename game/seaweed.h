#define vec3 Vector
#define vec2 Vector2D
#define mat4 Matrix4x4
#define vec4 Vector4D

#define M_TAU (2*M_PI)

inline float dot(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float dot(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

inline float dot(vec4 a, vec4 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline vec2 operator*(vec2 a, vec2 b)
{
	return vec2(a.x * b.x, a.y * b.y);
}

inline vec2 operator*(float f, vec2 a)
{
	return vec2(a.x * f, a.y * f);
}

inline float lengthsqr(vec3 a)
{
	return dot(a, a);
}

inline vec3 normalize(vec3 a)
{
	return a / sqrt(dot(a, a));
}

inline vec4 normalize(vec4 a)
{
	float s = (1/sqrt(dot(a, a)));
	return vec4(a.x * s, a.y * s, a.z * s, a.w * s);
}

inline float length(vec3 a)
{
	return sqrt(dot(a, a));
}

inline bool operator!=(vec3 a, vec3 b)
{
	return !(a.x == b.x && a.y == b.y && a.z == b.z);
}

inline vec4 cross(vec4 a, vec4 b)
{
	Vector4D c;

	c.x = a.y*b.z - a.z*b.y;
	c.y = a.z*b.x - a.x*b.z;
	c.z = a.x*b.y - a.y*b.x;
	c.w = 1;

	return c;
}

inline vec4 operator-(vec4 a)
{
	return vec4(-a.x, -a.y, -a.z, -a.w);
}

#define SEAWEED_FRAMERATE (1.0f/60)
#define SEAWEED_LINKS 16
struct Seaweed
{
	vec3 m_positions[2][SEAWEED_LINKS];
};

#define DO_SEAWEED 0
#define NUM_SEAWEEDS 29
Seaweed g_seaweed[NUM_SEAWEEDS];
float g_seaweed_link_length = 3.5f;
float g_mass_per_link = 1.0f;
int g_seaweed_current_list = 0;
float g_seaweed_simulation_time = 0.0f;
float g_seaweed_thickness = 0.3f;
float g_seaweed_player_distance = 4;

const vec2 g_seaweed_positions[] = {
	vec2(-25, 5),
	vec2(-25.5f, 5.5f),
	vec2(-25.3f, 5.3f),
	vec2(-24.8f, 5.4f),
	vec2(-20, -6),
	vec2(-21, -5.7f),
	vec2(-20.5f, -6.1f),
	vec2(-20.3f, -6.7f),
	vec2(-19, -4),
	vec2(-18, -3),
	vec2(-15, 0.4f),
	vec2(-15.5f, 0.9f),
	vec2(-15.3f, 0.6f),
	vec2(-14.8f, 0.8f),
	vec2(-13, 7),
	vec2(-10, 4),
	vec2(8, 5),
	vec2(16, -3),
	vec2(13, -1),
	vec2(12, 2.5f),
	vec2(17, 0),
	vec2(16, 1.5f),
	vec2(13, 1),
	vec2(14.5f, -3.2f),
	vec2(16.5f, -1.1f),
	vec2(17.5f, 2.3f),
	vec2(14.3f, 0.2f),
	vec2(13.7f, 1.3f),
	vec2(16.8f, 1.1f),
};

const vec2 g_wave_vectors[4] = {
	vec2(0.955f, 0.296f)*0.1f,
	vec2(0.866f, 0.5f)*3.0f,
	vec2(0.866f, -0.5f)*3.0f,
	vec2(-1.0f, 0.0f)*3.0f
};

vec2 g_wave_vectors_cube[4] = {
	vec2(0, 0),
	vec2(0, 0),
	vec2(0, 0),
	vec2(0, 0)
};

const float g_wave_magnitudes[4] = {
	43.0f,
	0.014f,
	0.02f,
	0.025f
};

const float g_wave_frequencies[4] = {
	0.5f,
	4.0f,
	5.0f,
	3.0f
};

// We need the second derivative cycloid here because we need it for f = ma
vec2 seaweed_cycloid_ddv(vec2 position, float time, float depth)
{
	time = -time;
	return depth*(
		g_wave_vectors_cube[0] * sin(dot(g_wave_vectors[0], position) + g_wave_frequencies[0] * time) +
		g_wave_vectors_cube[1] * sin(dot(g_wave_vectors[1], position) + g_wave_frequencies[1] * time) +
		g_wave_vectors_cube[2] * sin(dot(g_wave_vectors[2], position) + g_wave_frequencies[2] * time) +
		g_wave_vectors_cube[3] * sin(dot(g_wave_vectors[3], position) + g_wave_frequencies[3] * time)
		);
}

const float g_burst_time = 31.0f;
const float g_grow_time = 37.0f;
const vec3 g_blue_position = vec3(15.0f, 0, -6);
//const float g_burst_time = 4.0;
//const float g_grow_time = 10.0;
//const vec3 g_blue_position = vec3(-26.0, 2.2, -12);

float BlueBurst(float t)
{
	float burst_time = t - g_burst_time;
	return -2.4f * burst_time * exp(-burst_time*burst_time);
}

void SimulateSeaweed()
{
	g_seaweed_simulation_time += SEAWEED_FRAMERATE;

	if (!DO_SEAWEED)
		return;

	if (!g_wave_vectors_cube[0].x)
	{
		g_wave_vectors_cube[0] = g_wave_vectors[0] * g_wave_vectors[0] * g_wave_vectors[0] * g_wave_magnitudes[0];
		g_wave_vectors_cube[1] = g_wave_vectors[1] * g_wave_vectors[1] * g_wave_vectors[1] * g_wave_magnitudes[1];
		g_wave_vectors_cube[2] = g_wave_vectors[2] * g_wave_vectors[2] * g_wave_vectors[2] * g_wave_magnitudes[2];
		g_wave_vectors_cube[3] = g_wave_vectors[3] * g_wave_vectors[3] * g_wave_vectors[3] * g_wave_magnitudes[3];
	}

	float surface_height = 10;
	int curr_list = g_seaweed_current_list;
	int last_list = !g_seaweed_current_list;

	float dt_squared = SEAWEED_FRAMERATE * SEAWEED_FRAMERATE;
	float link_length_squared = g_seaweed_link_length*g_seaweed_link_length;
	float player_distance_squared = g_seaweed_player_distance*g_seaweed_player_distance;

	for (int k = 0; k < sizeof(g_seaweed) / sizeof(g_seaweed[0]); k++)
	{
		for (int n = 1; n < SEAWEED_LINKS; n++)
		{
			vec3 curr_position = g_seaweed[k].m_positions[curr_list][n];
			vec3 last_position = g_seaweed[k].m_positions[last_list][n];
			vec3 lower_link_position = g_seaweed[k].m_positions[last_list][n - 1]; // Use last_list because the updated value went there.
			float depth = (surface_height - curr_position.z) / 11.0f;
			int links_to_push = SEAWEED_LINKS - n;

			vec3 velocity = (curr_position - last_position) / SEAWEED_FRAMERATE;
			float velocity_sqr = dot(velocity, velocity);
			vec3 drag = velocity * -1.0f * velocity_sqr;

			vec2 surge = 10*seaweed_cycloid_ddv(vec2(curr_position.x, curr_position.y), (float)Game()->GetTime(), depth);
			vec3 bouyancy = vec3(0, (float)n, 0);
			vec3 avoid_player = vec3(0, 0, 0);

			vec3 away_from_player = curr_position - Game()->m_hPlayer->GetGlobalAABB().GetCenter();
			float distance_to_player_sqr = lengthsqr(away_from_player);
			if (player_distance_squared > distance_to_player_sqr)
				avoid_player = normalize(away_from_player) * 400.0f;

			vec3 force = vec3(surge.x, 0, surge.y) + bouyancy + drag + avoid_player;

			vec3 acceleration = force / (g_mass_per_link * links_to_push);

			// This is just a regular euler integration. Not using previously calculated velocity here to maintain precision.
			vec3 new_position = 2.0f*curr_position - last_position + acceleration * dt_squared;

			new_position.y = vmax((float)new_position.y, g_seaweed_thickness);

			vec3 section = new_position - lower_link_position;

			float l = g_seaweed_link_length;

			// Do a fixed-point iteration to reset the section length
			float r = 2 * l * l / (dot(section, section) + l * l);
			vec3 new_section = section * r;
			vec3 normalized_position = lower_link_position + new_section;

#ifdef _DEBUG
			if (normalized_position != normalized_position)// || length(section) < g_seaweed_link_length / 2)
				VDebugBreak();
#endif

			g_seaweed[k].m_positions[last_list][n] = normalized_position; // last_list will soon become the current, so write the new value into there.
		}
	}
	g_seaweed_current_list = !g_seaweed_current_list;
}


void RenderSeaweed()
{
	if (!DO_SEAWEED)
		return;

	CRenderingContext c(Game()->GetRenderer(), true);

	Vector camera = Game()->m_hPlayer->GetGlobalView();

	c.UseProgram("model");

	c.SetUniform("vecColor", Vector4D(0, 0, 0, 1));

	mat4 scale;
	scale.SetScale(vec3(g_seaweed_thickness, g_seaweed_thickness, g_seaweed_thickness)*1.5f);

	for (int k = 0; k < sizeof(g_seaweed) / sizeof(g_seaweed[0]); k++)
	{
		vec3 centroid = (g_seaweed[k].m_positions[g_seaweed_current_list][0] + g_seaweed[k].m_positions[g_seaweed_current_list][SEAWEED_LINKS - 1]) * 0.5f;
		float distance_to_camera = length(centroid - Game()->m_hPlayer->GetGlobalOrigin());
		//if (distance_to_camera > 3 && dot((centroid - Game()->m_hPlayer->GetGlobalOrigin()) / distance_to_camera, camera) < 0.5f)
		//	continue;

		//if (distance_to_camera > 30)
		//	continue;

		for (int n = 0; n < SEAWEED_LINKS; n++)
		{
			float offset = (float)((2 * n) % 5) * (float)M_TAU / 5;
			vec3 offset_vector = vec3(cos(offset), sin(offset), 0.0)*g_seaweed_thickness;

			Matrix4x4 m;
			m.SetTranslation(g_seaweed[k].m_positions[g_seaweed_current_list][n] + offset_vector);
			c.LoadTransform(m);
			c.RenderBox(Vector(-0.1f, -0.1f, -0.1f), Vector(0.1f, 0.1f, 0.1f));
		}
	}

	for (int k = 0; k < sizeof(g_seaweed) / sizeof(g_seaweed[0]); k++)
	{
		vec3 centroid = (g_seaweed[k].m_positions[g_seaweed_current_list][0] + g_seaweed[k].m_positions[g_seaweed_current_list][SEAWEED_LINKS - 1]) * 0.5f;
		float distance_to_camera = length(centroid - Game()->m_hPlayer->GetGlobalOrigin());
		//if (distance_to_camera > 3 && dot((centroid - Game()->m_hPlayer->GetGlobalOrigin()) / distance_to_camera, camera) < 0.5f)
		//	continue;

		//if (distance_to_camera > 30)
		//	continue;

		c.ResetTransformations();
		for (int n = 0; n < SEAWEED_LINKS - 1; n++)
		{
			//c.LoadTransform(m2);
//			c.RenderBox(Vector(-1, -1, -1), Vector(1, 1, 1));
			c.BeginRenderLines();
				c.Vertex(g_seaweed[k].m_positions[g_seaweed_current_list][n]);
				c.Vertex(g_seaweed[k].m_positions[g_seaweed_current_list][n + 1]);
			c.EndRender();
		}
	}
}

