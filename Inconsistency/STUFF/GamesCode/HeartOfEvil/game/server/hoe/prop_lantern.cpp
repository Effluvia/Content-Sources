#include "cbase.h"
#ifdef PROP_DERIVED
#include "props.h"
#else
#include "player_pickup.h"
#endif
#include "physics_saverestore.h"
#include "vphysics/constraints.h"
#include "rope.h"
#include "Sprite.h"
#define LANTERN_TRANSITION
#ifdef LANTERN_TRANSITION
#include "globalstate.h"
#endif
#include "materialsystem/imaterialsystemhardwareconfig.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Singleton interfaces
//-----------------------------------------------------------------------------
extern IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;

#define LANTERN_MODEL "models/lantern.mdl"
#define LANTERN2_MODEL "models/lantern2.mdl"
#define LANTERN_MODEL_BROKEN "models/lantern_broken.mdl"
#define LANTERN2_MODEL_BROKEN "models/lantern2_broken.mdl"

#define LANTERN_ROTDAMPING 3.0 // stops extreme motion on the end of a rope

#ifdef LANTERN_TRANSITION
class CInfoLantern : public CBaseEntity
{
	DECLARE_CLASS( CInfoLantern, CBaseEntity )
public:
	DECLARE_DATADESC()

	string_t m_iszMapName;
	Vector m_Position;
	QAngle m_Angles;
	Vector m_Velocity;
	AngularImpulse m_AngVelocity;
	bool m_bOn;
	bool m_bBroken;
};

