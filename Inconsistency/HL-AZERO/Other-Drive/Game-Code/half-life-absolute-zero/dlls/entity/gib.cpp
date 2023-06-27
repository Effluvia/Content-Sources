

#include "extdll.h"

#include "gib.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "soundent.h"
#include "decals.h"
#include "util_model.h"
#include "func_break.h"
#include "game.h"
#include "player.h"

EASY_CVAR_EXTERN_DEBUGONLY(cheat_iwantguts)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)

extern float globalPSEUDO_canApplyGermanCensorship;
extern BOOL globalPSEUDO_germanModel_hgibFound;



void EstablishGutLoverGib(CGib* pGib, entvars_t* pevVictim, entvars_t* pevPlayer, BOOL isHead) {
	if (!pevVictim)return;
	EstablishGutLoverGib(pGib, pevVictim, pevVictim->origin + pevVictim->view_ofs, pevPlayer, isHead);
}

void EstablishGutLoverGib(CGib* pGib, entvars_t* pevVictim, const Vector gibSpawnOrigin, entvars_t* pevPlayer, BOOL isHead) {
	//can't do this without the victim.
	if (!pevVictim)return;

	if (isHead == TRUE) {
		pGib->pev->origin = gibSpawnOrigin;

	}
	else {
		// spawn the gib somewhere in the monster's bounding volume
		pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT(0, 1));
		pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT(0, 1));
		pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT(0, 1)) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

	}

	Vector dest;
	if (isHead == TRUE) {
		dest = pevPlayer->origin + pevPlayer->view_ofs + Vector(RANDOM_FLOAT(-5, 5), RANDOM_FLOAT(-5, 5), RANDOM_FLOAT(-5, 1));
	}
	else {
		dest = pevPlayer->origin + pevPlayer->view_ofs + Vector(RANDOM_FLOAT(-36, 36), RANDOM_FLOAT(-36, 36), RANDOM_FLOAT(-9, 2));
	}

	Vector distVector = (dest)-pGib->pev->origin;

	Vector distVector2D = Vector(distVector.x, distVector.y, 0);

	Vector towardsPlayer = distVector.Normalize();
	Vector towardsPlayer2D = distVector2D.Normalize();

	float distFloorwise = distVector.Length2D();
	float distVertical = distVector.z;

	//angle...
	/*
	float ang = 0;
	if(distVertical ==0){
		ang = 90.0f *(M_PI / 180.0f);
	}else{
		ang = atan(distVertical / distFloorwise);
	}
	*/

	//velocity must be at least X.
	float velocitySpeed = max(400, distFloorwise);

	if (isHead) {
		pGib->pev->origin = pGib->pev->origin + (towardsPlayer) * 8;

		//little faster.
		velocitySpeed *= 1.11f;
	}


	float timeToReachDest = distFloorwise / velocitySpeed;

	//grav?   sv_grav?


	float gravity = g_psv_gravity->value;
	//easyForcePrintLine("???GGG %.2f", gravity);
	Vector velocityFinal = towardsPlayer2D * velocitySpeed;
	float velocityVertical = (distVertical + 0.5 * gravity * pow(timeToReachDest, 2.0f)) / (timeToReachDest);

	//easyForcePrintLine("WHYYYYYY %.2f::%.2f %.2f %.2f   %.2f %.2f", velocityVertical, distVertical, gravity, timeToReachDest, towardsPlayer.x, towardsPlayer.y);

	velocityFinal.z = velocityVertical * 1.0f;
	velocityFinal.x *= 1.0;
	velocityFinal.y *= 1.0;

	//pGib->pev->velocity.z += 100;


	//Vector tempp = towardsPlayer2D * velocity;

	pGib->pev->velocity = velocityFinal;
}





