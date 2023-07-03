#include "cbase.h"
#include "hoe_human.h"
#include "hoe_deathsound.h"
#include "hoe_corpse.h"
#include "hl2_player.h"
#include "saverestore_utlmap.h"
#include "ai_senses.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef HOE_HUMAN_RR

ConVar hoe_debug_human_speech( "hoe_debug_human_speech", "1" );

extern bool ConceptStringLessFunc( const string_t &lhs, const string_t &rhs );

CHumanSpeechManager *CHumanSpeechManager::gm_pSpeechManager[] = { NULL, NULL, NULL };

static SpeechManagerConceptInfo_t conceptInfo[] =
{
	{ TLK_ALERT, 20, 20 },
	{ TLK_CHARGE, 20, 20 },
	{ TLK_DEAD, 300, 300 },
	{ TLK_FOUND, 20, 20 },
	{ TLK_HEAD, 300, 300 },
	{ TLK_HEAL, 30, 30 },
	{ TLK_HEALED, 120, 120 },
	{ TLK_HEAR, 30, 30 },
	{ TLK_HELP, 120, 120 },
	{ TLK_IDLE, 120, 120 },
	{ TLK_KILL, 20, 20 },
	{ TLK_MEDIC, 120, 120 },
	{ TLK_PAIN, 3, 4 },
	{ TLK_QUESTION, 120, 120 },
	{ TLK_SMELL, 120, 120 },
	{ TLK_STARE, 60, 60 },
	{ TLK_TAUNT, 20, 20 },
	{ NULL, -1, -1 },
};

BEGIN_DATADESC( CHumanSpeechManager )

	DEFINE_FIELD( m_ID, FIELD_INTEGER ),
	DEFINE_EMBEDDED_AUTO_ARRAY( m_SpeechCategoryTimer ),
	DEFINE_UTLMAP( m_ConceptTimers, FIELD_STRING, FIELD_EMBEDDED ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( human_speech_manager, CHumanSpeechManager );

//-----------------------------------------------------------------------------
CHumanSpeechManager::CHumanSpeechManager()
{
	m_ID = INVALID_SPEECH_MANAGER;
	m_ConceptTimers.SetLessFunc( ConceptStringLessFunc );
	m_ConceptInfoMap.SetLessFunc( CaselessStringLessThan );
}

//-----------------------------------------------------------------------------
CHumanSpeechManager::~CHumanSpeechManager()
{
	Assert( m_ID != INVALID_SPEECH_MANAGER );
	gm_pSpeechManager[m_ID] = NULL;
}

//-----------------------------------------------------------------------------
void CHumanSpeechManager::Spawn( void )
{
	Assert( m_ID != INVALID_SPEECH_MANAGER );
	Assert( gm_pSpeechManager[m_ID] == NULL );
	gm_pSpeechManager[m_ID] = this;

	for ( int i = 0; conceptInfo[i].concept; i++ )
	{
		m_ConceptInfoMap.Insert( conceptInfo[i].concept, new SpeechManagerConceptInfo_t( conceptInfo[i] ) );
		m_ConceptTimers.Insert( AllocPooledString( conceptInfo[i].concept ), CSimpleSimTimer() );
	}
}

//-----------------------------------------------------------------------------
void CHumanSpeechManager::OnRestore( void )
{
	Assert( m_ID != INVALID_SPEECH_MANAGER );
	Assert( gm_pSpeechManager[m_ID] == NULL );
	gm_pSpeechManager[m_ID] = this;

	for ( int i = 0; conceptInfo[i].concept; i++ )
	{
		m_ConceptInfoMap.Insert( conceptInfo[i].concept, new SpeechManagerConceptInfo_t( conceptInfo[i] ) );
	}
}

//-----------------------------------------------------------------------------
CHumanSpeechManager *CHumanSpeechManager::GetSpeechManager( SpeechManagerID_t managerID )
{
	if ( !gm_pSpeechManager[managerID] )
	{
		CBaseEntity *pEnt = CreateEntityByName( "human_speech_manager" );
		Assert( pEnt );
		if ( !pEnt )
			return NULL;

		CHumanSpeechManager *pMgr = assert_cast<CHumanSpeechManager *>(pEnt);
		if ( pMgr )
			pMgr->m_ID = managerID;

		DispatchSpawn( pEnt );
		Assert( gm_pSpeechManager[managerID] != NULL );
	}

	return gm_pSpeechManager[managerID];
}

//-----------------------------------------------------------------------------
SpeechManagerConceptInfo_t *CHumanSpeechManager::GetConceptInfo( AIConcept_t concept )
{
	int iResult = m_ConceptInfoMap.Find( concept );

	return ( iResult != m_ConceptInfoMap.InvalidIndex() ) ? m_ConceptInfoMap[iResult] : NULL;
}

//-----------------------------------------------------------------------------
void CHumanSpeechManager::OnSpokeConcept( CHOEHuman *pSpeaker, AIConcept_t concept, AI_Response *response  )
{
	if ( hoe_debug_human_speech.GetBool() )
		DevMsg( "CHumanSpeechManager::OnSpokeConcept %s (%s)\n", concept, pSpeaker->GetDebugName() );

	SpeechManagerConceptInfo_t *pConceptInfo = GetConceptInfo( concept );

	if ( pConceptInfo && pConceptInfo->minDelay != -1 )
	{
		Assert( pConceptInfo->maxDelay != -1 );
		char iConceptTimer = m_ConceptTimers.Find( MAKE_STRING(concept) );
		if ( iConceptTimer != m_ConceptTimers.InvalidIndex() )
			m_ConceptTimers[iConceptTimer].Set( pConceptInfo->minDelay, pConceptInfo->maxDelay );
	}
}

//-----------------------------------------------------------------------------
CSimpleSimTimer &CHumanSpeechManager::GetSpeechCategoryTimer( SpeechManagerCategory_t category )
{
	return m_SpeechCategoryTimer[category];
}

//-----------------------------------------------------------------------------
void CHumanSpeechManager::ExtendSpeechCategoryTimer( SpeechManagerCategory_t category, float flDelay )
{
	if ( m_SpeechCategoryTimer[category].GetRemaining() < flDelay )
	{
		m_SpeechCategoryTimer[category].Set( flDelay );
	}
}

//-----------------------------------------------------------------------------
void CHumanSpeechManager::ExtendSpeechConceptTimer( AIConcept_t concept, float flDelay )
{
	char iConceptTimer = m_ConceptTimers.Find( MAKE_STRING(concept) );
	if ( iConceptTimer != m_ConceptTimers.InvalidIndex() &&
		m_ConceptTimers[iConceptTimer].GetRemaining() < flDelay )
	{
		m_ConceptTimers[iConceptTimer].Set( flDelay );
	}
}

//-----------------------------------------------------------------------------
bool CHumanSpeechManager::ConceptDelayExpired( AIConcept_t concept )
{
	char iConceptTimer = m_ConceptTimers.Find( MAKE_STRING(concept) );
	if ( iConceptTimer != m_ConceptTimers.InvalidIndex() )
		return m_ConceptTimers[iConceptTimer].Expired();
	return true;
}

//-----------------------------------------------------------------------------
CHumanSpeechManager *CHOEHuman::GetSpeechManager( void )
{
	return CHumanSpeechManager::GetSpeechManager( GetSpeechManagerID() );
}

//-----------------------------------------------------------------------------
void CHOEHuman::OnSpokeConcept( AIConcept_t concept, AI_Response *response )
{
	if ( GetSpeechManager() )
		GetSpeechManager()->OnSpokeConcept( this, concept, response );
}

//-----------------------------------------------------------------------------
void CHOEHuman::OnStartSpeaking( void )
{
	m_nSpeakClosedCaptionID = m_nClosedCaptionID;
}

//-----------------------------------------------------------------------------
void CHOEHuman::LookAtTalkTarget( float flDuration )
{
	if ( GetTalkTarget() == NULL )
		return;

	AddLookTarget( GetTalkTarget(), 1.0, flDuration + random->RandomFloat( 0.4, 1.2 ), 0.5 );
	AddFacingTarget( GetTalkTarget(), 1.0, flDuration, 0.2 );

	// Interrupt SCHED_HUMAN_IDLE_STAND so we will face the listener
	SetCondition( COND_PROVOKED );
}

//-----------------------------------------------------------------------------
void CHOEHuman::LookAtListenTarget( float flDuration )
{
	if ( GetListenTarget() == NULL )
		return;

	AddLookTarget( GetListenTarget(), 1.0, flDuration + random->RandomFloat( 0.4, 1 ), 0.7 );
	AddFacingTarget( GetListenTarget(), 1.0, flDuration, 0.2 );

	// Interrupt SCHED_HUMAN_IDLE_STAND so we will face the speaker
	SetCondition( COND_PROVOKED );
}

//-----------------------------------------------------------------------------
void CHOEHuman::PostSpeakDispatchResponse( AIConcept_t concept, AI_Response *response )
{
	if ( GetTalkTarget() != NULL )
	{
		float flDuration = GetExpresser()->GetSemaphoreAvailableTime(this) - gpGlobals->curtime;

		LookAtTalkTarget( flDuration );

		CHOEHuman *pHuman = HumanPointer( GetTalkTarget() );
		if ( pHuman != NULL && pHuman->GetListenTarget() == this )
		{
			pHuman->LookAtListenTarget( flDuration );
		}
	}
}

//-----------------------------------------------------------------------------
int CHOEHuman::PlayScriptedSentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, bool bConcurrent, CBaseEntity *pListener )
{
	int captionIndex = 0;
	if ( IsSpeaking() )
	{
		captionIndex = m_nSpeakClosedCaptionID;
	}

//	ClearCondition( COND_PLAYER_PUSHING );	// Forget about moving!  I've got something to say!

	float stop = GetTimeSpeechComplete();

	int sentenceIndex = BaseClass::PlayScriptedSentence( pszSentence, delay, volume, soundlevel, bConcurrent, pListener );

	if ( sentenceIndex != -1 && GetTimeSpeechComplete() > stop )
	{
		if ( captionIndex != 0 )
		{
			m_nClosedCaptionID = captionIndex;
			m_nSpeakClosedCaptionID = 0;
			RescindClosedCaption();
		}

		SetTalkTarget( pListener );

		m_flScriptedSentence = GetTimeSpeechComplete();

		float flDuration = GetTimeSpeechComplete() - gpGlobals->curtime;

		if ( GetTalkTarget() != NULL )
		{
			LookAtTalkTarget( flDuration );

			CHOEHuman *pHuman = HumanPointer( GetTalkTarget() );
			if ( pHuman != NULL )
			{
				pHuman->SetListenTarget( this );
				pHuman->LookAtListenTarget( flDuration );
			}
		}

		// Don't let anyone make idle speech for a while after a scripted speech
		if ( GetSpeechManager() )
		{
			GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, flDuration + 15 );
		}
	}

	return sentenceIndex;
}

