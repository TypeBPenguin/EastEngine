#pragma once

namespace EastEngine
{
	namespace GameObject
	{
		enum EmObjectType
		{
			eActor = 0,
			eTerrain,
			eSky,
			eCloud,
		};

		class IGameObject
		{
		public:
			IGameObject();
			virtual ~IGameObject() = 0;

		public:
			virtual EmObjectType GetType() const = 0;
		};
	}
}