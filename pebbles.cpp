#define _USE_MATH_DEFINES

// Header
#include "pebbles.hpp"
#include "common.hpp"

#include <cmath>
#include <iostream>

#define randf() (float(rand()) / float(RAND_MAX))

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

static const int MAX_PEBBLES = 25;
constexpr int NUM_SEGMENTS = 12;
static const float GRAVITY = -1.2f;


bool Pebbles::init()
{
	std::vector<GLfloat> screen_vertex_buffer_data;
	constexpr float z = -0.1;

	for (int i = 0; i < NUM_SEGMENTS; i++) {
		screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(z);

		screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(z);

		screen_vertex_buffer_data.push_back(0);
		screen_vertex_buffer_data.push_back(0);
		screen_vertex_buffer_data.push_back(z);

	}

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, screen_vertex_buffer_data.size()*sizeof(GLfloat), screen_vertex_buffer_data.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m_instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("pebble.vs.glsl"), shader_path("pebble.fs.glsl")))
		return false;

	return true;
}

// Releases all graphics resources
void Pebbles::destroy() {
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &m_instance_vbo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);

	m_pebbles.clear();
}

void Pebbles::update(float ms) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE UPDATES HERE
	// You will need to handle both the motion of pebbles
	// and the removal of dead pebbles.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	std::vector<int> dead_pebbles;

	for (int i = 0; i < m_pebbles.size(); i++) {
		//m_pebbles[i].velocity =
		//	add(m_pebbles[i].velocity, mul(GRAVITY, ms/2));
				m_pebbles[i].velocity.y -= GRAVITY*ms/10000;
		m_pebbles[i].position =
			add(m_pebbles[i].position, mul(m_pebbles[i].velocity, ms));
 // multiply by radius?
		m_pebbles[i].life -= 0.5f;
		if (m_pebbles[i].life < 0.0f) {
			dead_pebbles.push_back(i);
		}
	}

	for (int i = 0; i < dead_pebbles.size(); i++) {
		m_pebbles[dead_pebbles[i]] = m_pebbles.back();
		m_pebbles.pop_back();
	}
}

void Pebbles::spawn_pebble(vec2 position)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE SPAWNING HERE
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Pebble pebble;

	if (m_pebbles.size() < MAX_PEBBLES) {
		float rand_velx = -1.f + (rand() / (RAND_MAX / 2.f));
		float rand_vely = -1.f + (rand() / (RAND_MAX / 2.f));

		pebble.life = 100.f;
		pebble.position = position;
		pebble.velocity = {rand_velx, rand_vely};
		pebble.radius = 10.f + (rand() / (RAND_MAX / 10.f));
		m_pebbles.emplace_back(pebble);
	}
}

void Pebbles::collides_with()
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE COLLISIONS HERE
	// You will need to write additional functions from scratch.
	// Make sure to handle both collisions between pebbles
	// and collisions between pebbles and salmon/fish/turtles.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (size_t i = 1; i < m_pebbles.size(); i++) {
		vec2 p1 = m_pebbles[i].position;
		vec2 p2 = m_pebbles[i-1].position;
		vec2 new_vel1 = {0, 0};
		vec2 new_vel2 = {0, 0};

		if (isCollision(m_pebbles[i], m_pebbles[i-1])) {

					vec2 v1 = m_pebbles[i].velocity;
					vec2 v2 = m_pebbles[i-1].velocity;

					float mass1 = m_pebbles[i].radius;
					float mass2 = m_pebbles[i-1].radius;

					float mass_var1 = (2*mass2)/(mass1 + mass2);
					float mass_var2 = (2*mass1)/(mass1 + mass2);

					vec2 vel_var1 = sub(v1, v2);
					vec2 vel_var2 = sub(v2, v1);

					float pos_sqr1 = len(sub(p1, p2)) * len(sub(p1, p2));
					float pos_sqr2 = len(sub(p2, p1)) * len(sub(p2, p1));

					vec2 pos1 = sub(p1, p2);
					vec2 pos2 = sub(p2, p1);

					float dot1 = dot(pos1, vel_var1);
					float dot2 = dot(pos2, vel_var2);

					float div1 = dot1 / pos_sqr1;
					float div2 = dot2 / pos_sqr2;

					float mult1 = mass_var1 * div1;
					float mult2 = mass_var2 * div2;

					vec2 final1 = mul(pos1, mult1);
					vec2 final2 = mul(pos2, mult2);

					new_vel1 = sub(v1, final1);
					new_vel2 = sub(v2, final2);

					m_pebbles[i].velocity = new_vel1;
					m_pebbles[i-1].velocity = new_vel2;
		}
	}
}

bool Pebbles::isCollision (Pebble &a, Pebble &b) {
	float dist = (b.position.x - a.position.x) * (b.position.x - a.position.x) +
								(a.position.y - b.position.y) * (a.position.y - b.position.y);
	float rad = (a.radius + b.radius) * (a.radius + b.radius);
	if (dist <= rad) {
		return true;
	}
	return false;
}

// Draw pebbles using instancing
void Pebbles::draw(const mat3& projection)
{
	// Setting shaders
	glUseProgram(effect.program);

  	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint color_uloc = glGetUniformLocation(effect.program, "color");

	// Pebble color
	float color[] = { 0.4f, 0.4f, 0.4f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Draw the screen texture on the geometry
	// Setting vertices
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Mesh vertex positions
	// Bind to attribute 0 (in_position) as in the vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(0, 0);

	// Load up pebbles into buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_pebbles.size() * sizeof(Pebble), m_pebbles.data(), GL_DYNAMIC_DRAW);

	// Pebble translations
	// Bind to attribute 1 (in_translate) as in vertex shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Pebble), (GLvoid*)offsetof(Pebble, position));
	glVertexAttribDivisor(1, 1);

	// Pebble radii
	// Bind to attribute 2 (in_scale) as in vertex shader
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Pebble), (GLvoid*)offsetof(Pebble, radius));
	glVertexAttribDivisor(2, 1);

	// Draw using instancing
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawArraysInstanced.xhtml
	glDrawArraysInstanced(GL_TRIANGLES, 0, NUM_SEGMENTS*3, m_pebbles.size());

  	// Reset divisor
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
}
