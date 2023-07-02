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
/*

===== monsters.cpp ========================================================

  Monster-related utility code

*/

#include "extdll.h"
#include "animating.h"
#include "basemonster.h"  //just to do some testing, not necessary.
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "util_model.h"
#include "saverestore.h"


EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)
EASY_CVAR_EXTERN_DEBUGONLY(animationPrintouts)



//MODDD - NOTE - found, as-is, using class "CBaseMonster" for class instead of "CBaseAnimating".  I'll just go with that I guess, reason for this?
TYPEDESCRIPTION	CBaseAnimating::m_SaveData[] =
{
	DEFINE_FIELD( CBaseMonster, m_flFrameRate, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseMonster, m_flGroundSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CBaseMonster, m_flLastEventCheck, FIELD_TIME ),
	DEFINE_FIELD( CBaseMonster, m_fSequenceFinished, FIELD_BOOLEAN ),
	//MODDD - save you too.
	DEFINE_FIELD( CBaseMonster, m_fSequenceFinishedSinceLoop, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseMonster, m_fSequenceLoops, FIELD_BOOLEAN ),

	//MODDD - save this?
	DEFINE_FIELD( CBaseMonster, m_flFramerateSuggestion, FIELD_BOOLEAN),

};
//IMPLEMENT_SAVERESTORE( CBaseAnimating, CBaseDelay );

