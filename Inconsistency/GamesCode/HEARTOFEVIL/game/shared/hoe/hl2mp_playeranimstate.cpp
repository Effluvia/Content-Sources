//========= Copyright � 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "base_playeranimstate.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"

#include "hl2mp_playeranimstate.h"
#include "base_playeranimstate.h"
#include "datacache/imdlcache.h"

#ifdef CLIENT_DLL
#include "c_basehlplayer.h"
#else
#include "hl2_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h" // HOE_THIRDPERSON added

#define HL2MP_RUN_SPEED				320.0f
#define HL2MP_WALK_SPEED			75.0f
#define HL2MP_CROUCHWALK_SPEED		110.0f

#undef PLAYER_9WAY // HOE_DLL
#undef PLAYER_DOD // DoD model test

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : CMultiPlayerAnimState*
//-----------------------------------------------------------------------------
CHL2MPPlayerAnimState* CreateHL2MPPlayerAnimState( HL2_PLAYER_CLASS *pPlayer )
{
	MDLCACHE_CRITICAL_SECTION();

	// Setup the movement data.
	MultiPlayerMovementData_t movementData;
	movementData.m_flBodyYawRate = 720.0f;
	movementData.m_flRunSpeed = HL2MP_RUN_SPEED;
	movementData.m_flWalkSpeed = HL2MP_WALK_SPEED;
	movementData.m_flSprintSpeed = -1.0f;

	// Create animation state for this player.
	CHL2MPPlayerAnimState *pRet = new CHL2MPPlayerAnimState( pPlayer, movementData );

	// Specific HL2MP player initialization.
	pRet->InitHL2MPAnimState( pPlayer );

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CHL2MPPlayerAnimState::CHL2MPPlayerAnimState()
{
	m_pHL2MPPlayer = NULL;

	// Don't initialize HL2MP specific variables here. Init them in InitHL2MPAnimState()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			&movementData - 
//-----------------------------------------------------------------------------
CHL2MPPlayerAnimState::CHL2MPPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData )
: CMultiPlayerAnimState( pPlayer, movementData )
{
	m_pHL2MPPlayer = NULL;

	// Don't initialize HL2MP specific variables here. Init them in InitHL2MPAnimState()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CHL2MPPlayerAnimState::~CHL2MPPlayerAnimState()
{
}

//-----------------------------------------------------------------------------
// Purpose: Initialize HL2MP specific animation state.
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CHL2MPPlayerAnimState::InitHL2MPAnimState( HL2_PLAYER_CLASS *pPlayer )
{
	m_pHL2MPPlayer = pPlayer;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPPlayerAnimState::ClearAnimationState( void )
{
	BaseClass::ClearAnimationState();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : actDesired - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CHL2MPPlayerAnimState::TranslateActivity( Activity actDesired )
{
	// Hook into baseclass when / if hl2mp player models get swim animations.
	Activity translateActivity = actDesired; //BaseClass::TranslateActivity( actDesired );

	if ( GetHL2MPPlayer()->GetActiveWeapon() )
	{
		translateActivity = GetHL2MPPlayer()->GetActiveWeapon()->ActivityOverride( translateActivity, false );
	}

#ifdef PLAYER_DOD
	switch (translateActivity)
	{
	case ACT_IDLE: return ACT_DOD_STAND_IDLE_BAR;
	case ACT_IDLE_SMG1: return ACT_DOD_STAND_IDLE_BAR;
	case ACT_IDLE_ANGRY: return ACT_DOD_STAND_AIM_BAR;
	case ACT_IDLE_ANGRY_MELEE: return ACT_DOD_STAND_AIM_SPADE;
	case ACT_RANGE_AIM_AR2_LOW: return ACT_DOD_STAND_AIM_MG;
	case ACT_RANGE_AIM_LOW: return ACT_DOD_CROUCH_AIM_BAR;
	case ACT_RUN_AIM_SHOTGUN: return ACT_DOD_RUN_AIM_MG;
	case ACT_RUN: return ACT_DOD_RUN_IDLE_BAR;
	case ACT_RUN_RIFLE: return ACT_DOD_RUN_IDLE_BAR;
	case ACT_RUN_AIM: return ACT_DOD_RUN_AIM_BAR;
	case ACT_RUN_AIM_RIFLE: return ACT_DOD_RUN_AIM_BAR;
	case ACT_HL2MP_RUN_MELEE: return ACT_DOD_RUN_AIM_SPADE;
	case ACT_WALK_CROUCH: return ACT_DOD_CROUCHWALK_IDLE_BAR;
	case ACT_CROUCHIDLE: return ACT_DOD_CROUCH_IDLE_BAR;
	case ACT_GESTURE_RELOAD: return ACT_DOD_RELOAD_BAR;
	case ACT_GESTURE_RANGE_ATTACK_AR2: return ACT_DOD_PRIMARYATTACK_MG;
	case ACT_GESTURE_RANGE_ATTACK_SMG1: return ACT_DOD_PRIMARYATTACK_BAR;
	case ACT_GESTURE_RANGE_ATTACK_SHOTGUN: return ACT_DOD_PRIMARYATTACK_BAR;
	case ACT_GESTURE_MELEE_ATTACK_SWING: return ACT_DOD_PRIMARYATTACK_SPADE;
	}
#endif // PLAYER_DOD

	return translateActivity;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPPlayerAnimState::Update( float eyeYaw, float eyePitch )
{
	// Profile the animation update.
	VPROF( "CHL2MPPlayerAnimState::Update" );

	// Get the HL2MP player.
	HL2_PLAYER_CLASS *pHL2MPPlayer = GetHL2MPPlayer();
	if ( !pHL2MPPlayer )
		return;

	// Get the studio header for the player.
	CStudioHdr *pStudioHdr = pHL2MPPlayer->GetModelPtr();
	if ( !pStudioHdr )
		return;

	// Check to see if we should be updating the animation state - dead, ragdolled?
	if ( !ShouldUpdateAnimState() )
	{
		ClearAnimationState();
		return;
	}

	// Store the eye angles.
	m_flEyeYaw = AngleNormalize( eyeYaw );
	m_flEyePitch = AngleNormalize( eyePitch );

	// Compute the player sequences.
	ComputeSequences( pStudioHdr );

	if ( SetupPoseParameters( pStudioHdr ) )
	{
		// Pose parameter - what direction are the player's legs running in.
		ComputePoseParam_MoveYaw( pStudioHdr );

		// Pose parameter - Torso aiming (up/down).
		ComputePoseParam_AimPitch( pStudioHdr );

		// Pose parameter - Torso aiming (rotation).
		ComputePoseParam_AimYaw( pStudioHdr );
	}

#ifdef CLIENT_DLL 
	if ( C_BasePlayer::ShouldDrawLocalPlayer() )
	{
		m_pHL2MPPlayer->SetPlaybackRate( 1.0f );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : event - 
//-----------------------------------------------------------------------------
void CHL2MPPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	Activity iGestureActivity = ACT_INVALID;

	switch( event )
	{
	case PLAYERANIMEVENT_ATTACK_PRIMARY:
		{
			// Weapon primary fire.
			if ( m_pHL2MPPlayer->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_GESTURE_RANGE_ATTACK1_LOW );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_GESTURE_RANGE_ATTACK1 );

			iGestureActivity = ACT_VM_PRIMARYATTACK;
			break;
		}

	case PLAYERANIMEVENT_VOICE_COMMAND_GESTURE:
		{
			if ( !IsGestureSlotActive( GESTURE_SLOT_ATTACK_AND_RELOAD ) )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, (Activity)nData );

			break;
		}
	case PLAYERANIMEVENT_ATTACK_SECONDARY:
		{
			// Weapon secondary fire.
			if ( m_pHL2MPPlayer->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_GESTURE_RANGE_ATTACK2_LOW );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_GESTURE_RANGE_ATTACK2 );

			iGestureActivity = ACT_VM_PRIMARYATTACK;
			break;
		}
	case PLAYERANIMEVENT_ATTACK_PRE:
		{
			if ( m_pHL2MPPlayer->GetFlags() & FL_DUCKING ) 
			{
				// Weapon pre-fire. Used for minigun windup, sniper aiming start, etc in crouch.
				iGestureActivity = ACT_MP_ATTACK_CROUCH_PREFIRE;
			}
			else
			{
				// Weapon pre-fire. Used for minigun windup, sniper aiming start, etc.
				iGestureActivity = ACT_MP_ATTACK_STAND_PREFIRE;
			}
			RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, iGestureActivity, false );
			break;
		}
	case PLAYERANIMEVENT_ATTACK_POST:
		{
			RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_POSTFIRE );
			break;
		}

	case PLAYERANIMEVENT_RELOAD:
		{
			// Weapon reload.
			if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_RELOAD_LOW );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_GESTURE_RELOAD );
			break;
		}
	case PLAYERANIMEVENT_RELOAD_LOOP:
		{
			// Weapon reload.
			if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH_LOOP );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND_LOOP );
			break;
		}
	case PLAYERANIMEVENT_RELOAD_END:
		{
			// Weapon reload.
			if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH_END );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND_END );
			break;
		}
	default:
		{
			BaseClass::DoAnimationEvent( event, nData );
			break;
		}
	}

