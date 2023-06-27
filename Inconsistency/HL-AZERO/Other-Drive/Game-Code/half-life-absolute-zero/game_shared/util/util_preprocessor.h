






// Use before involving m_iPrimaryAmmoType or m_iSecondaryAmmoType of CBasePlayerWeapon.
// It can sometimes be -1 (invalid memory) although 0 is also an impossible place to be assigned.
// Hard to say if high values could come from unsigned variables seeing -1, so checking for that too.
#define IS_AMMOTYPE_VALID(argVarRef) (argVarRef > 0 && argVarRef < MAX_AMMO_TYPES)



//MODDD - moved from monsters.h. This area is made for preprocessor script generator methods like this.
#define CUSTOM_SCHEDULES\
		virtual Schedule_t *ScheduleFromName( const char *pName );\
		static Schedule_t *m_scheduleList[];

#define DEFINE_CUSTOM_SCHEDULES(derivedClass)\
	Schedule_t *derivedClass::m_scheduleList[] =

#define IMPLEMENT_CUSTOM_SCHEDULES(derivedClass, baseClass)\
		Schedule_t *derivedClass::ScheduleFromName( const char *pName )\
		{\
			Schedule_t *pSchedule = ScheduleInList( pName, m_scheduleList, ARRAYSIZE(m_scheduleList) );\
			if ( !pSchedule )\
				return baseClass::ScheduleFromName(pName);\
			return pSchedule;\
		}





#ifdef CLIENT_DLL
// OLD WAY
/*
	inline float CVAR_GET_FLOAT( const char *x ) {	return gEngfuncs.pfnGetCvarFloat( (char*)x ); }
	inline char* CVAR_GET_STRING( const char *x ) {	return gEngfuncs.pfnGetCvarString( (char*)x ); }
	inline struct cvar_s *CVAR_CREATE( const char *cv, const char *val, const int flags ) {	return gEngfuncs.pfnRegisterVariable( (char*)cv, (char*)val, flags ); }
*/


	// Also, note that clientside CVAR_CREATE takes a few parameters ( char *szName, char *szValue, int flags ) to work, and returns
	// a pointer to the 'struct cvar_s' (or cvar_t).
	#define CVAR_CREATE (*gEngfuncs.pfnRegisterVariable)


	#define CVAR_GET_FLOAT (*gEngfuncs.pfnGetCvarFloat)
	#define CVAR_GET_STRING (*gEngfuncs.pfnGetCvarString)
	

	/*
	// Some of the serverside enginecallback.h's defines for reference.
	// there isn't an equivalent engine call clientside of serverside's CVAR_SET_STRING because...?
	#define CVAR_GET_FLOAT	(*g_engfuncs.pfnCVarGetFloat)
	#define CVAR_GET_STRING	(*g_engfuncs.pfnCVarGetString)
	#define CVAR_SET_FLOAT	(*g_engfuncs.pfnCVarSetFloat)
	#define CVAR_SET_STRING	(*g_engfuncs.pfnCVarSetString)
	#define CVAR_GET_POINTER (*g_engfuncs.pfnCVarGetPointer)
	*/

	#define CVAR_SET_FLOAT	(*gEngfuncs.Cvar_SetValue)
	
	#define CVAR_GET_POINTER (*gEngfuncs.pfnGetCvarPointer)



#else
	// Making "CVAR_CREATE" A synonym for serverside's CVAR_REGISTER, it's that way clientside so why not be able
	// to use the same name for both.
	// Also, on serverside, CVAR_CREATE takes a cvar_t* that's been filled out ahead of time as a parameter.
	// It does not return a reference to that.  If "CVAR_GET_POINTER" returns a reference to the exact same cvar_t, that
	// would be interesting.
	#define CVAR_REGISTER	(*g_engfuncs.pfnCVarRegister)
	#define CVAR_CREATE	(*g_engfuncs.pfnCVarRegister)

	#define CVAR_GET_FLOAT	(*g_engfuncs.pfnCVarGetFloat)
	#define CVAR_GET_STRING	(*g_engfuncs.pfnCVarGetString)
	#define CVAR_SET_FLOAT	(*g_engfuncs.pfnCVarSetFloat)
	#define CVAR_SET_STRING	(*g_engfuncs.pfnCVarSetString)
	#define CVAR_GET_POINTER (*g_engfuncs.pfnCVarGetPointer)



