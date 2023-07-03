
#include "extdll.h"
#include "util_entity.h"
#include "enginecallback.h"




/*
MODDD - question:  WHAT IS THE DIFFERENCE?   as seen in eiface.h

INDEXENT:
edict_t * (*pfnPEntityOfEntIndex)	(int iEntIndex);

ENT:
edict_t* (*pfnPEntityOfEntOffset)	(int iEntOffset);

 Also, "pfnPEntityOfEntIndex( 1 )" seems to be used to grab the 1st player for logic that assumes single-player
 (only one player to apply too, or a convenience feature for stuff never intended to be supported in multiplayer).
 Why not the INDEXENT macro?  Why never "ENT( 1 )"?   This and more will never be answered!

 pfnEntityOfEntIndex seems to be used almost exclusively for getting the 1st player in fact ( 1 ).
 Only a few places with some other variable source of edict, the macro INDEXENT is more common for that.
 wait... no, INDEXENT is almost exclusively for getting the world entity:  INDEXENT(0).   Madness!


 For what it's worth, looks like almost everywhere leans on using 'offset'-named lookups
 instead of 'index'-named lookups.  That is, odds are in any method that doesn't mention either
 but has to use either (CBaseEntity::Instance's calls to work from a number, edict, or entvars)
 will probably use the 'offset' lookups instead.  No clue why or why not the other.


*/






//#ifndef CLIENT_DLL

edict_t *ENT(edict_t *pent)		{ return pent; }   

edict_t *ENT(EOFFSET eoffset)			{ return (*g_engfuncs.pfnPEntityOfEntOffset)(eoffset); }
EOFFSET OFFSET(EOFFSET eoffset)			{ return eoffset; }


EOFFSET OFFSET(const edict_t *pent)	
{ 
#if _DEBUG
	if ( !pent )
		ALERT( at_error, "Bad ent in OFFSET()\n" );
#endif
	return (*g_engfuncs.pfnEntOffsetOfPEntity)(pent); 
}

EOFFSET OFFSET(entvars_t *pev)				
{ 
#if _DEBUG
	if ( !pev )
		ALERT( at_error, "Bad pev in OFFSET()\n" );
#endif
	return OFFSET(ENT(pev)); 
}




#ifdef DEBUG
	///////////////////////////////////////////////////////////////////////////////////////////
	#ifndef CLIENT_DLL
		// SERVER: normal.
		edict_t *DBG_EntOfVars( const entvars_t *pev )
		{
			if (pev->pContainingEntity != NULL)
				return pev->pContainingEntity;
			ALERT(at_console, "entvars_t pContainingEntity is NULL, calling into engine");
			edict_t* pent = (*g_engfuncs.pfnFindEntityByVars)((entvars_t*)pev);
			if (pent == NULL)
				ALERT(at_console, "DAMN!  Even the engine couldn't FindEntityByVars!");
			((entvars_t *)pev)->pContainingEntity = pent;
			return pent;
		}
	///////////////////////////////////////////////////////////////////////////////////////////
	#else
		// CLIENT: was dummied out in hl_baseentity.cpp, clientside. Keep it that way here?
		// ...although, keep in mind, for Release mode, "pev->ContainingEntity" is still returned
		// for the client.  Let's just force it that way here too then, no sense in Debug having
		// different behavior like that (returning NULL).
		// Even though pContainingEntity may be meaningless for clientside,
		// no idea.  This may not even matter at all.

		//edict_t *DBG_EntOfVars( const entvars_t *pev ){
		//	return NULL;
		//}

		edict_t* DBG_EntOfVars(const entvars_t* pev){
			return pev->pContainingEntity;
		}
		
	#endif
	///////////////////////////////////////////////////////////////////////////////////////////

#else
	// not in debug? no need, no DBG_EntOfVars mmethod at all and ENT is inlined (handled already)
	// and just skips calling DBG_EntOfVars.
#endif





entvars_t* VARS(EOFFSET eoffset)		{ return VARS(ENT(eoffset)); }
int ENTINDEX(edict_t *pEdict)			{ return (*g_engfuncs.pfnIndexOfEdict)(pEdict); }
edict_t* INDEXENT( int iEdictNum )		{ return (*g_engfuncs.pfnPEntityOfEntIndex)(iEdictNum); }




//#endif