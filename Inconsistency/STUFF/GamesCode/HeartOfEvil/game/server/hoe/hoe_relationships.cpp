#include "cbase.h"
#include "filesystem.h"
#include "utldict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *g_RelationshipClassNames[] =
{
	"CLASS_NONE",				
	"CLASS_PLAYER",
	"CLASS_PLAYER_ALLY",
	"CLASS_PLAYER_ALLY_VITAL",
	"CLASS_ANTLION",
	"CLASS_BARNACLE",
	"CLASS_BULLSEYE",
//	"CLASS_BULLSQUID",
	"CLASS_CITIZEN_PASSIVE",
	"CLASS_CITIZEN_REBEL",
	"CLASS_COMBINE",
	"CLASS_COMBINE_GUNSHIP",
	"CLASS_CONSCRIPT",
	"CLASS_HEADCRAB",
//	"CLASS_HOUNDEYE",
	"CLASS_MANHACK",
	"CLASS_METROPOLICE",
	"CLASS_MILITARY",
	"CLASS_SCANNER",
	"CLASS_STALKER",
	"CLASS_VORTIGAUNT",
	"CLASS_ZOMBIE",
	"CLASS_PROTOSNIPER",
	"CLASS_MISSILE",
	"CLASS_FLARE",
	"CLASS_EARTH_FAUNA",
	"CLASS_HACKED_ROLLERMINE",
	"CLASS_COMBINE_HUNTER",

	// HOE classes start here
	"CLASS_BARNEY",
	"CLASS_BARNEY_ZOMBIE",
	"CLASS_BULLSQUID",
	"CLASS_CHARLIE",
	"CLASS_CHUMTOAD",
	"CLASS_GORILLA",
	"CLASS_GRUNT_MEDIC",
	"CLASS_GRUNT_MEDIC_FRIEND",
	"CLASS_HOUNDEYE",
	"CLASS_HUEY",
	"CLASS_HUEY_FRIEND",
	"CLASS_HUMAN_GRUNT",
	"CLASS_HUMAN_GRUNT_FRIEND",
	"CLASS_KOPHYAEGER",
	"CLASS_KOPHYAEGER_ADULT",
	"CLASS_KURTZ",
	"CLASS_LEECH",
	"CLASS_MIKEFORCE",
	"CLASS_MIKEFORCE_MEDIC",
	"CLASS_PEASANT",
	"CLASS_ROACH",
	"CLASS_SNARK",
	"CLASS_SOG",
	"CLASS_SPX",
	"CLASS_SPX_FRIEND",
	"CLASS_SPX_BABY",
	"CLASS_SPX_FLYER",
	"CLASS_SUPERZOMBIE",
	"CLASS_SUPERZOMBIE_FRIEND",

	NULL
};

const char *g_RelationshipDispositionNames[] =
{
	"D_ER",
	"D_HT",
	"D_FR",
	"D_LI",
	"D_NU",
	NULL
};

class RelationshipGroup_t
{
public:
	RelationshipGroup_t() {};

	const char *name;
	CUtlVector<Class_T> members;
};

typedef unsigned short RELATIONSHIP_GROUP_HANDLE;
static CUtlDict< RelationshipGroup_t*, RELATIONSHIP_GROUP_HANDLE > g_RelationshipGroupDatabase;

static void InitBuiltinRelationshipGroups( void )
{
	for ( int i = 0; g_RelationshipClassNames[i] != NULL; i++ )
	{
		RelationshipGroup_t *group = new RelationshipGroup_t;
		group->name = g_RelationshipClassNames[i];
		group->members.AddToTail( (Class_T) i );

		RELATIONSHIP_GROUP_HANDLE h = g_RelationshipGroupDatabase.Insert( group->name, group );
		Assert( h != g_RelationshipGroupDatabase.InvalidIndex() );
	}
}

static bool LookupRelationshipClass( const char *s, Class_T &nClass )
{
	for ( int i = 0; g_RelationshipClassNames[i] != NULL; i++ )
	{
		if ( !Q_stricmp( g_RelationshipClassNames[i], s ) )
		{
			nClass = (Class_T) i;
			return true;
		}
	}
	DevWarning( "unknown class '%s' in scripts/relationships.txt\n", s );
	return false;
}

static bool LookupRelationshipDisposition( const char *s, Disposition_t &nDisposition )
{
	for ( int i = 0; g_RelationshipDispositionNames[i] != NULL; i++ )
	{
		if ( !Q_stricmp( g_RelationshipDispositionNames[i], s ) )
		{
			nDisposition = (Disposition_t) i;
			return true;
		}
	}
	DevWarning( "unknown disposition '%s' in scripts/relationships.txt\n", s );
	return false;
}

