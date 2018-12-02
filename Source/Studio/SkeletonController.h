#pragma once

#include "Model/ModelInterface.h"

struct ImDrawList;

namespace eastengine
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

		eastengine::graphics::IVertexBuffer* pVertexBuffer = nullptr;
		eastengine::graphics::IIndexBuffer* pIndexBuffer = nullptr;
		eastengine::physics::RigidBody* pRigidBody = nullptr;

		eastengine::math::Matrix matTransform;
	};

	std::array<std::array<Controller, Axis::AxisCount>, Type::TypeCount> m_controllers;
	eastengine::graphics::ISkeletonInstance::IBone* m_pSelectedBone;

	bool m_isBoneMoveMode;
	bool m_isShowControllerUI;
};