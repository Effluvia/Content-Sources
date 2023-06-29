//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basehlplayer.h"
#include "playerandobjectenumerator.h"
#include "engine/ivdebugoverlay.h"
#include "c_ai_basenpc.h"
#include "in_buttons.h"
#include "collisionutils.h"

#ifdef HOE_THIRDPERSON
#include "c_basetempentity.h"
#include "prediction.h"
#include "tempent.h" // laser dot
#include "view.h" // laser dot
#include "bone_setup.h"
#include "dlight.h" // flashlight elight
#include "iefx.h" // flashlight elight
#endif // HOE_THIRDPERSON

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// How fast to avoid collisions with center of other object, in units per second
#define AVOID_SPEED 2000.0f
extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;

extern ConVar zoom_sensitivity_ratio;
extern ConVar default_fov;
extern ConVar sensitivity;

ConVar cl_npc_speedmod_intime( "cl_npc_speedmod_intime", "0.25", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
ConVar cl_npc_speedmod_outtime( "cl_npc_speedmod_outtime", "1.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

IMPLEMENT_CLIENTCLASS_DT(C_BaseHLPlayer, DT_HL2_Player, CHL2_Player)
	RecvPropDataTable( RECVINFO_DT(m_HL2Local),0, &REFERENCE_RECV_TABLE(DT_HL2Local) ),
	RecvPropBool( RECVINFO( m_fIsSprinting ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseHLPlayer )
	DEFINE_PRED_TYPEDESCRIPTION( m_HL2Local, C_HL2PlayerLocalData ),
	DEFINE_PRED_FIELD( m_fIsSprinting, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

//-----------------------------------------------------------------------------
// Purpose: Drops player's primary weapon
//-----------------------------------------------------------------------------
void CC_DropPrimary( void )
{
	C_BasePlayer *pPlayer = (C_BasePlayer *) C_BasePlayer::GetLocalPlayer();
	
	if ( pPlayer == NULL )
		return;

	pPlayer->Weapon_DropPrimary();
}

static ConCommand dropprimary("dropprimary", CC_DropPrimary, "dropprimary: Drops the primary weapon of the player.");

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_BaseHLPlayer::C_BaseHLPlayer()
{
	AddVar( &m_Local.m_vecPunchAngle, &m_Local.m_iv_vecPunchAngle, LATCH_SIMULATION_VAR );
	AddVar( &m_Local.m_vecPunchAngleVel, &m_Local.m_iv_vecPunchAngleVel, LATCH_SIMULATION_VAR );

	m_flZoomStart		= 0.0f;
	m_flZoomEnd			= 0.0f;
	m_flZoomRate		= 0.0f;
	m_flZoomStartTime	= 0.0f;
	m_flSpeedMod		= cl_forwardspeed.GetFloat();

#ifdef HOE_THIRDPERSON
	m_PlayerAnimState = CreateHL2MPPlayerAnimState( this );

	// BUG? m_bClientSideAnimation hasn't come down from server when AddBaseAnimatingInterpolatedVars()
	// gets called in the C_BaseAnimating() constructor.
	// constructor.
	UseClientSideAnimation();
#endif // HOE_THIRDPERSON
}

#ifdef HOE_THIRDPERSON
C_BaseHLPlayer::~C_BaseHLPlayer()
{
	m_PlayerAnimState->Release();
}
#endif // HOE_THIRDPERSON

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::OnDataChanged( DataUpdateType_t updateType )
{
	// Make sure we're thinking
	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Weapon_DropPrimary( void )
{
	engine->ServerCmd( "DropPrimary" );
}

float C_BaseHLPlayer::GetFOV()
{
	//Find our FOV with offset zoom value
	float flFOVOffset = BaseClass::GetFOV() + GetZoom();

	// Clamp FOV in MP
	int min_fov = ( gpGlobals->maxClients == 1 ) ? 5 : default_fov.GetInt();
	
	// Don't let it go too low
	flFOVOffset = MAX( min_fov, flFOVOffset );

	return flFOVOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float C_BaseHLPlayer::GetZoom( void )
{
	float fFOV = m_flZoomEnd;

	//See if we need to lerp the values
	if ( ( m_flZoomStart != m_flZoomEnd ) && ( m_flZoomRate > 0.0f ) )
	{
		float deltaTime = (float)( gpGlobals->curtime - m_flZoomStartTime ) / m_flZoomRate;

		if ( deltaTime >= 1.0f )
		{
			//If we're past the zoom time, just take the new value and stop lerping
			fFOV = m_flZoomStart = m_flZoomEnd;
		}
		else
		{
			fFOV = SimpleSplineRemapVal( deltaTime, 0.0f, 1.0f, m_flZoomStart, m_flZoomEnd );
		}
	}

	return fFOV;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : FOVOffset - 
//			time - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Zoom( float FOVOffset, float time )
{
	m_flZoomStart		= GetZoom();
	m_flZoomEnd			= FOVOffset;
	m_flZoomRate		= time;
	m_flZoomStartTime	= gpGlobals->curtime;
}


//-----------------------------------------------------------------------------
// Purpose: Hack to zero out player's pitch, use value from poseparameter instead
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseHLPlayer::DrawModel( int flags )
{
	// Not pitch for player
	QAngle saveAngles = GetLocalAngles();

	QAngle useAngles = saveAngles;
	useAngles[ PITCH ] = 0.0f;

	SetLocalAngles( useAngles );

	int iret = BaseClass::DrawModel( flags );

#ifdef HOE_THIRDPERSON // HOE: beam to show aim
	if( this == C_BasePlayer::GetLocalPlayer() )
	{
#if 0
		if( !m_pAimBeam )
		{
			BeamInfo_t beamInfo;

			beamInfo.m_vecStart = EyePosition();
			beamInfo.m_vecEnd = EyePosition() + Vector(100);

			beamInfo.m_pStartEnt = NULL;
			beamInfo.m_pEndEnt = NULL;
			beamInfo.m_nStartAttachment = 0;
			beamInfo.m_nEndAttachment = 0;

			beamInfo.m_pszModelName = "effects/bluelaser1.vmt";
			beamInfo.m_pszHaloName = NULL; /*"sprites/light_glow03.vmt"*/;
			beamInfo.m_flHaloScale = 16.0f;
			beamInfo.m_flLife = 0.0f;
			beamInfo.m_flWidth = 1;
			beamInfo.m_flEndWidth = 1;
			beamInfo.m_flFadeLength = 0.0f;
			beamInfo.m_flAmplitude = 0;
			beamInfo.m_flBrightness = 48.0;
			beamInfo.m_flSpeed = 0.0;
			beamInfo.m_nStartFrame = 0.0;
			beamInfo.m_flFrameRate = 1.0f;

			beamInfo.m_flRed = 255.0f;
			beamInfo.m_flGreen = 255.0f;
			beamInfo.m_flBlue = 255.0f;

			beamInfo.m_nSegments = -1;
			beamInfo.m_bRenderable = true;
			beamInfo.m_nFlags = FBEAM_SHADEOUT|FBEAM_NOTILE|FBEAM_HALOBEAM;
			
			m_pAimBeam = beams->CreateBeamPoints( beamInfo );
		}

		if( m_pAimBeam )
		{
			BeamInfo_t beamInfo;
			beamInfo.m_vecStart = Weapon_ShootPosition();

			QAngle eyeAngles = EyeAngles();
			Vector vForward;
			AngleVectors( eyeAngles, &vForward );
			beamInfo.m_vecEnd = beamInfo.m_vecStart + vForward * 512;

			beamInfo.m_flRed = 255.0;
			beamInfo.m_flGreen = 255.0;
			beamInfo.m_flBlue = 255.0;

			beams->UpdateBeamInfo( m_pAimBeam, beamInfo );
		}
#endif
#if 0 // laser dot
		if ( !m_pAimLaserDot )
		{
			m_pAimLaserDot = tempents->TempSprite( vec3_origin, vec3_origin, 0.5f,
				modelinfo->GetModelIndex( "sprites/redglow1.vmt" ), kRenderTransAdd, 0,
					1.0f, 1, FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_NEVERDIE );
		}
		if ( m_pAimLaserDot )
		{
			QAngle eyeAngles = EyeAngles();
			Vector vForward;
			AngleVectors( eyeAngles, &vForward );
//			VectorNormalize( vForward );
			Vector vStart = Weapon_ShootPosition();
			Vector vEnd = vStart + vForward * MAX_TRACE_LENGTH;
			trace_t tr;
			UTIL_TraceLine( vStart, vEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
			m_pAimLaserDot->SetAbsOrigin( tr.endpos - 10 * vForward/* tr.plane.normal */ );
//			m_pAimLaserDot->SetAbsAngles( CurrentViewAngles() );
			float dist = (tr.endpos - vStart).Length();
			float	scale = RemapVal( dist, 32, 1024, 0.01f, 0.5f );
			float	scaleOffs = random->RandomFloat( -scale * 0.25f, scale * 0.25f );
			scale = clamp( scale + scaleOffs, 0.1f, 32.0f );
			m_pAimLaserDot->m_flSpriteScale = scale;
		}
#endif
	}
#endif

	SetLocalAngles( saveAngles );

	return iret;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::ExitLadder()
{
	if ( MOVETYPE_LADDER != GetMoveType() )
		return;
	
	SetMoveType( MOVETYPE_WALK );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
	// Remove from ladder
	m_HL2Local.m_hLadder = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Determines if a player can be safely moved towards a point
// Input:   pos - position to test move to, fVertDist - how far to trace downwards to see if the player would fall,
//			radius - how close the player can be to the object, objPos - position of the object to avoid,
//			objDir - direction the object is travelling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::TestMove( const Vector &pos, float fVertDist, float radius, const Vector &objPos, const Vector &objDir )
{
	trace_t trUp;
	trace_t trOver;
	trace_t trDown;
	float flHit1, flHit2;
	
	UTIL_TraceHull( GetAbsOrigin(), pos, GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );
	if ( trOver.fraction < 1.0f )
	{
		// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
		if ( objDir.IsZero() ||
			( IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && 
			( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) )
			)
		{
			// our first trace failed, so see if we can go farther if we step up.

			// trace up to see if we have enough room.
			UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, m_Local.m_flStepSize ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trUp );

			// do a trace from the stepped up height
			UTIL_TraceHull( trUp.endpos, pos + Vector( 0, 0, trUp.endpos.z - trUp.startpos.z ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );

			if ( trOver.fraction < 1.0f )
			{
				// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
				if ( objDir.IsZero() ||
					( IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && ( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) ) )
				{
					return false;
				}
			}
		}
	}

	// trace down to see if this position is on the ground
	UTIL_TraceLine( trOver.endpos, trOver.endpos - Vector( 0, 0, fVertDist ), 
		MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &trDown );

	if ( trDown.fraction == 1.0f ) 
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Client-side obstacle avoidance
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::PerformClientSideObstacleAvoidance( float flFrameTime, CUserCmd *pCmd )
{
	// Don't avoid if noclipping or in movetype none
	switch ( GetMoveType() )
	{
	case MOVETYPE_NOCLIP:
	case MOVETYPE_NONE:
	case MOVETYPE_OBSERVER:
		return;
	default:
		break;
	}

	// Try to steer away from any objects/players we might interpenetrate
	Vector size = WorldAlignSize();

	float radius = 0.7f * sqrt( size.x * size.x + size.y * size.y );
	float curspeed = GetLocalVelocity().Length2D();

	//int slot = 1;
	//engine->Con_NPrintf( slot++, "speed %f\n", curspeed );
	//engine->Con_NPrintf( slot++, "radius %f\n", radius );

	// If running, use a larger radius
	float factor = 1.0f;

	if ( curspeed > 150.0f )
	{
		curspeed = MIN( 2048.0f, curspeed );
		factor = ( 1.0f + ( curspeed - 150.0f ) / 150.0f );

		//engine->Con_NPrintf( slot++, "scaleup (%f) to radius %f\n", factor, radius * factor );

		radius = radius * factor;
	}

	Vector currentdir;
	Vector rightdir;

	QAngle vAngles = pCmd->viewangles;
	vAngles.x = 0;

	AngleVectors( vAngles, &currentdir, &rightdir, NULL );
		
	bool istryingtomove = false;
	bool ismovingforward = false;
	if ( fabs( pCmd->forwardmove ) > 0.0f || 
		fabs( pCmd->sidemove ) > 0.0f )
	{
		istryingtomove = true;
		if ( pCmd->forwardmove > 1.0f )
		{
			ismovingforward = true;
		}
	}

	if ( istryingtomove == true )
		 radius *= 1.3f;

	CPlayerAndObjectEnumerator avoid( radius );
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_SOLID_EDICTS, GetAbsOrigin(), radius, false, &avoid );

	// Okay, decide how to avoid if there's anything close by
	int c = avoid.GetObjectCount();
	if ( c <= 0 )
		return;

	//engine->Con_NPrintf( slot++, "moving %s forward %s\n", istryingtomove ? "true" : "false", ismovingforward ? "true" : "false"  );

	float adjustforwardmove = 0.0f;
	float adjustsidemove	= 0.0f;

	for ( int i = 0; i < c; i++ )
	{
		C_AI_BaseNPC *obj = dynamic_cast< C_AI_BaseNPC *>(avoid.GetObject( i ));

		if( !obj )
			continue;

		Vector vecToObject = obj->GetAbsOrigin() - GetAbsOrigin();

		float flDist = vecToObject.Length2D();
		
		// Figure out a 2D radius for the object
		Vector vecWorldMins, vecWorldMaxs;
		obj->CollisionProp()->WorldSpaceAABB( &vecWorldMins, &vecWorldMaxs );
		Vector objSize = vecWorldMaxs - vecWorldMins;

		float objectradius = 0.5f * sqrt( objSize.x * objSize.x + objSize.y * objSize.y );

		//Don't run this code if the NPC is not moving UNLESS we are in stuck inside of them.
		if ( !obj->IsMoving() && flDist > objectradius )
			  continue;

		if ( flDist > objectradius && obj->IsEffectActive( EF_NODRAW ) )
		{
			obj->RemoveEffects( EF_NODRAW );
		}

		Vector vecNPCVelocity;
		obj->EstimateAbsVelocity( vecNPCVelocity );
		float flNPCSpeed = VectorNormalize( vecNPCVelocity );

		Vector vPlayerVel = GetAbsVelocity();
		VectorNormalize( vPlayerVel );

		float flHit1, flHit2;
		Vector vRayDir = vecToObject;
		VectorNormalize( vRayDir );

		float flVelProduct = DotProduct( vecNPCVelocity, vPlayerVel );
		float flDirProduct = DotProduct( vRayDir, vPlayerVel );

		if ( !IntersectInfiniteRayWithSphere(
				GetAbsOrigin(),
				vRayDir,
				obj->GetAbsOrigin(),
				radius,
				&flHit1,
				&flHit2 ) )
			continue;

        Vector dirToObject = -vecToObject;
		VectorNormalize( dirToObject );

		float fwd = 0;
		float rt = 0;

		float sidescale = 2.0f;
		float forwardscale = 1.0f;
		bool foundResult = false;

		Vector vMoveDir = vecNPCVelocity;
		if ( flNPCSpeed > 0.001f )
		{
			// This NPC is moving. First try deflecting the player left or right relative to the NPC's velocity.
			// Start with whatever side they're on relative to the NPC's velocity.
			Vector vecNPCTrajectoryRight = CrossProduct( vecNPCVelocity, Vector( 0, 0, 1) );
			int iDirection = ( vecNPCTrajectoryRight.Dot( dirToObject ) > 0 ) ? 1 : -1;
			for ( int nTries = 0; nTries < 2; nTries++ )
			{
				Vector vecTryMove = vecNPCTrajectoryRight * iDirection;
				VectorNormalize( vecTryMove );
				
				Vector vTestPosition = GetAbsOrigin() + vecTryMove * radius * 2;

				if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
				{
					fwd = currentdir.Dot( vecTryMove );
					rt = rightdir.Dot( vecTryMove );
					
					//Msg( "PUSH DEFLECT fwd=%f, rt=%f\n", fwd, rt );
					
					foundResult = true;
					break;
				}
				else
				{
					// Try the other direction.
					iDirection *= -1;
				}
			}
		}
		else
		{
			// the object isn't moving, so try moving opposite the way it's facing
			Vector vecNPCForward;
			obj->GetVectors( &vecNPCForward, NULL, NULL );
			
			Vector vTestPosition = GetAbsOrigin() - vecNPCForward * radius * 2;
			if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
			{
				fwd = currentdir.Dot( -vecNPCForward );
				rt = rightdir.Dot( -vecNPCForward );

				if ( flDist < objectradius )
				{
					obj->AddEffects( EF_NODRAW );
				}

				//Msg( "PUSH AWAY FACE fwd=%f, rt=%f\n", fwd, rt );

				foundResult = true;
			}
		}

		if ( !foundResult )
		{
			// test if we can move in the direction the object is moving
			Vector vTestPosition = GetAbsOrigin() + vMoveDir * radius * 2;
			if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
			{
				fwd = currentdir.Dot( vMoveDir );
				rt = rightdir.Dot( vMoveDir );

				if ( flDist < objectradius )
				{
					obj->AddEffects( EF_NODRAW );
				}

				//Msg( "PUSH ALONG fwd=%f, rt=%f\n", fwd, rt );

				foundResult = true;
			}
			else
			{
				// try moving directly away from the object
				Vector vTestPosition = GetAbsOrigin() - dirToObject * radius * 2;
				if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
				{
					fwd = currentdir.Dot( -dirToObject );
					rt = rightdir.Dot( -dirToObject );
					foundResult = true;

					//Msg( "PUSH AWAY fwd=%f, rt=%f\n", fwd, rt );
				}
			}
		}

		if ( !foundResult )
		{
			// test if we can move through the object
			Vector vTestPosition = GetAbsOrigin() - vMoveDir * radius * 2;
			fwd = currentdir.Dot( -vMoveDir );
			rt = rightdir.Dot( -vMoveDir );

			if ( flDist < objectradius )
			{
				obj->AddEffects( EF_NODRAW );
			}

			//Msg( "PUSH THROUGH fwd=%f, rt=%f\n", fwd, rt );

			foundResult = true;
		}

		// If running, then do a lot more sideways veer since we're not going to do anything to
		//  forward velocity
		if ( istryingtomove )
		{
			sidescale = 6.0f;
		}

		if ( flVelProduct > 0.0f && flDirProduct > 0.0f )
		{
			sidescale = 0.1f;
		}

		float force = 1.0f;
		float forward = forwardscale * fwd * force * AVOID_SPEED;
		float side = sidescale * rt * force * AVOID_SPEED;

		adjustforwardmove	+= forward;
		adjustsidemove		+= side;
	}

	pCmd->forwardmove	+= adjustforwardmove;
	pCmd->sidemove		+= adjustsidemove;
	
	// Clamp the move to within legal limits, preserving direction. This is a little
	// complicated because we have different limits for forward, back, and side

	//Msg( "PRECLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );

	float flForwardScale = 1.0f;
	if ( pCmd->forwardmove > fabs( cl_forwardspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_forwardspeed.GetFloat() ) / pCmd->forwardmove;
	}
	else if ( pCmd->forwardmove < -fabs( cl_backspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_backspeed.GetFloat() ) / fabs( pCmd->forwardmove );
	}
	
	float flSideScale = 1.0f;
	if ( fabs( pCmd->sidemove ) > fabs( cl_sidespeed.GetFloat() ) )
	{
		flSideScale = fabs( cl_sidespeed.GetFloat() ) / fabs( pCmd->sidemove );
	}
	
	float flScale = MIN( flForwardScale, flSideScale );
	pCmd->forwardmove *= flScale;
	pCmd->sidemove *= flScale;

	//Msg( "POSTCLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );
}


void C_BaseHLPlayer::PerformClientSideNPCSpeedModifiers( float flFrameTime, CUserCmd *pCmd )
{
	if ( m_hClosestNPC == NULL )
	{
		if ( m_flSpeedMod != cl_forwardspeed.GetFloat() )
		{
			float flDeltaTime = (m_flSpeedModTime - gpGlobals->curtime);
			m_flSpeedMod = RemapValClamped( flDeltaTime, cl_npc_speedmod_outtime.GetFloat(), 0, m_flExitSpeedMod, cl_forwardspeed.GetFloat() );
		}
	}
	else
	{
		C_AI_BaseNPC *pNPC = dynamic_cast< C_AI_BaseNPC *>( m_hClosestNPC.Get() );

		if ( pNPC )
		{
			float flDist = (GetAbsOrigin() - pNPC->GetAbsOrigin()).LengthSqr();
			bool bShouldModSpeed = false; 

			// Within range?
			if ( flDist < pNPC->GetSpeedModifyRadius() )
			{
				// Now, only slowdown if we're facing & running parallel to the target's movement
				// Facing check first (in 2D)
				Vector vecTargetOrigin = pNPC->GetAbsOrigin();
				Vector los = ( vecTargetOrigin - EyePosition() );
				los.z = 0;
				VectorNormalize( los );
				Vector facingDir;
				AngleVectors( GetAbsAngles(), &facingDir );
				float flDot = DotProduct( los, facingDir );
				if ( flDot > 0.8 )
				{
					/*
					// Velocity check (abort if the target isn't moving)
					Vector vecTargetVelocity;
					pNPC->EstimateAbsVelocity( vecTargetVelocity );
					float flSpeed = VectorNormalize(vecTargetVelocity);
					Vector vecMyVelocity = GetAbsVelocity();
					VectorNormalize(vecMyVelocity);
					if ( flSpeed > 1.0 )
					{
						// Velocity roughly parallel?
						if ( DotProduct(vecTargetVelocity,vecMyVelocity) > 0.4  )
						{
							bShouldModSpeed = true;
						}
					} 
					else
					{
						// NPC's not moving, slow down if we're moving at him
						//Msg("Dot: %.2f\n", DotProduct( los, vecMyVelocity ) );
						if ( DotProduct( los, vecMyVelocity ) > 0.8 )
						{
							bShouldModSpeed = true;
						} 
					}
					*/

					bShouldModSpeed = true;
				}
			}

			if ( !bShouldModSpeed )
			{
				m_hClosestNPC = NULL;
				m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_outtime.GetFloat();
				m_flExitSpeedMod = m_flSpeedMod;
				return;
			}
			else 
			{
				if ( m_flSpeedMod != pNPC->GetSpeedModifySpeed() )
				{
					float flDeltaTime = (m_flSpeedModTime - gpGlobals->curtime);
					m_flSpeedMod = RemapValClamped( flDeltaTime, cl_npc_speedmod_intime.GetFloat(), 0, cl_forwardspeed.GetFloat(), pNPC->GetSpeedModifySpeed() );
				}
			}
		}
	}

	if ( pCmd->forwardmove > 0.0f )
	{
		pCmd->forwardmove = clamp( pCmd->forwardmove, -m_flSpeedMod, m_flSpeedMod );
	}
	else
	{
		pCmd->forwardmove = clamp( pCmd->forwardmove, -m_flSpeedMod, m_flSpeedMod );
	}
	pCmd->sidemove = clamp( pCmd->sidemove, -m_flSpeedMod, m_flSpeedMod );
   
	//Msg( "fwd %f right %f\n", pCmd->forwardmove, pCmd->sidemove );
}

//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{
	bool bResult = BaseClass::CreateMove( flInputSampleTime, pCmd );

	if ( !IsInAVehicle() )
	{
		PerformClientSideObstacleAvoidance( TICK_INTERVAL, pCmd );
		PerformClientSideNPCSpeedModifiers( TICK_INTERVAL, pCmd );
	}

	return bResult;
}

#ifdef HOE_THIRDPERSON

//=========================================================
// Purpose: from input of 75% to 200% of maximum range, rescale smoothly from 75% to 100%
//=========================================================
// copied from CBaseAnimating
float C_BaseHLPlayer::EdgeLimitPoseParameter( int iParameter, float flValue, float flBase )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );
	if ( !pstudiohdr )
	{
		return flValue;
	}

	if (iParameter < 0 || iParameter >= pstudiohdr->GetNumPoseParameters())
	{
		return flValue;
	}

	const mstudioposeparamdesc_t &Pose = pstudiohdr->pPoseParameter( iParameter );

	if (Pose.loop || Pose.start == Pose.end)
	{
		return flValue;
	}

	return RangeCompressor( flValue, Pose.start, Pose.end, flBase );
}

//-----------------------------------------------------------------------------
// copied from CBaseAnimating
void C_BaseHLPlayer::UpdateLookAt( void )
{
	if ( gpGlobals->frametime == 0 ) return;

	if ( m_headYawPoseParam < 0 || m_headPitchPoseParam < 0 )
		return;
	
	if ( m_iAttachmentEyes < 0 || m_iAttachmentForward < 0 )
		return;

	// orient eyes
	m_viewtarget = m_vLookAtTarget;

#if 0
	// blinking
	if (m_blinkTimer.IsElapsed())
	{
		m_blinktoggle = !m_blinktoggle;
		m_blinkTimer.Start( RandomFloat( 1.5f, 4.0f ) );
	}
#endif

	matrix3x4_t eyesToWorld;
	GetAttachment( m_iAttachmentEyes, eyesToWorld );
	
	matrix3x4_t forwardToWorld, worldToForward;
	GetAttachment( m_iAttachmentForward, forwardToWorld );
	MatrixInvert( forwardToWorld, worldToForward );

	QAngle angBias( 0, 0, 0 );

	// Figure out where we want to look in world space.
	Vector to = m_vLookAtTarget - EyePosition();

	matrix3x4_t targetXform;
	targetXform = forwardToWorld;
	Vector vTargetDir = m_vLookAtTarget - EyePosition();

	float flHeadInfluence = 1.0f;

	if ( 1 /*scene_clamplookat.GetBool()*/ )
	{
		// scale down pitch when the target is behind the head
		Vector vTargetLocal;
		VectorNormalize( vTargetDir );
		VectorIRotate( vTargetDir, forwardToWorld, vTargetLocal );
		vTargetLocal.z *= clamp( vTargetLocal.x, 0.1, 1.0 );
		VectorNormalize( vTargetLocal );
		VectorRotate( vTargetLocal, forwardToWorld, vTargetDir );

		// clamp local influence when target is behind the head
		flHeadInfluence = flHeadInfluence * clamp( vTargetLocal.x * 2.0 + 2.0, 0.0, 1.0 );
	}

	Studio_AlignIKMatrix( targetXform, vTargetDir );

	matrix3x4_t headXform;
	ConcatTransforms( worldToForward, targetXform, headXform );
	QAngle vTargetAngles;
	MatrixAngles( headXform, vTargetAngles );

	// partially debounce head goal
	float s0 = 1.0 - flHeadInfluence + 0.3/*GetHeadDebounce()*/ * flHeadInfluence;
	float s1 = (1.0 - s0);

	// limit velocity of head turns
	m_goalHeadCorrection.x = Approach( m_goalHeadCorrection.x * s0 + vTargetAngles.x * s1, m_goalHeadCorrection.x, 10.0 );
	m_goalHeadCorrection.y = Approach( m_goalHeadCorrection.y * s0 + vTargetAngles.y * s1, m_goalHeadCorrection.y, 30.0 );
	m_goalHeadCorrection.z = Approach( m_goalHeadCorrection.z * s0 + vTargetAngles.z * s1, m_goalHeadCorrection.z, 10.0 );
	
	float flTarget;
	float flLimit;

	flTarget = m_goalHeadCorrection.y /*+ Get( m_FlexweightHeadRightLeft )*/;
	flLimit = EdgeLimitPoseParameter( m_headYawPoseParam, flTarget, angBias.y );
	SetPoseParameter( m_headYawPoseParam, flLimit );

	flTarget = m_goalHeadCorrection.x /*+ Get( m_FlexweightHeadUpDown )*/;
	flLimit = EdgeLimitPoseParameter( m_headPitchPoseParam, flTarget, angBias.x );
	SetPoseParameter( m_headPitchPoseParam, flLimit );
}

void C_BaseHLPlayer::ClientThink( void )
{
	if ( gpGlobals->frametime == 0 ) return;

	bool bFoundViewTarget = false;
	
	Vector vForward;
#if 0
	AngleVectors( GetRenderAngles(), &vForward ); 
#else
	AngleVectors( GetLocalAngles(), &vForward );

	// FIXME: Coming back from main menu to game sometimes gives this...
	if ( vForward == Vector(1,0,0) ) return;
#endif
	for( int iClient = 1; iClient <= gpGlobals->maxClients; ++iClient )
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex( iClient );
		if(!pEnt || !pEnt->IsPlayer())
			continue;

		if ( pEnt->entindex() == entindex() )
			continue;

		Vector vTargetOrigin = pEnt->GetAbsOrigin();
		Vector vMyOrigin =  GetAbsOrigin();

		Vector vDir = vTargetOrigin - vMyOrigin;
		
		if ( vDir.Length() > 128 ) 
			continue;

		VectorNormalize( vDir );

		if ( DotProduct( vForward, vDir ) < 0.0f )
			 continue;

		m_vLookAtTarget = pEnt->EyePosition();
		bFoundViewTarget = true;
		break;
	}

	if ( bFoundViewTarget == false )
	{
		m_vLookAtTarget = GetAbsOrigin() + vForward * 512;
	}
#if 0
	UpdateIDTarget();

	// Avoidance
	if ( gpGlobals->curtime >= m_fNextThinkPushAway )
	{
		PerformObstaclePushaway( this );
		m_fNextThinkPushAway =  gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL;
	}
#endif

	UpdateFlashlightElight();
}

//-----------------------------------------------------------------------------
// Should this object receive shadows?
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::ShouldReceiveProjectedTextures( int flags )
{
	Assert( flags & SHADOW_FLAGS_PROJECTED_TEXTURE_TYPE_MASK );

	if ( IsEffectActive( EF_NODRAW ) )
		 return false;

	if( flags & SHADOW_FLAGS_FLASHLIGHT )
	{
return false; // false==my own flashlight, true==chumtoad flashlight
		return true;
	}

	return BaseClass::ShouldReceiveProjectedTextures( flags );
}

ShadowType_t C_BaseHLPlayer::ShadowCastType( void ) 
{
	if ( !IsVisible() )
		 return SHADOWS_NONE;

	return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
}

const QAngle& C_BaseHLPlayer::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}

void C_BaseHLPlayer::UpdateClientSideAnimation()
{
	m_PlayerAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );

UpdateLookAt();

	BaseClass::UpdateClientSideAnimation();
}

CStudioHdr *C_BaseHLPlayer::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();

	InitializePoseParams();

	m_iAttachmentEyes = LookupAttachment( "eyes" );
	m_iAttachmentChest = -1; // not in player model
	m_iAttachmentForward = LookupAttachment( "forward" );

	// Reset the players animation states, gestures
	if ( m_PlayerAnimState )
	{
		m_PlayerAnimState->OnNewModel();
	}

	return hdr;
}

//-----------------------------------------------------------------------------
// Purpose: Clear all pose parameters
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::InitializePoseParams( void )
{
	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );

	CStudioHdr *hdr = GetModelPtr();
	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}
}
// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_BaseHLPlayer *pPlayer = dynamic_cast< C_BaseHLPlayer* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}	
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()

void C_BaseHLPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( IsLocalPlayer() )
	{
		if ( ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() ) )
			return;
	}

	MDLCACHE_CRITICAL_SECTION();
	m_PlayerAnimState->DoAnimationEvent( event, nData );
}