#ifdef CLIENT_DLL
	// Make the weapon play the animation as well
	if ( iGestureActivity != ACT_INVALID )
	{
#ifdef HOE_DLL
		return; // Isn't the weapon animation sent from the server???
#else
		CBaseCombatWeapon *pWeapon = GetHL2MPPlayer()->GetActiveWeapon();
		if ( pWeapon )
		{
			pWeapon->EnsureCorrectRenderingModel();
			pWeapon->SendWeaponAnim( iGestureActivity );
			// Force animation events!
			pWeapon->ResetEventsParity();		// reset event parity so the animation events will occur on the weapon. 
			pWeapon->DoAnimationEvents( pWeapon->GetModelPtr() );
		}
#endif
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
//-----------------------------------------------------------------------------
bool CHL2MPPlayerAnimState::HandleSwimming( Activity &idealActivity )
{
#ifdef HOE_THIRDPERSON
	return false;
#else
	bool bInWater = BaseClass::HandleSwimming( idealActivity );

	return bInWater;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2MPPlayerAnimState::HandleMoving( Activity &idealActivity )
{
#ifdef HOE_THIRDPERSON
	if ( m_pHL2MPPlayer->IsSprinting() && GetOuterXYSpeed() > 250 ) // hl2_sprintspeed==320
	{
		idealActivity = ACT_RUN_RIFLE; // FIXME: ACT_SPRINT
		return true;
	}
#endif
	return BaseClass::HandleMoving( idealActivity );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2MPPlayerAnimState::HandleDucking( Activity &idealActivity )
{
	if ( m_pHL2MPPlayer->GetFlags() & FL_DUCKING )
	{
		if ( GetOuterXYSpeed() < MOVING_MINIMUM_SPEED )
		{
#ifdef HOE_THIRDPERSON
			CBaseCombatWeapon *pWpn = GetBasePlayer()->GetActiveWeapon();
			idealActivity = (pWpn != NULL && GetBasePlayer()->m_fThirdPersonAimMode) ?
				ACT_RANGE_AIM_LOW : ACT_CROUCHIDLE; // HOE_THIRDPERSON FIXME: add aim ACT_IDLE_ANGRY_LOW
#else
			idealActivity = ( GetBasePlayer()->GetActiveWeapon() != NULL ) ? ACT_RANGE_AIM_LOW : ACT_CROUCHIDLE; // HOE_THIRDPERSON FIXME: add aim ACT_IDLE_ANGRY_LOW
#endif
		}
		else
		{
			idealActivity = ACT_WALK_CROUCH; // HOE_THIRDPERSON FIXME: add aim while crouch-walking
		}

		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
bool CHL2MPPlayerAnimState::HandleJumping( Activity &idealActivity )
{
#ifdef HOE_THIRDPERSONxxx
	if ( IsGestureSlotPlaying( GESTURE_SLOT_JUMP, ACT_LAND ) )
	{
		idealActivity = ACT_LAND;
		return true;
	}
#endif

	Vector vecVelocity;
	GetOuterAbsVelocity( vecVelocity );

	if ( m_bJumping )
	{
#if defined(HOE_THIRDPERSON) && !defined(PLAYER_DOD)
		static bool bNewJump = true;
#else
		static bool bNewJump = false; //Tony; hl2mp players only have a 'hop'
#endif

		if ( m_bFirstJumpFrame )
		{
			m_bFirstJumpFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		// Reset if we hit water and start swimming.
		if ( m_pHL2MPPlayer->GetWaterLevel() >= WL_Waist )
		{
			m_bJumping = false;
			RestartMainSequence();
		}
		// Don't check if he's on the ground for a sec.. sometimes the client still has the
		// on-ground flag set right when the message comes in.
		else if ( gpGlobals->curtime - m_flJumpStartTime > 0.2f )
		{
			if ( m_pHL2MPPlayer->GetFlags() & FL_ONGROUND )
			{
				m_bJumping = false;
				RestartMainSequence();

				if ( bNewJump )
				{
					RestartGesture( GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND );					
				}
			}
		}

		// if we're still jumping
		if ( m_bJumping )
		{
			if ( bNewJump )
			{
				if ( gpGlobals->curtime - m_flJumpStartTime > 0.5 )
				{
					idealActivity = ACT_MP_JUMP_FLOAT;
				}
				else
				{
					idealActivity = ACT_MP_JUMP_START;
				}
			}
			else
			{
				idealActivity = ACT_MP_JUMP;
			}
		}
	}	

	if ( m_bJumping )
		return true;

	return false;
}

bool CHL2MPPlayerAnimState::SetupPoseParameters( CStudioHdr *pStudioHdr )
{
#ifdef PLAYER_9WAY
	return BaseClass::SetupPoseParameters( pStudioHdr );
#endif

	// Check to see if this has already been done.
	if ( m_bPoseParameterInit )
		return true;

	// Save off the pose parameter indices.
	if ( !pStudioHdr )
		return false;

	// Tony; just set them both to the same for now.
	m_PoseParameterData.m_iMoveX = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "move_yaw" );
	m_PoseParameterData.m_iMoveY = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "move_yaw" );
	if ( ( m_PoseParameterData.m_iMoveX < 0 ) || ( m_PoseParameterData.m_iMoveY < 0 ) )
		return false;

	// Look for the aim pitch blender.
	m_PoseParameterData.m_iAimPitch = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "aim_pitch" );
	if ( m_PoseParameterData.m_iAimPitch < 0 )
		return false;

	// Look for aim yaw blender.
	m_PoseParameterData.m_iAimYaw = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "aim_yaw" );
	if ( m_PoseParameterData.m_iAimYaw < 0 )
		return false;

	m_bPoseParameterInit = true;

	return true;
}
float SnapYawTo( float flValue );
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPPlayerAnimState::EstimateYaw( void )
{
	// Get the frame time.
	float flDeltaTime = gpGlobals->frametime;
	if ( flDeltaTime == 0.0f )
		return;

#if 0 // 9way
	// Get the player's velocity and angles.
	Vector vecEstVelocity;
	GetOuterAbsVelocity( vecEstVelocity );
	QAngle angles = GetBasePlayer()->GetLocalAngles();

	// If we are not moving, sync up the feet and eyes slowly.
	if ( vecEstVelocity.x == 0.0f && vecEstVelocity.y == 0.0f )
	{
		float flYawDelta = angles[YAW] - m_PoseParameterData.m_flEstimateYaw;
		flYawDelta = AngleNormalize( flYawDelta );

		if ( flDeltaTime < 0.25f )
		{
			flYawDelta *= ( flDeltaTime * 4.0f );
		}
		else
		{
			flYawDelta *= flDeltaTime;
		}

		m_PoseParameterData.m_flEstimateYaw += flYawDelta;
		AngleNormalize( m_PoseParameterData.m_flEstimateYaw );
	}
	else
	{
		m_PoseParameterData.m_flEstimateYaw = ( atan2( vecEstVelocity.y, vecEstVelocity.x ) * 180.0f / M_PI );
		m_PoseParameterData.m_flEstimateYaw = clamp( m_PoseParameterData.m_flEstimateYaw, -180.0f, 180.0f );
	}
#else
	float dt = gpGlobals->frametime;

	// Get the player's velocity and angles.
	Vector vecEstVelocity;
	GetOuterAbsVelocity( vecEstVelocity );
	QAngle angles = GetBasePlayer()->GetLocalAngles();

	if ( vecEstVelocity.y == 0 && vecEstVelocity.x == 0 )
	{
		float flYawDiff = angles[YAW] - m_PoseParameterData.m_flEstimateYaw;
		flYawDiff = flYawDiff - (int)(flYawDiff / 360) * 360;
		if (flYawDiff > 180)
			flYawDiff -= 360;
		if (flYawDiff < -180)
			flYawDiff += 360;

		if (dt < 0.25)
			flYawDiff *= dt * 4;
		else
			flYawDiff *= dt;

		m_PoseParameterData.m_flEstimateYaw += flYawDiff;
		m_PoseParameterData.m_flEstimateYaw = m_PoseParameterData.m_flEstimateYaw - (int)(m_PoseParameterData.m_flEstimateYaw / 360) * 360;
	}
	else
	{
		m_PoseParameterData.m_flEstimateYaw = (atan2(vecEstVelocity.y, vecEstVelocity.x) * 180 / M_PI);

		if (m_PoseParameterData.m_flEstimateYaw > 180)
			m_PoseParameterData.m_flEstimateYaw = 180;
		else if (m_PoseParameterData.m_flEstimateYaw < -180)
			m_PoseParameterData.m_flEstimateYaw = -180;
	}
#endif
}
//-----------------------------------------------------------------------------
// Purpose: Override for backpeddling
// Input  : dt - 
//-----------------------------------------------------------------------------
float SnapYawTo( float flValue );
void CHL2MPPlayerAnimState::ComputePoseParam_MoveYaw( CStudioHdr *pStudioHdr )
{
	// Get the estimated movement yaw.
	EstimateYaw();

#ifdef PLAYER_9WAY // 9way
	ConVarRef mp_slammoveyaw("mp_slammoveyaw");

	// Get the view yaw.
	float flAngle = AngleNormalize( m_flEyeYaw );

	// Calc side to side turning - the view vs. movement yaw.
	float flYaw = flAngle - m_PoseParameterData.m_flEstimateYaw;
	flYaw = AngleNormalize( flYaw );

	// Get the current speed the character is running.
	bool bIsMoving;
	float flPlaybackRate = 	CalcMovementPlaybackRate( &bIsMoving );

	// Setup the 9-way blend parameters based on our speed and direction.
	Vector2D vecCurrentMoveYaw( 0.0f, 0.0f );
	if ( bIsMoving )
	{
		if ( mp_slammoveyaw.GetBool() )
			flYaw = SnapYawTo( flYaw );

		vecCurrentMoveYaw.x = cos( DEG2RAD( flYaw ) ) * flPlaybackRate;
		vecCurrentMoveYaw.y = -sin( DEG2RAD( flYaw ) ) * flPlaybackRate;
	}

	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveX, vecCurrentMoveYaw.x );
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveY, vecCurrentMoveYaw.y );

	m_DebugAnimData.m_vecMoveYaw = vecCurrentMoveYaw;
#else
	// view direction relative to movement
	float flYaw;	 

	QAngle	angles = GetBasePlayer()->GetLocalAngles();
	float ang = angles[ YAW ];
	if ( ang > 180.0f )
	{
		ang -= 360.0f;
	}
	else if ( ang < -180.0f )
	{
		ang += 360.0f;
	}

	// calc side to side turning
	flYaw = ang - m_PoseParameterData.m_flEstimateYaw;
	// Invert for mapping into 8way blend
	flYaw = -flYaw;
	flYaw = flYaw - (int)(flYaw / 360) * 360;

	if (flYaw < -180)
	{
		flYaw = flYaw + 360;
	}
	else if (flYaw > 180)
	{
		flYaw = flYaw - 360;
	}

	//Tony; oops, i inverted this previously above.
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveY, flYaw );

