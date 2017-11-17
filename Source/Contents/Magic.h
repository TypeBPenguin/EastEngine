#pragma once

namespace Contents
{
	namespace EmMagic
	{
		enum Type
		{
			ePad = 0,
			eEmissive,
			eLaunch,
			eMaintain,
		};
	}

	class Magic
	{
	public:
		Magic();
		~Magic();
	};
}