void CGib::Spawn(const char* szGibModel)
{
	CGib::Spawn(szGibModel, TRUE);
}
//
// Throw a chunk
//
void CGib::Spawn(const char* szGibModel, BOOL spawnsDecal)
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->friction = 0.55; // deading the bounce a bit

	// sometimes an entity inherits the edict from a former piece of glass,
	// and will spawn using the same render FX or rendermode! bad!
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxNone;
	pev->solid = SOLID_SLIDEBOX;/// hopefully this will fix the VELOCITY TOO LOW crap
	pev->classname = MAKE_STRING("gib");

	SET_MODEL(ENT(pev), szGibModel);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink(&CGib::WaitTillLand);
	SetTouch(&CGib::BounceGibTouch);

	m_material = matNone;
	m_cBloodDecals = 5;// how many blood decals this gib can place (1 per bounce until none remain). 

	if (!spawnsDecal) {
		//none.
		m_cBloodDecals = 0;
	}

}



// HACKHACK -- The gib velocity equations don't work
void CGib::LimitVelocity(void)
{
	float length = pev->velocity.Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if (length > 1500.0)
		pev->velocity = pev->velocity.Normalize() * 1500;		// This should really be sv_maxvelocity * 0.75 or something
}

void CGib::SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs) {
	CGib::SpawnStickyGibs(pevVictim, vecOrigin, cGibs, TRUE);
}
void CGib::SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs, BOOL spawnsDecal)
{
	int i;

	//if ( g_Language == LANGUAGE_GERMAN )
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == 1)
	{
		// no sticky gibs in germany right now!
		//MODDD TODO - above comment found as-is.  Can re-enable and just use german gibs instead.
		//             ...it seems "SpawnStickyGibs" is never called as-is.  Huh.
		return;
	}

	for (i = 0; i < cGibs; i++)
	{
		CGib* pGib = GetClassPtr((CGib*)NULL);

		pGib->Spawn("models/stickygib.mdl", spawnsDecal);
		pGib->pev->body = RANDOM_LONG(0, 2);

		if (pevVictim) {

			pGib->pev->origin.x = vecOrigin.x;
			pGib->pev->origin.y = vecOrigin.y;
			pGib->pev->origin.z = vecOrigin.z;


			edict_t* pentPlayer;
			if (EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) && ((pentPlayer = FIND_CLIENT_IN_PVS(pGib->edict())) != NULL))
			{
				// 5% chance head will be thrown at player's face.
				entvars_t* pevPlayer;
				pevPlayer = VARS(pentPlayer);

				EstablishGutLoverGib(pGib, pevVictim, pevPlayer, FALSE);

			}
			else {
				//how it usually goes.
				pGib->pev->origin.x = vecOrigin.x + RANDOM_FLOAT(-3, 3);
				pGib->pev->origin.y = vecOrigin.y + RANDOM_FLOAT(-3, 3);
				pGib->pev->origin.z = vecOrigin.z + RANDOM_FLOAT(-3, 3);

				// make the gib fly away from the attack vector
				pGib->pev->velocity = g_vecAttackDir * -1;
				// mix in some noise

				pGib->pev->velocity.x += RANDOM_FLOAT(-0.15, 0.15);
				pGib->pev->velocity.y += RANDOM_FLOAT(-0.15, 0.15);
				pGib->pev->velocity.z += RANDOM_FLOAT(-0.15, 0.15);


				pGib->pev->velocity = pGib->pev->velocity * 900;
			}

			pGib->pev->avelocity.x = RANDOM_FLOAT(250, 400);
			pGib->pev->avelocity.y = RANDOM_FLOAT(250, 400);

			// copy owner's blood color
			// Woa there!   Make sure the result of ::Instance is non-null.  Zany shit.
			CBaseEntity* tempInst = CBaseEntity::Instance(pevVictim);
			if (tempInst != NULL) {
				pGib->m_bloodColor = tempInst->BloodColor();
			}


			//DONT THROW OFF THE EMPEROR'S GROVE
			if (EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) < 1) {
				if (pevVictim->health > -50)
				{
					pGib->pev->velocity = pGib->pev->velocity * 0.7;
				}
				else if (pevVictim->health > -200)
				{
					pGib->pev->velocity = pGib->pev->velocity * 2;
				}
				else
				{
					pGib->pev->velocity = pGib->pev->velocity * 4;
				}
			}

			pGib->pev->movetype = MOVETYPE_TOSS;
			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
			pGib->SetTouch(&CGib::StickyGibTouch);
			pGib->SetThink(NULL);
		}
		pGib->LimitVelocity();
	}
}


