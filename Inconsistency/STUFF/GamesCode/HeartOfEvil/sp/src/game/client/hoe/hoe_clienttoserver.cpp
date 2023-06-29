#include "cbase.h"
#include "c_baseentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

typedef void (*NPCBecameRagdollProc)( int entindex, void *ragdoll );
typedef void (*RagdollMovedProc)( void *ragdoll, const Vector &origin );

// THIS MUST MATCH hoe_corpse.cpp on the server
struct ClientToServerProcs
{
	NPCBecameRagdollProc NPCBecameRagdoll;
	RagdollMovedProc RagdollMoved;
};

class C_ClientToServer : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_ClientToServer, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_ClientToServer()
	{
		gm_ClientToServer = this;
	}
	~C_ClientToServer()
	{
		gm_ClientToServer = NULL;
	}
	void InitProcs( void )
	{
		if ( m_procs.NPCBecameRagdoll == NULL )
		{
			ClientToServerProcs *procs;
			if ( sscanf( m_szAddress, "%p", &procs ) == 1 )
			{
				m_procs = *procs;
			}
		}
	}

	char m_szAddress[128];
	ClientToServerProcs m_procs;
	static C_ClientToServer *gm_ClientToServer;
};

IMPLEMENT_CLIENTCLASS_DT( C_ClientToServer, DT_ClientToServer, CClientToServer )
	RecvPropString( RECVINFO( m_szAddress ) ),
END_RECV_TABLE()

//LINK_ENTITY_TO_CLASS( hoe_clientotserver, C_ClientToServer );

C_ClientToServer *C_ClientToServer::gm_ClientToServer = NULL;

void HOENPCBecameRagdoll( int entindex, void *ragdoll )
{
	if ( C_ClientToServer::gm_ClientToServer != NULL )
	{
		C_ClientToServer::gm_ClientToServer->InitProcs();
		if ( C_ClientToServer::gm_ClientToServer->m_procs.NPCBecameRagdoll != NULL )
		{
			C_ClientToServer::gm_ClientToServer->m_procs.NPCBecameRagdoll( entindex, ragdoll );
		}
	}
}

void HOERagdollMoved( void *ragdoll, const Vector &origin )
{
	if ( C_ClientToServer::gm_ClientToServer != NULL )
	{
		C_ClientToServer::gm_ClientToServer->InitProcs();
		if ( C_ClientToServer::gm_ClientToServer->m_procs.RagdollMoved != NULL )
		{
			C_ClientToServer::gm_ClientToServer->m_procs.RagdollMoved( ragdoll, origin );
		}
	}
}