#endif
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPPlayerAnimState::ComputePoseParam_AimPitch( CStudioHdr *pStudioHdr )
{
	// Get the view pitch.
	float flAimPitch = m_flEyePitch;
#ifdef PLAYER_9WAY
	flAimPitch *= -1;
#endif
	// Set the aim pitch pose parameter and save.
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iAimPitch, flAimPitch );
	m_DebugAnimData.m_flAimPitch = flAimPitch;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPPlayerAnimState::ComputePoseParam_AimYaw( CStudioHdr *pStudioHdr )
{
	// Get the movement velocity.
	Vector vecVelocity;
	GetOuterAbsVelocity( vecVelocity );

	// Check to see if we are moving.
	bool bMoving = ( vecVelocity.Length() > 1.0f ) ? true : false;

	// If we are moving or are prone and undeployed.
	if ( bMoving || m_bForceAimYaw )
	{
		// The feet match the eye direction when moving - the move yaw takes care of the rest.
		m_flGoalFeetYaw = m_flEyeYaw;
	}
	// Else if we are not moving.
	else
	{
		// Initialize the feet.
		if ( m_PoseParameterData.m_flLastAimTurnTime <= 0.0f )
		{
			m_flGoalFeetYaw	= m_flEyeYaw;
			m_flCurrentFeetYaw = m_flEyeYaw;
			m_PoseParameterData.m_flLastAimTurnTime = gpGlobals->curtime;
		}
		// Make sure the feet yaw isn't too far out of sync with the eye yaw.
		// TODO: Do something better here!
		else
		{
			float flYawDelta = AngleNormalize(  m_flGoalFeetYaw - m_flEyeYaw );

#ifdef HOE_THIRDPERSON // aim_yaw in models/mikeforce/movement.qci is 30 degrees
			if ( fabs( flYawDelta ) > 30.0f/*m_AnimConfig.m_flMaxBodyYawDegrees*/ )
			{
				float flSide = ( flYawDelta > 0.0f ) ? -1.0f : 1.0f;
				m_flGoalFeetYaw += ( 30.0f/*m_AnimConfig.m_flMaxBodyYawDegrees*/ * flSide );
			}
#else
			if ( fabs( flYawDelta ) > 45.0f )
			{
				float flSide = ( flYawDelta > 0.0f ) ? -1.0f : 1.0f;
				m_flGoalFeetYaw += ( 45.0f * flSide );
			}
#endif
		}
	}

	// Fix up the feet yaw.
	m_flGoalFeetYaw = AngleNormalize( m_flGoalFeetYaw );
	if ( m_flGoalFeetYaw != m_flCurrentFeetYaw )
	{
		if ( m_bForceAimYaw )
		{
			m_flCurrentFeetYaw = m_flGoalFeetYaw;
		}
		else
		{
			ConvergeYawAngles( m_flGoalFeetYaw, 720.0f, gpGlobals->frametime, m_flCurrentFeetYaw );
			m_flLastAimTurnTime = gpGlobals->curtime;
		}
	}

	// Rotate the body into position.
	m_angRender[YAW] = m_flCurrentFeetYaw;

	// Find the aim(torso) yaw base on the eye and feet yaws.
	float flAimYaw = m_flEyeYaw - m_flCurrentFeetYaw;
	flAimYaw = AngleNormalize( flAimYaw );

#ifdef PLAYER_9WAY
	flAimYaw *= -1;
#endif

	// Set the aim yaw and save.
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iAimYaw, flAimYaw );
	m_DebugAnimData.m_flAimYaw	= flAimYaw;

	// Turn off a force aim yaw - either we have already updated or we don't need to.
	m_bForceAimYaw = false;