void CGib::SpawnHeadGib(entvars_t* pevVictim)
{
	if (!pevVictim)return;
	SpawnHeadGib(pevVictim, pevVictim->origin + pevVictim->view_ofs, TRUE);
}
void CGib::SpawnHeadGib(entvars_t* pevVictim, BOOL spawnDecals)
{
	if (!pevVictim)return;
	SpawnHeadGib(pevVictim, pevVictim->origin + pevVictim->view_ofs, spawnDecals);
}
void CGib::SpawnHeadGib(entvars_t* pevVictim, const Vector gibSpawnOrigin)
{
	SpawnHeadGib(pevVictim, gibSpawnOrigin, TRUE);
}
void CGib::SpawnHeadGib(entvars_t* pevVictim, const Vector gibSpawnOrigin, BOOL spawnDecals)
{
	//MODDD - is this check ok?  Otherwise the spawned head gib doesn't get a location.
	if (!pevVictim)return;

	CGib* pGib = GetClassPtr((CGib*)NULL);

	//if ( g_Language == LANGUAGE_GERMAN )
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) == FALSE) {
		pGib->Spawn("models/hgibs.mdl", spawnDecals);// throw one head
		pGib->pev->body = 0;
	}
	else if (getGermanModelsAllowed() && globalPSEUDO_germanModel_hgibFound) {  //if(EASY_CVAR_GET(tryLoadGermanGibs) == 1){
	   //Just do this.
		pGib->Spawn(aryGibInfo[GIB_GERMAN_ID].modelPath, spawnDecals);
		pGib->pev->body = RANDOM_LONG(aryGibInfo[GIB_GERMAN_ID].bodyMin, aryGibInfo[GIB_GERMAN_ID].bodyMax);

	}
	else {
		//give up, no gib.
		return;
	}

	if (pevVictim)
	{
		pGib->pev->origin = gibSpawnOrigin;

		edict_t* pentPlayer = FIND_CLIENT_IN_PVS(pGib->edict());

		if ((RANDOM_LONG(0, 100) <= 5 || (EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) >= 1)) && pentPlayer)
		{
			// 5% chance head will be thrown at player's face.
			entvars_t* pevPlayer;
			pevPlayer = VARS(pentPlayer);

			if (EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) == 0) {
				//ordinary.
				pGib->pev->velocity = ((pevPlayer->origin + pevPlayer->view_ofs) - pGib->pev->origin).Normalize() * 300;
				pGib->pev->velocity.z += 100;
			}
			else {
				EstablishGutLoverGib(pGib, pevVictim, gibSpawnOrigin, pevPlayer, TRUE);

			}

		}
		else
		{
			pGib->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
		}

		pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
		pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

		// copy owner's blood color
		// Woa there!   Make sure the result of ::Instance is non-null.  Zany shit.
		CBaseEntity* tempInst = CBaseEntity::Instance(pevVictim);
		if (tempInst != NULL) {
			pGib->m_bloodColor = tempInst->BloodColor();
		}


		// DONT THROW OFF THE EMPEROR'S GROVE
		if (EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) < 1) {
			if (pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if (pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}
		}
	}
	pGib->LimitVelocity();
}



void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID) {
	//re-route to below to determine from the usual factors (violence CVar, is human, etc.)
	CGib::SpawnRandomGibs(pevVictim, cGibs, argSpawnGibID, TRUE);
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID, BOOL spawnDecals) {
	if (argSpawnGibID == GIB_DUMMY_ID) {
		//This is the dummy ID. Don't do anything, it is empty and signifies blocking a GIB call.
		return;
	}

	//SpawnRandomGibs(pevVictim, cGibs, aryGibInfo[argSpawnGibID], spawnDecals);
	const GibInfo_t& gibInfoChoice = aryGibInfo[argSpawnGibID];
	CGib::SpawnRandomGibs(pevVictim, cGibs, gibInfoChoice.modelPath, gibInfoChoice.bodyMin, gibInfoChoice.bodyMax, spawnDecals, gibInfoChoice.bloodColor);
}//END OF SpawnRandomGibs

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID, BOOL spawnDecals, int argBloodColor) {
	if (argSpawnGibID == GIB_DUMMY_ID) {
		//This is the dummy ID. Don't do anything, it is empty and signifies blocking a GIB call.
		return;
	}
	const GibInfo_t& gibInfoChoice = aryGibInfo[argSpawnGibID];
	//Since we provided a blood color, force this insetad of the one given by this gibInfo choice.
	CGib::SpawnRandomGibs(pevVictim, cGibs, gibInfoChoice.modelPath, gibInfoChoice.bodyMin, gibInfoChoice.bodyMax, spawnDecals, argBloodColor);
}


