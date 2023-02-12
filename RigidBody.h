#ifndef RIGIDBODY_H
#define	RIGIDBODY_H

#include <glm/glm.hpp>

#include "Environment.h"

class RigidBody {
public:
	// Rigidbody Data
	float mass;

	glm::vec3 pos;
	glm::vec3 velocity;
	glm::vec3 acceleration;

	// Constructors
	RigidBody(float mass = 1.0f, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 velocity = glm::vec3(0.0f), glm::vec3 acceleration = glm::vec3(0.0f));

	// Public Functions
	void update(float dt);

	void reset();

	void applyForce(glm::vec3 force);
	void applyForce(glm::vec3 direction, float magnitude);

	void applyAcceleration(glm::vec3 acceleration);
	void applyAcceleration(glm::vec3 direction, float magnitude);

	void applyImpulse(glm::vec3 force, float dt);
	void applyImpulse(glm::vec3 direction, float magnitude, float dt);

	void transferEnergy(float joules);

};

#endif RIGIDBODY_H

