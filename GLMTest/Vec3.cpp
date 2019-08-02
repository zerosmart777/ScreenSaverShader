#include "pch.h"
#include "Vec3.hpp"

namespace cw
{
	Vec3::Vec3(int x,int y , int z) : glm::vec3(x, y, z) 
	{	
	}
	/*
	Vec3::Vec3(int x,int y , int z)
	{
	 //easy implement but cost 3 cpu cycles.
		this->x = x;
		this->y = y;
		this->z = z;
		
	}
	*/

	Vec3::~Vec3()
	{
	}
	
}