void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice) {
	CGib::SpawnRandomGibs(pevVictim, cGibs, gibInfoChoice.modelPath, gibInfoChoice.bodyMin, gibInfoChoice.bodyMax, TRUE, gibInfoChoice.bloodColor);
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice, BOOL spawnDecals) {
	//goes straight to the final method. This lets the user provide just a gibInfo reference to work off of.
	CGib::SpawnRandomGibs(pevVictim, cGibs, gibInfoChoice.modelPath, gibInfoChoice.bodyMin, gibInfoChoice.bodyMax, spawnDecals, gibInfoChoice.bloodColor);
}


void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice, BOOL spawnDecals, int argBloodColor) {
	//Provided a blood color? Use that instead of the one that comes with this gibInfo choice.
	CGib::SpawnRandomGibs(pevVictim, cGibs, gibInfoChoice.modelPath, gibInfoChoice.bodyMin, gibInfoChoice.bodyMax, spawnDecals, argBloodColor);
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax) {
	CGib::SpawnRandomGibs(pevVictim, cGibs, argGibPath, gibBodyMin, gibBodyMax, TRUE, (CBaseEntity::Instance(pevVictim))->BloodColor());
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax, BOOL spawnDecals) {
	CGib::SpawnRandomGibs(pevVictim, cGibs, argGibPath, gibBodyMin, gibBodyMax, spawnDecals, (CBaseEntity::Instance(pevVictim))->BloodColor());
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax, BOOL spawnDecals, int argBloodColor) {
	int cSplat;

	for (cSplat = 0; cSplat < cGibs; cSplat++)
	{
		CGib* pGib = GetClassPtr((CGib*)NULL);

		pGib->Spawn(argGibPath, spawnDecals);
		pGib->pev->body = RANDOM_LONG(gibBodyMin, gibBodyMax);// start at one to avoid throwing random amounts of skulls (0th gib)


		if (pevVictim)
		{

			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (0.5);
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (0.5);
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (0.5) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box



			edict_t* pentPlayer;
			if (EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) && ((pentPlayer = FIND_CLIENT_IN_PVS(pGib->edict())) != NULL))
			{
				// 5% chance head will be thrown at player's face.
				entvars_t* pevPlayer;
				pevPlayer = VARS(pentPlayer);
				/*
				pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() ;  //* 300

				// mix in some noise
				pGib->pev->velocity.x += RANDOM_FLOAT ( -0.06, 0.06 );
				pGib->pev->velocity.y += RANDOM_FLOAT ( -0.06, 0.06 );
				pGib->pev->velocity.z += RANDOM_FLOAT ( -0.06, 0.06 );

				float pickedLength = RANDOM_FLOAT ( 300, 320 );
				pGib->pev->velocity = pGib->pev->velocity * pickedLength;

				//cheat the Z pos a bit..
				pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0.3 , 0.65 ) ) + 1;
				//pGib->pev->origin.z += 20;
				*/

				EstablishGutLoverGib(pGib, pevVictim, pevPlayer, FALSE);


			}
			else {
				//how it usually goes.
				pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT(0, 1));
				pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT(0, 1));
				pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT(0, 1)) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box


				// make the gib fly away from the attack vector
				pGib->pev->velocity = g_vecAttackDir * -1;

				// mix in some noise
				pGib->pev->velocity.x += RANDOM_FLOAT(-0.25, 0.25);
				pGib->pev->velocity.y += RANDOM_FLOAT(-0.25, 0.25);
				pGib->pev->velocity.z += RANDOM_FLOAT(-0.25, 0.25);

				pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT(300, 400);
			}



			pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
			pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

			// copy owner's blood color.
			//MODDD NOTE - only for decals it looks  like.
			pGib->m_bloodColor = argBloodColor;
			//pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();


			//DONT THROW OFF THE EMPEROR'S GROVE
			if (EASY_CVAR_GET_DEBUGONLY(cheat_iwantguts) < 1) {
				if (pevVictim->health > -50)
				{
					pGib->pev->velocity = pGib->pev->velocity * 0.7;
				}
				else if (pevVictim->health > -200)
				{
					pGib->pev->velocity = pGib->pev->velocity * 2;
				}
				else
				{
					pGib->pev->velocity = pGib->pev->velocity * 4;
				}
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
		}
		pGib->LimitVelocity();
	}
}//END OF SpawnRandomGibs


