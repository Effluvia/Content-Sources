

//Moved to its own file from cbase.h.


#ifndef ANIMATING_H
#define ANIMATING_H

#include "cbase.h"

//TEST. is this ok?
#include "studio.h"



//need this? m_iForceLoopsPreference
typedef enum{
	ANIM_LOOP_DEFAULT = -1,
	ANIM_LOOP_FORCE_NO = 0,
	ANIM_LOOP_FORCE_YES = 1,

} ANIM_LOOP_PREFERENCE;


class CBaseAnimating : public CBaseDelay
{
public:

	// animation needs
	float			m_flFrameRate;		// computed FPS for current sequence
	float			m_flGroundSpeed;	// computed linear movement rate for current sequence
	float			m_flLastEventCheck;	// last time the event list was checked
	BOOL			m_fSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	BOOL			m_fSequenceLoops;	// true if the sequence loops
	
	//MODDD - NEW. Stays on even after the animation loops around, only turned off on changing animation.
	//(properly with Reset that is)
	BOOL			m_fSequenceFinishedSinceLoop;// flag set when StudioAdvanceFrame moves across a frame boundry


	//MODDD - new, record the flInterval determined in by DetermineInterval calls for use throughout the rest of the methods,
	// presumably in the same frame of the monster's own think logic.
	float m_flInterval;



	float animFrameStart;
	float animFrameStartSuggestion;
	float animFrameCutoff;
	float animFrameCutoffSuggestion;
	
	float animationFPSSuggestion;
	float animationFramesSuggestion;
	float animationFPS;
	float animationFrames;

	
	//MODDD - immitate event animations.  Assign a max and fill the array with event times (just times, not including current time;  offsets from start of when to do events).
	//may place in CBaseMonster, basemonster.h (Monsters.cpp)...
	int animEventQueueMax;
	float animEventQueueStartTime;
	float animEventQueueTime[8];
	int animEventQueueID[8];
	int animEventQueueActive[8];



	//Anim Queue Stuff.
	void animEventQueuePush(float arg_timeFromStart, float arg_eventType);
	void resetEventQueue(void);
	void queuedAnimClear(void);

	//void resetEventQueue(void);
	//

	//MODDD
	char* queuedAnimName[8];
	char* queuedAnimNameSuggestion[8];

	int queuedAnimMax;
	int queuedAnimMaxSuggestion;
	int queuedAnimCurrent;
	int queuedAnimCurrentSuggestion;
	float queuedAnimFrameStart[8];
	float queuedAnimFrameStartSuggestion[8];
	float queuedAnimFrameCutoff[8];
	float queuedAnimFrameCutoffSuggestion[8];
	float queuedAnimFrameRate[8];
	float queuedAnimFrameRateSuggestion[8];
	
	//MODDD
	float m_flFramerateSuggestion;
	float m_iForceLoops;  //when negative one, ignore. Otherwise, 0 is false and 1 is true.




	//MODDD - constructor
	CBaseAnimating(void);




	virtual BOOL onResetBlend0(void);
	virtual BOOL onResetBlend1(void);
	virtual BOOL onResetBlend2(void);
	
	virtual BOOL canResetBlend0(void);
	virtual BOOL canResetBlend1(void);
	virtual BOOL canResetBlend2(void);




	void resetFrame(void);
	virtual int LookupActivityHard(int activity);
	virtual int tryActivitySubstitute(int activity);
	
	virtual void onAnimationLoop(void);

	void queuedAnimPush(char* arg_animName, float arg_frameStart, float arg_frameCutoff, float arg_framerate);
	void applyAnimationQueueSuggestion(void);

	void checkEndOfAnimation(void);
	



	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	// Basic Monster Animation functions

	//MODDD - NEW, flInterval-determining script ripped from StudioFrameAdvance, it requires an interval to be given
	// and does not return one now.
	float DetermineInterval(void);
	float DetermineInterval(float flInterval);
	float DetermineInterval_SAFE(void);
	float DetermineInterval_SAFE(float flInterval);
	

	//float StudioFrameAdvance( float flInterval = 0.0 ); // accumulate animation frame time from last time called until now
	void StudioFrameAdvance(float flInterval);
	// New versions of StudioFrameAdvance to act more like retail in one call, see notes in animating.cpp
	float StudioFrameAdvance_SIMPLE(void);
	float StudioFrameAdvance_SIMPLE(float flInterval);



	int GetSequenceFlags( void );
	
	//MODDD - virtual.  Able to be intercepted by a class whose model is missing the ACT linkups.
	virtual int LookupActivity ( int activity );
	virtual int LookupActivityHeaviest ( int activity );


	int LookupSequence ( const char *label );
	void ResetSequenceInfo ( );
	
	//MODDD
	void ResetSequenceInfoSafe ( );
	

	void DispatchAnimEvents ( float flInterval = 0.1 ); // Handle events that have happend since last time called up until X seconds into the future
	virtual void HandleAnimEvent( MonsterEvent_t *pEvent ) { return; };
	//MODDD - handle custom events!
	virtual void HandleEventQueueEvent(int arg_eventID){ return; };
	
	//MODDD - NEW
	void showHitboxInfoAll ();
	void showHitboxInfoOfBone (int argBone);
	void showHitboxInfoOfGroup (int argGroup);
	void showHitboxInfoNumber (int argID);


	void getHitboxInfoAll (mstudiobbox_t* argHitboxBuffer, int& argHitboxCount  );
	void getHitboxInfoOfBone (mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argBone);
	void getHitboxInfoOfGroup (mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argGroup);
	void getHitboxInfoNumber (mstudiobbox_t*& argHitbox, int argID);
	void getHitboxCount (int& argCount);
	
	float SetBoneController ( int iController, float flValue );


	//MODDD
	float SetBoneControllerUnsafe ( int iController, float flValue );
	


	void InitBoneControllers ( void );
	float SetBlending ( int iBlender, float flValue );
	void GetBonePosition ( int iBone, Vector &origin, Vector &angles );
	void GetAutomovement( Vector &origin, Vector &angles, float flInterval = 0.1 );
	int  FindTransition( int iEndingSequence, int iGoalSequence, int *piDir );
	void GetAttachment ( int iAttachment, Vector &origin, Vector &angles );
	void SetBodygroup( int iGroup, int iValue );
	int GetBodygroup( int iGroup );
	int ExtractBbox( int sequence, float *mins, float *maxs );
	void SetSequenceBox( void );

};

#endif //END OF ANIMATING_H