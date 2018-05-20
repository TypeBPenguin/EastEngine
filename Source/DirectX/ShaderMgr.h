#pragma once

#include "CommonLib/Singleton.h"

#include "EffectInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class ShaderManager : public Singleton<ShaderManager>
		{
			friend Singleton<ShaderManager>;
		private:
			ShaderManager();
			virtual ~ShaderManager();

		public:
			void Flush();

		public:
			IEffect* GetEffect(const String::StringID& strName);
			bool AddEffect(IEffect* pEffect);
			void RemoveEffect(IEffect* pEffect);

			void CompleteEffectAsyncLoad(IEffect* pEffect, bool isSuccess, std::function<void(IEffect*, bool)> funcCallback);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}