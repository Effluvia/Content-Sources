#include "cbase.h"
#include "hoe_human_follower.h"
#include "hoe_corpse.h"
#include "npcevent.h"
#include "props.h"
#include "schedule_hacks.h"

#include "animation.h" // FIXME: remove when helmet code moved to human.cpp

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MIKEFORCE_HEALTH 90 // 100, 90, 80
#define MIKEFORCE_MODEL "models/mikeforce/mikeforce.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	MIKEFORCE_BODYGROUP_HEAD = 0,
	MIKEFORCE_BODYGROUP_TORSO,
	MIKEFORCE_BODYGROUP_ARMS,
	MIKEFORCE_BODYGROUP_LEGS,
//	MIKEFORCE_BODYGROUP_WEAPON,
//	MIKEFORCE_BODYGROUP_WHISKY,
};

#define MIKEFORCE_NUM_HEADS 5

class CNPC_MikeForce : public CHOEHumanFollower
{
public:
	DECLARE_CLASS( CNPC_MikeForce, CHOEHumanFollower );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	CNPC_MikeForce()
	{
		m_nBody = -1; /* random */
	}

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	void SelectModelGroups( void );
	
	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return true; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	int GetHeadGroupNum( void ) { return MIKEFORCE_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return MIKEFORCE_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/mikeforce/head_off.mdl"; }

	virtual const char *GetHelmetModelName( void );
	virtual bool DropHelmet( void );

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_ALLY; }
	const char *GetSentenceGroup( void ) const { return "MF"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			"npc_barney",
			"npc_mikeforce",
			"npc_mikeforce_medic",
//			"npc_peasant",
			"player",
			NULL
		};
		return szClasses;
	};
	const char **GetWeaponClasses( void ) const {
		static const char *szClasses[] = {
			"weapon_870",
			"weapon_m16",
			"weapon_m60",
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return UTIL_VarArgs( "cc_mikeforce%d", m_nHeadNum + 1 );
	}

	int TranslateSchedule( int scheduleType );
	void OnScheduleChange( void );
	void HandleAnimEvent( animevent_t *pEvent );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	void CreateWhisky( void );
	void DropWhisky( void );
	void Event_Killed( const CTakeDamageInfo &info );

	EHANDLE m_hWhisky;

	static Animevent AE_MIKEFORCE_WHISKY_SHOW;
	static Animevent AE_MIKEFORCE_WHISKY_HIDE;
	static Animevent AE_MIKEFORCE_WHISKY_DRINK;

	enum {
		SCHED_MIKEFORCE_VICTORY_DANCE = BaseClass::NEXT_SCHEDULE,
		NEXT_SCHEDULE
	};

	enum {
		TASK_MIKEFORCE_VICTORY = BaseClass::NEXT_TASK,
		NEXT_TASK
	};
};

LINK_ENTITY_TO_CLASS( npc_mikeforce, CNPC_MikeForce );

//IMPLEMENT_SERVERCLASS_ST(CNPC_MikeForce, DT_NPC_HGrunt)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_MikeForce )
	DEFINE_FIELD( m_hWhisky, FIELD_EHANDLE ),
END_DATADESC()

