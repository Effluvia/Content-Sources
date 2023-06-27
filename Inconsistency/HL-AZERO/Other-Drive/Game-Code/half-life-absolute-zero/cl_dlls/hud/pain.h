

#ifndef PAIN_H
#define PAIN_H

#include "hudbase.h"

class CHudPain : public CHudBase
{
public:

	SpriteHandle_t m_painFlashSprite;
	//SpriteHandle_t m_hDamage;  ???

	float m_fAttackFront, m_fAttackRear, m_fAttackLeft, m_fAttackRight;
	float m_fAttackFrontDamage, m_fAttackRearDamage, m_fAttackLeftDamage, m_fAttackRightDamage;



	//MODDD - constructor given.
	CHudPain(void);

	virtual int Init(void);
	virtual int VidInit(void);
	virtual int Draw(float fTime);
	virtual void Reset(void);


	float cumulativeFadeDrown;
	float cumulativeFade;
	float fAttackFrontMem;
	float fAttackRearMem;
	float fAttackRightMem;
	float fAttackLeftMem;
	BOOL playerIsDrowning;




	int DrawPain(float fTime);
	//MODDD - now accepts the damage done too.
	void CalcDamageDirection(vec3_t vecFrom, int damageAmount, int rawDamageAmount);
	//MODDD - new
	void setUniformDamage(float damageAmount);
	void getPainColorMode(int mode, int& r, int& g, int& b);

};

#endif //PAIN_H