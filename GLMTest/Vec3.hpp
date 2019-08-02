#pragma once
#include "Thirdparty/glm/glm/vec3.hpp"

namespace cw
{
	class Vec3 : public glm::vec3
	{
	public:
		Vec3(int x, int y, int z);
		virtual ~Vec3();

		//Vec3 v1(int x, int y ,int z);
	};

}