int CBaseAnimating::Save( CSave &save )
{
	if ( !CBaseDelay::Save(save) )
		return 0;
	return save.WriteFields( "CBaseAnimating", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}
int CBaseAnimating::Restore( CRestore &restore )
{

	//if ( !CBaseDelay::Restore(restore) )
	//	return 0;
	//return restore.ReadFields( "CBaseAnimating", this, m_SaveData, ARRAYSIZE(m_SaveData) );
	int toReturn = 0;
	if ( !CBaseDelay::Restore(restore) ){
		toReturn = 0;
	}else{
		toReturn = restore.ReadFields( "CBaseAnimating", this, m_SaveData, ARRAYSIZE(m_SaveData) );
	}

	//to override a saved-value, you MUST edit it in this area, after having called the parent restore function.
	if(pev->deadflag != DEAD_DEAD){
		if (m_flFramerateSuggestion == 0) {
			// probably a mistake.
			m_flFramerateSuggestion = 1;
		}
	}

	//...here may be ok though?
	//setModelCustom();
	return toReturn;
}



//MODDD - new, constructor
CBaseAnimating::CBaseAnimating(void){
	//anim to jump to when this is done (disregarded by any animation change / stop).

	// should be set early on through a DetermineInterval before any think-logic sees it anyway.
	m_flInterval = 0.1;

	animationFPS = -1;
	animationFrames = -1;
	animationFPSSuggestion = -1;
	animationFramesSuggestion = -1;


	m_flFramerateSuggestion = -1;
	m_iForceLoops = -1;

	/*
	queuedAnim = NULL;
	queuedAnimSuggestion = NULL;
	earlyFrameCutoff = -1;
	earlyFrameCutoffSuggestion = -1;
	*/

	//char* queuedAnimName[8];
	//char* queuedAnimNameSuggestion[8];

	animFrameStart = -1;
	animFrameStartSuggestion = -1;
	animFrameCutoff = -1;
	animFrameCutoffSuggestion = -1;


	queuedAnimMax = 0;
	queuedAnimMaxSuggestion = 0;
	queuedAnimCurrent = 0;
	queuedAnimCurrentSuggestion = 0;
	//float queuedAnimFrameStart[8];
	//float queuedAnimFrameStartSuggestion[8];
	//float queuedAnimFrameCutoff[8];
	//float queuedAnimFrameCutoffSuggestion[8];

	m_fSequenceFinished = FALSE;
	m_fSequenceFinishedSinceLoop = FALSE;

}


void CBaseAnimating::onAnimationLoop(void){
	//nothing by default?
}

void CBaseAnimating::checkEndOfAnimation(void){

	// by default
	pev->renderfx &= ~STOPINTR;


	/*
	CBaseMonster* tempMon = this->GetMonsterPointer();
	if(tempMon != NULL && tempMon->monsterID >= -1){
		if(tempMon->monsterID == 6){
			int x = 45;
		}
	}
	*/

	//by default, this sequence is unfinished. The frame going past cutoff in the appropriate direction below will change this to TRUE instead.
	m_fSequenceFinished = FALSE;
	//But not you, m_fSequenceFinishedSinceLoop. You stay on until you have a reason to be set back to FALSE (new anim)
	//m_fSequenceFinishedSinceLoop = FALSE;


	if(pev->framerate == 0){
		return;
	}

	BOOL passedCutoff = FALSE;
	//A marker for whether cutoff was passed (I.E., at or passed the end of animation).

	//monster_stukabat
	//if(FClassnameIs(pev, "monster_human_grunt"))easyForcePrintLine("OH yeah?? %.2f : %.2f", pev->frame, animFrameCutoff);


	float tempCutoff;

	if(animFrameCutoff != -1){
		tempCutoff = animFrameCutoff;
	}else{
		//depends on framerate for direction.
		tempCutoff = (pev->framerate >= 0)?256:0;
	}

	if(FClassnameIs(this->pev, "monster_stukabat")){
		int wat = 4;
	}

	if(pev->framerate >= 0){
		//we count the frame being above the cutoff as "too far".
		if(pev->frame >= tempCutoff){
			passedCutoff = TRUE;

			if(animFrameStart != -1){
				//disable interp!
				pev->renderfx |= STOPINTR;
			}

			//positive framerate, play forward.
			if (m_fSequenceLoops){
				//Wait... why is this divided by 256, rounded to an int, and then multipled by 256? To remove the fraction? Why? Why not just subtract 256 solidly in the first place??
				//pev->frame -= (int)(pev->frame / 256.0) * 256.0;

				//start at 0, cutoff at 40?
				//at 41, subtract 40.

				pev->frame -= tempCutoff;
			}
			else{
				//pev->frame = (pev->frame < 0.0) ? 0 : 255;
				pev->frame = tempCutoff;
			}

		}
	}else{
		//we count the frame below the cutoff as "too far".
		if(pev->frame <= tempCutoff){
			passedCutoff = TRUE;
			
			if(animFrameStart != -1){
				//disable interp!
				pev->renderfx |= STOPINTR;
			}

			//negative framerate, play backwards.
			if (m_fSequenceLoops){
				//MODDD - any complications here??
				//pev->frame -= (int)(pev->frame / 256.0) * 256.0;
				//pev->frame += (int)((256+pev->frame) / 256.0) * 256.0;   //this won't be good. 256+negativeframe is most certainly less than 256. Divided by 256, that's a fraction, so rounded makes 0. times 256? 0! What?
				
				pev->frame += (256 - tempCutoff);
			}else{
				pev->frame = tempCutoff;
			}
			

		}
	}

	//(pev->frame <= 0.0 || pev->frame >= 255.0


	/*
	if(FClassnameIs(this->pev, "monster_friendly")){
	easyForcePrintLine("WELL...??? %d : %.2f", m_fSequenceLoops, pev->frame);
	}
	*/

	//easyPrintLine("WHAT DA hay %s %.2f %.2f !!! %.2f %.2f %d", STRING(pev->classname), pev->frame, animFrameCutoff, pev->framerate, m_flFramerateSuggestion, passedCutoff);


	//was originally < 0, >= 256.
	//if (pev->frame <= 0.0 || pev->frame >= 255.0 || (passedCutoff) )



	//General things for passing cutoff in either direction.
	if(passedCutoff)
	{
		
		m_fSequenceFinished = TRUE;
		m_fSequenceFinishedSinceLoop = TRUE;  //why yes it is.


		//Bound check just in case.
		if(pev->frame < 0){
			pev->frame = 0;
		}
		if(pev->frame > 256){
			pev->frame = 256;
		}


		if(m_fSequenceLoops){
			//all events active again!
			for(int i = 0; i < animEventQueueMax; i++){
				animEventQueueActive[i] = 1;
			}
			//for anything this may concern.
			onAnimationLoop();
		}


		/*

		//NO disregard THIS doodoo.
		//MODDD - Check for queued anim:
		if(queuedAnimMax > 0 && queuedAnimCurrent < queuedAnimMax){
			//use it.

			//easyPrintLine("OH BABY %d %d %s", queuedAnimCurrent, queuedAnimMax, queuedAnimName[queuedAnimCurrent]);

			int attemptSequence = LookupSequence(queuedAnimName[queuedAnimCurrent]);
			pev->sequence = attemptSequence;

			//ResetSequenceInfo();


			//easyPrintLine("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ANI");
			void *pmodel = GET_MODEL_PTR( ENT(pev) );

			GetSequenceInfo( pmodel, pev, &m_flFrameRate, &m_flGroundSpeed );

			//easyPrintLine("C THIS SEQUENCE SAYS SPEED IS %.2f", m_flGroundSpeed);

			m_fSequenceLoops = ((GetSequenceFlags() & STUDIO_LOOPING) != 0);
			pev->animtime = gpGlobals->time;

			//MODDD - "framerateSuggestion" can change this.
			//pev->framerate = 1.0;

			//framerateS = m_flFramerateSuggestion;
			//m_flFramerateSuggestion = -1;

			//!!!no, see later!
			//pev->framerate = m_flFramerateSuggestion;

			//m_flFramerateSuggestion = 1;
			//IS THAT OKAY?

			//pev->frame = 222;

			m_fSequenceFinished = FALSE;
			m_flLastEventCheck = gpGlobals->time;


			pev->framerate = queuedAnimFrameRate[queuedAnimCurrent];

			//resetFrame();
			//"-1" means, usual frame reset handling.
			if(queuedAnimFrameStart[queuedAnimCurrent] != -1){
				pev->frame = queuedAnimFrameStart[queuedAnimCurrent];
			}else{
				resetFrame();
			}

			if(queuedAnimFrameCutoff[queuedAnimCurrent] != -1){
				animFrameCutoff = queuedAnimFrameCutoff[queuedAnimCurrent];
			}else{
				animFrameCutoff = -1;
			}

			//DERP
			//SetYawSpeed();

			queuedAnimCurrent++;

		}//END OF if(queuedAnim is picked)
		else{

		*/
		//	m_fSequenceFinished = TRUE;	// just in case it wasn't caught in GetEvents
		//}

	}//END OF passedCutoff check.



	//if(FClassnameIs(pev, "monster_human_grunt")){
	//	easyForcePrintLine("SEQUENCE IS FINISHED? %d", m_fSequenceFinished);
	//}


	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1){
		CBaseMonster* tempMon = this->GetMonsterPointer();
		if(tempMon != NULL && tempMon->monsterID >= -1){
			easyForcePrintLine("%s:%d checkEndOfAnimation. Finished? %d", tempMon->getClassname(), tempMon->monsterID, m_fSequenceFinished);
		}
	}

}