Animevent CNPC_MikeForce::AE_MIKEFORCE_WHISKY_SHOW;
Animevent CNPC_MikeForce::AE_MIKEFORCE_WHISKY_HIDE;
Animevent CNPC_MikeForce::AE_MIKEFORCE_WHISKY_DRINK;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_MikeForce::Spawn( void )
{
	// If spawned by an npc_maker with no weapon specified then give a random weapon
	if ( m_spawnEquipment == NULL_STRING || !Q_strcmp(STRING(m_spawnEquipment), "random") )
	{
		switch ( random->RandomInt( 0, 5 ) )
		{
		case 0:
		case 1:
		case 2:
			m_spawnEquipment = AllocPooledString( "weapon_m16" );
			break;
		case 3:
		case 4:
			m_spawnEquipment = AllocPooledString( "weapon_870" );
			break;
		case 5:
			m_spawnEquipment = AllocPooledString( "weapon_m60" );
			break;
		}
	}
	else if ( !Q_strcmp(STRING(m_spawnEquipment), "none") )
	{
		m_spawnEquipment = NULL_STRING;
	}

	m_iHealth = MIKEFORCE_HEALTH;

	BaseClass::Spawn();

	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_MikeForce::SelectModel()
{
	SetModelName( AllocPooledString( MIKEFORCE_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_MikeForce::SelectModelGroups( void )
{
	if ( m_nBody == -1 )
	{
		m_nHeadNum = random->RandomInt( 0, MIKEFORCE_NUM_HEADS - 1 ); 
	}
	else 
	{
		m_nHeadNum = m_nBody;
	}

#if 0
	if ( m_nHeadNum < 3 )
	{
		CDynamicProp *pHelmet = assert_cast<CDynamicProp*>(CreateEntityByName( "prop_dynamic_override" ));
		if ( pHelmet )
		{
			int iAttachment = LookupAttachment( "head" );
			pHelmet->SetParent( this, iAttachment );
			pHelmet->SetOwnerEntity( this );
			pHelmet->SetModel( GetHelmetModelName() );
			iAttachment = pHelmet->LookupAttachment( "head" );

			pHelmet->AddSpawnFlags( SF_DYNAMICPROP_NO_VPHYSICS );
			pHelmet->AddEffects( EF_PARENT_ANIMATES );
			pHelmet->Spawn();

			if ( iAttachment > 0 )
			{
				Vector origin;
				QAngle angles;
				matrix3x4_t localToWorld;
				GetAttachmentLocalSpace( pHelmet->GetModelPtr(), iAttachment-1, localToWorld );
#if 1
				MatrixGetColumn( localToWorld, 3, origin );
				MatrixAngles( localToWorld, angles.Base() );

//				origin.y *= -1;

				angles.y -= 90; // studiomdl screw this up?

//				matrix3x4_t matrix;
//				AngleMatrix( -angles, vec3_origin, matrix );

				pHelmet->SetLocalOrigin( -origin );
				pHelmet->SetLocalAngles( /*vec3_angle*/-angles );

				Vector rotOrigin/*, absOriginAttach*/;
//				absOrigin = pHelmet->GetAbsOrigin();
//				pHelmet->GetAttachment( iAttachment, absOriginAttach );
//				pHelmet->SetLocalOrigin( absOrigin - absOriginAttach );
				VectorRotate( -origin, -angles, rotOrigin );
				pHelmet->SetLocalOrigin( rotOrigin );
#else
				MatrixGetColumn( localToWorld, 3, origin ); DevMsg( "origin %f,%f,%f\n", origin.x, origin.y, origin.z );
				swap( origin.x, origin.y );
//				origin.x *= -1;
				origin.y *= -1; /*origin.x += 0.8; origin.z += 0.1;*/
				pHelmet->SetLocalOrigin( -origin );
				MatrixAngles( localToWorld, angles.Base() ); DevMsg( "angles %f,%f,%f\n", angles.x, angles.y, angles.z );
				angles.y -= 90; // studiomdl screw this up?
				pHelmet->SetLocalAngles( -angles );
#endif
			}
		}
	}
#endif

	m_nBody = 0;
	SetBodygroup( MIKEFORCE_BODYGROUP_HEAD, m_nHeadNum );

	if (m_spawnEquipment != NULL_STRING && !Q_strcmp(STRING(m_spawnEquipment), "weapon_m60"))
		SetBodygroup( MIKEFORCE_BODYGROUP_TORSO, 4 );
	else
		SetBodygroup( MIKEFORCE_BODYGROUP_TORSO, random->RandomInt( 0, 3 ) );
}

//-----------------------------------------------------------------------------
const char *CNPC_MikeForce::GetHelmetModelName( void )
{
	switch ( m_nHeadNum )
	{
	case 0:
	case 1: 
		return "models/mikeforce/helmet_beret.mdl";
		break;
	case 2:
		return "models/mikeforce/helmet_boonie.mdl";
		break;
	case 3: 
		return "models/mikeforce/helmet_beret.mdl";
		break;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
bool CNPC_MikeForce::DropHelmet( void )
{
	if ( m_nHeadNum == 4 )
		return false;

	SetBodygroup( MIKEFORCE_BODYGROUP_HEAD, m_nHeadNum + 6 );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_MikeForce::Precache( void )
{
	PrecacheScriptSound( "NPC_MikeForce.FootstepLeft" );
	PrecacheScriptSound( "NPC_MikeForce.FootstepRight" );
	PrecacheScriptSound( "NPC_MikeForce.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_MikeForce.RunFootstepRight" );
	PrecacheScriptSound( "NPC_MikeForce.Glug" );

	UTIL_PrecacheOther( "item_whisky" );

	PrecacheModel( "models/mikeforce/helmet_beret.mdl" );
	PrecacheModel( "models/mikeforce/helmet_boonie.mdl" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_MikeForce::Classify( void )
{
	return CLASS_MIKEFORCE;
}

//-----------------------------------------------------------------------------
int CNPC_MikeForce::TranslateSchedule( int scheduleType )
{
	switch ( scheduleType )
	{
	case SCHED_VICTORY_DANCE:
		return SCHED_MIKEFORCE_VICTORY_DANCE;
	default:
		return BaseClass::TranslateSchedule( scheduleType );
	}
}

//-----------------------------------------------------------------------------
void CNPC_MikeForce::OnScheduleChange( void )
{
	// Must hide the whisky if ACT_VICTORY_DANCE is interrupted.
	if ( m_hWhisky )
		UTIL_Remove( m_hWhisky	); // could just hide it
	
	BaseClass::OnScheduleChange();
}

//-----------------------------------------------------------------------------
void CNPC_MikeForce::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_MIKEFORCE_WHISKY_SHOW )
	{
		CreateWhisky();
		return;
	}
	if ( pEvent->event == AE_MIKEFORCE_WHISKY_HIDE )
	{
		if ( m_hWhisky )
			UTIL_Remove( m_hWhisky	); // could just hide it
		return;
	}
	if ( pEvent->event == AE_MIKEFORCE_WHISKY_DRINK )
	{
		EmitSound( "NPC_MikeForce.Glug" );
		return;
	}

	return BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
void CNPC_MikeForce::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MIKEFORCE_VICTORY:
		{
			// 1/4 times just speak without drinking
			if ( random->RandomInt(0,3) == 0 )
			{
				TaskComplete();
				break;
			}

			SetIdealActivity( ACT_VICTORY_DANCE );

			// Block others' speech until our animation + speech ends
			int iSeq = LookupSequence( "ACT_VICTORY_DANCE" );
			if ( iSeq != ACT_INVALID )
			{
				float flDuration = SequenceDuration( iSeq );
				if ( GetExpresser() )
				{
					// FIXME: this fails if someone is talking
					CAI_TimedSemaphore *pSemaphore = GetSpeechSemaphore( this );
					if ( pSemaphore && pSemaphore->IsAvailable( this ) )
					{
						// TODO: handle schedule being interrupted
						pSemaphore->Acquire( flDuration + 2.0f, this );
					}
				}
			}
		}
		break;
	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_MikeForce::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MIKEFORCE_VICTORY:
		{
			AutoMovement( );
			if ( IsActivityFinished() )
			{
				SetIdealActivity( ACT_IDLE );
				TaskComplete();
			}
			break;
		}
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
ConVar sk_whisky_origin("sk_whisky_origin", "2 1 -2.5");
ConVar sk_whisky_angles("sk_whisky_angles", "0 -40 0");
void CNPC_MikeForce::CreateWhisky( void )
{
	if ( m_hWhisky )
		return;

	m_hWhisky = (CBaseAnimating*)CreateEntityByName( "prop_dynamic" );
	if ( m_hWhisky )
	{
		m_hWhisky->SetModel( "models/w_whisky.mdl" );
		int iAttachment = LookupAttachment( "lefthand" );
		m_hWhisky->SetParent(this, iAttachment);
		m_hWhisky->SetOwnerEntity(this);
		m_hWhisky->SetSolid( SOLID_NONE );

		float dx, dy, dz;
		sscanf(sk_whisky_origin.GetString(), "%g %g %g", &dx, &dy, &dz);
		m_hWhisky->SetLocalOrigin( Vector( dx, dy, dz ) );

		sscanf(sk_whisky_angles.GetString(), "%g %g %g", &dx, &dy, &dz);
		m_hWhisky->SetLocalAngles( QAngle( dx, dy, dz ) );

		m_hWhisky->AddEffects( EF_PARENT_ANIMATES );
		m_hWhisky->Spawn();
	}
}

//-----------------------------------------------------------------------------
void CNPC_MikeForce::DropWhisky( void )
{
	// Alway drop it if mikeforce died while drinking. Otherwise 1:20 chance.
	if ( !m_hWhisky && random->RandomInt(0, 20) != 0 )
		return;

	if ( m_hWhisky )
		UTIL_Remove( m_hWhisky	);

	Vector origin;
	QAngle angles;
	GetAttachment( LookupAttachment( "lefthand" ), origin, angles );

	CBaseEntity *pWhisky = CreateEntityByName( "item_whisky" );
	pWhisky->SetAbsOrigin( origin );
	pWhisky->SetAbsAngles( angles );
	pWhisky->Spawn();
}

//-----------------------------------------------------------------------------
void CNPC_MikeForce::Event_Killed( const CTakeDamageInfo &info )
{
	DropWhisky();

	if ( m_hWhisky )
		UTIL_Remove( m_hWhisky	);

	BaseClass::Event_Killed( info );
}

HOE_BEGIN_CUSTOM_NPC( npc_mikeforce, CNPC_MikeForce )
	DECLARE_ANIMEVENT( AE_MIKEFORCE_WHISKY_SHOW )
	DECLARE_ANIMEVENT( AE_MIKEFORCE_WHISKY_HIDE )
	DECLARE_ANIMEVENT( AE_MIKEFORCE_WHISKY_DRINK )
	DECLARE_TASK( TASK_MIKEFORCE_VICTORY )
	DECLARE_SCHEDULE( SCHED_MIKEFORCE_VICTORY_DANCE )
	LOAD_SCHEDULES_FILE( npc_mikeforce )
HOE_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define MIKEFORCE_MODEL_BYARMS "models/mikeforce/mikeforce_byarms.mdl"
#define MIKEFORCE_MODEL_BYFEET "models/mikeforce/mikeforce_byfeet.mdl"
#define MIKEFORCE_MODEL_HEADLESS "models/mikeforce/mikeforce_headless.mdl"

#define SF_CORPSE_HEADLESS (1 << 18)

class CDeadMikeForce : public CHOECorpse
{
public:
	DECLARE_CLASS( CDeadMikeForce, CHOECorpse );

	void Spawn( void )
	{
		CorpsePose poses[] = {
			{ "lying_on_back", false },
			{ "lying_on_side", false },
			{ "lying_on_stomach", false },
			{ "hanging_byfeet",	true, "rope_feet", NULL },
			{ "hanging_byarms", true, "lefthand", "rope_righthand" },
			{ "hanging_byneck", true, "rope_neck", NULL },
			{ "deadsitting", false },
			{ "deadseated", false },
			{ NULL, false }
		};

		SetBloodColor( BLOOD_COLOR_RED );

		// The "by arms" ragdoll allows the arms to rotate an extreme amount.
		const char *szModel = MIKEFORCE_MODEL_BYARMS;

		// The "by feet" ragdoll disables collisions between the legs.
		if ( m_pose == 3 )
			szModel = MIKEFORCE_MODEL_BYFEET;

		// The original game had a few "headless" corpses where the head was jammed into a wall.
		// That's no good for ragdoll corpses.  This ragdoll has no head physics and only a neck
		// in MIKEFORCE_BODYGROUP_HEAD.
		if ( HasSpawnFlags( SF_CORPSE_HEADLESS ) )
			szModel = MIKEFORCE_MODEL_HEADLESS;

		// NOTE: HL1 used namGrunt.mdl, not mikeforce.mdl!
		InitCorpse( szModel, poses, AllocPooledString( "npc_mikeforce" ) );

		BaseClass::Spawn();
	}

	void SelectModelGroups( void )
	{
		int headNum;

		if ( m_nBody == -1 )
		{
			headNum = random->RandomInt( 0, MIKEFORCE_NUM_HEADS - 1 ); 
		}
		else 
		{
			headNum = m_nBody;
		}

		m_nBody = 0;
		SetBodygroup( MIKEFORCE_BODYGROUP_TORSO, headNum );
		if ( HasSpawnFlags( SF_CORPSE_HEADLESS ) == false )
		{
			// Remove the helmet when hanging upside down or 25% of the time
			if ( ( m_pose == 3  /*|| !random->RandomInt(0, 3)*/ ) && headNum < 4 )
				headNum += 6;

			SetBodygroup( MIKEFORCE_BODYGROUP_HEAD, headNum );
		}
	}
};

LINK_ENTITY_TO_CLASS( npc_mikeforce_dead, CDeadMikeForce );
