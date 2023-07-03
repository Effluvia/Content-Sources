/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef FUNC_BREAK_H
#define FUNC_BREAK_H

typedef enum { expRandom, expDirected} Explosions;
typedef enum { matGlass = 0, matWood, matMetal, matFlesh, matCinderBlock, matCeilingTile, matComputer, matUnbreakableGlass, matRocks, matNone, matLastMaterial } Materials;

#define NUM_SHARDS 6 // this many shards spawned when breakable objects break;

class CBreakable : public CBaseDelay
{
public:
	//MODDD - new
	CBreakable();
	virtual BOOL isBreakableOrChild(void);
	virtual BOOL isDestructibleInanimate(void);

	char* m_idShardText;


	// basic functions
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData* pkvd);

	BOOL IsWorldAffiliated(void);

	void EXPORT BreakTouch( CBaseEntity *pOther );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void DamageSound( void );

	void ReportGeneric();

	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL


	virtual BOOL IsBreakable( void );
	BOOL SparkWhenHit( void );

	int  DamageDecal( int bitsDamageType );
	int  DamageDecal( int bitsDamageType, int bitsDamageTypeMod );

	void EXPORT		Die( void );
	virtual int	ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	inline BOOL		Explodable( void ) { return ExplosionMagnitude() > 0; }
	inline int	ExplosionMagnitude( void ) { return pev->impulse; }
	inline void	ExplosionSetMagnitude( int magnitude ) { pev->impulse = magnitude; }

	static void MaterialSoundPrecache( Materials precacheMaterial );
	static void MaterialSoundRandom( edict_t *pEdict, Materials soundMaterial, float volume );
	static const char **MaterialSoundList( Materials precacheMaterial, int &soundCount );

	static const char *pSoundsWood[];
	static const char *pSoundsFlesh[];
	static const char *pSoundsGlass[];
	static const char *pSoundsMetal[];
	static const char *pSoundsConcrete[];
	static const char *pSpawnObjects[];

	static	TYPEDESCRIPTION m_SaveData[];

	Materials	m_Material;
	Explosions	m_Explosion;
	int		m_idShard;
	float	m_angle;
	int		m_iszGibModel;
	int		m_iszSpawnObject;
};

#endif	// FUNC_BREAK_H