//Set the model's current frame to the start, 0 if we're playing forwards.  Set it to the end (256) if playing backwards.
void CBaseAnimating::resetFrame(void){
	if(m_flFramerateSuggestion >= 0){
		//forward, start at beginning.
		pev->frame = 0;
	}else{
		//backward, start from end.
		pev->frame = 256;
	}
}

void CBaseAnimating::queuedAnimPush(char* arg_animName, float arg_frameStart, float arg_frameCutoff, float arg_framerate){
	queuedAnimNameSuggestion[queuedAnimMaxSuggestion] = arg_animName;
	queuedAnimFrameStartSuggestion[queuedAnimMaxSuggestion] = arg_frameStart;
	queuedAnimFrameCutoffSuggestion[queuedAnimMaxSuggestion] = arg_frameCutoff;
	queuedAnimFrameRateSuggestion[queuedAnimMaxSuggestion] = arg_framerate;

	queuedAnimMaxSuggestion++;

}

void CBaseAnimating::applyAnimationQueueSuggestion(void){
	animFrameStart = animFrameStartSuggestion;
	animFrameStartSuggestion = -1;
	animFrameCutoff = animFrameCutoffSuggestion;
	animFrameCutoffSuggestion = -1;

	if(animFrameStart != -1){
		//this anim starts here.
		pev->frame = animFrameStart;
	}

	for(int i = 0; i < queuedAnimMaxSuggestion; i++){
		queuedAnimName[i] = queuedAnimNameSuggestion[i];
		queuedAnimFrameStart[i] = queuedAnimFrameStartSuggestion[i];
		queuedAnimFrameCutoff[i] = queuedAnimFrameCutoffSuggestion[i];
		queuedAnimFrameRate[i] = queuedAnimFrameRateSuggestion[i];
	}

	queuedAnimMax = queuedAnimMaxSuggestion;
	queuedAnimCurrent = queuedAnimCurrentSuggestion;
	queuedAnimClear();
}




void CBaseAnimating::queuedAnimClear(void){
	//effectively ignores the queue.
	queuedAnimMaxSuggestion = 0;
	queuedAnimCurrentSuggestion = 0;
}
void CBaseAnimating::resetEventQueue(void){
	//pev->framerate = 1;
	animEventQueueMax = 0;
	animEventQueueStartTime = -1;

}


void CBaseAnimating::animEventQueuePush(float arg_timeFromStart, float arg_eventType){

	if(m_flFramerateSuggestion == 0){
		//nope.
		return;
	}

	float framerateSuggestionUse = m_flFramerateSuggestion;
	if(framerateSuggestionUse < 0){
		//must be positive.  Even playing backwards, events would occur in "positive" amounts of time from start.
		framerateSuggestionUse *= -1;
	}

	animEventQueueStartTime = gpGlobals->time;


	//CHANGIN' THE RULES.  Instead, an event will be at what pev->frame value will cause the event at the earliest (play once until a reset has been observed, for compatability with looping - anims)
	//animEventQueueTime[animEventQueueMax] = arg_timeFromStart / framerateSuggestionUse;

	//it appears "m_flFrameRate" factors in the frames AND framerate of an animation so that pev->frame
	//always stretches from 0 to 255. (With a possible factor of the before never-adjusted "pev->framerate").
	//... or "framerateSuggestionUse" for the intents of pev->framerate.

	//animEventQueueTime[animEventQueueMax] = m_flFrameRate / framerateSuggestionUse;

	//We can derive animation length from script!  After all, it needs to know in order for m_fFinished to activate (or to scale right to go from 0 to 256 in the time suggested by frames / framerate per anim)
	//Like so:   256 / m_flFrameRate = (frames - 1) / framerate = duration of animation.

	//...factor in our suggestion too.

	/*
	float animDuration = 256 / m_flFrameRate / framerateSuggestionUse;

	animEventQueueTime[animEventQueueMax] = arg_timeFromStart / animDuration * 255;

	//Send the event as a frame to check for having reached.
	*/

	animEventQueueTime[animEventQueueMax] = arg_timeFromStart;
	//...make this fit the Byte range on "resetSequenceInfo" AFTER we are sure we have the new model's information for framerate.

	//easyPrintLine("TIME::::%.2f :::%.2f, %.2f", gpGlobals->time, arg_timeFromStart, animEventQueueTime[animEventQueueMax]);
	//easyPrintLine("??? %.2f %.2f %.2f", animDuration, m_flFrameRate, framerateSuggestionUse);
	
	//this newly added event begins "Active".
	animEventQueueActive[animEventQueueMax] = 1;

	animEventQueueID[animEventQueueMax] = arg_eventType;
	animEventQueueMax++;

}




//MODDD - interval now determined in its own method to give more control to the caller.   Call this early on in a think method.
// Now split into parameter-less and parameter versions.  Same net behavior as retail to avoid any chances of unintended behavior.

// No parameter version, will determine flInterval (as though 0 were given, and was the default used when StudioFrameAdvance was
// called without an interval).
float CBaseAnimating::DetermineInterval(void) {
	float flInterval;

	flInterval = (gpGlobals->time - pev->animtime);
	if (flInterval <= 0.001)
	{
		pev->animtime = gpGlobals->time;
		m_flInterval = 0.0;
		return 0.0;
	}

	if (!pev->animtime) {
		flInterval = 0.0;
	}

	m_flInterval = flInterval;
	return flInterval;
}//DetermineInterval


