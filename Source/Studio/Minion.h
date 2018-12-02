#pragma once

namespace eastengine
{
	namespace gameobject
	{
		class IActor;
	}

	namespace string
	{
		class StringID;
	}
}

class Minion
{
public:
	Minion();
	~Minion();

public:
	void Update(uint32_t eventID, float elapsedTime, float processTime);

private:
	eastengine::gameobject::IActor* m_pActor{ nullptr };

	std::string m_path;

	enum AttackType
	{
		eNormal = 0,
		eSetA,
		eSetB,
	};
	AttackType m_emAttackType{ AttackType::eNormal };

	enum AttackStep
	{
		eStep0,
		eStep1,
		eStep2,
		eStep3,
		eStep4,
	};
	AttackStep m_emAttackStep{ eStep4 };
	AttackStep m_emMaxAttackStep{ eStep0 };
	bool m_isRequestAttack{ false };
};