#endif












//Why... in the HELL... haven't I done this sooner?


//A starter:
/*
#define GENERATE_XXX_PROTOTYPE\
	void xxx(type arg1, type arg2, type arg3);\
	void xxx(type arg1, type arg2, type arg3, type arg4...);...

#define GENERATE_XXX_PROTOTYPE_VIRTUAL\
	virtual void xxx(type arg1, type arg2, type arg3);\
	virtual void xxx(type arg1, type arg2, type arg3, type arg4...);...


#define GENERATE_XXX_IMPLEMENTATION(derivedClass)\
void derivedClass::xxx(type arg1, type arg2, type arg3){\
	derivedClass::xxx(arg1, arg2, arg3, default);\
}\
void derivedClass::xxx(type arg1, type arg2, type arg3, type arg4...)


#define GENERATE_XXX_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
void derivedClass::xxx(type arg1, type arg2, type arg3){\
	parentClass::xxx(arg1, arg2, arg3, default);\
}\
void derivedClass::xxx(type arg1, type arg2, type arg3, type arg4...){\
	parentClass::xxx(arg1, arg2, arg3, arg4...);\
}


#define GENERATE_XXX_PARENT_CALL(parentClass)\
parentClass::xxx(arg1, arg2, arg3, arg4...);



*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--TRACEATTACK 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Use this in a class to prototype the standard TRACEATTACK headers. This includes the fourth, no further additions needed.
#define GENERATE_TRACEATTACK_PROTOTYPE\
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);\
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod);\
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect);\
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect, BOOL* useBulletHitSound);

//Use this in a class to do the same as above, but they are virtual. Use this for classes expected to be subclassed
//(have child classes make this their parent), like CBaseEntity, CBaseMonster or CSquadMonster.
#define GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL\
	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);\
	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod);\
	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect);\
	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect, BOOL* useBulletHitSound);

//Use this in a class's .cpp file (or wherever implementations go) to implement the method. Requires the current class and parent class as parameters.
//Turns out he plain version doesn't even need know the parent.
//Notice that the last implementation isn't included here. The user must provide the brackets { } for a body and relevant script for that particular monster.
#define GENERATE_TRACEATTACK_IMPLEMENTATION(derivedClass)\
void derivedClass::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType){\
	derivedClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, 0, TRUE, NULL);\
}\
void derivedClass::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod){\
	derivedClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod, TRUE, NULL);\
}\
void derivedClass::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect){\
	derivedClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod, useBloodEffect, NULL);\
}\
void derivedClass::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect, BOOL* useBulletHitSound)


//This is similar to the above, but even handles the final method's implementation for you. It just reroutes any calls to the parent class, little less redirect effort since this 
//method's final method does absolutely nothing anyways. Use for classes that just pass info along like CSquadMonster.
#define GENERATE_TRACEATTACK_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
void derivedClass::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType){\
	parentClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, 0, TRUE, NULL);\
}\
void derivedClass::TraceAttack( entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod){\
	parentClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod, TRUE, NULL);\
}\
void derivedClass::TraceAttack( entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect){\
	parentClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod, useBloodEffect, NULL);\
}\
void derivedClass::TraceAttack( entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect, BOOL* useBulletHitSound){\
	parentClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod, useBloodEffect, useBulletHitSound);\
}

//Same as above, but dummies out all versions of the method instead. Nothing happens, no routing.
#define GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY(derivedClass)\
void derivedClass::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType){\
}\
void derivedClass::TraceAttack( entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod){\
}\
void derivedClass::TraceAttack( entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect){\
}\
void derivedClass::TraceAttack( entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType, int bitsDamageTypeMod, BOOL useBloodEffect, BOOL* useBulletHitSound){\
}

//Exactly the same for now.
#define GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_TRACEATTACK_IMPLEMENTATION_DUMMY(derivedClass)



//This calls the parent class's TraceAttack method. Include it in the final TraceAttack body if needed.
#define GENERATE_TRACEATTACK_PARENT_CALL(parentClass)\
parentClass::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, bitsDamageTypeMod, useBloodEffect, useBulletHitSound)



	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--TAKEDAMAGE 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GENERATE_TAKEDAMAGE_PROTOTYPE\
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType);\
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);

#define GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL\
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType);\
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);


#define GENERATE_TAKEDAMAGE_IMPLEMENTATION(derivedClass)\
int derivedClass::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType){\
	return derivedClass::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, 0);\
}\
int derivedClass::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod)


#define GENERATE_TAKEDAMAGE_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
int derivedClass::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType){\
	return parentClass::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, 0);\
}\
int derivedClass::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){\
	return parentClass::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod);\
}

#define GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY(derivedClass)\
int derivedClass::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType){\
	return 0;\
}\
int derivedClass::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){\
	return 0;\
}

//For whatever reason the dummy stub in cl_dlls/hl/hl_baseentity.cpp, clientside, constantly returns 1 instead of 0. Probably doesn't matter but better stay safe here.
//...Changed. That's only for CBaseEntity's implementation. The others didn't do that. Assuming it does not matter at all.
#define GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_TAKEDAMAGE_IMPLEMENTATION_DUMMY(derivedClass)

#define GENERATE_TAKEDAMAGE_PARENT_CALL(parentClass)\
parentClass::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod)







//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--DEADTAKEDAMAGE 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GENERATE_DEADTAKEDAMAGE_PROTOTYPE\
	int DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType);\
	int DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);

#define GENERATE_DEADTAKEDAMAGE_PROTOTYPE_VIRTUAL\
	virtual int DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType);\
	virtual int DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod);


#define GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(derivedClass)\
int derivedClass::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType){\
	return derivedClass::DeadTakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, 0);\
}\
int derivedClass::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod)


#define GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
int derivedClass::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType){\
	return parentClass::DeadTakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, 0);\
}\
int derivedClass::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){\
	return parentClass::DeadTakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod);\
}

#define GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION_DUMMY(derivedClass)\
int derivedClass::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,int bitsDamageType){\
	return 0;\
}\
int derivedClass::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){\
	return 0;\
}

//For whatever reason the dummy stub in cl_dlls/hl/hl_baseentity.cpp, clientside, constantly returns 1 instead of 0. Probably doesn't matter but better stay safe here.
//...Changed. That's only for CBaseEntity's implementation. The others didn't do that. Assuming it does not matter at all.
#define GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION_DUMMY(derivedClass)

#define GENERATE_DEADTAKEDAMAGE_PARENT_CALL(parentClass)\
parentClass::DeadTakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod)













//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--GIBMONSTER 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*

#define GENERATE_GIBMONSTER_PROTOTYPE\
	void GibMonster(void);\
	void GibMonster(BOOL spawnHeadBlock);\
	void GibMonster(BOOL spawnHeadBlock, BOOL gibsSpawnDecal);


#define GENERATE_GIBMONSTER_PROTOTYPE_VIRTUAL\
	virtual void GibMonster(void);\
	virtual void GibMonster(BOOL spawnHeadBlock);\
	virtual void GibMonster(BOOL spawnHeadBlock, BOOL gibsSpawnDecal);



#define GENERATE_GIBMONSTER_IMPLEMENTATION(derivedClass)\
void derivedClass::GibMonster(){\
	derivedClass::GibMonster(DetermineGibHeadBlock(), TRUE);\
}\
void derivedClass::GibMonster(BOOL spawnHeadBlock){\
	derivedClass::GibMonster(spawnHeadBlock, TRUE);\
}\
void derivedClass::GibMonster(BOOL spawnHeadBlock, BOOL gibsSpawnDecal)



#define GENERATE_GIBMONSTER_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
void derivedClass::GibMonster(){\
	parentClass::GibMonster(DetermineGibHeadBlock(), TRUE);\
}\
void derivedClass::GibMonster(BOOL spawnHeadBlock){\
	parentClass::GibMonster(spawnHeadBlock, TRUE);\
}\
void derivedClass::GibMonster(BOOL spawnHeadBlock, BOOL gibsSpawnDecal){\
	parentClass::GibMonster(spawnHeadBlock, gibsSpawnDecal);\
}


#define GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY(derivedClass)\
void derivedClass::GibMonster(){\
}\
void derivedClass::GibMonster(BOOL spawnHeadBlock){\
}\
void derivedClass::GibMonster(BOOL spawnHeadBlock, BOOL gibsSpawnDecal){\
}

#define GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY(derivedClass)

#define GENERATE_GIBMONSTER_PARENT_CALL(parentClass)\
parentClass::GibMonster(spawnHeadBlock, gibsSpawnDecal)

#define GENERATE_GIBMONSTER_CALL\
	GibMonster(DetermineGibHeadBlock(), gibsSpawnDecals);


*/