BEGIN_DATADESC( CInfoLantern )
	DEFINE_FIELD( m_iszMapName, FIELD_STRING ),
	DEFINE_FIELD( m_Position, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_Angles, FIELD_VECTOR ),
	DEFINE_FIELD( m_Velocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_AngVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_bOn, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBroken, FIELD_BOOLEAN ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( info_lantern, CInfoLantern );

#endif // LANTERN_TRANSITION

#ifdef PROP_DERIVED
class CPropLantern : public CPhysicsProp
{
	DECLARE_CLASS( CPropLantern, CPhysicsProp );
#else
class CPropLantern : public CBaseAnimating, public CDefaultPlayerPickupVPhysics
{
	DECLARE_CLASS( CPropLantern, CBaseAnimating );
#endif
public:
	DECLARE_DATADESC();

	CPropLantern()
	{
		m_bOn = true;

		m_Color.r = 243;
		m_Color.g = 194;
		m_Color.b = 48;

		m_Radius = 196;
		m_Exponent = 6;
		m_LightStyle = 6;
	}
	~CPropLantern()
	{
		if ( m_pConstraint )
		{
			m_pConstraint->Deactivate();
			physenv->DestroyConstraint( m_pConstraint );
		}
		if ( m_pConstraintGroup )
			physenv->DestroyConstraintGroup( m_pConstraintGroup );
	}
	void Spawn( void );
	void Precache( void );
	void Activate( void );
	void UpdateOnRemove( void );
#ifndef PROP_DERIVED
	bool CreateVPhysics( void );
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer ) { return true; }
	virtual QAngle PreferredCarryAngles( void ) { return QAngle( 0, 180, 0 ); }
#endif
	int ObjectCaps( void )
	{
		int caps = BaseClass::ObjectCaps();
#ifndef PROP_DERIVED
		caps |= FCAP_IMPULSE_USE; // pickup
#endif
		if ( /*m_bRope ||*/ m_iszInfoLantern != NULL_STRING )
			caps &= ~FCAP_ACROSS_TRANSITION;
		return caps;
	}
#ifndef PROP_DERIVED
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
	{
		CBasePlayer *pPlayer = ToBasePlayer( pActivator );
		if ( pPlayer )
		{
			pPlayer->PickupObject( this );
		}
	}
#endif
	int OnTakeDamage( const CTakeDamageInfo &info )
	{
		CTakeDamageInfo info2 = info;
		if ( info.GetDamageType() & DMG_BULLET )
		{
//			info2.SetDamageForce( info.GetDamageForce() * 0.05 );
		}
		return BaseClass::OnTakeDamage( info2 );
	}

	// Don't treat as a live target
	bool IsAlive( void ) { return false; }

	void Event_Killed( const CTakeDamageInfo &info );
	void DyingThink( void );
	const char *GetBrokenModel( void );
	float m_flTimeKilled;

	void CreateLight( void );
	void CreateRope( void );
	void CreateSprite( void );

	void TurnOn( void )
	{
		if ( m_bOn )
			return;
		variant_t emptyVariant;
		if ( m_hLight )
			m_hLight->AcceptInput( "TurnOn", this, this, emptyVariant, 0 );
		if ( m_hSprite )
			m_hSprite->AcceptInput( "ShowSprite", this, this, emptyVariant, 0 );
		m_nSkin = 0;
		m_bOn = true;
	}
	void TurnOff( void )
	{
		if ( !m_bOn )
			return;
		variant_t emptyVariant;
		if ( m_hLight )
			m_hLight->AcceptInput( "TurnOff", this, this, emptyVariant, 0 );
		if ( m_hSprite )
			m_hSprite->AcceptInput( "HideSprite", this, this, emptyVariant, 0 );
		m_nSkin = 1;
		m_bOn = false;
	}

	void InputSetBrightness( inputdata_t &inputdata )
	{
		if ( m_hLight )
			m_hLight->AcceptInput( "brightness", inputdata.pActivator, inputdata.pCaller, inputdata.value, inputdata.nOutputID );
	}
	void InputSetColor( inputdata_t &inputdata )
	{
		if ( m_hLight )
		{
			color32 color = inputdata.value.Color32();
			m_hLight->SetRenderColor( color.r, color.g, color.b );
		}
	}
	void InputSetDistance( inputdata_t &inputdata )
	{
		if ( m_hLight )
			m_hLight->AcceptInput( "distance", inputdata.pActivator, inputdata.pCaller, inputdata.value, inputdata.nOutputID );
	}
	void InputSetStyle( inputdata_t &inputdata )
	{
		// BUG in SDK: The FIELD_CHARACTER "style" input doesn't even work!
		if ( m_hLight )
			m_hLight->AcceptInput( "SetStyle", inputdata.pActivator, inputdata.pCaller, inputdata.value, inputdata.nOutputID );
	}
	void InputTurnOn( inputdata_t &inputdata )
	{
		TurnOn();
	}
	void InputTurnOff( inputdata_t &inputdata )
	{
		TurnOff();
	}
	void InputToggle( inputdata_t &inputdata )
	{
		if ( m_bOn )
			TurnOff();
		else
			TurnOn();
	}

	EHANDLE m_hLight;
	CHandle<CSprite> m_hSprite;
	bool m_bRope;
	bool m_bOn;
	bool m_bBroken;
	int m_nLanternModel;

	// Temp values passed to light_dynamic
	float m_Radius;
	int m_Exponent;
	color32 m_Color;
	int m_LightStyle;

	CHandle<CRopeKeyframe> m_hRope; // Visible textured rope
	IPhysicsConstraintGroup *m_pConstraintGroup;
	IPhysicsConstraint *m_pConstraint; // Length constraint

#define WATER_THINK_CONTEXT "LanternWaterThinkContext"

	void WaterThink( void )
	{
		if ( !m_bOn )
		{
			SetContextThink( NULL, TICK_NEVER_THINK, WATER_THINK_CONTEXT );
			return;
		}

		if ( UTIL_FindClientInPVS( edict() ) )
		{
			Vector origin;
			QAngle angles;
			int nAttach = LookupAttachment( "light" );
			GetAttachment( nAttach, origin, angles );

			if ( !VectorsAreEqual( origin, m_vLastWaterOrigin, 1.0f ) )
			{
				if ( UTIL_PointContents( origin ) & (CONTENTS_WATER | CONTENTS_SLIME) )
				{
					TurnOff();
					SetContextThink( NULL, TICK_NEVER_THINK, WATER_THINK_CONTEXT );
					return;
				}
			}

			m_vLastWaterOrigin = origin;
		}

		SetNextThink( gpGlobals->curtime + 0.5, WATER_THINK_CONTEXT );
	}

	void SetBrokenModel( void );

	void OnEntityEvent( EntityEvent_t event, void *pEventData )
	{
		BaseClass::OnEntityEvent( event, pEventData );
		switch( event )
		{
		case ENTITY_EVENT_WATER_TOUCH:
			{
				m_vLastWaterOrigin = vec3_origin;
				SetContextThink( &CPropLantern::WaterThink, gpGlobals->curtime + 0.1f, WATER_THINK_CONTEXT );
			}
			break;
		case ENTITY_EVENT_WATER_UNTOUCH:
			{
				SetContextThink( NULL, TICK_NEVER_THINK, WATER_THINK_CONTEXT );

				// Assume the player lights it again
				if ( !m_bOn && !m_bBroken )
					TurnOn();
			}
			break;
		}
	}

	Vector m_vLastWaterOrigin;

#ifdef LANTERN_TRANSITION
	CBaseEntity *FindGlobalEntity( string_t classname, string_t globalname )
	{
		CBaseEntity *pReturn = NULL;

		while ( (pReturn = gEntList.NextEnt( pReturn )) != NULL )
		{
			if ( FStrEq( STRING(pReturn->m_iGlobalname), STRING(globalname)) )
				break;
		}
			
		if ( pReturn )
		{
			if ( !FClassnameIs( pReturn, STRING(classname) ) )
			{
				Warning( "Global entity found %s, wrong class %s [expects class %s]\n", STRING(globalname), STRING(pReturn->m_iClassname), classname );
				pReturn = NULL;
			}
		}

		return pReturn;
	}
	int Save( ISave &save )
	{
		int ret = BaseClass::Save( save );

		// env_sprites don't cross level transitions.
		// Since this lantern has 2 children (sprite & light) and the sprite doesn't
		// cross over, the list of children gets messed up. 
		// Add the sprite back as a child of ours.
		if ( m_hSprite )
		{
			m_hSprite->SetAttachment( this, LookupAttachment( "light" ) );
		}

		return ret;
	}
	void OnSave( IEntitySaveUtils *pSaveUtils )
	{
		if ( m_iszInfoLantern != NULL_STRING )
		{
			CBaseEntity *pEnt = FindGlobalEntity( AllocPooledString("info_lantern"), m_iszInfoLantern );
			if ( pEnt != NULL )
			{
				CInfoLantern *pInfo = assert_cast<CInfoLantern*>(pEnt);
				pInfo->m_iszMapName = gpGlobals->mapname;
				pInfo->m_Position = GetAbsOrigin();
				pInfo->m_Angles = GetAbsAngles();
				GetVelocity( &pInfo->m_Velocity, &pInfo->m_AngVelocity );
				pInfo->m_bOn = m_bOn;
				pInfo->m_bBroken = m_bBroken;
				pInfo->SetHealth( GetHealth() );
			}
		}

		// env_sprites don't cross level transitions.
		// Since this lantern has 2 children (sprite & light) and the sprite doesn't
		// cross over, the list of children gets messed up.
		// Forget the sprite is a child of ours before saving.
		if ( m_hSprite )
		{
			m_hSprite->StopFollowingEntity();
			m_hSprite->m_hAttachedToEntity = NULL;
			m_hSprite->m_nAttachment = 0;
		}

		BaseClass::OnSave( pSaveUtils );
	}
	void OnRestore( void )
	{
		BaseClass::OnRestore();
//		CopyInfoLantern();
	}
	void CopyInfoLantern( void )
	{
		if ( gpGlobals->eLoadType != MapLoad_Transition )
			return;

		if ( m_iszInfoLantern == NULL_STRING )
			return;

		CBaseEntity *pEnt = FindGlobalEntity( AllocPooledString("info_lantern"), m_iszInfoLantern );
		if ( pEnt != NULL )
		{
			CInfoLantern *pInfo = assert_cast<CInfoLantern*>(pEnt);

			// Don't copy info we saved, or if we just spawned, info should come from another map
			if ( pInfo->m_iszMapName == gpGlobals->mapname )
				return;

			// If the previous level didn't have this global entity we will end up at 0,0,0
			if ( pInfo->m_iszMapName == NULL_STRING )
			{
//				DevWarning( "global lantern '%s' with bogus transition setup\n", STRING(m_iszInfoLantern) );
				return;
			}

			DevMsg( "restoring global lantern '%s'\n", STRING(m_iszInfoLantern) );

			Teleport( &pInfo->m_Position, &pInfo->m_Angles, &pInfo->m_Velocity );
			if ( VPhysicsGetObject() )
				VPhysicsGetObject()->SetVelocity( NULL, &pInfo->m_AngVelocity );
			SetHealth( pInfo->GetHealth() );
			if ( pInfo->m_bBroken != m_bBroken )
			{
				Assert( pInfo->m_bBroken );
				Assert( !pInfo->m_bOn );
				m_takedamage = DAMAGE_EVENTS_ONLY;
				SetBrokenModel();
				m_bBroken = true;
			}
			if ( pInfo->m_bOn != m_bOn )
			{
				if ( pInfo->m_bOn )
					TurnOn();
				else
					TurnOff();
			}
		}
	}
	string_t m_iszInfoLantern; // our m_iGlobalname if it was specified, name of CInfoLantern
#endif // LANTERN_TRANSITION
};

LINK_ENTITY_TO_CLASS( prop_lantern, CPropLantern );	

BEGIN_DATADESC( CPropLantern )
	DEFINE_KEYFIELD( m_bRope, FIELD_BOOLEAN, "rope" ),
	DEFINE_KEYFIELD( m_bOn, FIELD_BOOLEAN, "StartOn" ),
	DEFINE_KEYFIELD( m_bBroken, FIELD_BOOLEAN, "broken" ),
	DEFINE_KEYFIELD( m_nLanternModel, FIELD_INTEGER, "LanternModel" ),

	DEFINE_KEYFIELD/*_NOT_SAVED*/( m_Radius, FIELD_FLOAT, "distance" ),
	DEFINE_KEYFIELD/*_NOT_SAVED*/( m_Exponent, FIELD_INTEGER, "brightness" ),
	DEFINE_KEYFIELD_NOT_SAVED( m_Color, FIELD_COLOR32, "_light" ),
	DEFINE_KEYFIELD_NOT_SAVED( m_LightStyle, FIELD_INTEGER, "style" ),

	DEFINE_FIELD( m_hLight, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSprite, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hRope, FIELD_EHANDLE ),
	DEFINE_PHYSPTR( m_pConstraint ),
	DEFINE_PHYSPTR( m_pConstraintGroup ),

	DEFINE_FIELD( m_vLastWaterOrigin, FIELD_POSITION_VECTOR ),
	DEFINE_THINKFUNC( WaterThink ),

#ifdef LANTERN_TRANSITION
	DEFINE_FIELD( m_iszInfoLantern, FIELD_STRING ),
#endif

	DEFINE_THINKFUNC( DyingThink ),
	DEFINE_FIELD( m_flTimeKilled, FIELD_TIME ),

	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetBrightness", InputSetBrightness ),
	DEFINE_INPUTFUNC( FIELD_COLOR32, "SetColor", InputSetColor ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetDistance", InputSetDistance ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetStyle", InputSetStyle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
END_DATADESC()

//-----------------------------------------------------------------------------
void CPropLantern::Spawn( void )
{
//	SetClassname( "prop_physics" );
//	SetModel( LANTERN_MODEL );

#ifdef LANTERN_TRANSITION
	// Lanterns on ropes don't cross level transitions.  If this lantern has a global name
	// then create a new entity to pass the entity state between maps.
	// Ropeless lanterns with a globalname don't cross level transitions due to a bug/misfeature of 
	// light_dynamic entities with a globalname not being created on the client.
	if ( /*m_bRope == true &&*/ m_iGlobalname != NULL_STRING )
	{
		m_iszInfoLantern = m_iGlobalname;
		m_iGlobalname = NULL_STRING;
	}

	if ( m_iszInfoLantern != NULL_STRING )
	{
		CBaseEntity *pEnt = FindGlobalEntity( AllocPooledString("info_lantern"), m_iszInfoLantern );
		if ( pEnt == NULL )
		{
			pEnt = CBaseEntity::CreateNoSpawn( "info_lantern", GetAbsOrigin(), GetAbsAngles() );
			pEnt->m_iGlobalname = m_iszInfoLantern;
			DispatchSpawn( pEnt ); // sets up global entity stuff
		}
	}
#endif

	Precache();
#ifndef PROP_DERIVED
	SetModel( STRING(GetModelName()) );
#endif
	BaseClass::Spawn();

	m_iHealth = m_iMaxHealth = 10;
	m_takedamage = DAMAGE_YES;

#ifndef PROP_DERIVED
	CreateVPhysics();
	SetBlocksLOS( false );
#endif

	if ( m_bBroken )
	{
		m_iHealth = 0;
		m_takedamage = DAMAGE_EVENTS_ONLY;
		m_bOn = false;
	}
	if ( !m_bOn )
		m_nSkin = 1;

	CreateLight();
#ifndef LANTERN_TRANSITION
	CreateSprite();
#endif
	if ( m_nLanternModel == 1 && m_bRope )
	{
		DevWarning( "lantern model %d with rope not allowed\n", m_nBody );
		m_bRope = false;
	}

	if ( m_bRope )
	{
		IPhysicsObject *pPhysObj = VPhysicsGetObject();
		if ( pPhysObj )
		{
			// Stop extreme motion on the end of a rope
			const float rotdamping = LANTERN_ROTDAMPING;
			pPhysObj->SetDamping( NULL, &rotdamping );
		}
		CreateRope();
	}
}

//-----------------------------------------------------------------------------
void CPropLantern::Precache( void )
{
	UTIL_PrecacheOther( "light_dynamic" );
	UTIL_PrecacheOther( "env_sprite", "sprites/yelflare2.spr" );

	if ( GetModelName() == NULL_STRING )
	{
		if ( m_nLanternModel < 0 || m_nLanternModel > 2 )
		{
			DevWarning( "bogus lantern model %d, setting to 0\n", m_nBody );
			m_nLanternModel = 0;
		}
		if ( m_bBroken )
		{
			SetModelName( AllocPooledString(GetBrokenModel()) );
		}
		else
		{
			const char *szModel[] = { LANTERN_MODEL, LANTERN2_MODEL };
			SetModelName( AllocPooledString(szModel[m_nLanternModel]) );
		}
	}

#ifndef PROP_DERIVED
	PrecacheModel( STRING(GetModelName()) );
	PrecacheModel( GetBrokenModel() );
#endif
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CPropLantern::Activate( void )
{
	BaseClass::Activate();

#ifdef LANTERN_TRANSITION
	CopyInfoLantern();
#endif

#ifdef LANTERN_TRANSITION
	// env_sprites don't cross level transitions.
	// Since this lantern has 2 children (sprite & light) and the sprite doesn't
	// cross over, the list of children gets messed up.  So recreate the sprite
	// every time.
	if ( m_hSprite == NULL && !m_bBroken )
		CreateSprite();
#endif
}

//-----------------------------------------------------------------------------
void CPropLantern::UpdateOnRemove( void )
{
	if ( m_hLight )
	{
		UTIL_Remove( m_hLight );
		m_hLight = NULL;
	}
	if ( m_hSprite )
	{
		UTIL_Remove( m_hSprite );
		m_hSprite = NULL;
	}
	BaseClass::UpdateOnRemove();
}

#ifndef PROP_DERIVED
//-----------------------------------------------------------------------------
bool CPropLantern::CreateVPhysics( void )
{
	/*IPhysicsObject *pPhysicsObject =*/ VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );
	return true;
}
#endif

//-----------------------------------------------------------------------------
void CPropLantern::CreateLight( void )
{
	Assert( m_hLight.Get() == NULL );

	CBaseEntity *pEnt = CBaseEntity::CreateNoSpawn( "light_dynamic", WorldSpaceCenter(), GetAbsAngles(), this );
	if ( pEnt )
	{
		pEnt->KeyValue( "_cone", "0" ); // this disables the spotlight
		pEnt->KeyValue( "_innercone", "0" ); // this disables the spotlight
		Vector color( m_Color.r, m_Color.g, m_Color.b );
		pEnt->KeyValue( "_light", color /*"243 194 48"*/ ); // actually just rendercolor with brightness ignored
		if ( ( g_pMaterialSystemHardwareConfig != NULL ) && ( g_pMaterialSystemHardwareConfig->GetHDRType() != HDR_TYPE_NONE ) )
			pEnt->KeyValue( "brightness", m_Exponent/*"6"*/*0.75 );
		else
			pEnt->KeyValue( "brightness", m_Exponent/*"6"*/ );
		pEnt->KeyValue( "distance", m_Radius/*"196"*/ );
		pEnt->KeyValue( "style", m_LightStyle/*"6"*/ );

		pEnt->SetParent( this );

#ifdef LANTERN_TRANSITIONxxx // a dynamic light with global name doesn't get recreated on the client after transition???
		if ( m_bRope == false && m_iGlobalname != NULL_STRING )
		{
			char szName[128];
			Q_snprintf( szName, sizeof(szName), "%s_light", STRING(m_iGlobalname) );
			pEnt->m_iGlobalname = AllocPooledString( szName );
		}
#endif

		DispatchSpawn( pEnt );
//		pEnt->Activate();

		if ( !m_bOn )
		{
			variant_t emptyVariant;
			pEnt->AcceptInput( "TurnOff", this, this, emptyVariant, 0 );
		}

		m_hLight = pEnt;
	}
}

//-----------------------------------------------------------------------------
void CPropLantern::CreateSprite( void )
{
	Assert( m_hSprite.Get() == NULL );

	Vector origin;
	QAngle angles;
	int nAttach = LookupAttachment( "light" );
	GetAttachment( nAttach, origin, angles );

	origin += Vector(0,0,2); // so it doesn't get clipped by the wick
	CBaseEntity *pEnt = CBaseEntity::CreateNoSpawn( "env_sprite", origin, angles, this );
	if ( pEnt )
	{
		pEnt->KeyValue( "framerate", "10.0" );
		pEnt->KeyValue( "GlowProxySize", "6.0" );
		pEnt->KeyValue( "model", "sprites/yelflare2.spr" );
		pEnt->KeyValue( "renderamt", "200" );
		pEnt->KeyValue( "renderfx", "0"/*"3"*/ );
		pEnt->KeyValue( "rendermode", "9" );
		pEnt->KeyValue( "scale", "1" );

		((CSprite *)pEnt)->SetAttachment( this, nAttach );
#ifdef LANTERN_TRANSITION
		((CSprite *)pEnt)->SetAsTemporary();
#endif
		DispatchSpawn( pEnt );
//		pEnt->Activate();

		if ( !m_bOn )
			((CSprite *)pEnt)->TurnOff();

		m_hSprite = assert_cast<CSprite *>(pEnt);
	}
}

// I've stuggled to give the ropes a realistic amount of slack when the rope endpoints
// get closer. Short ropes need a high slack ( > 2 or 3 times the rope's rest length)
// while long ropes need slack ~0.5 rest length.
ConVar hoe_lantern_rope_slack( "hoe_lantern_rope_slack", "0.5" );

//-----------------------------------------------------------------------------
void CPropLantern::CreateRope( void )
{
	int nAttach = LookupAttachment( "rope" );
	Vector vAttach;
	GetAttachment( nAttach, vAttach );

	trace_t tr;
	UTIL_TraceLine( vAttach, vAttach + Vector(0,0,2048), CONTENTS_SOLID, this, COLLISION_GROUP_NONE, &tr );
	if ( !tr.DidHitWorld() )
	{
		DevWarning( "CPropLantern::CreateRope: can't find solid surface for rope '%s'\n", GetDebugName() );
		return;
	}

#if 1
	CRopeKeyframe *pRope = CRopeKeyframe::CreateWithStartPosition( tr.endpos, this, nAttach, 2, "cable/cablerope.vmt", 5 );
	CBaseEntity *pEnt = pRope;
	if ( pRope )
	{
#else
	CBaseEntity *pEnt = CBaseEntity::Create( "info_target", tr.endpos, vec3_angle );
	if ( pEnt == NULL )
	{
		DevWarning( "CPropLantern::CreateRope: failed to create rope entity '%s'\n", GetDebugName() );
		return;
	}
	CRopeKeyframe *pRope = CRopeKeyframe::Create( pEnt, this, 0, nAttach, 2, "cable/cablerope.vmt", 5 );
	if ( pRope )
	{
#endif
//		pRope->SetupHangDistance( (pEnt->GetAbsOrigin() - vAttach).Length() );
		pRope->SetupHangDistance( 0.0f );
		pRope->EnableWind( false );
		pRope->m_RopeLength = (pEnt->GetAbsOrigin() - vAttach).Length();
//		pRope->m_Slack = pRope->m_RopeLength * hoe_lantern_rope_slack.GetFloat();
		pRope->m_TextureScale.Set( 1.0f );
		m_hRope = pRope;
	}

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
#if 0
#define CRANE_SPRING_CONSTANT_HANGING			2e5f
#define CRANE_SPRING_DAMPING					2e5f
#define CRANE_SPRING_RELATIVE_DAMPING			2
	springparams_t spring;
	spring.constant = CRANE_SPRING_CONSTANT_HANGING;
	spring.damping = CRANE_SPRING_DAMPING;
	spring.naturalLength = (pEnt->GetAbsOrigin() - vAttach).Length();
	spring.relativeDamping = CRANE_SPRING_RELATIVE_DAMPING;
	spring.startPosition = pEnt->GetAbsOrigin();
	spring.endPosition = vAttach;
	spring.useLocalPositions = false;
	spring.onlyStretch = true;
	/*m_pConstraint =*/ physenv->CreateSpring( g_PhysWorldObject, pPhysicsObject, &spring );
#else
		// Create our constraint group
		constraint_groupparams_t group;
		group.Defaults();
		m_pConstraintGroup = physenv->CreateConstraintGroup( group );

		constraint_lengthparams_t length;
		length.Defaults();
		length.InitWorldspace( g_PhysWorldObject, pPhysicsObject, pEnt->GetAbsOrigin(), vAttach );
		m_pConstraint = physenv->CreateLengthConstraint( g_PhysWorldObject, pPhysicsObject, m_pConstraintGroup, length );

		m_pConstraintGroup->Activate();
#endif
	}
}

//-----------------------------------------------------------------------------
void CPropLantern::SetBrokenModel( void )
{
	if ( m_pConstraint )
	{
		physenv->DestroyConstraint( m_pConstraint );
		m_pConstraint = NULL;
	}

	Vector velocity;
	AngularImpulse angular;
	GetVelocity( &velocity, &angular );

	VPhysicsDestroyObject();

	SetModel( GetBrokenModel() );

	CreateVPhysics();

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( m_hRope && pPhysicsObject )
	{
		// Stop extreme motion on the end of a rope
		const float rotdamping = LANTERN_ROTDAMPING;
		pPhysicsObject->SetDamping( NULL, &rotdamping );

		int nAttach = LookupAttachment( "rope" );
		Vector vAttach;
		GetAttachment( nAttach, vAttach );

		constraint_lengthparams_t length;
		length.Defaults();
		length.InitWorldspace( g_PhysWorldObject, pPhysicsObject, m_hRope->GetAbsOrigin(), vAttach );
		m_pConstraint = physenv->CreateLengthConstraint( g_PhysWorldObject, pPhysicsObject, m_pConstraintGroup, length );
	}

	if ( pPhysicsObject )
		pPhysicsObject->SetVelocity( &velocity, &angular );
}

#define DYING_DURATION 1.5f

//-----------------------------------------------------------------------------
void CPropLantern::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage = DAMAGE_EVENTS_ONLY;
//	m_lifeState = LIFE_DYING;

	SetBrokenModel();
	m_bBroken = true;
	m_bOn = false;

//	variant_t value;
//	value.SetInt( 7 );
//	m_hLight->AcceptInput( "SetStyle", this, this, value, 0 );

	if ( m_hSprite )
		m_hSprite->FadeAndDie( DYING_DURATION );

	m_flTimeKilled = gpGlobals->curtime;
	SetThink( &CPropLantern::DyingThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
void CPropLantern::DyingThink( void )
{
	if ( !m_hLight || (m_flTimeKilled < gpGlobals->curtime - DYING_DURATION) )
	{
		UTIL_Remove( m_hLight );
		UTIL_Remove( m_hSprite );
		SetThink( NULL );
		return;
	}

	float ratio = 1.0 - (gpGlobals->curtime - m_flTimeKilled) / DYING_DURATION;

	if ( ratio <= 0.25 )
		m_nSkin = 1; // turn off fullbright hack

	float flFlicker = cosf( gpGlobals->curtime * 7.0f ) * sinf( gpGlobals->curtime * 25.0f );
	flFlicker = clamp( flFlicker, 0.5, 1.0 );

	variant_t value;
	value.SetFloat( ratio * m_Radius );
	m_hLight->AcceptInput( "distance", this, this, value, 0 );

	value.SetFloat( m_Exponent * flFlicker );
	m_hLight->AcceptInput( "brightness", this, this, value, 0 );

	SetNextThink( gpGlobals->curtime + 0.05 );
}

//-----------------------------------------------------------------------------
const char *CPropLantern::GetBrokenModel( void )
{
	static const char *szModel[] = { LANTERN_MODEL_BROKEN, LANTERN2_MODEL_BROKEN };
	if ( m_nLanternModel < 0 || m_nLanternModel > ARRAYSIZE(szModel) )
		return "models/error.mdl";
	return szModel[m_nLanternModel];
}