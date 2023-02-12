#include "Bounds.h"

#include <iostream>
using namespace std;


// Constructors
// --------------------

// initialize with type
BoundingRegion::BoundingRegion(BoundTypes type)
	: type(type)
{

}

// initialize as sphere
BoundingRegion::BoundingRegion(glm::vec3 center, float radius)
	: type(BoundTypes::SPHERE), center(center), radius(radius)
{

}

// initialize as AABB (Axis-Aligned Bounding Box)
BoundingRegion::BoundingRegion(glm::vec3 min, glm::vec3 max)
	: type(BoundTypes::AABB), min(min), max(max)
{

}



// Calculating values for each region
// --------------------

// Center
glm::vec3 BoundingRegion::calculateCenter()
{
	if (type == BoundTypes::AABB)
	{
		return (min + max) / 2.0f;	// If AABB, Return the average between the min and max values
	}
	if (type == BoundTypes::SPHERE)
	{
		return center;				// If SPHERE, Return the center of the sphere
	}
}

// Calculate Dimensions
glm::vec3 BoundingRegion::calculateDimensions()
{
	if (type == BoundTypes::AABB)
	{
		return (max - min);					// If AABB, Return the difference between the min and max values
	}
	if (type == BoundTypes::SPHERE)
	{
		return glm::vec3(2.0f * radius);	// If SPHERE, Return a box surrounding the sphere (for simpler implementation)
	}
}


// Testing Methods
// --------------------

// Determine if point is inside
bool BoundingRegion::containsPoint(glm::vec3 point)
{
	// Check for AABB
	if (type == BoundTypes::AABB)	
	{
		// box --> point must be larget than the min and smaller than the max value in all 3 dimensions
		return (point.x >= min.x) && (point.x <= max.x) &&
				(point.y >= min.y) && (point.y <= max.y) &&
				(point.z >= min.z) && (point.z <= max.z);		
	}
	// Check for SPHERE
	else
	{
		// sphere --> distance must be less than the radius
		// x^2 + y^2 + z^2 <= r^2
		float distanceSquared = 0.0f;
		for (int i = 0; i < 3; i++)
		{
			distanceSquared += (center[i] - point[i]) * (center[i] - point[i]);
		}
		return distanceSquared <= (radius * radius);
	}
}

// Determine if region is COMPLETELY inside
bool BoundingRegion::containsRegion(BoundingRegion br)
{
	// br is AABB, the other doesn't matter if it's AABB or SPHERE
	if (br.type == BoundTypes::AABB)
	{
		// if br is a box, just has to contain min and max
		return containsPoint(br.min) && containsPoint(br.max);
	}
	// Both SPHERE
	else if (type == BoundTypes::SPHERE && br.type == BoundTypes::SPHERE)
	{
		// if both Spheres, combination of distance from centers and br.radius is less than radius
		return glm::length(center - br.center) + br.radius < radius;
	}
	// br is a SPHERE and the other is an AABB
	else
	{
		// center is outside of the box
		if (!containsPoint(br.center))
		{
			return false;
		}

		// center is inside the box
		/*
			for each axis (x, y, z)
			--> if distance to each side is smaller than the radius, return false
		*/
		for (int i = 0; i < 3; i++)
		{
			if (abs(max[i] - br.center[i]) < br.radius ||
				abs(br.center[i] - min[i]) < br.radius)
			{
				return false;
			}
		}

		// If it passed all the checks, br is COMPLETELY inside the other object,
		// so it returns true
		return true;
	}
}

// Determing if region intersects (partial containment)
bool BoundingRegion::intersectsWith(BoundingRegion br)
{
	/*
	In all cases, we will check for overlap on all axes
	*/
	
	
	// Both AABB
	if (type == BoundTypes::AABB && br.type == BoundTypes::AABB)
	{
		glm::vec3 rad = calculateDimensions() / 2.0f;			// "radius" of this box
		glm::vec3 radBr = br.calculateDimensions() / 2.0f;		// "radius" of br

		glm::vec3 center = calculateCenter();					// center of this box
		glm::vec3 centerBr = br.calculateCenter();				// center of br

		glm::vec3 distance = abs(center - centerBr);

		for (int i = 0; i < 3; i++)
		{	
			if (distance[i] > rad[i] + radBr[i])
			{
				// no overlap on this axis
				return false;
			}	
		}
		
		// If it passed the check for all axes, returns true
		return true;
	}
	// Both SPHERE
	else if (type == BoundTypes::SPHERE && br.type == BoundTypes::SPHERE)
	{
		// if both are SPHERE, the distance between their centers must be less than their combined radius'
		return glm::length(center - br.center) < (radius + br.radius);
	}
	// br is an AABB, the other is a SPHERE
	else if (type == BoundTypes::SPHERE && br.type == BoundTypes::AABB)
	{
		// determine if SPHERE is above the top, below bottom, etc.
		// find distance (squared) to the closest plane
		float distanceSquared = 0.0f;
		for (int i = 0; i < 3; i++) {
			// determine closest side
			float closestPt = std::max(br.min[i], std::min(center[i], br.max[i]));
			// add distance
			distanceSquared += (closestPt - center[i]) * (closestPt - center[i]);
		}

		// if lower than min or higher than max, the distanceSquared is gonna be > radius^2, so it returns false.
		// if not the above checks, then one is inside the other, so < radius^2, so it returns true.
		return distanceSquared < (radius * radius);

	}
	// br is a SPHERE, the other is an AABB
	// call algorithm for br (defined in preceding else if block)
	else
	{
		//br.intersectsWith(*this);


		float distanceSquared = 0.0f;
		for (int i = 0; i < 3; i++)
		{
			// lower than min
			if (br.center[i] < min[i])
			{
				distanceSquared += (min[i] - br.center[i]) * (min[i] - br.center[i]);
			}
			// higher than max
			else if (br.center[i] > max[i])
			{
				distanceSquared += (br.center[i] - max[i]) * (br.center[i] - max[i]);
			}
			// else inside

		}

		// if lower than min or higher than max, the distanceSquared is gonna be > radius^2, so it returns false.
		// if not the above checks, then one is inside the other, so < radius^2, so it returns true.
		return distanceSquared < (br.radius * br.radius);
	}
}