// If an interval is given, only re-determine it if it is 0.  (but still do the !pev->animtime otherwise).
float CBaseAnimating::DetermineInterval(float flInterval) {
	if (flInterval == 0.0)
	{
		// includes the '!pev->animtime' check.  Also sets m_flInterval.
		flInterval = DetermineInterval();
		return flInterval;
	}else {
		// Do this check, retail would have.  And set m_flInterval.
		if (!pev->animtime) {
			flInterval = 0.0;
		}
		m_flInterval = flInterval;
		return flInterval;
	}
}//DetermineInterval



// Like above, but can't set pev->animtime.
float CBaseAnimating::DetermineInterval_SAFE(void) {
	float flInterval;

	// ???
	//return 0.5;

	flInterval = (gpGlobals->time - pev->animtime);
	if (flInterval <= 0.001)
	{
		//pev->animtime = gpGlobals->time;
		//m_flInterval = 0.0;
		return 0.0;
	}
	if (!pev->animtime) {
		flInterval = 0.0;
	}
	//m_flInterval = flInterval;
	return flInterval;
}//DetermineInterval_SAFE
float CBaseAnimating::DetermineInterval_SAFE(float flInterval) {
	if (flInterval == 0.0)
	{
		// includes the '!pev->animtime' check.  Also sets m_flInterval.
		flInterval = DetermineInterval_SAFE();
		return flInterval;
	}else {
		// Do this check, retail would have.  And set m_flInterval.
		if (!pev->animtime) {
			flInterval = 0.0;
		}
		m_flInterval = flInterval;
		return flInterval;
	}
}//DetermineInterval_SAFE





//=========================================================
// StudioFrameAdvance - advance the animation frame up to the current time
// if an flInterval is passed in, only advance animation that number of seconds
//=========================================================
// ALSO, no longer returns anything.  Cakk DetermineInterval and send that here for the same effect.
void CBaseAnimating::StudioFrameAdvance ( float flInterval )
{
	//const char* whut = STRING(pev->classname); //whut.

	
	//MODDD - old flInterval-on-0 determining script was here


	//MODDD NOTE - "m_flFrameRate" is unused, or supposed to be. "m_flFrameRateSuggestion" is a bit more flexible and is independent of it,, being the new
	//             value for "pev->framerate" at the start of the next called anim. Check first for any updates / replacements, but consider removal.

	//keep this for reference. ... TODO if we bother to do this.
	float recentFrameAdvance = flInterval * m_flFrameRate * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti);
	
	/*
	if(FClassnameIs(this->pev, "monster_panthereye")){
		int x = 666;
		easyForcePrintLine("MY RECENT FRAMETIME %.2f : %.2f", pev->frame, recentFrameAdvance);
		//	easyPrintLine("DOES IT LOOP?! %d", m_fSequenceLoops);
	}
	*/


	if(!FClassnameIs(pev, "player")){
		int x = 45;  //breakpoint.
	}

	pev->frame += recentFrameAdvance;
	pev->animtime = gpGlobals->time;
	//if (FClassnameIs(pev, "monster_zombie"))easyForcePrintLine("pev->animtime set B %.2f", pev->animtime);
	
	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1){
		CBaseMonster* tempMon = this->GetMonsterPointer();
		if(tempMon != NULL && tempMon->monsterID >= -1){
			easyForcePrintLine("%s:%d StudioFrameAdvance. Frame change: %.2f", tempMon->getClassname(), tempMon->monsterID, recentFrameAdvance);
		}
	}

	if(this->crazyPrintout == TRUE){
		//easyForcePrintLine("STUDIOFRAMEADVANCE:::frame:%.2f interv:%.2f animtime:%.2f m_flFR:%.2f pev->fr:%.2f time:%.2f", pev->frame, flInterval, pev->animtime, m_flFrameRate, pev->framerate, gpGlobals->time);
	}

	checkEndOfAnimation();

	//MODDD - method no longer gives interval, call DetermineInterval above before calling StudioFrameAdvance and give the interval from
	// that for the same effect.
	//return flInterval;
}



//MODDD - and new versions of StudioFrameAdvance that act more directly like retail.
// Both assume flInterval has to be checked by DetermineInterval, so no need to do that
// in the classes that use these forms.
// But still, it is better to call DetermineInterval separately early on in a think cycle
// for the benefit of other monsterthink logic that might need to see m_flInterval ahead of
// time.
// Returns it too.
// SHORT VERSION:   use this in place of calls to StudioFrameAdvance that don't have a value supplied from DetermineInterval.
// Even calls given constants like 'StudioFrameAdvance( 0.1 )' in barnacle.cpp or aflock.cpp. They don't set m_flFrame at all then.
float CBaseAnimating::StudioFrameAdvance_SIMPLE(void) {
	float flInterval = DetermineInterval();
	StudioFrameAdvance(flInterval);
	return flInterval;
}
float CBaseAnimating::StudioFrameAdvance_SIMPLE(float flInterval) {
	flInterval = DetermineInterval(flInterval);
	StudioFrameAdvance(flInterval);
	return flInterval;
}




//=========================================================
// LookupActivity
//=========================================================
int CBaseAnimating::LookupActivity ( int activity )
{
	ASSERT( activity != 0 );
	if(activity == 0){
		int x = 45;
	}

	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return ::LookupActivity( pmodel, pev, activity );
}


//=========================================================
// LookupActivity
//=========================================================
//same as normal lookup by default...
int CBaseAnimating::LookupActivityHard ( int activity )
{
	ASSERT( activity != 0 );
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return ::LookupActivity( pmodel, pev, activity );
}