#ifndef CLIENT_DLL
	QAngle angle = GetBasePlayer()->GetAbsAngles();
	angle[YAW] = m_flCurrentFeetYaw;

	GetBasePlayer()->SetAbsAngles( angle );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Override the default, because hl2mp models don't use moveX
// Input  :  - 
// Output : float
//-----------------------------------------------------------------------------
float CHL2MPPlayerAnimState::GetCurrentMaxGroundSpeed()
{
	CStudioHdr *pStudioHdr = GetBasePlayer()->GetModelPtr();

	if ( pStudioHdr == NULL )
		return 1.0f;

//	float prevX = GetBasePlayer()->GetPoseParameter( m_PoseParameterData.m_iMoveX );
	float prevY = GetBasePlayer()->GetPoseParameter( m_PoseParameterData.m_iMoveY );

	float d = sqrt( /*prevX * prevX + */prevY * prevY );
	float newY;//, newX;
	if ( d == 0.0 )
	{ 
//		newX = 1.0;
		newY = 0.0;
	}
	else
	{
//		newX = prevX / d;
		newY = prevY / d;
	}

//	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveX, newX );
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveY, newY );

	float speed = GetBasePlayer()->GetSequenceGroundSpeed( GetBasePlayer()->GetSequence() );

//	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveX, prevX );
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveY, prevY );

	return speed;
}