#define GENERATE_GIBMONSTER_PROTOTYPE\
	void GibMonster(void);\
	void GibMonster(BOOL fGibSpawnsDecal);


#define GENERATE_GIBMONSTER_PROTOTYPE_VIRTUAL\
	virtual void GibMonster(void);\
	virtual void GibMonster(BOOL fGibSpawnsDecal);


#define GENERATE_GIBMONSTER_IMPLEMENTATION(derivedClass)\
void derivedClass::GibMonster(){\
	derivedClass::GibMonster(GIB_NORMAL);\
}\
void derivedClass::GibMonster(BOOL fGibSpawnsDecal)


#define GENERATE_GIBMONSTER_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
void derivedClass::GibMonster(){\
	parentClass::GibMonster(GIB_NORMAL);\
}\
void derivedClass::GibMonster(BOOL fGibSpawnsDecal){\
	parentClass::GibMonster(fGibSpawnsDecal);\
}


#define GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY(derivedClass)\
void derivedClass::GibMonster(){\
}\
void derivedClass::GibMonster(BOOL fGibSpawnsDecal){\
}

#define GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_GIBMONSTER_IMPLEMENTATION_DUMMY(derivedClass)

#define GENERATE_GIBMONSTER_PARENT_CALL(parentClass)\
parentClass::GibMonster(fGibSpawnsDecal)