//ditto.
int CBaseAnimating::tryActivitySubstitute(int activity){
	ASSERT( activity != 0 );
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return ::LookupActivity( pmodel, pev, activity );
}


//=========================================================
// LookupActivityHeaviest
//
// Get activity with highest 'weight'
//
//=========================================================
int CBaseAnimating::LookupActivityHeaviest ( int activity )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return ::LookupActivityHeaviest( pmodel, pev, activity );
}

//=========================================================
//=========================================================
int CBaseAnimating::LookupSequence ( const char *label )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return ::LookupSequence( pmodel, label );
}



//=========================================================
//=========================================================
void CBaseAnimating::ResetSequenceInfo ( )
{
	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1){
		CBaseMonster* tempMon = this->GetMonsterPointer();
		if(tempMon != NULL && tempMon->monsterID >= -1){
			easyForcePrintLine("%s:%d ResetSequenceInfo.", tempMon->getClassname(), tempMon->monsterID);
		}
	}

	//if(FClassnameIs(this->pev, "monster_human_grunt")){
	//	easyPrintLine("ahh its a grunt");
	//}

	//easyPrintLine("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ANI");
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	//m_flGroundSpeed?!!   NICE!
	//"m_flGroundSpeed" is what will be used to move this monster by X units (location-wise) in the actual world.
	GetSequenceInfo( pmodel, pev, &m_flFrameRate, &m_flGroundSpeed );
	//easyPrintLine("A THIS SEQUENCE SAYS SPEED IS %.2f", m_flGroundSpeed);

	
	if(m_iForceLoops == 0){
		//force off.
		m_fSequenceLoops = FALSE;
	}else if(m_iForceLoops == 1){
		m_fSequenceLoops = TRUE;
	}else{
		//get from the model as usual.
		m_fSequenceLoops = ((GetSequenceFlags() & STUDIO_LOOPING) != 0);
	}


	pev->animtime = gpGlobals->time;
	//if (FClassnameIs(pev, "monster_zombie"))easyForcePrintLine("pev->animtime set C %.2f", pev->animtime);

	//MODDD - "framerateSuggestion" can change this.
	//pev->framerate = 1.0;

	//framerateS = m_flFramerateSuggestion;
	//m_flFramerateSuggestion = -1;

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//MODDD NOTE - looks like "m_flFrameRate" stores the raw framerate of the model, while "m_flFramerateSuggestion" stores the multiple
	//             to apply, such as "2" for twice as fast or "0.5" for half as fast. As in, it is relative.

	
	animationFPS = animationFPSSuggestion;
	animationFrames = animationFramesSuggestion;
	animationFPSSuggestion = -1;
	animationFramesSuggestion = -1;

	pev->framerate = m_flFramerateSuggestion;
	//m_flFramerateSuggestion = 1;
	//IS THAT OKAY?

	//pev->frame = 222;

	m_fSequenceFinished = FALSE;
	m_fSequenceFinishedSinceLoop = FALSE;  //new anim? a loop has not had a chance to be completed (that counts).

	m_flLastEventCheck = gpGlobals->time;


	//MODDD - CHECK: are there consequences for doing this???
	//pev->frame = 0;

	//queuedAnim = NULL;
	//earlyFrameCutoff = -1;

	//easyPrintLine("WHO ARE YOU EVEN???!!! %.2f", m_flFramerateSuggestion);

	float framerateSuggestionUse = m_flFramerateSuggestion;
	if(framerateSuggestionUse < 0){
		//must be positive.  Even playing backwards, events would occur in "positive" amounts of time from start.
		framerateSuggestionUse *= -1;
	}


	framerateSuggestionUse = 1;
	//NOTE: events care not about the framerate picked.  They are still meant to be made proprtional to 255.

	if(m_flFrameRate != 0 && framerateSuggestionUse != 0){

		float animDuration = 256.0f / m_flFrameRate / framerateSuggestionUse;

		for(int i = 0; i<animEventQueueMax; i++){
			//easyPrintLine("ANIMEVE %d:::(norm: %.2f moddd: %.2f)", i, animEventQueueTime[i] / animDuration * 255.0f, animEventQueueTime[i] / (256.0f / m_flFrameRate / 1) * 255.0f);
			animEventQueueTime[i] = animEventQueueTime[i] / animDuration * 255.0f;
		}
	}else{
		this->resetEventQueue();
	}


	if(canResetBlend0()){
		BOOL resetThisBlend = onResetBlend0();
		if(!resetThisBlend){
			SetBlending(0, 0);
		}
	}
	if(canResetBlend1()){
		BOOL resetThisBlend = onResetBlend1();
		if(!resetThisBlend){
			SetBlending(1, 0);
		}
	}
	if(canResetBlend2()){
		BOOL resetThisBlend = onResetBlend2();
		if(!resetThisBlend){
			SetBlending(2, 0);
		}
	}


	applyAnimationQueueSuggestion();

	//MODDD
	//something in an outside context, (whatever is calling "ResetSequenceInfo") needs to set "usingCustomSequence" to TRUE if the method is called
	//with the intention of keeping the anim past looping during an idle animation, and only AFTER calling this method.
	//This forced setting to FALSE in ResetSequenceInfo is a safety feature so that animations later are not treated as custom / idle-blocking.
	usingCustomSequence = FALSE;
	//MODDD - NEW.  Safety.  If told to reset this sequence to itself (reset frame), the frame-reset
	// will not happen, if this is on.  By default, OFF.
	doNotResetSequence = FALSE;

}

