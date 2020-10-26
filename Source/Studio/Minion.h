#pragma once

class Minion
{
public:
	Minion();
	~Minion();

public:
	void Update(uint32_t eventID, float elapsedTime, float processTime);

private:
	est::gameobject::ActorPtr m_pActor;

	std::wstring m_path;

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