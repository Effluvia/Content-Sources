#define TLK_ALERT "TLK_ALERT" // I just saw an enemy
#define TLK_CHARGE "TLK_CHARGE" // I'm about to shoot the enemy
#define TLK_COVER "TLK_COVER" // I'm about to take cover
#define TLK_DEAD "TLK_DEAD" // I see a dead friend
#define TLK_EXPL "TLK_EXPL" // I got blown to bits
#define TLK_FOUND "TLK_FOUND" // Found an enemy that eluded me
#define TLK_GREN "TLK_GREN" // Hear a grenade
#define TLK_HEAD "TLK_HEAD" // See a severed head
#define TLK_HEAL "TLK_HEAL" // I'm a medic announcing I'm about to heal someone
#define TLK_HEALED "TLK_HEALED" // A medic healed me
#define TLK_HEAR "TLK_HEAR" // I hear something
#define TLK_HELLO "TLK_HELLO" // Say hello to player/npc
#define TLK_HELP "TLK_HELP" // I need help
#define TLK_HURTA "TLK_HURTA" // Friend has a small wound
#define TLK_HURTB "TLK_HURTB" // Friend has a medium wound
#define TLK_HURTC "TLK_HURTC" // Friend has a large wound
#define TLK_IDLE "TLK_IDLE" // Idle speech
#define TLK_KILL "TLK_KILL" // I killed an enemy
#define TLK_MAD "TLK_MAD" // I'm a friendly shot by player, get mad at player and kill him
#define TLK_MEDIC "TLK_MEDIC" // I need a medic
#define TLK_MORTAL "TLK_MORTAL" // I've got a mortal wound
#define TLK_NOSHOOT "TLK_NOSHOOT" // Tell the player not to shoot my buddy
#define TLK_OUTWAY "TLK_OUTWAY" // I'm moving out of the player's way
#define TLK_PAIN "TLK_PAIN" // Ouch!
#define TLK_PIDLE "TLK_PIDLE" // Pre-disaster idle speech
#define TLK_POK "TLK_POK" // Pre-disaster +use speech
#define TLK_QUESTION "TLK_QUESTION" // I'm asking a friend something
#define TLK_SHOT "TLK_SHOT" // I'm a friendly shot by player, tell player to be careful
#define TLK_SMELL "TLK_SMELL" // I smell something stinky
#define TLK_SQUAD_ACK_GENERIC "TLK_SQUAD_ACK_GENERIC" // Acknowledge a squad order
#define TLK_SQUAD_ACK_CHECK "TLK_SQUAD_ACK_CHECK" // Acknowledge check-in order
#define TLK_SQUAD_ACK_RETREAT "TLK_SQUAD_ACK_RETREAT" // Acknowledge retreat order
#define TLK_SQUAD_ACK_SEARCH "TLK_SQUAD_ACK_SEARCH" // Acknowledge search-and-destroy order
#define TLK_SQUAD_ACK_SUPPRESS "TLK_SQUAD_ACK_SUPPRESS" // Acknowledge suppressing-fire order
#define TLK_SQUAD_ATTACK "TLK_SQUAD_ATTACK" // Tell my squad to attack the enemy
#define TLK_SQUAD_CHECK "TLK_SQUAD_CHECK" // Tell my squad to check in
#define TLK_SQUAD_COME "TLK_SQUAD_COME" // Tell my squad to come to my position
#define TLK_SQUAD_RETREAT "TLK_SQUAD_RETREAT" // Tell my squad to retreat
#define TLK_SQUAD_SEARCH "TLK_SQUAD_SEARCH" // Tell my squad to search-and-destroy
#define TLK_SQUAD_SUPPRESS "TLK_SQUAD_SUPPRESS" // Tell my squad to lay down suppressing fire
#define TLK_STARE "TLK_STARE" // The player is staring at me
#define TLK_TAUNT "TLK_TAUNT" // Tell enemy I'm gonna get him
#define TLK_THROW "TLK_THROW" // Announce I'm throwing a grenade
#define TLK_UNUSE "TLK_UNUSE" // Kicked out of the player's squad
#define TLK_USE "TLK_USE" // Added to the player's squad
#define TLK_WOUND "TLK_WOUND" // I've got a wound

enum SpeechManagerID_t
{
	INVALID_SPEECH_MANAGER = -1,
	SPEECH_MANAGER_ALLY = 0,
	SPEECH_MANAGER_CHARLIE,
	SPEECH_MANAGER_GRUNT,
	MAX_SPEECH_MANAGER,
};

enum SpeechManagerCategory_t
{
	INVALID_SPEECH_CATEGORY = -1,
	SPEECH_CATEGORY_IDLE = 0,
	SPEECH_CATEGORY_INJURY,
	MAX_SPEECH_CATEGORY
};

struct SpeechManagerConceptInfo_t
{
	AIConcept_t concept;
	float minDelay;
	float maxDelay;
};

class CHOEHuman;

class CHumanSpeechManager : public CLogicalEntity
{
public:
	DECLARE_CLASS( CHumanSpeechManager, CLogicalEntity );
	DECLARE_DATADESC();

	CHumanSpeechManager();
	~CHumanSpeechManager();

	void Spawn( void );
	void OnRestore( void );

	void OnSpokeConcept( CHOEHuman *pSpeaker, AIConcept_t concept, AI_Response *response  );
	SpeechManagerConceptInfo_t *GetConceptInfo( AIConcept_t concept );

	void ExtendSpeechConceptTimer( AIConcept_t concept, float flDelay );
	bool ConceptDelayExpired( AIConcept_t concept );
	CUtlMap<string_t, CSimpleSimTimer, char> m_ConceptTimers;
	CUtlMap<AIConcept_t, SpeechManagerConceptInfo_t *> m_ConceptInfoMap;

	void ExtendSpeechCategoryTimer( SpeechManagerCategory_t category, float flDelay );
	CSimpleSimTimer &GetSpeechCategoryTimer( SpeechManagerCategory_t category );
	CSimpleSimTimer	m_SpeechCategoryTimer[MAX_SPEECH_CATEGORY];
	SpeechManagerID_t m_ID;

	static CHumanSpeechManager *GetSpeechManager( SpeechManagerID_t managerID );
	static CHumanSpeechManager *gm_pSpeechManager[MAX_SPEECH_MANAGER];
};

struct SpeechSelection_t
{
	SpeechSelection_t()
	 :	pResponse(NULL)
	{
	}
	
	AIConcept_t 		concept;
	AI_Response *		pResponse;
	EHANDLE				hSpeechTarget;

	bool				bFaceTarget;
	bool				bTargetFaceSpeaker;
};