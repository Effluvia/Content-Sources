#include "c_basehlcombatweapon.h"

class C_Weapon870 : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_Weapon870, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

#ifdef HOE_THIRDPERSON
	DECLARE_ACTTABLE();	
#endif // HOE_THIRDPERSON

	C_Weapon870();

#define MAX_CLIP 10				// the actual clip size is in scripts/weapon_870.txt
	unsigned char m_AmmoQueue[MAX_CLIP];	// Q_xxx ammo that is loaded
};