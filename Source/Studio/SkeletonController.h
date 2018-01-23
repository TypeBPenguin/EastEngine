#pragma once

#include "Model/ModelInterface.h"

struct ImDrawList;

namespace EastEngine
{
	namespace Physics
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
	bool Process(float fElapsedTime);
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

		EastEngine::Graphics::IVertexBuffer* pVertexBuffer = nullptr;
		EastEngine::Graphics::IIndexBuffer* pIndexBuffer = nullptr;
		EastEngine::Physics::RigidBody* pRigidBody = nullptr;

		EastEngine::Math::Matrix matTransform;
	};

	std::array<std::array<Controller, Axis::AxisCount>, Type::TypeCount> m_controllers;
	EastEngine::Graphics::ISkeletonInstance::IBone* m_pSelectedBone;

	bool m_isBoneMoveMode;
	bool m_isShowControllerUI;
};