//-----------------------------------------------------------------------------
void CHOEHuman::FriendsSpokeConcept( AIConcept_t concept )
{
	for ( int i = 0; GetFriendClasses()[i] ; i++ )
	{
		const char *pszFriend = GetFriendClasses()[i];

		if ( !Q_strcmp( pszFriend, "player" ) )
			continue;

		CBaseEntity *pEntity = NULL;
		while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, pszFriend, GetAbsOrigin(), HUMAN_SPEECH_RADIUS ) ) != 0 )
		{
			if ( pEntity == this || !pEntity->IsAlive() || pEntity->m_lifeState == LIFE_DYING )
				continue;

			CHOEHuman *pHuman = HumanPointer( pEntity );
			if ( pHuman && ( FVisible( pHuman ) || pHuman->FVisible( this ) ) )
			{
				pHuman->SetSpokeConcept( concept, NULL, false );
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool CHOEHuman::OkToShout( void )
{
	if ( !IsAlive() )
		return false;

	if ( !HasAHead() )
		return false;

	// Lifted by a barnacle perhaps
	if ( HasSpawnFlags( SF_NPC_GAG ) )
		return false;

	// Don't speak if playing a script.
	if ( IsInAScript() )
		return false;

	if ( !GetExpresser()->CanSpeak() )
		return false;

	// if someone else is talking, don't speak
	if ( !GetExpresser()->SemaphoreIsAvailable( this ) )
		return false;
#if 0
	// if player is not in pvs, don't speak
	if ( !UTIL_FindClientInPVS(edict()) )
		return false;
#endif
	return true;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::OkToSpeak( void )
{
	// If it's not okay to shout then it's definitely not okay to speak
	if ( !OkToShout() )
		return false;

	// If I have said something not very long ago
//	if ( SpokeRecently( 10.0 ) )
//		return false;

	// If I'm in combat then I don't want to chat but can still shout
	if ( GetState() == NPC_STATE_COMBAT )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
void CHOEHuman::AlertSound( void )
{
	if ( GetEnemy() == NULL ) return;
	if ( !OkToShout() ) return;
//	if ( SpokeRecently( 2.0f ) ) return;
	SpeechSelection_t selection;
	if ( SelectSpeechResponse( TLK_ALERT, NULL, selection ) )
	{
		if ( GetSpeechManager() )
			GetSpeechManager()->ExtendSpeechConceptTimer( TLK_CHARGE, 5 );
		DispatchSpeechSelection( selection );
	}
}

//-----------------------------------------------------------------------------
CBaseEntity *CHOEHuman::FindNearestSpeechTarget( bool bPlayerOK )
{
	CBaseEntity *pNearest = NULL;
	float flDistToClosest = FLT_MAX;

	for ( int i = 0; GetFriendClasses()[i] ; i++ )
	{
		const char *pszFriend = GetFriendClasses()[i];

		if ( !bPlayerOK && !Q_strcmp( pszFriend, "player" ) )
			continue;

		CBaseEntity *pEntity = NULL;
		while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, pszFriend, GetAbsOrigin(), HUMAN_SPEECH_RADIUS ) ) != 0 )
		{
			if ( !IsValidSpeechTarget( pEntity ) )
				continue;

			float flDist2 = (pEntity->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
			if ( flDistToClosest < flDist2 )
				continue;

			pNearest = pEntity;
			flDistToClosest = flDist2;
		}
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
CBaseEntity *CHOEHuman::FindNearestDeadFriend( void )
{
	// If I looked for bodies within the last 2 seconds don't look again
	if ( TimeRecent( m_flTimeScannedCorpses, 2.0f ) )
		return NULL;
	m_flTimeScannedCorpses = gpGlobals->curtime;

	CBaseEntity *pNearest = NULL;
	float flDistToClosest = FLT_MAX;

	CBaseEntity *pEntity = NULL;
	while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, "hoe_corpse_marker", GetAbsOrigin(), HUMAN_SPEECH_RADIUS ) ) != 0 )
	{
		if ( !pEntity->GetParent() )
			continue;

		if ( !FInViewCone( pEntity->GetParent() ) )
			continue;

		// FVisible doesn't work for dead CAI_BaseNPC corpses (FL_NOTARGET?)
		// Added z=+1 because CHOECorpse seem to sit a bit below ground...
		trace_t tr;
		AI_TraceLOS( EyePosition(), pEntity->GetAbsOrigin() + Vector(0,0,1), this, &tr );
		if ( tr.fraction != 1.0f && tr.m_pEnt != pEntity->GetParent() )
		{
//			NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, true, 0.5f );
			continue;
		}
//		NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 255, 0, true, 0.5f );

		float flDist2 = (pEntity->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
		if ( flDistToClosest < flDist2 )
			continue;

		CHOECorpseMarker *pMarker = dynamic_cast<CHOECorpseMarker *>(pEntity);
		if ( pMarker == NULL || pMarker->m_iszLivingClass == NULL_STRING )
			continue;
		bool bMatch = false;
		for ( int i = 0; GetFriendClasses()[i] ; i++ )
		{
			const char *pszFriend = GetFriendClasses()[i];
			if ( !Q_stricmp( pszFriend, STRING(pMarker->m_iszLivingClass) ) )
			{
				bMatch = true;
				break;
			}
		}
		if ( !bMatch )
			continue;

		pNearest = pEntity;
		flDistToClosest = flDist2;
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
CBaseEntity *CHOEHuman::FindNearestHead( void )
{
	// If I looked for corpses within the last 2 seconds don't look again
	if ( TimeRecent( m_flTimeScannedCorpses, 2.0f ) )
		return NULL;
//	m_flTimeScannedCorpses = gpGlobals->curtime;

	CBaseEntity *pNearest = NULL;
	float flDistToClosest = FLT_MAX;

	CBaseEntity *pEntity = NULL;
	while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, "hoe_corpse_marker", GetAbsOrigin(), HUMAN_SPEECH_RADIUS ) ) != 0 )
	{
		if ( !pEntity->GetParent() )
			continue;

		if ( !FInViewCone( pEntity->GetParent() ) )
			continue;

		trace_t tr;
		AI_TraceLOS( EyePosition(), pEntity->WorldSpaceCenter(), this, &tr );
		if ( tr.fraction != 1.0f && tr.m_pEnt != pEntity->GetParent() )
		{
			continue;
		}

		float flDist2 = (pEntity->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
		if ( flDistToClosest < flDist2 )
			continue;

		CHOECorpseMarker *pMarker = dynamic_cast<CHOECorpseMarker *>(pEntity);
		if ( pMarker == NULL || pMarker->m_iszLivingClass == NULL_STRING )
			continue;

		if ( Q_stricmp( "head", STRING(pMarker->m_iszLivingClass) ) )
		{
			continue;
		}

		pNearest = pEntity;
		flDistToClosest = flDist2;
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::IsValidSpeechTarget( CBaseEntity *pTarget )
{
	if ( pTarget == this )
		return false;

	if ( !pTarget->IsAlive() )
		return false;

	if ( pTarget->GetFlags() & FL_NOTARGET )
		return false;

	if ( pTarget->IsMoving() )
		return false;

	if ( pTarget->IsPlayer() )
		return HasCondition( COND_SEE_PLAYER );

	CAI_BaseNPC *pNPC = pTarget->MyNPCPointer();
	if ( pNPC == false )
		return false;

	if ( pNPC->GetState() == NPC_STATE_PRONE )
		return false;

	if ( pNPC->IsInAScript() )
		return false;

	if ( !pNPC->CanBeUsedAsAFriend() )
		return false;

	if ( !FInViewCone( pTarget ) )
		return false;

	if ( !FVisible( pTarget ) )
		return false;

#if 1
	trace_t tr;
	UTIL_TraceLine( EyePosition(), pTarget->EyePosition(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
#else
	Vector vecCheck;
	trace_t tr;
	pTarget->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 1.0f ), &vecCheck );
	UTIL_TraceLine( GetAbsOrigin(), vecCheck, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
#endif
	if ( tr.fraction < 1.0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SelectSpeechResponse( AIConcept_t concept, const char *pszModifiers, SpeechSelection_t &selection )
{
	// Can anyone speak this concept?
	if ( GetSpeechManager() && !GetSpeechManager()->ConceptDelayExpired( concept ) )
	{
//		if ( hoe_debug_human_speech.GetBool() )
//			DevMsg( "CHOEHuman::SelectSpeechResponse %s delayed\n", concept );
		return false;
	}

	// Can this individual speak this concept?
	if ( !GetExpresser()->CanSpeakConcept( concept ) )
		return false;

	AI_Response *pResponse = SpeakFindResponse( concept, pszModifiers );
	if ( pResponse )
	{
		selection.concept = concept;
		selection.pResponse = pResponse;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::DispatchSpeechSelection( SpeechSelection_t &selection )
{
	if ( IsSpeaking() )
	{
		m_nClosedCaptionID = m_nSpeakClosedCaptionID;
		m_nSpeakClosedCaptionID = 0;
		RescindClosedCaption();
	}
	return SpeakDispatchResponse( selection.concept, selection.pResponse );
}

//-----------------------------------------------------------------------------
// Return true if an NPC can see the source of any smell
bool CHOEHuman::CanSeeSmellOrigin( void )
{
	Assert( HasCondition( COND_SMELL ) );

	// If I looked for smells within the last 5 seconds don't look again
	if ( TimeRecent( m_flTimeScannedSmells, 5.0f ) )
		return false;
	m_flTimeScannedSmells = gpGlobals->curtime;

	CAI_Senses *pSenses = GetSenses();
	if ( pSenses == NULL )
		return false;

	AISoundIter_t iter;
	
	CSound *pCurrent = pSenses->GetFirstHeardSound( &iter );

	Vector earPosition = EarPosition();
	
	while ( pCurrent )
	{
		if ( pCurrent->FIsScent() )
		{
			if ( /*pCurrent->IsSoundType( ALL_SCENTS ) && */!ShouldIgnoreSound( pCurrent ) )
			{
//				flDist = ( pCurrent->GetSoundOrigin() - earPosition ).LengthSqr();

//				if ( flDist < pCurrent->Volume() )
				if ( FInViewCone( pCurrent->GetSoundOrigin() ) )
				{
					trace_t tr;
					AI_TraceLOS( EyePosition(), pCurrent->GetSoundOrigin(), this, &tr );
					DevMsg("smell fraction=%.2f ent=%s\n", tr.fraction, tr.m_pEnt ? tr.m_pEnt->GetDebugName() : "NULL" );
					if ( tr.fraction == 1.0 )
						return true;
				}
			}
		}
		
		pCurrent = pSenses->GetNextHeardSound( &iter );
	}
	
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SelectIdleSpeech( SpeechSelection_t &selection )
{
	// Don't idle-talk for a while after spawning
	if ( TimeNotPassed( m_flIdleSpeechDelay ) )
		return false;

	CHumanSpeechManager *pSpeechMgr = GetSpeechManager();

	CBaseEntity *pFriend = selection.hSpeechTarget;
	CHOEHuman *pHuman = pFriend ? HumanPointer( pFriend ) : NULL;
	CHL2_Player *pPlayer = (pFriend && pFriend->IsPlayer()) ? (CHL2_Player *)pFriend : NULL;

	// It takes a deliberate effort to trigger staring so we don't restrict
	// it to the idle speech timer.
	if (
		pPlayer &&
		GetTimePlayerStaring() > 6 &&
		!IsMoving() &&
		SelectSpeechResponse( TLK_STARE, NULL, selection )
		)
	{
		// A medic shouldn't tell the player to f*** off if the player stares at him,
		// unless the player is fully healed.
		if ( !IsMedic() || pPlayer->GetHealth() == pPlayer->GetMaxHealth() )
		{
			selection.bFaceTarget = true;
			selection.bTargetFaceSpeaker = false;
			if ( pSpeechMgr )
				pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			return true;
		}
	}

	if ( pSpeechMgr && !pSpeechMgr->GetSpeechCategoryTimer( SPEECH_CATEGORY_IDLE ).Expired() )
		return false;

	if (
		( pPlayer || ( pHuman && pHuman->IsSquadLeader() ) ) &&
		!SpokeConcept( TLK_HELLO ) &&
		SelectSpeechResponse( TLK_HELLO, NULL, selection )
		)
	{
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = true;
		FriendsSpokeConcept( TLK_HELLO );
		if ( pSpeechMgr )
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
		return true;
	}

	// Talk about stink even if there is nobody to talk to.
	if (
		HasCondition( COND_SMELL ) &&
		!SpokeConcept( TLK_SMELL ) &&
		CanSeeSmellOrigin() &&
		SelectSpeechResponse( TLK_SMELL, NULL, selection )
		)
	{
		selection.bFaceTarget = false;
		selection.bTargetFaceSpeaker = false;
		if ( pSpeechMgr )
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
		return true;
	}

	if (
		pFriend &&
//		random->RandomInt( 0, 4 ) == 0 &&
		!SpokeConcept( TLK_QUESTION ) &&
		SelectSpeechResponse( TLK_QUESTION, NULL, selection )
		)
	{
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = true;
		if ( pSpeechMgr )
		{
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );

			// We don't support question/answer speech yet, so TLK_IDLE and
			// TLK_QUESTION are equivalent.
			pSpeechMgr->ExtendSpeechConceptTimer( TLK_IDLE, 120 );
		}
		return true;
	}
	
	if (
		pFriend &&
//		random->RandomInt( 0, 4 ) == 0 &&
		!SpokeConcept( TLK_IDLE ) &&
		SelectSpeechResponse( TLK_IDLE, NULL, selection )
		)
	{
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = true;
		if ( pSpeechMgr )
		{
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );

			// We don't support question/answer speech yet, so TLK_IDLE and
			// TLK_QUESTION are equivalent.
			pSpeechMgr->ExtendSpeechConceptTimer( TLK_QUESTION, 120 );
		}
		return true;
	}
	
#if 0
	// FIXME: saw a dead ally
	else if ( !SpokeConcept( HUMAN_SC_DEAD_ALLY ) &&
		HasCondition( COND_SQUADMEMBER_KILLED ) )
	{
		Speak( "DEAD", HUMAN_SC_DEAD_ALLY );
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::SelectInjurySpeech( SpeechSelection_t &selection )
{
	if ( m_RappelBehavior.IsRunning() || m_HueyRappelBehavior.IsRunning() )
		return false;

	CHumanSpeechManager *pSpeechMgr = GetSpeechManager();

	if ( TimeRecent( m_flLastMedicHealed, 12.0f ) &&
		TimeNotRecent( m_flLastMedicHealed, 3.0f ) &&
		m_hMedicThatHealedMe != NULL &&
		!SpokeConcept( TLK_HEALED ) &&
		SelectSpeechResponse( TLK_HEALED, NULL, selection ) )
	{
		selection.hSpeechTarget = m_hMedicThatHealedMe;
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = false; // The medic does not need to face me
		m_hMedicThatHealedMe = NULL;
		if ( pSpeechMgr )
		{
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 30 );
		}
		return true;
	}

	CBaseEntity *pHead;
	if (
		!SpokeConcept( TLK_HEAD ) &&
		pSpeechMgr &&
		pSpeechMgr->ConceptDelayExpired( TLK_HEAD ) && // avoid unneeded calls to FindNearestHead
		( pHead = FindNearestHead() ) != NULL &&
		SelectSpeechResponse( TLK_HEAD, NULL, selection )
		)
	{
		selection.hSpeechTarget = pHead;
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = false;
		if ( pSpeechMgr )
		{
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 30 );
		}
		return true;
	}
	// Talk about dead friend even if there is nobody to speak to.
	// This is so the player might hear an out-of-sight enemy.
	CBaseEntity *pCorpse;
	if (
		!SpokeConcept( TLK_DEAD ) &&
		pSpeechMgr &&
		pSpeechMgr->ConceptDelayExpired( TLK_DEAD ) && // avoid unneeded calls to FindNearestDeadFriend
		( pCorpse = FindNearestDeadFriend() ) != NULL &&
		SelectSpeechResponse( TLK_DEAD, NULL, selection )
		)
	{
		selection.hSpeechTarget = pCorpse;
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = false;
		if ( pSpeechMgr )
		{
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 30 );
		}
		return true;
	}

	if ( pSpeechMgr && !pSpeechMgr->GetSpeechCategoryTimer( SPEECH_CATEGORY_INJURY ).Expired() )
		return false;

	if ( selection.hSpeechTarget == NULL )
		return false;

	CBaseEntity *pFriend = selection.hSpeechTarget;
	CHOEHuman *pHuman = HumanPointer( pFriend );
	CHL2_Player *pPlayer = pFriend->IsPlayer() ? (CHL2_Player *)pFriend : NULL;

	// Try talking about my own injuries
	AIConcept_t concept = NULL;
	if ( TimeNotRecent( GetLastDamageTime(), 30.0f ) )
		; // don't talk about my wounds unless I received them recently
	else if ( IsHealthInRange( 1.0/3.0, 2.0/3.0 ) )
		concept = TLK_WOUND;
	else if ( IsHealthInRange( 0.0, 1.0/3.0 ) )
		concept = TLK_MORTAL;

	if ( concept != NULL &&
		TimeNotRecent( m_flTimeInjuriesMentioned, 30 ) &&
		TimeNotRecent( m_flLastMedicHealed, 30 ) &&
		!SpokeConcept( concept ) &&
		SelectSpeechResponse( concept, NULL, selection )
		)
	{
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = true;
		m_flTimeInjuriesMentioned = gpGlobals->curtime;
		if ( pSpeechMgr )
		{
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 30 );
		}
		return true;
	}

	// Try talking about my friend's injuries
	float flTimeInjuriesMentioned = pHuman ? pHuman->m_flTimeInjuriesMentioned :
		( pPlayer ? pPlayer->m_flTimeInjuriesMentioned : 0 );
	if ( TimeRecent( flTimeInjuriesMentioned, 30 ) )
		return false;

	float flTimeHealedByMedic = pHuman ? pHuman->m_flLastMedicHealed :
		( pPlayer ? pPlayer->m_flTimeHealedByMedic : 0 );
	if ( TimeRecent( flTimeHealedByMedic, 30 ) )
		return false;

	if ( pHuman && TimeNotRecent( pHuman->GetLastDamageTime(), 30.0f ) )
		return false; // don't talk about someone's wounds unless he received them recently

	if ( IsHealthInRange( pFriend, 0.0, 1.0/8.0 ) )
		concept = TLK_HURTC;
	else if ( IsHealthInRange( pFriend, 1.0/8.0, 1.0/4.0 ) )
		concept = TLK_HURTB;
	else if ( IsHealthInRange( pFriend, 1.0/4.0, 1.0/2.0 ) )
		concept = TLK_HURTA;
	else
		return false;

	if ( !SpokeConcept( concept ) &&
		SelectSpeechResponse( concept, NULL, selection ) )
	{
		selection.bFaceTarget = true;
		selection.bTargetFaceSpeaker = true;
		if ( pHuman )
		{
			pHuman->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		}
		else if ( pPlayer )
			pPlayer->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		if ( pSpeechMgr )
		{
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			pSpeechMgr->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 30 );
		}
		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
void CHOEHuman::IdleSound( void )
{
}

//-----------------------------------------------------------------------------
bool CHOEHuman::IdleSpeech( void )
{
#if 1
	if ( !OkToSpeak() )
		return false;

	if ( m_RappelBehavior.IsRunning() || m_HueyRappelBehavior.IsRunning() )
		return false;

	// if player is not in pvs, don't speak
	if ( !UTIL_FindClientInPVS(edict()) )
		return false;

	bool bProvokedByPlayer = HasMemory( bits_MEMORY_PROVOKED );
	CBaseEntity *pFriend = FindNearestSpeechTarget( !bProvokedByPlayer );
//	if ( pFriend == NULL )
//		return false;

	SpeechSelection_t selection;
	selection.hSpeechTarget = pFriend;

	if ( SelectInjurySpeech( selection ) ||
		SelectIdleSpeech( selection ) )
	{
		if ( selection.hSpeechTarget )
		{
			if ( selection.bFaceTarget )
				SetTalkTarget( selection.hSpeechTarget );
			if ( selection.bTargetFaceSpeaker && HumanPointer( selection.hSpeechTarget ) )
				HumanPointer( selection.hSpeechTarget )->SetListenTarget( this );
		}
		DispatchSpeechSelection( selection );
		return true;
	}

	return false;
#else
	if ( !OkToSpeak() )
		return false;

	// if player is not in pvs, don't speak
	if ( !UTIL_FindClientInPVS(edict()) )
		return false;

	if ( TimeRecent( m_flLastMedicHealed, 12.0f ) &&
		m_hMedicThatHealedMe != NULL )
	{
		Speak( TLK_HEALED );
		m_hMedicThatHealedMe = NULL;
		return true;
	}

	CHumanSpeechManager *pSpeechMgr = GetSpeechManager();
	if ( pSpeechMgr && !pSpeechMgr->m_IdleSpeechTimer.Expired() )
		return false;

	bool bProvokedByPlayer = HasMemory( bits_MEMORY_PROVOKED );
	CBaseEntity *pFriend = FindNearestSpeechTarget( !bProvokedByPlayer );
	if ( pFriend == NULL )
		return false;
	CHOEHuman *pHuman = HumanPointer( pFriend );
	CHL2_Player *pPlayer = pFriend->IsPlayer() ? (CHL2_Player *)pFriend : NULL;

	float flTimeInjuriesMentioned = pHuman ? pHuman->m_flTimeInjuriesMentioned :
		( pPlayer ? pPlayer->m_flTimeInjuriesMentioned : 0 );

	AI_Response *pResponse;

	if (
		( pPlayer || ( pHuman && pHuman->IsSquadLeader() ) ) &&
		!GetExpresser()->SpokeConcept( TLK_HELLO ) &&
		GetExpresser()->CanSpeakConcept( TLK_HELLO ) &&
		((pResponse = SpeakFindResponse( TLK_HELLO )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		FriendsSpokeConcept( TLK_HELLO );
		SpeakDispatchResponse( TLK_HELLO, pResponse );
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
		IsHealthInRange( pFriend, 0.0, 1.0/8.0 ) &&
		TimeNotRecent( flTimeInjuriesMentioned, 30 ) &&
		!SpokeConcept( TLK_HURTC ) &&
		GetExpresser()->CanSpeakConcept( TLK_HURTC ) &&
		((pResponse = SpeakFindResponse( TLK_HURTC )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_HURTC, pResponse );
		if ( pHuman )
			pHuman->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		else if ( pPlayer )
			pPlayer->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
		IsHealthInRange( pFriend, 1.0/8.0, 1.0/4.0 ) &&
		TimeNotRecent( flTimeInjuriesMentioned, 30 ) &&
		!SpokeConcept( TLK_HURTB ) &&
		GetExpresser()->CanSpeakConcept( TLK_HURTB ) &&
		((pResponse = SpeakFindResponse( TLK_HURTB )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_HURTB, pResponse );
		if ( pHuman )
			pHuman->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		else if ( pPlayer )
			pPlayer->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
		IsHealthInRange( pFriend, 1.0/4.0, 1.0/2.0 ) &&
		TimeNotRecent( flTimeInjuriesMentioned, 30 ) &&
		!SpokeConcept( TLK_HURTA ) &&
		GetExpresser()->CanSpeakConcept( TLK_HURTA ) &&
		((pResponse = SpeakFindResponse( TLK_HURTA )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_HURTA, pResponse );
		if ( pHuman )
			pHuman->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		else if ( pPlayer )
			pPlayer->m_flTimeInjuriesMentioned = gpGlobals->curtime;
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
		IsHealthInRange( 1.0/3.0, 2.0/3.0 ) &&
		TimeNotRecent( m_flTimeInjuriesMentioned, 30 ) &&
		!SpokeConcept( TLK_WOUND ) &&
		GetExpresser()->CanSpeakConcept( TLK_WOUND ) &&
		((pResponse = SpeakFindResponse( TLK_WOUND )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_WOUND, pResponse );
		m_flTimeInjuriesMentioned = gpGlobals->curtime;
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
		IsHealthInRange( 0.0, 1.0/3.0 ) &&
		TimeNotRecent( m_flTimeInjuriesMentioned, 30 ) &&
		!SpokeConcept( TLK_MORTAL ) &&
		GetExpresser()->CanSpeakConcept( TLK_MORTAL ) &&
		((pResponse = SpeakFindResponse( TLK_MORTAL )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_MORTAL, pResponse );
		m_flTimeInjuriesMentioned = gpGlobals->curtime;
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
//		random->RandomInt( 0, 4 ) == 0 &&
		!SpokeConcept( TLK_QUESTION ) &&
		GetExpresser()->CanSpeakConcept( TLK_QUESTION ) &&
		((pResponse = SpeakFindResponse( TLK_QUESTION )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_QUESTION, pResponse );
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
//		random->RandomInt( 0, 4 ) == 0 &&
		!SpokeConcept( TLK_IDLE ) &&
		((pResponse = SpeakFindResponse( TLK_IDLE )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_IDLE, pResponse );
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
	else if (
		pPlayer &&
		GetTimePlayerStaring() > 6 &&
		!IsMoving() &&
		((pResponse = SpeakFindResponse( TLK_STARE )) != NULL)
		)
	{
		SetTalkTarget( pFriend );
		SpeakDispatchResponse( TLK_STARE, pResponse );
		if ( pSpeechMgr )
			pSpeechMgr->m_IdleSpeechTimer.Set( 30 );
		return true;
	}
#if 0
	// FIXME: saw a dead ally
	else if ( !SpokeConcept( HUMAN_SC_DEAD_ALLY ) &&
		HasCondition( COND_SQUADMEMBER_KILLED ) )
	{
		Speak( "DEAD", HUMAN_SC_DEAD_ALLY );
	}
#endif
	return false;
#endif
}

//-----------------------------------------------------------------------------
void CHOEHuman::PainSound( const CTakeDamageInfo &info )
{
	// With closed captioning we don't want to display the PainSound if the NPC dies from
	// the damage. Also it is wasteful to emit the PainSound then stop it and immediately
	// play the DeathSound. We can't just check if the damage would kill us because the
	// machete chop comes afterward, and multiple attacks could happen this frame resulting
	// in death.
	m_flDelayedPainSoundTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
void CHOEHuman::DelayedPainSound( void )
{
	// No head etc
	if ( !OkToShout() ) return;

	// Make sure he doesn't just repeatedly go uh-uh-uh-uh-uh when hit by rapid fire
//	if ( TimeNotPassed( m_flPainTime ) ) return;

	SpeechSelection_t selection;
	if ( SelectSpeechResponse( TLK_PAIN, NULL, selection ) )
		DispatchSpeechSelection( selection );

//	m_flPainTime = max(gpGlobals->curtime,m_flStopTalkTime) + random->RandomFloat( 0.5, 0.75 );

	SquadIssueCommand( SQUADCMD_DISTRESS ); 
}

#if 1

void CNPCDeathSound::Spawn( void )
{
	BaseClass::Spawn();

	// Delete 20 seconds after spawning
	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 20.0f );
}

BEGIN_DATADESC( CNPCDeathSound )
	DEFINE_AUTO_ARRAY( m_szSoundName, FIELD_CHARACTER ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPCDeathSound, DT_NPCDeathSound )
	SendPropString( SENDINFO( m_szSoundName ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( npc_death_sound, CNPCDeathSound );

#endif

//-----------------------------------------------------------------------------
void CHOEHuman::DeathSound( const CTakeDamageInfo &info )
{
	/* Stop talking. */
	if ( IsSpeaking() )
	{
		m_nClosedCaptionID = m_nSpeakClosedCaptionID;
		m_nSpeakClosedCaptionID = 0;
		RescindClosedCaption();
	}
	SentenceStop();

	if ( !HasAHead() ) return;

	if ( ShouldGib( info ) ) return;

#if 1
	if ( HasSpawnFlags( SF_HUMAN_SERVERSIDE_RAGDOLL ) )
	{
		// The death sound will be emitted from the server-side ragdoll so it
		// follows the ragdoll. See this class' BecomeRagdollOnClient() method.
	}
	else
	{
	//	Q_snprintf( m_szDeathSound.GetForModify(), sizeof(m_szDeathSound), "!%s_%s", GetSentenceGroup(), "DIE" );
	//	Q_strcpy( m_szDeathSound.GetForModify(), "NPC_Mikeforce.Die" );

		// Create an entity to follow the ragdoll on the client which will emit the
		// death sound.
		CNPCDeathSound *pEnt = (CNPCDeathSound *) CBaseEntity::Create( "npc_death_sound", GetAbsOrigin(), GetAbsAngles(), NULL );
		if ( pEnt )
		{
			Assert( 0 );
			char szSentence[128];
			Q_snprintf( szSentence, sizeof(szSentence), "!%s_%s", GetSentenceGroup(), "DIE" );
			Q_strcpy( pEnt->m_szSoundName.GetForModify(), szSentence );
			m_hDeathSound = pEnt;
		}

		// This lets zombies know where the eating is good
		// see CGib::WaitTillLand
		// FIXME: the ragdoll may fly away.
		CSoundEnt::InsertSound( SOUND_CARCASS, GetAbsOrigin(), 384, 25 );
	}
#else
	// FIXME: would be nice if the death sound was emitted from the ragdoll
	Speak( "DIE" );
#endif
	SquadIssueCommand( SQUADCMD_DISTRESS ); 
}

//-----------------------------------------------------------------------------
void CHOEHuman::FoundEnemySound( void )
{
	if ( !OkToShout() )
		return;

	SpeechSelection_t selection;
	if ( SelectSpeechResponse( TLK_FOUND, NULL, selection ) )
	{
		DispatchSpeechSelection( selection );
		if ( GetSpeechManager() )
		{
			GetSpeechManager()->ExtendSpeechConceptTimer( TLK_ALERT, 5 );
			GetSpeechManager()->ExtendSpeechConceptTimer( TLK_CHARGE, 5 );
		}
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::GrenadeSound( void )
{
	if ( !OkToShout() )
		return;

	Speak( TLK_GREN ); // FIXME: heard barney say it 3 times in a row
}

//-----------------------------------------------------------------------------
bool CHOEHuman::AnnounceAttack( void )
{
	if ( !OkToShout() )
		return false;

//	if ( SpokeRecently( 10.0 ) /* <-- quiet! */  )
//		return false;

	Speak( TLK_SQUAD_ATTACK );
	return true;
}

#else // HOE_HUMAN_RR

//-----------------------------------------------------------------------------
void CHOEHuman::InitSentences( void )
{
	const char *szSentenceGroup = GetSentenceGroup();
	enginesound->PrecacheSentenceGroup( szSentenceGroup );
}

//-----------------------------------------------------------------------------
int CHOEHuman::PlaySentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, CBaseEntity *pListener )
{
	// Overide this so we know about scripted_sentences
	int sentenceIndex = BaseClass::PlaySentence( pszSentence, delay, volume, soundlevel,
		pListener );

	if ( sentenceIndex != -1 )
	{
		float flDuration = engine->SentenceLength( sentenceIndex );
		/* if ( flDuration <= 0 )*/ DevMsg( "CHOEHuman::PlaySentence duration %.1f\n", flDuration );
		m_flStopTalkTime = gpGlobals->curtime + flDuration;
		m_flLastTalkTime = m_flStopTalkTime;
		g_flHumanSpeechTime = max( g_flHumanSpeechTime, m_flStopTalkTime );

		if ( pListener )
		{
			SetTalkTarget( pListener );
			AddLookTarget( pListener, 1.0, flDuration );
			AddFacingTarget( pListener, 1.0, flDuration, 0.2 );

			// Interrupt SCHED_HUMAN_IDLE_STAND so we will face the listener
			SetCondition( COND_PROVOKED );

			CHOEHuman *pHuman = HumanPointer( pListener );
			if ( pHuman )
			{
				pHuman->SetListenTarget( this );
				pHuman->AddLookTarget( this, 1.0, flDuration );
				pHuman->AddFacingTarget( this, 1.0, flDuration, 0.2 );

				// Interrupt SCHED_HUMAN_IDLE_STAND so we will face the speaker
				SetCondition( COND_PROVOKED );
			}
		}
	}
	else
		DevMsg( "CHOEHuman::PlaySentence %s bogus sentence %s\n", GetDebugName(), pszSentence );

	return sentenceIndex;
}

//-----------------------------------------------------------------------------
int CHOEHuman::PlayScriptedSentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, bool bConcurrent, CBaseEntity *pListener )
{
	DevMsg( "CHOEHuman::PlayScriptedSentence %s\n", pszSentence);
	float stop = m_flStopTalkTime;
	int sentenceIndex = BaseClass::PlayScriptedSentence( pszSentence, delay, volume, soundlevel, bConcurrent, pListener );
	if ( stop < m_flStopTalkTime )
		m_flScriptedSentence = m_flStopTalkTime;
	return sentenceIndex;
}

//-----------------------------------------------------------------------------
void CHOEHuman::Speak( const char *pSentence, int concept, CBaseEntity *pListener )
{
	char szSentence[128];
	Q_snprintf( szSentence, sizeof(szSentence), "%s_%s", GetSentenceGroup(), pSentence );
	DevMsg( "CHOEHuman::Speak %s %s\n", GetDebugName(), szSentence );

	float volume = VOL_NORM;
	float delay = 0.0;
//	int flags = 0;
//	int pitch = PITCH_NORM;
//	int index = SENTENCEG_PlayRndSz( edict(), szSentence, volume, SNDLVL_TALKING, flags, pitch);
	PlaySentence( szSentence, delay, volume, SNDLVL_TALKING, pListener );

	m_iSpokenConcepts |= concept;
}

//-----------------------------------------------------------------------------
void CHOEHuman::FriendsSpokeConcept( int concept )
{
	for ( int i = 0; GetFriendClasses()[i] ; i++ )
	{
		const char *pszFriend = GetFriendClasses()[i];

		if ( !Q_strcmp( pszFriend, "player" ) )
			continue;

		CBaseEntity *pEntity = NULL;
		while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, pszFriend, GetAbsOrigin(), HUMAN_SPEECH_RADIUS ) ) != 0 )
		{
			if ( pEntity == this || !pEntity->IsAlive() || pEntity->m_lifeState == LIFE_DYING )
				continue;

			CHOEHuman *pHuman = HumanPointer( pEntity );
			if ( pHuman && FVisible( pHuman ) || pHuman->FVisible( this ) )
			{
				pHuman->m_iSpokenConcepts |= concept;
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool CHOEHuman::OkToShout( void )
{
	if ( !IsAlive() )
		return false;

	if ( !HasAHead() )
		return false;

	// Lifted by a barnacle perhaps
	if ( HasSpawnFlags( SF_NPC_GAG ) )
		return false;

	// Don't speak if playing a script.
	if ( IsInAScript() )
		return false;

	if ( IsTalking() )
		return false;
#if 0
	// if player is not in pvs, don't speak
	if ( !UTIL_FindClientInPVS(edict()) )
		return false;
#endif
	return true;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::OkToSpeak( void )
{
	// If it's not okay to shout then it's definitely not okay to speak
	if ( !OkToShout() )
		return false;

	// if someone else is talking, don't speak
	if ( IsAnyoneTalking() )
		return false;

	// If I have said something not very long ago
	if ( SpokeRecently( 10.0 ) )
		return false;

	// If I'm in combat then I don't want to chat but can still shout
	if ( GetState() == NPC_STATE_COMBAT )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
void CHOEHuman::SetQuestioner( CHOEHuman *pSpeaker, float timeToAnswer )
{
/*	ChangeSchedule( SCHED_HUMAN_IDLE_RESPONSE ); */
	SetTalkTarget( pSpeaker );
	m_flStopTalkTime = timeToAnswer;
}

//-----------------------------------------------------------------------------
void CHOEHuman::AlertSound( void )
{
	if ( GetEnemy() == NULL ) return;
	if ( !OkToShout() ) return;
	if ( SpokeRecently( 2.0f ) ) return;
	Speak( "ALERT" );
}

//-----------------------------------------------------------------------------
CBaseEntity *CHOEHuman::FindNearestSpeechTarget( bool bPlayerOK )
{
	CBaseEntity *pNearest = NULL;
	float flDistToClosest = FLT_MAX;

	for ( int i = 0; GetFriendClasses()[i] ; i++ )
	{
		const char *pszFriend = GetFriendClasses()[i];

		if ( !bPlayerOK && !Q_strcmp( pszFriend, "player" ) )
			continue;

		CBaseEntity *pEntity = NULL;
		while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, pszFriend, GetAbsOrigin(), HUMAN_SPEECH_RADIUS ) ) != 0 )
		{
			if ( !IsValidSpeechTarget( pEntity ) )
				continue;

			float flDist2 = (pEntity->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
			if ( flDistToClosest < flDist2 )
				continue;

			pNearest = pEntity;
			flDistToClosest = flDist2;
		}
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::IsValidSpeechTarget( CBaseEntity *pTarget )
{
	if ( pTarget == this )
		return false;

	if ( !pTarget->IsAlive() )
		return false;

	if ( pTarget->GetFlags() & FL_NOTARGET )
		return false;

	if ( pTarget->IsMoving() )
		return false;

	if ( pTarget->IsPlayer() )
		return HasCondition( COND_SEE_PLAYER );

	CAI_BaseNPC *pNPC = pTarget->MyNPCPointer();
	if ( pNPC == false )
		return false;

	if ( pNPC->GetState() == NPC_STATE_PRONE )
		return false;

	if ( pNPC->IsInAScript() )
		return false;

	if ( !pNPC->CanBeUsedAsAFriend() )
		return false;

	if ( !FInViewCone( pTarget ) )
		return false;

	if ( !FVisible( pTarget ) )
		return false;

#if 1
	trace_t tr;
	UTIL_TraceLine( EyePosition(), pTarget->EyePosition(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
#else
	Vector vecCheck;
	trace_t tr;
	pTarget->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 1.0f ), &vecCheck );
	UTIL_TraceLine( GetAbsOrigin(), vecCheck, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
#endif
	if ( tr.fraction < 1.0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
void CHOEHuman::IdleSound( void )
{
	if ( !OkToSpeak() )
		return;

	// if player is not in pvs, don't speak
	if ( !UTIL_FindClientInPVS(edict()) )
		return;

	if ( TimeRecent( m_flLastMedicHealed, 12.0f ) &&
		m_hMedicThatHealedMe != NULL )
	{
		Speak( "HEALED" );
		m_hMedicThatHealedMe = NULL;
		return;
	}

	bool bProvokedByPlayer = HasMemory( bits_MEMORY_PROVOKED );
	CBaseEntity *pFriend = FindNearestSpeechTarget( !bProvokedByPlayer );
	CHOEHuman *pHuman = HumanPointer( pFriend );

	if ( pFriend )
	{
		if ( !SpokeConcept( HUMAN_SC_HELLO ) &&
			( pFriend->IsPlayer() || ( pHuman && pHuman->IsSquadLeader() ) ) )
		{
			Speak( "HELLO", HUMAN_SC_HELLO, pFriend );
			FriendsSpokeConcept( HUMAN_SC_HELLO );
		}
		else if ( !SpokeConcept( HUMAN_SC_DMG_HEAVY ) &&
			IsHealthInRange( pFriend, 0.0, 1.0/8.0 ) )
		{
			Speak( "HURTC", HUMAN_SC_DMG_HEAVY, pFriend );
		}
		else if ( !SpokeConcept( HUMAN_SC_DMG_MEDIUM ) &&
			IsHealthInRange( pFriend, 1.0/8.0, 1.0/4.0 ) )
		{
			Speak( "HURTB", HUMAN_SC_DMG_MEDIUM, pFriend );
		}
		else if ( !SpokeConcept( HUMAN_SC_DMG_LIGHT ) &&
			IsHealthInRange( pFriend, 1.0/4.0, 1.0/2.0 ) )
		{
			Speak( "HURTA", HUMAN_SC_DMG_LIGHT, pFriend );
		}
		else if ( !SpokeConcept( HUMAN_SC_WOUND ) &&
			IsHealthInRange( 1.0/3.0, 2.0/3.0 ) )
		{
			Speak( "WOUND", HUMAN_SC_WOUND, pFriend );
		}
		else if ( !SpokeConcept( HUMAN_SC_MORTAL ) &&
			IsHealthInRange( 0.0, 1.0/3.0 ) )
		{
			Speak( "MORTAL", HUMAN_SC_MORTAL, pFriend );
		}
		else if ( !SpokeConcept( HUMAN_SC_QUESTION ) &&
			random->RandomInt( 0, 4 ) == 0 )
		{
			Speak( "QUESTION", HUMAN_SC_QUESTION, pFriend );
			if ( pHuman )
				pHuman->SetQuestioner( this, m_flStopTalkTime );
		}
		else if ( !SpokeConcept( HUMAN_SC_IDLE ) &&
			random->RandomInt( 0, 4 ) == 0 )
		{
			Speak( "IDLE", HUMAN_SC_IDLE, pFriend );
		}
		else if ( pFriend->IsPlayer() && GetTimePlayerStaring() > 6 && !IsMoving() )
		{
			Speak( "STARE", HUMAN_SC_STARE, pFriend );
		}
#if 0
		// FIXME: saw a dead ally
		else if ( !SpokeConcept( HUMAN_SC_DEAD_ALLY ) &&
			HasCondition( COND_SQUADMEMBER_KILLED ) )
		{
			Speak( "DEAD", HUMAN_SC_DEAD_ALLY );
		}
#endif
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::PainSound( const CTakeDamageInfo &info )
{
	// No head etc
	if ( !OkToShout() ) return;

	// Make sure he doesn't just repeatedly go uh-uh-uh-uh-uh when hit by rapid fire
	if ( TimeNotPassed( m_flPainTime ) ) return;

	// FIXME: Don't play this if the damage killed us
	Speak( "PAIN" );

	m_flPainTime = max(gpGlobals->curtime,m_flStopTalkTime) + random->RandomFloat( 0.5, 0.75 );

	SquadIssueCommand( SQUADCMD_DISTRESS ); 
}

#if 1
class CNPCDeathSound : public CBaseEntity
{
public:
	DECLARE_CLASS( CNPCDeathSound, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNPCDeathSound()
	{
	};

	int UpdateTransmitState( void )
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	CNetworkString( m_szSoundName, 128 );
};

BEGIN_DATADESC( CNPCDeathSound )
	DEFINE_FIELD( m_szSoundName, FIELD_STRING ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPCDeathSound, DT_NPCDeathSound )
	SendPropString( SENDINFO( m_szSoundName ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( npc_death_sound, CNPCDeathSound );

#endif

//-----------------------------------------------------------------------------
void CHOEHuman::DeathSound( const CTakeDamageInfo &info )
{
	if ( !HasAHead() ) return;

	if ( ShouldGib( info ) ) return;

#if 1
	/* The death sound will play from our ragdoll. */
	SentenceStop();

	if ( HasSpawnFlags( SF_HUMAN_SERVERSIDE_RAGDOLL ) )
	{
	}
	else
	{
	//	Q_snprintf( m_szDeathSound.GetForModify(), sizeof(m_szDeathSound), "!%s_%s", GetSentenceGroup(), "DIE" );
	//	Q_strcpy( m_szDeathSound.GetForModify(), "NPC_Mikeforce.Die" );

		CNPCDeathSound *pEnt = (CNPCDeathSound *) CBaseEntity::Create( "npc_death_sound", GetAbsOrigin(), GetAbsAngles(), NULL );
		if ( pEnt )
		{
			Q_strcpy( pEnt->m_szSoundName.GetForModify(), "NPC_Mikeforce.Die" );
			m_hDeathSound = pEnt;
		}
	}
#else
	// FIXME: would be nice if the death sound was emitted from the ragdoll
	Speak( "DIE" );
#endif
	SquadIssueCommand( SQUADCMD_DISTRESS ); 

	// This lets zombies know where the eating is good
	// see CGib::WaitTillLand
	// FIXME: the ragdoll may fly away
	CSoundEnt::InsertSound( SOUND_CARCASS, GetAbsOrigin(), 384, 25 );
}

//-----------------------------------------------------------------------------
void CHOEHuman::FoundEnemySound( void )
{
	if ( !OkToShout() )
		return;

	Speak( "FOUND" );
}

#endif // HOE_HUMAN_RR