void C_BaseHLPlayer::UpdateFlashlightElight( void )
{
	// The dim light is the flashlight.
	if ( IsEffectActive( EF_DIMLIGHT ) )
	{
		if ( m_FlashlightElightKey == 0 )
			StartFlashlightElight();
	}
	else
	{
		if ( m_FlashlightElightKey != 0 )
			StopFlashlightElight();
	}

	// if the elight has gone away, bail out
	if (m_FlashlightElightKey == 0)
	{
//		SetNextClientThink( CLIENT_THINK_NEVER );
		return;
	}

	// get the elight
	dlight_t * el = effects->GetElightByKey(m_FlashlightElightKey);
	if (!el)
	{
		// the elight has been invalidated. bail out.
		m_FlashlightElightKey = 0;

//		SetNextClientThink( CLIENT_THINK_NEVER );
		return;
	}
	else
	{
		Vector vecForward, vecRight, vecUp;
		EyeVectors( &vecForward, &vecRight, &vecUp );

		el->origin = WorldSpaceCenter() - vecForward * 32;

		el->color.exponent = 3.0f;
		el->radius = 6.0f * 12.0f;
	}
}

void C_BaseHLPlayer::StartFlashlightElight( void )
{
	AssertMsg(m_FlashlightElightKey == 0 , "Advisor trying to create new elight on top of old one!");
	if ( m_FlashlightElightKey != 0 )
	{
		Warning("Advisor tried to start his elight when it was already one.\n");
	}
	else
	{
		m_FlashlightElightKey = LIGHT_INDEX_TE_DYNAMIC + this->entindex();
		dlight_t * el = effects->CL_AllocElight( m_FlashlightElightKey );

		if ( el )
		{
			// create an elight on top of me
			el->origin	= this->WorldSpaceCenter();

			el->color.r = 235;
			el->color.g = 255;
			el->color.b = 255;
			el->color.exponent = 3;

			el->radius	= 52*12;
			el->decay	= 0.0f;
			el->die = gpGlobals->curtime + 2000.0f; // 1000 just means " a long time "

//			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
		else
		{	// null out the light value
			m_FlashlightElightKey = 0;
		}
	}
}

void C_BaseHLPlayer::StopFlashlightElight( void )
{
	AssertMsg( m_FlashlightElightKey != 0, "Advisor tried to stop elight when none existed!");
	dlight_t * el;
	// note: the following conditional sets el if not short-circuited
	if ( m_FlashlightElightKey == 0 || (el = effects->GetElightByKey(m_FlashlightElightKey)) == NULL ) 
	{
		Warning("Advisor tried to stop its elight when it had none.\n");
	}
	else
	{
		// kill the elight by setting the die value to now
		el->die = gpGlobals->curtime;
	}
}

#endif // HOE_THIRDPERSON

//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::BuildTransformations( CStudioHdr *hdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	BaseClass::BuildTransformations( hdr, pos, q, cameraTransform, boneMask, boneComputed );
	BuildFirstPersonMeathookTransformations( hdr, pos, q, cameraTransform, boneMask, boneComputed, "ValveBiped.Bip01_Head1" );
}

