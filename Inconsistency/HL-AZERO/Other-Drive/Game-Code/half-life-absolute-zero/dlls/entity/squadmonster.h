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
//=========================================================
// CSquadMonster - all the extra data for monsters that
// form squads.
//=========================================================



#ifndef SQUADMONSTER_H
#define SQUADMONSTER_H



#define SF_SQUADMONSTER_LEADER	32


#define bits_NO_SLOT		0

// HUMAN GRUNT SLOTS
#define bits_SLOT_HGRUNT_ENGAGE1	( 1 << 0 )
#define bits_SLOT_HGRUNT_ENGAGE2	( 1 << 1 )
#define bits_SLOTS_HGRUNT_ENGAGE	( bits_SLOT_HGRUNT_ENGAGE1 | bits_SLOT_HGRUNT_ENGAGE2 )

#define bits_SLOT_HGRUNT_GRENADE1	( 1 << 2 ) 
#define bits_SLOT_HGRUNT_GRENADE2	( 1 << 3 ) 
#define bits_SLOTS_HGRUNT_GRENADE	( bits_SLOT_HGRUNT_GRENADE1 | bits_SLOT_HGRUNT_GRENADE2 )

// ALIEN GRUNT SLOTS
#define bits_SLOT_AGRUNT_HORNET1	( 1 << 4 )
#define bits_SLOT_AGRUNT_HORNET2	( 1 << 5 )
#define bits_SLOT_AGRUNT_CHASE		( 1 << 6 )
#define bits_SLOTS_AGRUNT_HORNET	( bits_SLOT_AGRUNT_HORNET1 | bits_SLOT_AGRUNT_HORNET2 )

// HOUNDEYE SLOTS
#define bits_SLOT_HOUND_ATTACK1		( 1 << 7 )
#define bits_SLOT_HOUND_ATTACK2		( 1 << 8 )
#define bits_SLOT_HOUND_ATTACK3		( 1 << 9 )
#define bits_SLOTS_HOUND_ATTACK		( bits_SLOT_HOUND_ATTACK1 | bits_SLOT_HOUND_ATTACK2 | bits_SLOT_HOUND_ATTACK3 )

// global slots
#define bits_SLOT_SQUAD_SPLIT		( 1 << 10 )// squad members don't all have the same enemy

#define NUM_SLOTS			11// update this every time you add/remove a slot.

#define MAX_SQUAD_MEMBERS	5

//=========================================================
// CSquadMonster - for any monster that forms squads.
//=========================================================
class CSquadMonster : public CBaseMonster 
{
public:

	BOOL alreadyDoneNetnameLeaderCheck;
	BOOL checkLeaderlessSquadByNetname(void);

	// squad leader info
	EHANDLE	m_hSquadLeader;		// who is my leader
	EHANDLE	m_hSquadMember[MAX_SQUAD_MEMBERS-1];	// valid only for leader
	int	m_afSquadSlots;
	float m_flLastEnemySightTime; // last time anyone in the squad saw the enemy
	BOOL m_fEnemyEluded;

	// squad member info
	int	m_iMySlot;// this is the behaviour slot that the monster currently holds in the squad. 

	//MODDD used to tell replacements (hgrunt -> hassault) not to re-squad; they should already start out with the old leader's squad.
	BOOL skipSquadStartup;
	BOOL disableLeaderChange;




	//MODDD - new.
	CSquadMonster(void);
	void ChangeLeader(CSquadMonster* oldLeader, CSquadMonster* newLeader);

	int CheckEnemy ( CBaseEntity *pEnemy );

	//MODDD - made virtual, never know if something could subclass even this and make its own version.
	virtual void StartMonster ( void );

	void VacateSlot( void );
	virtual void ScheduleChange( void );

	//MODDD - THIS USED TO NOT BE VIRTUAL!  WHY NOT???
	GENERATE_KILLED_PROTOTYPE_VIRTUAL


	//MODDD
	virtual void setModel(void);
	virtual void setModel(const char* m);
	virtual void Activate(void);
	virtual void Spawn(void);
	virtual const char* getGermanModel(void);
	virtual const char* getNormalModel(void);


	BOOL OccupySlot( int iDesiredSlot );
	virtual BOOL NoFriendlyFire( void );

	// squad functions still left in base class
	CSquadMonster *MySquadLeader( ) 
	{ 
		CSquadMonster *pSquadLeader = (CSquadMonster *)((CBaseEntity *)m_hSquadLeader); 
		if (pSquadLeader != NULL)
			return pSquadLeader;
		return this;
	}
	CSquadMonster *MySquadMember( int i ) 
	{ 
		if (i >= MAX_SQUAD_MEMBERS-1)
			return this;
		else
			return (CSquadMonster *)((CBaseEntity *)m_hSquadMember[i]); 
	}
	//TEMP DEBUG
	int InSquad ( void ) { return m_hSquadLeader != NULL; }
	//int InSquad ( void ) { return TRUE; }

	//TEMP DEBUG
	int IsLeader ( void ) { return m_hSquadLeader == this; }
	//int IsLeader ( void ) { return TRUE; }
	

	//MODDD - method never implemented.  No corresponding method (actions) body.  What's up, devs.
	//int SquadJoin ( int searchRadius );
	////UPDATE: gave it an implementation, and a new prototype that is better suited to what I have in mind.  I'm guessing the intent was to seek an existing squad to join if the spawned one can't find any other unassigned to start a squad with.
	virtual void SquadJoin ( int searchRadius, int maxMembers );

	//MODDD - virutal
	virtual int SquadRecruit ( int searchRadius, int maxMembers );

	void PrescheduleThink ( void );

	int SquadCount( void );
	void SquadRemove( CSquadMonster *pRemove );
	//MODDD - as-is never implemented methods, whoops
	//void SquadUnlink( void );
	BOOL SquadAdd( CSquadMonster *pAdd );
	//void SquadDisband( void );
	//void SquadAddConditions ( int iConditions );
	void SquadMakeEnemy ( CBaseEntity *pEnemy );
	void SquadPasteEnemyInfo ( void );
	void SquadCopyEnemyInfo ( void );
	BOOL SquadEnemySplit ( void );
	BOOL SquadMemberInRange( const Vector &vecLocation, float flDist );

	virtual CSquadMonster *MySquadMonsterPointer( void ) { return this; }

	static TYPEDESCRIPTION m_SaveData[];

	int Save( CSave &save ); 
	int Restore( CRestore &restore );

	//MADE VIRTUAL!!!
	virtual BOOL FValidateCover ( const Vector &vecCoverLocation );
	virtual MONSTERSTATE GetIdealState ( void );
	virtual Schedule_t	*GetScheduleOfType ( int iType );
	virtual void OnAlertedOfEnemy(void);


	//MODDD - HOW WERE YOU NOT VIRTUAL??!
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL
	

	virtual void SetActivity ( Activity NewActivity );
	//virtual void SetActivity ( Activity NewActivity, BOOL forceReset );
	
};


#endif //SQUADMONSTER_H