static void ParseRelationshipGroup( KeyValues *kv )
{
	const char *name = kv->GetString( "name" );
	if ( !name[0] )
	{
		DevWarning( "group with no name in scripts/relationships.txt\n" );
		return;
	}

	RELATIONSHIP_GROUP_HANDLE h = g_RelationshipGroupDatabase.Find( name );
	if ( h != g_RelationshipGroupDatabase.InvalidIndex() )
	{
		DevWarning( "duplicate group '%s' in scripts/relationships.txt\n", name );
		return;
	}

	RelationshipGroup_t *group = new RelationshipGroup_t;
	group->name = name;

	for ( KeyValues *pValue = kv->GetFirstValue(); pValue != NULL ; pValue = pValue->GetNextValue() )
	{
		if ( FStrEq( pValue->GetName(), "member" ) )
		{
			Class_T nClass;
			if ( LookupRelationshipClass( pValue->GetString(), nClass ) == false )
				return;
			group->members.AddToTail( nClass ); // FIXME: check for uniqueness
		}
		else if ( FStrEq( pValue->GetName(), "name" ) )
		{
		}
		else
		{
			DevWarning( "unknown key '%s' in group '%s' in scripts/relationships.txt\n",
				pValue->GetName(), group->name );
		}
	}

	h = g_RelationshipGroupDatabase.Insert( group->name, group );
	Assert( h != g_RelationshipGroupDatabase.InvalidIndex() );
}

void ReadRelationshipsScript( void )
{
	g_RelationshipGroupDatabase.PurgeAndDeleteElements();

	Assert( ARRAYSIZE( g_RelationshipClassNames ) == NUM_AI_CLASSES + 1 );

	InitBuiltinRelationshipGroups();

	KeyValues *manifest = new KeyValues( "relationships" );
	if ( manifest->LoadFromFile( filesystem, "scripts/relationships.txt", "GAME" ) )
	{
		for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{
			if ( FStrEq( sub->GetName(), "group" ) )
			{
				ParseRelationshipGroup( sub );
				continue;
			}

			RELATIONSHIP_GROUP_HANDLE h = g_RelationshipGroupDatabase.Find( sub->GetName() );
			if ( h == g_RelationshipGroupDatabase.InvalidIndex() )
			{
				DevWarning( "unknown class/group '%s' in scripts/relationships.txt\n", sub->GetName() );
				continue;
			}
			RelationshipGroup_t *pGroup = g_RelationshipGroupDatabase[h];

			for ( KeyValues *sub1 = sub->GetFirstSubKey(); sub1 != NULL ; sub1 = sub1->GetNextKey() )
			{
				h = g_RelationshipGroupDatabase.Find( sub1->GetName() );
				if ( h == g_RelationshipGroupDatabase.InvalidIndex() )
				{
					DevWarning( "unknown class/group '%s' in scripts/relationships.txt\n", sub1->GetName() );
					continue;
				}
				RelationshipGroup_t *pGroupTarget = g_RelationshipGroupDatabase[h];

				Disposition_t nDisposition = D_NU;
				int nPriority = 0;
				for ( KeyValues *pValue = sub1->GetFirstValue(); pValue != NULL ; pValue = pValue->GetNextValue() )
				{
					if ( !Q_stricmp( pValue->GetName(), "disposition" ) )
						LookupRelationshipDisposition( pValue->GetString(), nDisposition );
					else if ( !Q_stricmp( pValue->GetName(), "priority" ) )
						nPriority = pValue->GetInt();
					else
					{
						DevWarning( "unknown key '%s' in scripts/relationships.txt\n", pValue->GetName() );
					}
				}

				for ( int i = 0; i < pGroup->members.Count(); i++ )
				{
					Class_T nClass = pGroup->members[i];
					for ( int j = 0; j < pGroupTarget->members.Count(); j++ )
					{
						Class_T nClassTarget = pGroupTarget->members[j];
						CBaseCombatCharacter::SetDefaultRelationship( nClass, nClassTarget, nDisposition, nPriority );
					}
				}
			}
		}
	}
    else
        Assert( false );
	manifest->deleteThis();
}

const char *RelationshipClassText( int classType )
{
	if ( classType >= 0 && classType < ARRAYSIZE(g_RelationshipClassNames ) - 1 )
	{
		return g_RelationshipClassNames[classType];
	}
	Assert( 0 );
	return "MISSING CLASS in AIClassText()";
}