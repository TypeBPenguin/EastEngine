#pragma once

#include "Model/ModelInterface.h"

struct ImDrawList;

namespace est
{
	namespace physics
	{
		class RigidBody;
	}
}

class SkeletonController
{
public:
	SkeletonController();
	~SkeletonController();

public:
	bool Process(float elapsedTime);
	void RenderUI();

private:
	enum Axis
	{
		eX = 0,
		eY,
		eZ,
		AxisCount,
	};

	enum Type
	{
		ePosition = 0,
		eScale,
		eRotation,
		TypeCount,
	};

	struct Controller
	{
		Axis emAxis = Axis::eX;
		Type emType = Type::ePosition;

		est::graphics::IVertexBuffer* pVertexBuffer = nullptr;
		est::graphics::IIndexBuffer* pIndexBuffer = nullptr;
		est::physics::RigidBody* pRigidBody = nullptr;

		est::math::Matrix matTransform;
	};

	std::array<std::array<Controller, Axis::AxisCount>, Type::TypeCount> m_controllers;
	est::graphics::ISkeletonInstance::IBone* m_pSelectedBone;

	bool m_isBoneMoveMode;
	bool m_isShowControllerUI;
};