//MODDD - new.  The BOOL says, if TRUE,  "I already set the blend myself, no need to do it yourself."
BOOL CBaseAnimating::onResetBlend0(void){
	return FALSE;
}
BOOL CBaseAnimating::onResetBlend1(void){
	return FALSE;
}
BOOL CBaseAnimating::onResetBlend2(void){
	return FALSE;
}

//can this class even reset the blend for #0, 1, 2?
BOOL CBaseAnimating::canResetBlend0(void){
	return FALSE;
}
BOOL CBaseAnimating::canResetBlend1(void){
	return FALSE;
}
BOOL CBaseAnimating::canResetBlend2(void){
	return FALSE;
}




void CBaseAnimating::ResetSequenceInfoSafe ( )
{
	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1){
		CBaseMonster* tempMon = this->GetMonsterPointer();
		if(tempMon != NULL && tempMon->monsterID >= -1){
			easyForcePrintLine("%s:%d ResetSequenceInfoSafe.", tempMon->getClassname(), tempMon->monsterID);
		}
	}

	//if(FClassnameIs(this->pev, "monster_human_grunt")){
	//	easyPrintLine("ahhhh its a grunt 2");
	//}

	//easyPrintLine("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ANI");
	void *pmodel = GET_MODEL_PTR( ENT(pev) );


	GetSequenceInfoSafe( pmodel, &m_flFrameRate, pev );
	//easyPrintLine("B THIS SEQUENCE AINT GOT TIME FO SPEED");
	
	if(m_iForceLoops == 0){
		//force off.
		m_fSequenceLoops = FALSE;
	}else if(m_iForceLoops == 1){
		m_fSequenceLoops = TRUE;
	}else{
		//get from the model as usual.
		m_fSequenceLoops = ((GetSequenceFlags() & STUDIO_LOOPING) != 0);
	}


	pev->animtime = gpGlobals->time;
	//if (FClassnameIs(pev, "monster_zombie"))easyForcePrintLine("pev->animtime set D %.2f", pev->animtime);

	//MODDD - "framerateSuggestion" can change this.
	//pev->framerate = 1.0;
	pev->framerate = m_flFramerateSuggestion;
	//pev->frame = 222;
	
	m_fSequenceFinished = FALSE;
	m_fSequenceFinishedSinceLoop = FALSE;  //new anim? a loop has not had a chance to be completed (that counts).

	m_flLastEventCheck = gpGlobals->time;


	//MODDD - CHECK: are there consequences for doing this???
	//pev->frame = 0;

	//queuedAnim = NULL;
	//earlyFrameCutoff = -1;

	
	float framerateSuggestionUse = m_flFramerateSuggestion;
	if(framerateSuggestionUse < 0){
		//must be positive.  Even playing backwards, events would occur in "positive" amounts of time from start.
		framerateSuggestionUse *= -1;
	}

	if(m_flFrameRate != 0 && framerateSuggestionUse != 0){
		float animDuration = 256 / m_flFrameRate / framerateSuggestionUse;
		for(int i = 0; i<animEventQueueMax; i++){
			animEventQueueTime[i] = animEventQueueTime[i] / animDuration * 255;
		}
	}else{
		this->resetEventQueue();
	}
	
	applyAnimationQueueSuggestion();
	//MODDD - see note above about "usingCustomSequence".
	usingCustomSequence = FALSE;
	doNotResetSequence = FALSE;
}



//=========================================================
//=========================================================
BOOL CBaseAnimating::GetSequenceFlags( )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return ::GetSequenceFlags( pmodel, pev );
}