//intended to be used in the Killed method in combat.cpp or any other place that has "iGib" available for confirming against GIB_ALWAYS_NODECAL.
//...also, why is a tab required for a preprocessor method without a ( )  parameter list? The world may never know.
#define GENERATE_GIBMONSTER_CALL\
	GibMonster( (iGib != GIB_ALWAYS_NODECAL ) );





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--GIBMONSTERGIB
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GENERATE_GIBMONSTERGIB_PROTOTYPE\
	BOOL GibMonsterGib(BOOL fGibSpawnsDecal);


#define GENERATE_GIBMONSTERGIB_PROTOTYPE_VIRTUAL\
	virtual BOOL GibMonsterGib(BOOL fGibSpawnsDecal);


#define GENERATE_GIBMONSTERGIB_IMPLEMENTATION(derivedClass)\
BOOL derivedClass::GibMonsterGib(BOOL fGibSpawnsDecal)


#define GENERATE_GIBMONSTERGIB_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
BOOL derivedClass::GibMonsterGib(BOOL fGibSpawnsDecal){\
	parentClass::GibMonsterGib(fGibSpawnsDecal);\
}


#define GENERATE_GIBMONSTERGIB_IMPLEMENTATION_DUMMY(derivedClass)\
BOOL derivedClass::GibMonsterGib(BOOL fGibSpawnsDecal){\
	return FALSE;\
}

#define GENERATE_GIBMONSTERGIB_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_GIBMONSTERGIB_IMPLEMENTATION_DUMMY(derivedClass)

//only do this if making no changes to how a monster gibs. Any changes should completely replace the parent method's general case.
#define GENERATE_GIBMONSTERGIB_PARENT_CALL(parentClass)\
parentClass::GibMonsterGib(fGibSpawnsDecal)


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--GIBMONSTERSOUND
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GENERATE_GIBMONSTERSOUND_PROTOTYPE\
	void GibMonsterSound(BOOL fGibbed);


