#pragma once

namespace est
{
	namespace physics
	{
		struct Initializer
		{
			math::float3 Gravity{ 0.f, -9.80665f, 0.f };
		};

		enum ForceMode
		{
			eForce,				// parameter has unit of mass * distance/ time^2, i.e. a force
			eImpulse,			// parameter has unit of mass * distance /time
			eVelocityChange,	// parameter has unit of distance / time, i.e. the effect is mass independent: a velocity change.
			eAcceleration		// parameter has unit of distance/ time^2, i.e. an acceleration. It gets treated just like a force except the mass is not divided out before integration.
		};

		struct FilterData
		{
			uint32_t word0{ 0 };
			uint32_t word1{ 0 };
			uint32_t word2{ 0 };
			uint32_t word3{ 0 };
		};

		struct Bounds
		{
			math::float3 minimum;
			math::float3 maximum;
		};

		struct Transform
		{
			math::Quaternion rotation;
			math::float3 position;
		};

		struct HitLocation
		{
			math::float3 position;
			math::float3 normal;
			float distance{ 0.f };
		};

		struct TolerancesScale
		{
			float length{ 1.f };
			float speed{ 10.f };
		};

		struct Spring
		{
			float stiffness{ 0.f };
			float damping{ 0.f };

			Spring(float stiffness, float damping)
				: stiffness(stiffness)
				, damping(damping)
			{
			}
		};

		struct JointLimitParameters
		{
			float restitution{ 0.f };		// [0 ~ 1]
			float bounceThreshold{ 0.f };
			float stiffness{ 0.f };			// [0, MAX_FLOAT]
			float damping{ 0.f };			// [0, MAX_FLOAT]
			float contactDistance{ 0.f };
		};

		struct JointLinearLimit : public JointLimitParameters
		{
			float value{ std::numeric_limits<float>::max() };	// [0 ~ MAX_FLOAT]

			// construct a linear hard limit
			JointLinearLimit(const TolerancesScale& scale, float extent, float contactDist = -1.f)
				: value(extent)
			{
				contactDistance = math::IsEqual(contactDist, -1.f) ? 0.01f * scale.length : contactDist;
			}

			// construct a linear soft limit 
			JointLinearLimit(float extent, const Spring& spring)
				: value(extent)
			{
				stiffness = spring.stiffness;
				damping = spring.damping;
			}
		};

		struct JointLinearLimitPair : public JointLimitParameters
		{
			float lower{ -std::numeric_limits<float>::max() / 3.f };
			float upper{ std::numeric_limits<float>::max() / 3.f };

			JointLinearLimitPair(const TolerancesScale& scale, float lowerLimit = -std::numeric_limits<float>::max() / 3.f, float upperLimit = std::numeric_limits<float>::max() / 3.f, float contactDist = -1.f)
				: lower(lowerLimit)
				, upper(upperLimit)
			{
				contactDistance = math::IsEqual(contactDist, -1.f) ? std::min(scale.length * 0.01f, (upperLimit * 0.49f - lowerLimit * 0.49f)) : contactDist;
			}

			JointLinearLimitPair(float lowerLimit, float upperLimit, const Spring& spring)
				: lower(lowerLimit)
				, upper(upperLimit)
			{
				stiffness = spring.stiffness;
				damping = spring.damping;
			}
		};

		struct JointAngularLimitPair : public JointLimitParameters
		{
			float lower{ -math::PIDIV2 };
			float upper{ math::PIDIV2 };

			JointAngularLimitPair(float lowerLimit, float upperLimit, float contactDist = -1.f)
				: lower(lowerLimit)
				, upper(upperLimit)
			{
				contactDistance = math::IsEqual(contactDist, -1.f) ? std::min(0.1f, 0.49f * (upperLimit - lowerLimit)) : contactDist;
				bounceThreshold = 0.5f;
			}

			JointAngularLimitPair(float lowerLimit, float upperLimit, const Spring& spring)
				: lower(lowerLimit)
				, upper(upperLimit)
			{
				stiffness = spring.stiffness;
				damping = spring.damping;
			}
		};

		struct JointLimitCone : public JointLimitParameters
		{
			float yAngle{ math::PIDIV2 };	// [0 ~ PI]
			float zAngle{ math::PIDIV2 };	// [0 ~ PI]

			JointLimitCone(float yLimitAngle, float zLimitAngle, float contactDist = -1.f)
				: yAngle(yLimitAngle)
				, zAngle(zLimitAngle)
			{
				contactDistance = math::IsEqual(contactDist, -1.f) ? std::min(0.1f, std::min(yLimitAngle, zLimitAngle) * 0.49f) : contactDist;
				bounceThreshold = 0.5f;
			}

			JointLimitCone(float yLimitAngle, float zLimitAngle, const Spring& spring)
				: yAngle(yLimitAngle)
				, zAngle(zLimitAngle)
			{
				stiffness = spring.stiffness;
				damping = spring.damping;
			}
		};

		struct JointLimitPyramid : public JointLimitParameters
		{
			float yAngleMin{ -math::PIDIV2 };	// [-PI ~ PI]
			float yAngleMax{ math::PIDIV2 };	// [-PI ~ PI]

			float zAngleMin{ -math::PIDIV2 };	// [-PI ~ PI]
			float zAngleMax{ math::PIDIV2 };	// [-PI ~ PI]

			JointLimitPyramid(float yLimitAngleMin, float yLimitAngleMax, float zLimitAngleMin, float zLimitAngleMax, float contactDist = -1.f)
				: yAngleMin(yLimitAngleMin)
				, yAngleMax(yLimitAngleMax)
				, zAngleMin(zLimitAngleMin)
				, zAngleMax(zLimitAngleMax)
			{
				if (math::IsEqual(contactDist, -1.f) == true)
				{
					const float contactDistY = std::min(0.1f, 0.49f * (yLimitAngleMax - yLimitAngleMin));
					const float contactDistZ = std::min(0.1f, 0.49f * (zLimitAngleMax - zLimitAngleMin));
					contactDistance = contactDist == std::min(contactDistY, contactDistZ);
				}
				else
				{
					contactDistance = contactDist;
				}

				bounceThreshold = 0.5f;
			}

			JointLimitPyramid(float yLimitAngleMin, float yLimitAngleMax, float zLimitAngleMin, float zLimitAngleMax, const Spring& spring)
				: yAngleMin(yLimitAngleMin)
				, yAngleMax(yLimitAngleMax)
				, zAngleMin(zLimitAngleMin)
				, zAngleMax(zLimitAngleMax)
			{
				stiffness = spring.stiffness;
				damping = spring.damping;
			}
		};
	}
}