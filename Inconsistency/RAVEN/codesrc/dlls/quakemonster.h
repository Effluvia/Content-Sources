/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "com_model.h"
#include "alias.h"
#include "alias_shared.h"

//=========================================================
// CSquadMonster - for any monster that forms squads.
//=========================================================
class CQuakeMonster : public CBaseMonster 
{
public:
	virtual void Precache( void );

	void SetAliasData( void );

public:
	float StudioFrameAdvance( float flInterval = 0.0 ); // accumulate animation frame time from last time called until now
	int	 GetSequenceFlags( void );

	int  LookupActivity ( int activity );
	int  LookupActivityHeaviest ( int activity );
	int  LookupSequence ( const char *label );

	void ResetSequenceInfo ( );
	int GetAnimationEvent( MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index );
	void DispatchAnimEvents ( float flFutureInterval = 0.1 ); // Handle events that have happend since last time called up until X seconds into the future

	float SetBoneController ( int iController, float flValue ) { return 0; };
	void InitBoneControllers ( void ) { };

	float SetBlending ( int iBlender, float flValue ) { return 0; };
	void GetBonePosition ( int iBone, Vector &origin, Vector &angles ) { };
	void GetAutomovement( Vector &origin, Vector &angles, float flInterval = 0.1 ) { };
	int  FindTransition( int iEndingSequence, int iGoalSequence, int *piDir ) { return 0; };
	void GetAttachment ( int iAttachment, Vector &origin, Vector &angles ) { };
	void SetBodygroup( int iGroup, int iValue ) { };
	int GetBodygroup( int iGroup ) { return 0; };
	void SetEyePosition ( void );

	int ExtractBbox( int sequence, float *mins, float *maxs );
	void SetSequenceBox( void );

	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	BOOL IsQuakeMonster( void ) { return TRUE; }

private:
	// Model/animation data for alias mdl
	alias_extradata_t m_modelExtraData;
};