#define GENERATE_GIBMONSTERSOUND_PROTOTYPE_VIRTUAL\
	virtual void GibMonsterSound(BOOL fGibbed);


#define GENERATE_GIBMONSTERSOUND_IMPLEMENTATION(derivedClass)\
void derivedClass::GibMonsterSound(BOOL fGibbed)


#define GENERATE_GIBMONSTERSOUND_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
void derivedClass::GibMonsterSound(BOOL fGibbed){\
	parentClass::GibMonsterSound(fGibbed);\
}


#define GENERATE_GIBMONSTERSOUND_IMPLEMENTATION_DUMMY(derivedClass)\
void derivedClass::GibMonsterSound(BOOL fGibbed){\
}

#define GENERATE_GIBMONSTERSOUND_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_GIBMONSTERSOUND_IMPLEMENTATION_DUMMY(derivedClass)

//only do this if making no changes to how a monster determines a gib sound. Any changes should completely replace the parent method's general case.
#define GENERATE_GIBMONSTERSOUND_PARENT_CALL(parentClass)\
parentClass::GibMonsterSound(fGibbed)


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--GIBMONSTEREND
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GENERATE_GIBMONSTEREND_PROTOTYPE\
	void GibMonsterEnd(BOOL fGibbed);


#define GENERATE_GIBMONSTEREND_PROTOTYPE_VIRTUAL\
	virtual void GibMonsterEnd(BOOL fGibbed);


#define GENERATE_GIBMONSTEREND_IMPLEMENTATION(derivedClass)\
void derivedClass::GibMonsterEnd(BOOL fGibbed)


#define GENERATE_GIBMONSTEREND_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
void derivedClass::GibMonsterEnd(BOOL fGibbed){\
	parentClass::GibMonsterEnd(fGibbed);\
}


#define GENERATE_GIBMONSTEREND_IMPLEMENTATION_DUMMY(derivedClass)\
void derivedClass::GibMonsterEnd(BOOL fGibbed){\
}

#define GENERATE_GIBMONSTEREND_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_GIBMONSTEREND_IMPLEMENTATION_DUMMY(derivedClass)


#define GENERATE_GIBMONSTEREND_PARENT_CALL(parentClass)\
parentClass::GibMonsterEnd(fGibbed)



















//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--Killed 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//void CBaseMonster::Killed( entvars_t *pevAttacker, int iGib )

#define GENERATE_KILLED_PROTOTYPE\
	void Killed(entvars_t* pevAttacker, int iGib);\
	void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib);


#define GENERATE_KILLED_PROTOTYPE_VIRTUAL\
	virtual void Killed(entvars_t* pevAttacker, int iGib);\
	virtual void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib);


#define GENERATE_KILLED_IMPLEMENTATION(derivedClass)\
void derivedClass::Killed(entvars_t* pevAttacker, int iGib){\
	derivedClass::Killed(NULL, pevAttacker, iGib);\
}\
void derivedClass::Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib)


#define GENERATE_KILLED_IMPLEMENTATION_ROUTETOPARENT(derivedClass,parentClass)\
void derivedClass::Killed(entvars_t* pevAttacker, int iGib){\
	parentClass::Killed(NULL, pevAttacker, iGib);\
}\
void derivedClass::Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib){\
	parentClass::Killed(pevInflictor, pevAttacker, iGib);\
}


#define GENERATE_KILLED_IMPLEMENTATION_DUMMY(derivedClass)\
void derivedClass::Killed(entvars_t* pevAttacker, int iGib){\
}\
void derivedClass::Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib){\
}

#define GENERATE_KILLED_IMPLEMENTATION_DUMMY_CLIENT(derivedClass) GENERATE_KILLED_IMPLEMENTATION_DUMMY(derivedClass)

#define GENERATE_KILLED_PARENT_CALL(parentClass)\
parentClass::Killed(pevInflictor, pevAttacker, iGib)

