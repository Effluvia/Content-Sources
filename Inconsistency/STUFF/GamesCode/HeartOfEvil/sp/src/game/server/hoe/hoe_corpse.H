#define CORPSE_ROPES
#ifdef CORPSE_ROPES
#include "vphysics/constraints.h"
#endif

class CHOECorpse : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CHOECorpse, CAI_BaseNPC );
	DECLARE_DATADESC();

	struct CorpsePose
	{
		const char *sequence;
		bool hanging;
#ifdef CORPSE_ROPES
		const char *rope1;
		const char *rope2;
#endif
	};

	CHOECorpse();
	void Spawn( void );
	void Precache( void );
	void InitCorpse( const char *model, CorpsePose poses[], string_t iszLivingClass = NULL_STRING );
	virtual void SelectModelGroups( void ) {};
	void AnimThink( void );

	void InitRagdollCorpse( void );

	int m_pose;
	CorpsePose *m_poses;
	string_t m_iszLivingClass;

#ifdef CORPSE_ROPES
	void InitRope( string_t iszEntName, const char *szAttach );

	string_t m_nameRope1;
	string_t m_nameRope2;
#endif
};

class CHOECorpseMarker : public CPointEntity
{
public:
	DECLARE_CLASS( CHOECorpseMarker, CPointEntity );
	DECLARE_DATADESC();

	void InitMarker( CBaseEntity *pParent, string_t iszLivingClass, bool bSmell = true );
	void MarkerThink( void );

	// Cross the level with our parent
	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_ACROSS_TRANSITION; }

	string_t m_iszLivingClass; // npc_mikeforce, npc_human_grunt etc
};