//=========================================================
// DispatchAnimEvents
//=========================================================
void CBaseAnimating::DispatchAnimEvents ( float flInterval )
{

	MonsterEvent_t	event;

	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	if(pev->framerate == 0){
		//no framerate? No events.
		return;
	}


	if ( !pmodel )
	{
		ALERT( at_aiconsole, "Gibbed monster is thinking!\n" );
		return;
	}

	// FIXME: I have to do this or some events get missed, and this is probably causing the problem below
	flInterval = 0.1;


	/*
	if(!this->m_fSequenceLoops){
		//Option to stop event checks. If not looping and on the last frame, don't do checks.
		if(pev->framerate >= 0){
			if(pev->frame >= 256){
				return;
			}
		}else if(pev->framerate < 0){
			if(pev->frame <= 0){
				return;
			}
		}
	}
	*/

	// FIX: this still sometimes hits events twice
	float flStart = pev->frame + (m_flLastEventCheck - pev->animtime) * m_flFrameRate * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti);
	float flEnd = pev->frame + flInterval * m_flFrameRate * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti);


	m_flLastEventCheck = pev->animtime + flInterval;

	//if(FClassnameIs(pev, "monster_stukabat") || FClassnameIs(pev, "monster_human_assassin")){
	//	easyForcePrintLine("fin:%d loops:%d fr:%.2f START: %.2f END:%.2f", m_fSequenceFinished, m_fSequenceLoops, pev->frame, flStart, flEnd);
	//}


	//MODDD - Any harm in only letting "StudioFrameAdvance" handle this...?

	if(this->crazyPrintout == TRUE){
		//easyForcePrintLine("DISPATCHANIMEVENTS:::flStart:%.2f flEnd: %.2f  interv: %.2f animtime:%.2f frame:%.2f m_flFR:%.2f pev->FR:%.2f", flStart, flEnd, flInterval, pev->animtime, pev->frame, m_flFrameRate, pev->framerate);
	}

	
	if(EASY_CVAR_GET_DEBUGONLY(animationPrintouts) == 1){
		CBaseMonster* tempMon = this->GetMonsterPointer();
		if(tempMon != NULL && tempMon->monsterID >= -1){
			easyForcePrintLine("%s:%d DispatchAnimEvents. Setting m_fSequenceFinished to false and re-checking.", tempMon->getClassname(), tempMon->monsterID);
		}
	}


	//m_fSequenceFinished = FALSE;
	//NO. This does not take the cutoff into effect!
	//
	//if (flEnd >= 256 || flEnd <= 0.0)
	//	m_fSequenceFinished = TRUE;


	/*
	m_fSequenceFinished = FALSE;
	//wait... what's the point of the end of animation check here? We expect "StudioFrameAdvance" to handle
	//moving the frame foward and doing the check, which is called separately from the same place calling here?
	checkEndOfAnimation();
	*/

	//Yea... just don't do the check here, we only really need to record whether this anim is finished or not above.


	int index = 0;

	//MODDD - now sends along "m_fSequenceLoops", which may have been forced by script compared to what the model states to be.
	while ( (index = GetAnimationEvent( pmodel, pev, &event, flStart, flEnd, index, m_fSequenceLoops ) ) != 0 )
	{
		HandleAnimEvent( &event );
	}



	//MODDD TODO - there is already a great animation system in GetAnimationEvent. It could be better replicated in the custom
	//             animation system below too.


	//easyPrintLine("MAXIMUM event: %d", animEventQueueMax);
	//MODDD - Do my custom events here too, why not?
	for(int i = 0; i < animEventQueueMax; i++){

		/*
		if(FClassnameIs(this->pev, "monster_human_grunt") ){
			//easyPrintLine("cusevent: %d %d G: %.2f %.2f E: %d", i, animEventQueueActive[i], gpGlobals->time, animEventQueueTime[i], animEventQueueID[i]);
		}
		*/

		if(animEventQueueActive[i] == 0){
			//if turned off, nope.
			continue;
		}

		BOOL eventDue = FALSE;

		if(pev->framerate >= 0){

			//event checks see that the event is above the picked frame.
			if(pev->frame >= animEventQueueTime[i]){
				eventDue = TRUE;
			}
		}else{
			//below the picked frame.
			if(pev->frame <= animEventQueueTime[i]){
				eventDue = TRUE;
			}
		}

		if(eventDue){
			animEventQueueActive[i] = 0;
			HandleEventQueueEvent( animEventQueueID[i]);
		}
	}//END OF for(...)   for custom events.

}//END OF DispatchAnimEvents




//=========================================================
//=========================================================
void CBaseAnimating::showHitboxInfoAll ( )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::showHitboxInfoAll( pmodel, pev );
}
void CBaseAnimating::showHitboxInfoOfBone (int argBone)
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::showHitboxInfoOfBone( pmodel, pev, argBone );
}
void CBaseAnimating::showHitboxInfoOfGroup (int argGroup)
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::showHitboxInfoOfGroup( pmodel, pev, argGroup );
}
void CBaseAnimating::showHitboxInfoNumber (int argID)
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::showHitboxInfoNumber( pmodel, pev, argID );
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
//First three methods: getHitboxInfoAll, getHitboxInfoOfBone, and getHitboxInfoOfGroup,
//    expect two arguments at least: a place (array) to put copies of found hitboxes for
//    reading, and an int to put the size (# of items) of the array after use.
//    (may want to heck for a returned quantity of 0: no results found)
//Last method, getHitboxInfoNumber, only requires an ordinary pointer (not array).
//    If found, sets the pointer to refer to a single hitbox if found. Otherwise, it is null.
//To use one of the first three methods (demonstrating getHitboxInfoOfGroup, tested on hgrunt):
/*
	int i = 0;
	mstudiobbox_t hitboxBuffer[100];
	int hitboxBufferCount;
	this->getHitboxInfoOfGroup(hitboxBuffer, hitboxBufferCount, 6);
	for(i = 0; i < hitboxBufferCount; i++)
	{
	    // do something with this particular "currentHitboxRef".
		// Runs for each hitbox that maps to the picked bone. Example:
		easyForcePrintLine("HITBOX STATS - max:(%.2F, %.2F, %.2F) bone:%d grp:%d", i, hitboxBuffer[i].bbmax.x, hitboxBuffer[i].bbmax.y, hitboxBuffer[i].bbmax.z, hitboxBuffer[i].bone, hitboxBuffer[i].group);
	}
*/
//To iterate through hitboxes on your own:
/*
    int i = 0;
    mstudiobbox_t* currentHitboxRef;
	int hitboxCount;
    getHitboxInfoNumber(currentHitboxRef, 0);
	getHitboxCount(hitboxCount);
    for(i = 0; i < hitboxCount; i++, currentHitboxRef++)
	{
	    // do something with this particular "currentHitboxRef".
		// Runs for each hitbox there is. Example:
		easyForcePrintLine("HITBOX STATS - max:(%.2F, %.2F, %.2F) bone:%d grp:%d", i, currentHitboxRef->bbmax.x, currentHitboxRef->bbmax.y, currentHitboxRef->bbmax.z, currentHitboxRef->bone, currentHitboxRef->group);
	}
*/
void CBaseAnimating::getHitboxInfoAll (mstudiobbox_t* argHitboxBuffer, int& argHitboxCount  )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::getHitboxInfoAll( pmodel, pev, argHitboxBuffer, argHitboxCount );
}
void CBaseAnimating::getHitboxInfoOfBone (mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argBone)
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::getHitboxInfoOfBone( pmodel, pev, argHitboxBuffer, argHitboxCount, argBone );
}
void CBaseAnimating::getHitboxInfoOfGroup (mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argGroup)
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::getHitboxInfoOfGroup( pmodel, pev, argHitboxBuffer, argHitboxCount, argGroup );
}
void CBaseAnimating::getHitboxInfoNumber (mstudiobbox_t*& argHitbox, int argID)
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::getHitboxInfoNumber( pmodel, pev, argHitbox, argID );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

