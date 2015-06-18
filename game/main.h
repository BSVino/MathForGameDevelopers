#pragma once

double g_current_time = 0;
double g_frame_time = 1/30.0f;
Vector g_gravity = Vector(0, 0, -9.8);

float RandFloat(float min, float max)
{
	return ((float)mtrand()) * (max-min) /MTRAND_MAX + min;
}

struct Particle
{
	Vector m_position;
	Vector m_velocity;
	bool   m_gravity;
};

struct ParticleSystem
{
	vector<Particle> m_particles;
	int m_first_with_gravity;

	void Initialize()
	{
		m_particles.reserve(1024*10);
	}

	void SpawnParticle()
	{
		m_particles.push_back(Particle());
		Particle* new_particle = &m_particles[m_particles.size()-1];

		new_particle->m_position.x = 0;
		new_particle->m_position.y = 0;
		new_particle->m_position.z = 0;

		new_particle->m_velocity.x = RandFloat(-1, 1);
		new_particle->m_velocity.y = RandFloat(-1, 1);
		new_particle->m_velocity.z = 1;
	}

	void Update()
	{
		int particles = m_particles.size();

		for (size_t i = 0; i < particles; i++)
		{
			Particle* p = &m_particles[i];

			p->m_position = p->m_position + p->m_velocity * g_frame_time;
			if (p->m_gravity)
				p->m_velocity = p->m_velocity + g_gravity;
		}
	}
};