float CGib::massInfluence(void) {
	return 0.11f;
}//END OF massInfluence



GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY(CGib)
GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY(CGib)






//=========================================================
// WaitTillLand - in order to emit their meaty scent from
// the proper location, gibs should wait until they stop 
// bouncing to emit their scent. That's what this function
// does.
//=========================================================
void CGib::WaitTillLand(void)
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->velocity == g_vecZero)
	{
		SetThink(&CBaseEntity::SUB_StartFadeOut);
		pev->nextthink = gpGlobals->time + m_lifeTime;

		// If you bleed, you stink!... unless you're a robot, gears don't attract eaters.
		// But exceptions are exceptions (GermanModelOrganicLogic).
		if (m_bloodColor != DONT_BLEED && (GermanModelOrganicLogic() || m_bloodColor != BLOOD_COLOR_BLACK))
		{
			// ok, start stinkin!
			CSoundEnt::InsertSound(bits_SOUND_MEAT, pev->origin, 384, 25);
		}
	}
	else
	{
		// wait and check again in another half second.
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

//
// Gib bounces on the ground or wall, sponges some blood down, too!
//
void CGib::BounceGibTouch(CBaseEntity* pOther)
{
	Vector	vecSpot;
	TraceResult	tr;

	//if ( RANDOM_LONG(0,1) )
	//	return;// don't bleed everytime

	if (pev->flags & FL_ONGROUND)
	{
		pev->velocity = pev->velocity * 0.9;
		pev->angles.x = 0;
		pev->angles.z = 0;
		pev->avelocity.x = 0;
		pev->avelocity.z = 0;
	}
	else
	{
		//if ( g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED )
		if (UTIL_ShouldShowBlood(m_bloodColor) && m_cBloodDecals > 0)
		{
			vecSpot = pev->origin + Vector(0, 0, 8);//move up a bit, and trace down.
			UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -24), ignore_monsters, ENT(pev), &tr);

			UTIL_BloodDecalTrace(&tr, m_bloodColor);

			m_cBloodDecals--;
		}
		//MODDD TODO: leave holes in a "robot" we can assume we hit, if it is a "human" we hit (always a robot)?

		if (m_material != matNone && RANDOM_LONG(0, 2) == 0)
		{
			float volume;
			float zvel = fabs(pev->velocity.z);

			volume = 0.8 * min(1.0, ((float)zvel) / 450.0);

			CBreakable::MaterialSoundRandom(edict(), (Materials)m_material, volume);
		}
	}
}

//
// Sticky gib puts blood on the wall and stays put. 
//
void CGib::StickyGibTouch(CBaseEntity* pOther)
{
	Vector	vecSpot;
	TraceResult	tr;

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 10;

	if (!FClassnameIs(pOther->pev, "worldspawn"))
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 32, ignore_monsters, ENT(pev), &tr);

	if (UTIL_ShouldShowBlood(m_bloodColor)) {
		UTIL_BloodDecalTrace(&tr, m_bloodColor);
	}

	pev->velocity = tr.vecPlaneNormal * -1;
	pev->angles = UTIL_VecToAngles(pev->velocity);
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
}






