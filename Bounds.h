#ifndef BOUNDS_H
#define BOUNDS_H

#include <glm/glm.hpp>


enum class BoundTypes
{
	AABB,	// Axis-Aligned Bounding Box
	SPHERE	// Sphere
};


class BoundingRegion
{
public:

	// BoundTypes
	BoundTypes type;

	// Sphere Values
	glm::vec3 center;
	float radius;

	// Bounding Box Values
	glm::vec3 min;
	glm::vec3 max;

	// Constructors
	// --------------------

	// initialize with type
	BoundingRegion(BoundTypes type = BoundTypes::AABB);

	// initialize as sphere
	BoundingRegion(glm::vec3 center, float radius);

	// initialize as AABB (Axis-Aligned Bounding Box)
	BoundingRegion(glm::vec3 min, glm::vec3 max);



	// Calculating values for each region
	// --------------------
	
	// Center
	glm::vec3 calculateCenter();

	// Calculate Dimensions
	glm::vec3 calculateDimensions();


	// Testing Methods
	// --------------------

	// Determine if point is inside
	bool containsPoint(glm::vec3 point);

	// Determine if region is COMPLETELY inside
	bool containsRegion(BoundingRegion br);

	// Determing if region intersects (partial containment)
	bool intersectsWith(BoundingRegion br);




private:

};



#endif 