//Get the number of hitboxes in this model.
void CBaseAnimating::getHitboxCount (int& argCount)
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	::getHitboxCount( pmodel, pev, argCount );
}




//=========================================================
//=========================================================
float CBaseAnimating::SetBoneController ( int iController, float flValue )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return SetController( pmodel, pev, iController, flValue );
}


//MODDD
// also, WARNING!!! This skips boundary checks for the controller (Besides being a valid
// controller ID at all) and does not scale the given flValue to what the controller
// expects in value.  That is, if the bone controller expects a value between 0 and 20,
// and you give the call a value of 18, the bone will be set to purely that 18,
// even though that's way smaller of an effect than you might have intended
// (always out of 255!).
float CBaseAnimating::SetBoneControllerUnsafe ( int iController, float flValue )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return SetControllerUnsafe( pmodel, pev, iController, flValue );
}



//=========================================================
//=========================================================
void CBaseAnimating::InitBoneControllers ( void )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	SetController( pmodel, pev, 0, 0.0 );
	SetController( pmodel, pev, 1, 0.0 );
	SetController( pmodel, pev, 2, 0.0 );
	SetController( pmodel, pev, 3, 0.0 );
}

//=========================================================
//=========================================================
float CBaseAnimating::SetBlending ( int iBlender, float flValue )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	return ::SetBlending( pmodel, pev, iBlender, flValue );
}

//=========================================================
//=========================================================
void CBaseAnimating::GetBonePosition ( int iBone, Vector &origin, Vector &angles )
{
	GET_BONE_POSITION( ENT(pev), iBone, origin, angles );
}

//=========================================================
//=========================================================
void CBaseAnimating::GetAttachment ( int iAttachment, Vector &origin, Vector &angles )
{
	GET_ATTACHMENT( ENT(pev), iAttachment, origin, angles );
}

//=========================================================
//=========================================================
int CBaseAnimating::FindTransition( int iEndingSequence, int iGoalSequence, int *piDir )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	if (piDir == NULL)
	{
		int iDir;
		int sequence = ::FindTransition( pmodel, iEndingSequence, iGoalSequence, &iDir );
		if (iDir != 1)
			return -1;
		else
			return sequence;
	}

	return ::FindTransition( pmodel, iEndingSequence, iGoalSequence, piDir );
}


//MODDD - NOTE - method found empty.  Uh, what?
//=========================================================
//=========================================================
void CBaseAnimating::GetAutomovement( Vector &origin, Vector &angles, float flInterval )
{

}

void CBaseAnimating::SetBodygroup( int iGroup, int iValue )
{
	::SetBodygroup( GET_MODEL_PTR( ENT(pev) ), pev, iGroup, iValue );
}

int CBaseAnimating::GetBodygroup( int iGroup )
{
	return ::GetBodygroup( GET_MODEL_PTR( ENT(pev) ), pev, iGroup );
}


int CBaseAnimating::ExtractBbox( int sequence, float *mins, float *maxs )
{
	return ::ExtractBbox( GET_MODEL_PTR( ENT(pev) ), sequence, mins, maxs );
}

//=========================================================
//=========================================================

void CBaseAnimating::SetSequenceBox( void )
{
	Vector mins, maxs;

	// Get sequence bbox
	if ( ExtractBbox( pev->sequence, mins, maxs ) )
	{
		// expand box for rotation
		// find min / max for rotations
		float yaw = pev->angles.y * (M_PI / 180.0f);

		Vector xvector, yvector;
		xvector.x = cos(yaw);
		xvector.y = sin(yaw);
		yvector.x = -sin(yaw);
		yvector.y = cos(yaw);
		Vector bounds[2];

		bounds[0] = mins;
		bounds[1] = maxs;

		Vector rmin( 9999, 9999, 9999 );
		Vector rmax( -9999, -9999, -9999 );
		Vector base, transformed;

		for (int i = 0; i <= 1; i++ )
		{
			base.x = bounds[i].x;
			for ( int j = 0; j <= 1; j++ )
			{
				base.y = bounds[j].y;
				for ( int k = 0; k <= 1; k++ )
				{
					base.z = bounds[k].z;

				// transform the point
					transformed.x = xvector.x*base.x + yvector.x*base.y;
					transformed.y = xvector.y*base.x + yvector.y*base.y;
					transformed.z = base.z;

					if (transformed.x < rmin.x)
						rmin.x = transformed.x;
					if (transformed.x > rmax.x)
						rmax.x = transformed.x;
					if (transformed.y < rmin.y)
						rmin.y = transformed.y;
					if (transformed.y > rmax.y)
						rmax.y = transformed.y;
					if (transformed.z < rmin.z)
						rmin.z = transformed.z;
					if (transformed.z > rmax.z)
						rmax.z = transformed.z;
				}
			}
		}
		rmin.z = 0;
		rmax.z = rmin.z + 1;
		UTIL_SetSize( pev, rmin, rmax );
	}
}

