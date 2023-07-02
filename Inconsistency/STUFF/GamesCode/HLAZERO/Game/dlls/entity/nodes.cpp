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
// nodes.cpp - AI node tree stuff.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "nodes.h"
#include "util_model.h"
#include "doors.h"
#include "weapons.h"
#include "util_debugdraw.h"

EASY_CVAR_EXTERN_DEBUGONLY(pathfindPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(nodeSearchStartVerticalOffset)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindIgnoreIsolatedNodes)
EASY_CVAR_EXTERN_DEBUGONLY(hideNodeGraphRebuildNotice)
EASY_CVAR_EXTERN_DEBUGONLY(nodeConnectionBreakableCheck)
EASY_CVAR_EXTERN_DEBUGONLY(nodeDetailPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindIgnoreNearestNodeCache)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindIgnoreStaticRoutes)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindNearestNodeExtra)
EASY_CVAR_EXTERN_DEBUGONLY(pathfindMonsterclipFreshLogic)

//MODDD - retail behavior HULL_STEP_SIZE is 16!
#define HULL_STEP_SIZE 16// how far the test hull moves on each step
// to help eliminate node clutter by level designers, this is used to cap how many other nodes
// any given node is allowed to 'see' in the first stage of graph creation "LinkVisibleNodes()".
#define MAX_NODE_INITIAL_LINKS	128
#define MAX_NODES               1024



//MODDD - NEW.  The player can schedule a node update for the next time a map is loaded.
BOOL scheduleNodeUpdate;

//MODDD - split into the two variables below, now settable by KeyValues for the entire map (CWorld, see world.cpp's KeyValue method)
// Any calls to displaying nodes or routes use "node_linktest_height" in place of NODE_HEIGHT now.
//#define NODE_HEIGHT	8	// how high to lift nodes off the ground after we drop them all (make stair/ramp mapping easier)

//MODDD - how much height to add to nodes when doing the line traces between them to see what nodes can see each other.
//This lets nodes reach greater heights just for visibility-checks between nodes (raw connnections) without interfering
//with the WALK_MOVE hullchecks, which can be upset by too great of a height to start from apparently.
//But those checks never get a ch8nce to run if a ramp/incline is in the way that is otherwise completely passable.
//For exact retail behavior, use 0.
float node_linktest_height;

//MODDD - this too. Vertical offset applied to nodes before the test hull goes to each of them.
//Independent of the TRACE_EXTRA offset above.
//For exact retail behavior, use 8.
float node_hulltest_height;

//MODDD - new. If this constant is defined, a hull test between two nodes where the start is higher than the other
//        will be swapped to begin at the lower node instead. This seems to work better with checks that work one way
//        but not the other, to report both ways work.
BOOL node_hulltest_heightswap;

//MODDD - new. Has this map ever had any "Air" type nodes placed?  Some things may want to query this.
//        Generally fliers should not be placed in maps without any air nodes, but some things, like hte kingpin's super homing
//        electric ball, can still behave like an ordinary controller head ball if not.
BOOL map_anyAirNodes;



// Test!
CBaseMonster* g_routeTempMonster = NULL;
CBaseEntity* g_routeTempMonster_GoalEnt = NULL;










CGraph WorldGraph;


extern DLL_GLOBAL edict_t* g_pBodyQueueHead;
extern DLL_GLOBAL short g_sModelIndexLaser;
extern BOOL gTouchDisabled;


LINK_ENTITY_TO_CLASS(info_node, CNodeEnt);
LINK_ENTITY_TO_CLASS(info_node_air, CNodeEnt);

//MODDD - include for unix file 'unistd.h' moved to 'external_lib_include.h' for going
// everywhere if needed.




// a possible value for m_pHashLinks, of CGraph ('WorldGraph' instance most likely)
#define ENTRY_STATE_EMPTY -1

struct tagNodePair
{
	short iSrc;
	short iDest;
};


/*
//MODDD - constructor for CLink
CLink::CLink(void){
	breakableBetweenOut = FALSE;
}
*/

//=========================================================
// CGraph - InitGraph - prepares the graph for use. Frees any
// memory currently in use by the world graph, NULLs 
// all pointers, and zeros the node count.
//=========================================================
void CGraph::InitGraph(void)
{
	//MODDD - new. This keeps track of whether any air nodes have been spawned this time.
	map_anyAirNodes = FALSE;


	// Make the graph unavailable
	//
	m_fGraphPresent = FALSE;
	m_fGraphPointersSet = FALSE;
	m_fRoutingComplete = FALSE;

	// Free the link pool
	//
	if (m_pLinkPool)
	{
		free(m_pLinkPool);
		m_pLinkPool = NULL;
	}

	// Free the node info
	//
	if (m_pNodes)
	{
		free(m_pNodes);
		m_pNodes = NULL;
	}

	if (m_di)
	{
		free(m_di);
		m_di = NULL;
	}

	// Free the routing info.
	//
	if (m_pRouteInfo)
	{
		free(m_pRouteInfo);
		m_pRouteInfo = NULL;
	}

	if (m_pHashLinks)
	{
		free(m_pHashLinks);
		m_pHashLinks = NULL;
	}

	// Zero node and link counts
	//
	m_cNodes = 0;
	m_cLinks = 0;
	m_nRouteInfo = 0;

	m_iLastActiveIdleSearch = 0;
	m_iLastCoverSearch = 0;

	//MODDD - if this was forced by a user call, forget it now.
	scheduleNodeUpdate = FALSE;

}

//=========================================================
// CGraph - AllocNodes - temporary function that mallocs a
// reasonable number of nodes so we can build the path which
// will be saved to disk.
//=========================================================
int CGraph::AllocNodes(void)
{
	//  malloc all of the nodes
	WorldGraph.m_pNodes = (CNode*)calloc(sizeof(CNode), MAX_NODES);

	// could not malloc space for all the nodes!
	if (!WorldGraph.m_pNodes)
	{
		ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d nodes!\n", WorldGraph.m_cNodes);
		return FALSE;
	}

	return TRUE;
}

//=========================================================
// CGraph - LinkEntForLink - sometimes the ent that blocks
// a path is a usable door, in which case the monster just
// needs to face the door and fire it. In other cases, the
// monster needs to operate a button or lever to get the 
// door to open. This function will return a pointer to the
// button if the monster needs to hit a button to open the 
// door, or returns a pointer to the door if the monster 
// need only use the door.
//
// pNode is the node the monster will be standing on when it
// will need to stop and trigger the ent.
//=========================================================
entvars_t* CGraph::LinkEntForLink(CLink* pLink, CNode* pNode)
{
	edict_t* pentSearch;
	edict_t* pentTrigger;
	entvars_t* pevTrigger;
	entvars_t* pevLinkEnt;
	TraceResult	tr;

	pevLinkEnt = pLink->m_pLinkEnt;
	if (!pevLinkEnt)
		return NULL;

	pentSearch = NULL;// start search at the top of the ent list.

	if (FClassnameIs(pevLinkEnt, "func_door") || FClassnameIs(pevLinkEnt, "func_door_rotating"))
	{

		///!!!UNDONE - check for TOGGLE or STAY open doors here. If a door is in the way, and is 
		// TOGGLE or STAY OPEN, even monsters that can't open doors can go that way.

		if ((pevLinkEnt->spawnflags & SF_DOOR_USE_ONLY))
		{// door is use only, so the door is all the monster has to worry about
			return pevLinkEnt;
		}

		while (1)
		{
			pentTrigger = FIND_ENTITY_BY_TARGET(pentSearch, STRING(pevLinkEnt->targetname));// find the button or trigger

			if (FNullEnt(pentTrigger))
			{// no trigger found

				// right now this is a problem among auto-open doors, or any door that opens through the use 
				// of a trigger brush. Trigger brushes have no models, and don't show up in searches. Just allow
				// monsters to open these sorts of doors for now. 
				return pevLinkEnt;
			}

			pentSearch = pentTrigger;
			pevTrigger = VARS(pentTrigger);

			if (FClassnameIs(pevTrigger, "func_button") || FClassnameIs(pevTrigger, "func_rot_button"))
			{// only buttons are handled right now. 

				// trace from the node to the trigger, make sure it's one we can see from the node.
				// !!!HACKHACK Use bodyqueue here cause there are no ents we really wish to ignore!
				UTIL_TraceLine(pNode->m_vecOrigin, VecBModelOrigin(pevTrigger), ignore_monsters, g_pBodyQueueHead, &tr);


				if (VARS(tr.pHit) == pevTrigger)
				{// good to go!
					return VARS(tr.pHit);
				}
			}
		}
	}
	else
	{
		ALERT(at_aiconsole, "Unsupported PathEnt:\n'%s'\n", STRING(pevLinkEnt->classname));
		return NULL;
	}
}

//=========================================================
// CGraph - HandleLinkEnt - a brush ent is between two
// nodes that would otherwise be able to see each other. 
// Given the monster's capability, determine whether
// or not the monster can go this way. 
//=========================================================
int CGraph::HandleLinkEnt(int iNode, entvars_t* pevLinkEnt, int afCapMask, NODEQUERY queryType)
{
	edict_t* pentWorld;
	CBaseEntity* pDoor;
	TraceResult	tr;


	if (!m_fGraphPresent || !m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available
		ALERT(at_aiconsole, "Graph not ready!\n");
		return FALSE;
	}

	if (FNullEnt(pevLinkEnt))
	{
		ALERT(at_aiconsole, "dead path ent!\n");
		return TRUE;
	}
	pentWorld = NULL;


	const char* daClassname;
	
	CBaseEntity* tempEnt = CBaseEntity::Instance(pevLinkEnt);
	if(tempEnt != NULL){
		daClassname = tempEnt->getClassname();
	}else{
		daClassname = "???";
	}

	easyPrintLine("LINK ENT::: node: %d  linkedEnt: %s queryType: %d ", iNode, daClassname, queryType);


	// func_door
	if (FClassnameIs(pevLinkEnt, "func_door") || FClassnameIs(pevLinkEnt, "func_door_rotating"))
	{// ent is a door.

		pDoor = (CBaseEntity::Instance(pevLinkEnt));

		if ((pevLinkEnt->spawnflags & SF_DOOR_USE_ONLY))
		{// door is use only.

			if ((afCapMask & bits_CAP_OPEN_DOORS))
			{// let monster right through if he can open doors

				//MODDD - why was this check missing?  The door can still deny monsters.
				if (!(pevLinkEnt->spawnflags & SF_DOOR_NOMONSTERS) || queryType == NODEGRAPH_STATIC) {
					//!!!
					return TRUE;
				}
				else {
					return FALSE;
				}
			}
			else
			{
				// monster should try for it if the door is open and looks as if it will stay that way
				if (pDoor->GetToggleState() == TS_AT_TOP && (pevLinkEnt->spawnflags & SF_DOOR_NO_AUTO_RETURN))
				{
					return TRUE;
				}

				return FALSE;
			}
		}
		else
		{// door must be opened with a button or trigger field.

			// monster should try for it if the door is open and looks as if it will stay that way
			if (pDoor->GetToggleState() == TS_AT_TOP && (pevLinkEnt->spawnflags & SF_DOOR_NO_AUTO_RETURN))
			{
				return TRUE;
			}
			if ((afCapMask & bits_CAP_OPEN_DOORS))
			{
				if (!(pevLinkEnt->spawnflags & SF_DOOR_NOMONSTERS) || queryType == NODEGRAPH_STATIC)
					return TRUE;
			}

			return FALSE;
		}
	}
	// func_breakable	
	else if (FClassnameIs(pevLinkEnt, "func_breakable") && queryType == NODEGRAPH_STATIC)
	{
		//MODDD - BUT WHY THOUGH?!
		//return TRUE;
		//return FALSE;

		return (EASY_CVAR_GET_DEBUGONLY(nodeConnectionBreakableCheck) == 1 || EASY_CVAR_GET_DEBUGONLY(nodeConnectionBreakableCheck) == 3);
	}
	else
	{
		ALERT(at_aiconsole, "Unhandled Ent in Path %s\n", STRING(pevLinkEnt->classname));
		return FALSE;
	}

	return FALSE;
}

#if 0
//=========================================================
// FindNearestLink - finds the connection (line) nearest
// the given point. Returns FALSE if fails, or TRUE if it
// has stuffed the index into the nearest link pool connection
// into the passed int pointer, and a BOOL telling whether or 
// not the point is along the line into the passed BOOL pointer.
//=========================================================
int CGraph::FindNearestLink(const Vector& vecTestPoint, int* piNearestLink, BOOL* pfAlongLine)
{
	int		i, j;// loops

	int		iNearestLink;// index into the link pool, this is the nearest node at any time. 
	float	flMinDist;// the distance of of the nearest case so far
	float	flDistToLine;// the distance of the current test case

	BOOL		fCurrentAlongLine;
	BOOL		fSuccess;

	//float	flConstant;// line constant
	Vector		vecSpot1, vecSpot2;
	Vector2D	vec2Spot1, vec2Spot2, vec2TestPoint;
	Vector2D	vec2Normal;// line normal
	Vector2D	vec2Line;

	TraceResult	tr;

	iNearestLink = -1;// prepare for failure
	fSuccess = FALSE;

	flMinDist = 9999;// anything will be closer than this

// go through all of the nodes, and each node's connections	
	int cSkip = 0;// how many links proper pairing allowed us to skip
	int cChecked = 0;// how many links were checked

	for (i = 0; i < m_cNodes; i++)
	{
		vecSpot1 = m_pNodes[i].m_vecOrigin;

		if (m_pNodes[i].m_cNumLinks <= 0)
		{// this shouldn't happen!
			ALERT(at_aiconsole, "**Node %d has no links\n", i);
			continue;
		}

		for (j = 0; j < m_pNodes[i].m_cNumLinks; j++)
		{
			/*
			!!!This optimization only works when the node graph consists of properly linked pairs.
			if ( INodeLink ( i, j ) <= i )
			{
				// since we're going through the nodes in order, don't check
				// any connections whose second node is lower in the list
				// than the node we're currently working with. This eliminates
				// redundant checks.
				cSkip++;
				continue;
			}
			*/

			vecSpot2 = PNodeLink(i, j)->m_vecOrigin;

			// these values need a little attention now and then, or sometimes ramps cause trouble.
			if (fabs(vecSpot1.z - vecTestPoint.z) > 48 && fabs(vecSpot2.z - vecTestPoint.z) > 48)
			{
				// if both endpoints of the line are 32 units or more above or below the monster, 
				// the monster won't be able to get to them, so we do a bit of trivial rejection here.
				// this may change if monsters are allowed to jump down. 
				// 
				// !!!LATER: some kind of clever X/Y hashing should be used here, too
				continue;
			}

			// now we have two endpoints for a line segment that we've not already checked. 
			// since all lines that make it this far are within -/+ 32 units of the test point's
			// Z Plane, we can get away with doing the point->line check in 2d.

			cChecked++;

			vec2Spot1 = vecSpot1.Make2D();
			vec2Spot2 = vecSpot2.Make2D();
			vec2TestPoint = vecTestPoint.Make2D();

			// get the line normal.
			vec2Line = (vec2Spot1 - vec2Spot2).Normalize();
			vec2Normal.x = -vec2Line.y;
			vec2Normal.y = vec2Line.x;

			if (DotProduct(vec2Line, (vec2TestPoint - vec2Spot1)) > 0)
			{// point outside of line
				flDistToLine = (vec2TestPoint - vec2Spot1).Length();
				fCurrentAlongLine = FALSE;
			}
			else if (DotProduct(vec2Line, (vec2TestPoint - vec2Spot2)) < 0)
			{// point outside of line
				flDistToLine = (vec2TestPoint - vec2Spot2).Length();
				fCurrentAlongLine = FALSE;
			}
			else
			{// point inside line
				flDistToLine = fabs(DotProduct(vec2TestPoint - vec2Spot2, vec2Normal));
				fCurrentAlongLine = TRUE;
			}

			if (flDistToLine < flMinDist)
			{// just found a line nearer than any other so far

				UTIL_TraceLine(vecTestPoint, SourceNode(i, j).m_vecOrigin, ignore_monsters, g_pBodyQueueHead, &tr);

				if (tr.flFraction != 1.0)
				{// crap. can't see the first node of this link, try to see the other

					UTIL_TraceLine(vecTestPoint, DestNode(i, j).m_vecOrigin, ignore_monsters, g_pBodyQueueHead, &tr);
					if (tr.flFraction != 1.0)
					{// can't use this link, cause can't see either node!
						continue;
					}

				}

				fSuccess = TRUE;// we know there will be something to return.
				flMinDist = flDistToLine;
				iNearestLink = m_pNodes[i].m_iFirstLink + j;
				*piNearestLink = m_pNodes[i].m_iFirstLink + j;
				*pfAlongLine = fCurrentAlongLine;
			}
		}
	}

	/*
		if ( fSuccess )
		{
			WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(MSG_BROADCAST, TE_SHOWLINE);

			WRITE_COORD(MSG_BROADCAST, m_pNodes[ m_pLinkPool[ iNearestLink ].m_iSrcNode ].m_vecOrigin.x );
			WRITE_COORD(MSG_BROADCAST, m_pNodes[ m_pLinkPool[ iNearestLink ].m_iSrcNode ].m_vecOrigin.y );
			WRITE_COORD(MSG_BROADCAST, m_pNodes[ m_pLinkPool[ iNearestLink ].m_iSrcNode ].m_vecOrigin.z + node_linktest_height);

			WRITE_COORD(MSG_BROADCAST, m_pNodes[ m_pLinkPool[ iNearestLink ].m_iDestNode ].m_vecOrigin.x );
			WRITE_COORD(MSG_BROADCAST, m_pNodes[ m_pLinkPool[ iNearestLink ].m_iDestNode ].m_vecOrigin.y );
			WRITE_COORD(MSG_BROADCAST, m_pNodes[ m_pLinkPool[ iNearestLink ].m_iDestNode ].m_vecOrigin.z + node_linktest_height);
		}
	*/

	ALERT(at_aiconsole, "%d Checked\n", cChecked);
	return fSuccess;
}

#endif



int CGraph::HullIndex(CBaseEntity* pEntity){
	return HullIndex(pEntity, 0);
}

//MODDD - NEW.  Pass along the iNodeTypeInfo (type(s) of node that can be taken: LAND, AIR, WATER)
// which can inflence the 'getHull' method used for this entity.
int CGraph::HullIndex(CBaseEntity* pEntity, int iNodeTypeInfo)
{
	//MODDD - other behavior.
	//If we specifically return a type of hull for this entity, rely on that instead.
	int nodeTypeAttempt;
	if( iNodeTypeInfo == bits_NODE_LAND ){
		// Exclusive bits_NODE_LAND?  only the groundnodes one
		nodeTypeAttempt = pEntity->getHullIndexForGroundNodes();
	}else{
		// normal
		nodeTypeAttempt = pEntity->getHullIndexForNodes();
	}


	if (nodeTypeAttempt == NODE_DEFAULT_HULL) {
		//continue with retail's way of determining node type below I guess.
	}else {
		//If it isn't DEFAULT we made it a different way for a reason. Use this.
		return nodeTypeAttempt;
	}

	//MODDD - accept the possiblity of other flying movetypes as well!
	//if ( pEntity->pev->movetype == MOVETYPE_FLY)
	if (pEntity->isMovetypeFlying())
		return NODE_FLY_HULL;

	if (pEntity->pev->mins == Vector(-12, -12, 0))
		return NODE_SMALL_HULL;
	else if (pEntity->pev->mins == VEC_HUMAN_HULL_MIN)
		return NODE_HUMAN_HULL;
	else if (pEntity->pev->mins == Vector(-32, -32, 0))
		return NODE_LARGE_HULL;

	//	ALERT ( at_aiconsole, "Unknown Hull Mins!\n" );
	return NODE_HUMAN_HULL;
}


int CGraph::NodeType(CBaseEntity* pEntity)
{
	int nodeTypeTest = pEntity->getNodeTypeAllowed();

	if(nodeTypeTest != -1){
		return nodeTypeTest;
	}

	//if ( pEntity->pev->movetype == MOVETYPE_FLY)
	if (pEntity->isMovetypeFlying())
	{

		//MODDD TODO - checking for the presence of the FL_SWIM flag may be more helpful here than checking
		//whether the thing happens to be in the water or not?

		if (pEntity->pev->waterlevel != 0)
		{
			return bits_NODE_WATER;
		}
		else
		{
			return bits_NODE_AIR;
		}
	}
	return bits_NODE_LAND;
}


// Sum up graph weights on the path from iStart to iDest to determine path length
float CGraph::PathLength(int iStart, int iDest, int iHull, int afCapMask)
{
	float distance = 0;
	int	iNext;

	int	iMaxLoop = m_cNodes;

	int iCurrentNode = iStart;
	int iCap = CapIndex(afCapMask);

	while (iCurrentNode != iDest)
	{
		if (iMaxLoop-- <= 0)
		{
			ALERT(at_console, "Route Failure\n");
			return 0;
		}

		iNext = NextNodeInRoute(iCurrentNode, iDest, iHull, iCap);
		if (iCurrentNode == iNext)
		{
			//ALERT(at_aiconsole, "SVD: Can't get there from here..\n");
			return 0;
		}

		int iLink;
		HashSearch(iCurrentNode, iNext, iLink);
		if (iLink < 0)
		{
			ALERT(at_console, "HashLinks is broken from %d to %d.\n", iCurrentNode, iDest);
			return 0;
		}
		CLink& link = Link(iLink);
		distance += link.m_flWeight;

		iCurrentNode = iNext;
	}

	return distance;
}




BOOL CGraph::pathBetweenClear(int curNode, int destNode) {
	TraceResult tr;

	Vector vecStart = m_pNodes[curNode].m_vecOrigin + Vector(0, 0, 8);
	Vector vecEnd = m_pNodes[destNode].m_vecOrigin + Vector(0, 0, 8);

	//UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, NULL, &tr);

	//trace...
	UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, NULL, &tr);
	//m_pNodes[ curNode ].m_vecOrigin


	if (tr.flFraction != 1.0) {
		if (tr.pHit != NULL) {
			int theMonsterID = -1;
			CBaseEntity* tempEnt = CBaseEntity::Instance(tr.pHit);
			
			if(tempEnt != NULL){
				CBaseMonster* tempMon = tempEnt->GetMonsterPointer();
				if (tempMon != NULL) {
					theMonsterID = tempMon->monsterID;
				}
				easyForcePrintLine("!!!! BLOCKED BY pathBetweenClear! Notes: %d - %d. blocker:%s:%d", curNode, destNode, tempEnt->getClassname(), theMonsterID);
			}
		}
		else {
			easyForcePrintLine("!!!! BLOCKED BY pathBetweenClear! Notes: %d - %d. blocker:???", curNode, destNode);
		}
	}

	return (tr.flFraction == 1.0);
}



// Parse the routing table at iCurrentNode for the next node on the shortest path to iDest
int CGraph::NextNodeInRoute(int iCurrentNode, int iDest, int iHull, int iCap)
{
	int iNext = iCurrentNode;
	int nCount = iDest + 1;
	char* pRoute = m_pRouteInfo + m_pNodes[iCurrentNode].m_pNextBestNode[iHull][iCap];

	// Until we decode the next best node
	//
	while (nCount > 0)
	{
		char ch = *pRoute++;
		//ALERT(at_aiconsole, "C(%d)", ch);
		if (ch < 0)
		{
			// Sequence phrase
			//
			ch = -ch;
			if (nCount <= ch)
			{
				//MODDD - checks first.
				if (EASY_CVAR_GET_DEBUGONLY(nodeConnectionBreakableCheck) <= 1 || pathBetweenClear(iCurrentNode, iDest)) {
					iNext = iDest;
				}//...

				nCount = 0;
				//ALERT(at_aiconsole, "SEQ: iNext/iDest=%d\n", iNext);
			}
			else
			{
				//ALERT(at_aiconsole, "SEQ: nCount + ch (%d + %d)\n", nCount, ch);
				nCount = nCount - ch;
			}
		}
		else
		{
			//ALERT(at_aiconsole, "C(%d)", *pRoute);

			// Repeat phrase
			//
			if (nCount <= ch + 1)
			{

				//MODDD - remember me...
				int iNextPrevious = iNext;

				iNext = iCurrentNode + *pRoute;
				if (iNext >= m_cNodes) iNext -= m_cNodes;
				else if (iNext < 0) iNext += m_cNodes;
				nCount = 0;


				//MODDD - intervention again..
				if (EASY_CVAR_GET_DEBUGONLY(nodeConnectionBreakableCheck) <= 1 || pathBetweenClear(iCurrentNode, iNext)) {
					//is this valid?  if so, proceed.  

				}//END OF new if...
				else {
					//not ok?  hm..
					iNext = iNextPrevious;
				}

				//ALERT(at_aiconsole, "REP: iNext=%d\n", iNext);
			}
			else
			{
				//ALERT(at_aiconsole, "REP: nCount - ch+1 (%d - %d+1)\n", nCount, ch);
				nCount = nCount - ch - 1;
			}
			pRoute++;
		}
	}

	return iNext;
}


//=========================================================
// CGraph - FindShortestPath 
//
// accepts a capability mask (afCapMask), and will only 
// find a path usable by a monster with those capabilities
// returns the number of nodes copied into supplied array
//=========================================================
int CGraph::FindShortestPath(int* piPath, int iStart, int iDest, int iHull, int afCapMask)
{
	int	iVisitNode;
	int	iCurrentNode;
	int	iNumPathNodes;
	int	iHullMask;

	if (!m_fGraphPresent || !m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available or built
		ALERT(at_aiconsole, "Graph not ready!\n");
		return FALSE;
	}

	if (iStart < 0 || iStart > m_cNodes)
	{// The start node is bad?
		ALERT(at_aiconsole, "Can't build a path, iStart is %d!\n", iStart);
		return FALSE;
	}



	//MODDD - NEW SECTION
	// Picked the same node to be used as the start and end node?  That's... interesting, a sign that the target is in
	// an odd place unmarked by any node so it picked up the same one the montser's standing in as the 'best' too?
	// Send it back anyway and check for matching start/dest nodes.
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (iStart == iDest)
	{
		if (EASY_CVAR_GET_DEBUGONLY(pathfindPrintout) == 1) {
			easyForcePrintLine("WARNING!!! FindShortestPath reports the start and end nodes are exactly the same. Returning start position, satisfied!");
		}

		piPath[0] = iStart;
		piPath[1] = iDest;

		//MODDD TODO - send a special flag to the monster to move somewhere nearby. Ranged AI should realize it is blocked at this location
		//             and want to find another nearby that has a shot of the enemy, period.

		return 2;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Is routing information present.
	//MODDD - MAJOR.  If pathfindIgnoreStaticRouges is on, forbid using these.  Debug feature.
	// ALSO.  Require a valid iHull choice.  It is used to access the static route table (array)
	// so it can't be out of range.
	// ALSO.  Forbid using static routes if this monster has MonsterClip on, none of the node-testing done
	// to fill the static route table involved FL_MONSTERCLIP'd entities, so they aren't accurate for them.
	// Then again, this is kindof the case for all entities if anything about the placement of MONSTERCLIP bounded
	// things can change over time, but that's rare and not nearly as often of a problem for non-FL_MONSTERCLIP'd
	// entities.  So, idea:  ignore static routes for any MONSTERCLIP'd entity regardless of this CVar.
	if (
		EASY_CVAR_GET(pathfindIgnoreStaticRoutes) != 1 &&
		m_fRoutingComplete &&
		(EASY_CVAR_GET_DEBUGONLY(pathfindMonsterclipFreshLogic) == 0 || g_routeTempMonster==NULL || !(g_routeTempMonster->pev->flags & FL_MONSTERCLIP)) &&
		iHull >= 0 && iHull < MAX_NODE_HULLS
	)
	{
		int iCap = CapIndex(afCapMask);

		iNumPathNodes = 0;
		piPath[iNumPathNodes++] = iStart;
		iCurrentNode = iStart;
		int iNext;

		//ALERT(at_aiconsole, "GOAL: %d to %d\n", iStart, iDest);

		// Until we arrive at the destination
		//
		while (iCurrentNode != iDest)
		{
			iNext = NextNodeInRoute(iCurrentNode, iDest, iHull, iCap);
			if (iCurrentNode == iNext)
			{
				//ALERT(at_aiconsole, "SVD: Can't get there from here..\n");
				return 0;
				break;
			}
			if (iNumPathNodes >= MAX_PATH_SIZE)
			{
				//ALERT(at_aiconsole, "SVD: Don't return the entire path.\n");
				break;
			}
			piPath[iNumPathNodes++] = iNext;
			iCurrentNode = iNext;
		}
		//ALERT( at_aiconsole, "SVD: Path with %d nodes.\n", iNumPathNodes);
	}
	else
	{
		// Can't rely on built-in logic for FL_MONSTERCLIP, didn't factor that in.
		BOOL theHardWay;
		
		if(EASY_CVAR_GET_DEBUGONLY(pathfindMonsterclipFreshLogic) != 0){
			theHardWay = (g_routeTempMonster != NULL && g_routeTempMonster->pev->flags & FL_MONSTERCLIP);
		}else{
			// no check.
			theHardWay = FALSE;
		}


		CQueuePriority	queue;

		switch (iHull)
		{
			//MODDD - new. POINT sets iHullMask to 0 to mean, no requirements.  "(...someMask & iHullMask) != iHullMask" will never pass to skip
			//        a node since "any bitmask & 0" has to result in 0 and match 0.
		case NODE_POINT_HULL:
			iHullMask = 0;
			break;
		case NODE_SMALL_HULL:
			iHullMask = bits_LINK_SMALL_HULL;
			break;
		case NODE_HUMAN_HULL:
			iHullMask = bits_LINK_HUMAN_HULL;
			break;
		case NODE_LARGE_HULL:
			iHullMask = bits_LINK_LARGE_HULL;
			break;
		case NODE_FLY_HULL:
			iHullMask = bits_LINK_FLY_HULL;
			break;
		}

		// Mark all the nodes as unvisited.
		//
		int i;
		for (i = 0; i < m_cNodes; i++)
		{
			m_pNodes[i].m_flClosestSoFar = -1.0;
		}

		m_pNodes[iStart].m_flClosestSoFar = 0.0;
		m_pNodes[iStart].m_iPreviousNode = iStart;// tag this as the origin node
		queue.Insert(iStart, 0.0);// insert start node 

		while (!queue.Empty())
		{
			// now pull a node out of the queue
			float flCurrentDistance;
			iCurrentNode = queue.Remove(flCurrentDistance);

			// For straight-line weights, the following Shortcut works. For arbitrary weights,
			// it doesn't.
			//
			if (iCurrentNode == iDest) break;

			CNode* pCurrentNode = &m_pNodes[iCurrentNode];

			for (i = 0; i < pCurrentNode->m_cNumLinks; i++)
			{// run through all of this node's neighbors

				iVisitNode = INodeLink(iCurrentNode, i);


				if(m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_iDestNode == 11){
					int x = 45;
				}

				if(!theHardWay){
					if ((m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_afLinkInfo & iHullMask) != iHullMask)
					{// monster is too large to walk this connection
						//ALERT ( at_aiconsole, "fat ass %d/%d\n",m_pLinkPool[ m_pNodes[ iCurrentNode ].m_iFirstLink + i ].m_afLinkInfo, iMonsterHull );
						continue;
					}
				}else{

					CNode* srcNode = &m_pNodes[m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_iSrcNode];
					CNode* destNode = &m_pNodes[m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_iDestNode];

					if(EASY_CVAR_GET_DEBUGONLY(pathfindMonsterclipFreshLogic) == 1){
						TraceResult tr;
						UTIL_TraceLine(srcNode->m_vecOriginPeek, destNode->m_vecOriginPeek, dont_ignore_monsters, ENT(g_routeTempMonster->pev), &tr);
						if(tr.flFraction >= 1.0 || (tr.pHit != NULL && g_routeTempMonster_GoalEnt != NULL && tr.pHit == g_routeTempMonster_GoalEnt->edict())  ){
							// uninterrupted line, or the thing hit was the goal ent?  Allow
						}else{
							// nope
							continue;
						}
					}else if(EASY_CVAR_GET_DEBUGONLY(pathfindMonsterclipFreshLogic) == 2){
						// running into your enemy is always a good thing.  Right?
						BOOL succ = g_routeTempMonster->CheckLocalMove(srcNode->m_vecOrigin, destNode->m_vecOrigin, g_routeTempMonster_GoalEnt, TRUE, NULL) == LOCALMOVE_VALID;
						if(!succ){
							continue;
						}
					}
				}//theHardWay



				/*

				CNode* currentNode = &m_pNodes[iCurrentNode];
				CNode* destNode = &m_pNodes[m_pLinkPool[m_pNodes[ iCurrentNode ].m_iFirstLink + i].m_iDestNode];

				TraceResult trrr;


				TraceResult tr;

				Vector vecStart = currentNode->m_vecOrigin + Vector(0, 0, 8);
				Vector vecEnd = destNode->m_vecOrigin + Vector(0, 0, 8);

				//UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, NULL, &tr);

				//trace...
				UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, dont_ignore_glass, NULL, &tr);
				//m_pNodes[ curNode ].m_vecOrigin


				//if(tr.flFraction != 1.0){
				//	easyForcePrintLine("!!!! BLOCKED BY pathBetweenClear! Notes: %d - %d. Monster:%s:%d", curNode, destNode);
				//}


				if(tr.flFraction != 1.0){
					continue;
				}
				*/

				//iCurrentNode, m_pLinkPool[ m_pNodes[ iCurrentNode ].m_iFirstLink + i ]




				// check the connection from the current node to the node we're about to mark visited and push into the queue				
				if (m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_pLinkEnt != NULL)
				{// there's a brush ent in the way! Don't mark this node or put it into the queue unless the monster can negotiate it

					// This doesn't need to work with 'theHardWay' (FL_MONSTER), does it?
					if (!HandleLinkEnt(iCurrentNode, m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_pLinkEnt, afCapMask, NODEGRAPH_STATIC))
					{// monster should not try to go this way.
						continue;
					}
				}
				float flOurDistance = flCurrentDistance + m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_flWeight;
				if (m_pNodes[iVisitNode].m_flClosestSoFar < -0.5
					|| flOurDistance < m_pNodes[iVisitNode].m_flClosestSoFar - 0.001)
				{
					m_pNodes[iVisitNode].m_flClosestSoFar = flOurDistance;
					m_pNodes[iVisitNode].m_iPreviousNode = iCurrentNode;

					queue.Insert(iVisitNode, flOurDistance);
				}
			}
		}
		if (m_pNodes[iDest].m_flClosestSoFar < -0.5)
		{// Destination is unreachable, no path found.
			return 0;
		}

		// the queue is not empty

			// now we must walk backwards through the m_iPreviousNode field, and count how many connections there are in the path
		iCurrentNode = iDest;
		iNumPathNodes = 1;// count the dest

		while (iCurrentNode != iStart)
		{
			iNumPathNodes++;
			iCurrentNode = m_pNodes[iCurrentNode].m_iPreviousNode;
		}

		//MODDD - IDEA.  If this is after routing is complete, this is being called to skip the static route table.
		// The supplied 'piPath' for taking the list of nodes in the route is likely not greater than MAX_PATH.
		// Enforce a cutoff.
		int skipper = 0;

		if(m_fRoutingComplete){
			if(iNumPathNodes > MAX_PATH_SIZE){
				skipper = iNumPathNodes - MAX_PATH_SIZE;
				iNumPathNodes = MAX_PATH_SIZE;
			}
		}

		iCurrentNode = iDest;

		//MODDD - For every node iNumPathNodes is over MAX_PATH_SIZE,
		// skip a node counting backwards.  This ensures that at least
		// the earliest 16 or so nodes get written out.
		while (skipper > 0)
		{
			iCurrentNode = m_pNodes[iCurrentNode].m_iPreviousNode;
			skipper -= 1;
		}

		for (i = iNumPathNodes - 1; i >= 0; i--)
		{
			piPath[i] = iCurrentNode;
			iCurrentNode = m_pNodes[iCurrentNode].m_iPreviousNode;
		}
	}

#if 0

	if (m_fRoutingComplete)
	{
		// This will draw the entire path that was generated for the monster.

		for (int i = 0; i < iNumPathNodes - 1; i++)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_SHOWLINE);

			WRITE_COORD(m_pNodes[piPath[i]].m_vecOrigin.x);
			WRITE_COORD(m_pNodes[piPath[i]].m_vecOrigin.y);
			WRITE_COORD(m_pNodes[piPath[i]].m_vecOrigin.z + node_linktest_height);

			WRITE_COORD(m_pNodes[piPath[i + 1]].m_vecOrigin.x);
			WRITE_COORD(m_pNodes[piPath[i + 1]].m_vecOrigin.y);
			WRITE_COORD(m_pNodes[piPath[i + 1]].m_vecOrigin.z + node_linktest_height);
			MESSAGE_END();
		}
	}

#endif
#if 0 // MAZE map
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_SHOWLINE);

	WRITE_COORD(m_pNodes[4].m_vecOrigin.x);
	WRITE_COORD(m_pNodes[4].m_vecOrigin.y);
	WRITE_COORD(m_pNodes[4].m_vecOrigin.z + node_linktest_height);

	WRITE_COORD(m_pNodes[9].m_vecOrigin.x);
	WRITE_COORD(m_pNodes[9].m_vecOrigin.y);
	WRITE_COORD(m_pNodes[9].m_vecOrigin.z + node_linktest_height);
	MESSAGE_END();
#endif

	return iNumPathNodes;
}

inline ULONG Hash(void* p, int len)
{
	CRC32_t ulCrc;
	CRC32_INIT(&ulCrc);
	CRC32_PROCESS_BUFFER(&ulCrc, p, len);
	return CRC32_FINAL(ulCrc);
}

void inline CalcBounds(int& Lower, int& Upper, int Goal, int Best)
{
	int Temp = 2 * Goal - Best;
	if (Best > Goal)
	{
		Lower = max(0, Temp);
		Upper = Best;
	}
	else
	{
		Upper = min(255, Temp);
		Lower = Best;
	}
}

// Convert from [-8192,8192] to [0, 255]
//
inline int CALC_RANGE(int x, int lower, int upper)
{
	return NUM_RANGES * (x - lower) / ((upper - lower + 1));
}


void inline UpdateRange(int& minValue, int& maxValue, int Goal, int Best)
{
	int Lower, Upper;
	CalcBounds(Lower, Upper, Goal, Best);
	if (Upper < maxValue) maxValue = Upper;
	if (minValue < Lower) minValue = Lower;
}



void CGraph::CheckNode(Vector vecOrigin, int iNode)
{
	// Have we already seen this point before?.
	//
	if (m_di[iNode].m_CheckedEvent == m_CheckedCounter) return;
	m_di[iNode].m_CheckedEvent = m_CheckedCounter;

	float flDist = (vecOrigin - m_pNodes[iNode].m_vecOriginPeek).Length();

	//MODDD - is this node dead?
	// NOTICE!!!   This is the cause of the scientist at the vending machine, a1a0, not backing up
	// on having a path to the machines interrupted.   This isn't really behavior worth preserving
	// for what could weaken other pathfind checks into thinking a route that leads nowhere could 
	// really work.
	if (EASY_CVAR_GET_DEBUGONLY(pathfindIgnoreIsolatedNodes) == 1 && m_pNodes[iNode].m_cNumLinks < 1) {
		//BAM!  Class dismissed for you.
		return;
	}

	if(iNode == 41){
		//if(g_routeTempMonster != NULL){
		//	DebugLine_Setup(1, g_routeTempMonster->pev->origin, m_pNodes[iNode].m_vecOriginPeek, 255, 255, 255);
		//}
	}
	if(iNode == 36){
		//if(g_routeTempMonster != NULL){
		//	DebugLine_Setup(1, g_routeTempMonster->pev->origin, m_pNodes[iNode].m_vecOriginPeek, 255, 255, 255);
		//}
	}

	if (flDist < m_flShortest)
	{
		TraceResult tr;
		BOOL passed;
		

		// MODDD - TODO!  Would be neat to be able to tell if pathfinding would like the choice from here to there in advance,
		// like place a sample entity of the given hull size (also sent in this method) and try to see if CheckLocalMove
		// (or at least WALK_MOVE?) from here to there would have worked.  CheckLocalMove would be as accurate as it gets though.
		
		// Letting that happen if pathfindNearestNodeExtra is on.
		if(EASY_CVAR_GET_DEBUGONLY(pathfindNearestNodeExtra) == 1 && g_routeTempMonster != NULL ){
			Vector vecStart = vecOrigin;
			vecStart.z = EASY_CVAR_GET_DEBUGONLY(nodeSearchStartVerticalOffset);
			
			// running into your enemy is always a good thing.  Right?
			passed = (g_routeTempMonster->CheckLocalMove(vecStart, m_pNodes[iNode].m_vecOriginPeek, g_routeTempMonster_GoalEnt, TRUE, NULL) == LOCALMOVE_VALID);

		}else{
			// !!!   AS-IS WAY.   mostly.
			/////////////////////////////////////////////////////////////////////////////////////////////////
			// make sure that vecOrigin can trace to this node!
			//MODDD - involving nodeSearchStartVerticalOffset now.
			Vector vecStart = vecOrigin;

			//if(g_routeTempMonster->isMovetypeFlying()){
			///
			//}

			if(m_pNodes[iNode].m_afNodeInfo & (bits_NODE_LAND)){
				// has the LAND flag?  Add the vertical offset
				//MODDD - TODO.  ish.
				// If ever doing any hack to treat land nodes as air nodes to force using them, that needs to be checked separately)
				vecStart.z += EASY_CVAR_GET_DEBUGONLY(nodeSearchStartVerticalOffset);
			}

			//UTIL_TraceLine ( vecOrigin, m_pNodes[ iNode ].m_vecOriginPeek, ignore_monsters, 0, &tr );
			UTIL_TraceLine(vecStart, m_pNodes[iNode].m_vecOriginPeek, ignore_monsters, 0, &tr);

			passed = (tr.flFraction >= 1.0);
			/////////////////////////////////////////////////////////////////////////////////////////////////			
		}
		//WALK_MOVE(testEnt, yaw, dist, iMode);



		if (passed)
		{
			m_iNearest = iNode;
			m_flShortest = flDist;

			UpdateRange(m_minX, m_maxX, CALC_RANGE(vecOrigin.x, m_RegionMin[0], m_RegionMax[0]), m_pNodes[iNode].m_Region[0]);
			UpdateRange(m_minY, m_maxY, CALC_RANGE(vecOrigin.y, m_RegionMin[1], m_RegionMax[1]), m_pNodes[iNode].m_Region[1]);
			UpdateRange(m_minZ, m_maxZ, CALC_RANGE(vecOrigin.z, m_RegionMin[2], m_RegionMax[2]), m_pNodes[iNode].m_Region[2]);

			// From maxCircle, calculate maximum bounds box. All points must be
			// simultaneously inside all bounds of the box.
			//
			m_minBoxX = CALC_RANGE(vecOrigin.x - flDist, m_RegionMin[0], m_RegionMax[0]);
			m_maxBoxX = CALC_RANGE(vecOrigin.x + flDist, m_RegionMin[0], m_RegionMax[0]);
			m_minBoxY = CALC_RANGE(vecOrigin.y - flDist, m_RegionMin[1], m_RegionMax[1]);
			m_maxBoxY = CALC_RANGE(vecOrigin.y + flDist, m_RegionMin[1], m_RegionMax[1]);
			m_minBoxZ = CALC_RANGE(vecOrigin.z - flDist, m_RegionMin[2], m_RegionMax[2]);
			m_maxBoxZ = CALC_RANGE(vecOrigin.z + flDist, m_RegionMin[2], m_RegionMax[2]);
		}
	}
}

//=========================================================
// CGraph - FindNearestNode - returns the index of the node nearest
// the given vector -1 is failure (couldn't find a valid
// near node )
//=========================================================
int CGraph::FindNearestNode(const Vector& vecOrigin, CBaseEntity* pEntity)
{
	// TODO - why not 'pEntity->getNodeTypeAllowed()' ?
	return FindNearestNode(vecOrigin, NodeType(pEntity));
}

int CGraph::FindNearestNode(const Vector& vecOrigin, int afNodeTypes)
{
	int i;
	TraceResult tr;

	//MODDD - DANGEROUS HACK.
	//afNodeTypes |= bits_NODE_LAND | bits_NODE_WATER | bits_NODE_AIR;

	if (!m_fGraphPresent || !m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available
		ALERT(at_aiconsole, "Graph not ready!\n");
		return -1;
	}




	// Check with the cache
	//
	//MODDD - renamed CACHE_SIZE, as seen in nodes.h.
	// Also to ensure the CACHE_SIZE meant for m_Cache is used.
	
	ULONG iHash = (GRAPH_CACHE_SIZE - 1) & Hash((void*)(const float*)vecOrigin, sizeof(vecOrigin));

	//MODDD - MAJOR.  ALSO.  Can skip using nearest-node cache by CVar, debug feature.
	// Surrounds as-is script.
	// ALSO.  If pathfindNearestNodeExtra is on and g_routeTempMonster isn't NULL (a sign it would be involved in finding the nearest node),
	// automatically ignore the node cache.  What one entity size calls nearest another may not be able to get to with the size-involved
	// checks.  Even though retail had no size-checking whatsoever for this, just a point-to-point line trace...
	// Involving the HULL size (or best fit?) in m_Cache, and cacheing per size type would be ok there.
	////////////////////////////////////////////////////////////////////////////////////
	// Should monsters with the FL_MONSTERCLIP flag skip the nearest-node cache?  They're much less common and woulnd't benefit
	// from setting the cache for ordinary monsters to fool them into thinking a node close to them that they can't traceline to is valid
	// (but the MONSTERCLIP one could), and vice versa.
	// Doing this, and that means don't set the cache in such a case, leave it only for normal monsters.
	// If a huge number of monsters had MONSTERCLIP a separate cache for that type would make sense (same for the static route table
	// having an entry for each type of hull size really)

	BOOL ignoreCacheException;

	ignoreCacheException = (g_routeTempMonster != NULL && (g_routeTempMonster->pev->flags & FL_MONSTERCLIP));
	

	if(
		EASY_CVAR_GET(pathfindIgnoreNearestNodeCache) != 1 &&
		!(EASY_CVAR_GET_DEBUGONLY(pathfindNearestNodeExtra) == 1 && g_routeTempMonster != NULL) &&
		!ignoreCacheException
	){

		//MODDD - extra check.  allowed to return a -1 node?
		//...then again, memory is memory. If it failed before, it won't change. guess this is ok.

		//if (m_Cache[iHash].n != -1 && m_Cache[iHash].v == vecOrigin)
		if (m_Cache[iHash].v == vecOrigin)
		{
			//ALERT(at_aiconsole, "Cache Hit.\n");
			return m_Cache[iHash].n;
		}
		else
		{
			//ALERT(at_aiconsole, "Cache Miss.\n");
		}

	}//END OF CVar check
	////////////////////////////////////////////////////////////////////////////////////

	// Mark all points as unchecked.
	//
	m_CheckedCounter++;
	if (m_CheckedCounter == 0)
	{
		for (i = 0; i < m_cNodes; i++)
		{
			m_di[i].m_CheckedEvent = 0;
		}
		m_CheckedCounter++;
	}

	m_iNearest = -1;
	m_flShortest = 999999.0; // just a big number.

	// If we can find a visible point, then let CalcBounds set the limits, but if
	// we have no visible point at all to start with, then don't restrict the limits.
	//
#if 1
	m_minX = 0; m_maxX = 255;
	m_minY = 0; m_maxY = 255;
	m_minZ = 0; m_maxZ = 255;
	m_minBoxX = 0; m_maxBoxX = 255;
	m_minBoxY = 0; m_maxBoxY = 255;
	m_minBoxZ = 0; m_maxBoxZ = 255;
#else
	m_minBoxX = CALC_RANGE(vecOrigin.x - flDist, m_RegionMin[0], m_RegionMax[0]);
	m_maxBoxX = CALC_RANGE(vecOrigin.x + flDist, m_RegionMin[0], m_RegionMax[0]);
	m_minBoxY = CALC_RANGE(vecOrigin.y - flDist, m_RegionMin[1], m_RegionMax[1]);
	m_maxBoxY = CALC_RANGE(vecOrigin.y + flDist, m_RegionMin[1], m_RegionMax[1]);
	m_minBoxZ = CALC_RANGE(vecOrigin.z - flDist, m_RegionMin[2], m_RegionMax[2]);
	m_maxBoxZ = CALC_RANGE(vecOrigin.z + flDist, m_RegionMin[2], m_RegionMax[2])
		CalcBounds(m_minX, m_maxX, CALC_RANGE(vecOrigin.x, m_RegionMin[0], m_RegionMax[0]), m_pNodes[m_iNearest].m_Region[0]);
	CalcBounds(m_minY, m_maxY, CALC_RANGE(vecOrigin.y, m_RegionMin[1], m_RegionMax[1]), m_pNodes[m_iNearest].m_Region[1]);
	CalcBounds(m_minZ, m_maxZ, CALC_RANGE(vecOrigin.z, m_RegionMin[2], m_RegionMax[2]), m_pNodes[m_iNearest].m_Region[2]);
#endif

	int halfX = (m_minX + m_maxX) / 2;
	int halfY = (m_minY + m_maxY) / 2;
	int halfZ = (m_minZ + m_maxZ) / 2;

	int j;

	for (i = halfX; i >= m_minX; i--)
	{
		for (j = m_RangeStart[0][i]; j <= m_RangeEnd[0][i]; j++)
		{
			if (!(m_pNodes[m_di[j].m_SortedBy[0]].m_afNodeInfo & afNodeTypes)) continue;

			int rgY = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[1];
			if (rgY > m_maxBoxY) break;
			if (rgY < m_minBoxY) continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[2];
			if (rgZ < m_minBoxZ) continue;
			if (rgZ > m_maxBoxZ) continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[0]);
		}
	}

	for (i = max(m_minY, halfY + 1); i <= m_maxY; i++)
	{
		for (j = m_RangeStart[1][i]; j <= m_RangeEnd[1][i]; j++)
		{
			if (!(m_pNodes[m_di[j].m_SortedBy[1]].m_afNodeInfo & afNodeTypes)) continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[2];
			if (rgZ > m_maxBoxZ) break;
			if (rgZ < m_minBoxZ) continue;
			int rgX = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[0];
			if (rgX < m_minBoxX) continue;
			if (rgX > m_maxBoxX) continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[1]);
		}
	}

	for (i = min(m_maxZ, halfZ); i >= m_minZ; i--)
	{
		for (j = m_RangeStart[2][i]; j <= m_RangeEnd[2][i]; j++)
		{
			if (!(m_pNodes[m_di[j].m_SortedBy[2]].m_afNodeInfo & afNodeTypes)) continue;

			int rgX = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[0];
			if (rgX > m_maxBoxX) break;
			if (rgX < m_minBoxX) continue;
			int rgY = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[1];
			if (rgY < m_minBoxY) continue;
			if (rgY > m_maxBoxY) continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[2]);
		}
	}

	for (i = max(m_minX, halfX + 1); i <= m_maxX; i++)
	{
		for (j = m_RangeStart[0][i]; j <= m_RangeEnd[0][i]; j++)
		{
			if (!(m_pNodes[m_di[j].m_SortedBy[0]].m_afNodeInfo & afNodeTypes)) continue;

			int rgY = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[1];
			if (rgY > m_maxBoxY) break;
			if (rgY < m_minBoxY) continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[2];
			if (rgZ < m_minBoxZ) continue;
			if (rgZ > m_maxBoxZ) continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[0]);
		}
	}

	for (i = min(m_maxY, halfY); i >= m_minY; i--)
	{
		for (j = m_RangeStart[1][i]; j <= m_RangeEnd[1][i]; j++)
		{
			if (!(m_pNodes[m_di[j].m_SortedBy[1]].m_afNodeInfo & afNodeTypes)) continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[2];
			if (rgZ > m_maxBoxZ) break;
			if (rgZ < m_minBoxZ) continue;
			int rgX = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[0];
			if (rgX < m_minBoxX) continue;
			if (rgX > m_maxBoxX) continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[1]);
		}
	}

	for (i = max(m_minZ, halfZ + 1); i <= m_maxZ; i++)
	{
		for (j = m_RangeStart[2][i]; j <= m_RangeEnd[2][i]; j++)
		{
			if (!(m_pNodes[m_di[j].m_SortedBy[2]].m_afNodeInfo & afNodeTypes)) continue;

			int rgX = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[0];
			if (rgX > m_maxBoxX) break;
			if (rgX < m_minBoxX) continue;
			int rgY = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[1];
			if (rgY < m_minBoxY) continue;
			if (rgY > m_maxBoxY) continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[2]);
		}
	}

#if 0
	// Verify our answers.
	//
	int iNearestCheck = -1;
	m_flShortest = 8192;// find nodes within this radius

	for (i = 0; i < m_cNodes; i++)
	{
		float flDist = (vecOrigin - m_pNodes[i].m_vecOriginPeek).Length();

		if (flDist < m_flShortest)
		{
			// make sure that vecOrigin can trace to this node!
			UTIL_TraceLine(vecOrigin, m_pNodes[i].m_vecOriginPeek, ignore_monsters, 0, &tr);

			if (tr.flFraction == 1.0)
			{
				iNearestCheck = i;
				m_flShortest = flDist;
			}
		}
	}

	if (iNearestCheck != m_iNearest)
	{
		ALERT(at_aiconsole, "NOT closest %d(%f,%f,%f) %d(%f,%f,%f).\n",
			iNearestCheck,
			m_pNodes[iNearestCheck].m_vecOriginPeek.x,
			m_pNodes[iNearestCheck].m_vecOriginPeek.y,
			m_pNodes[iNearestCheck].m_vecOriginPeek.z,
			m_iNearest,
			(m_iNearest == -1 ? 0.0 : m_pNodes[m_iNearest].m_vecOriginPeek.x),
			(m_iNearest == -1 ? 0.0 : m_pNodes[m_iNearest].m_vecOriginPeek.y),
			(m_iNearest == -1 ? 0.0 : m_pNodes[m_iNearest].m_vecOriginPeek.z));
	}
	if (m_iNearest == -1)
	{
		ALERT(at_aiconsole, "All that work for nothing.\n");
	}
#endif

	//MODDD - only set the cache if I'm not a special case.
	if(!ignoreCacheException){
		m_Cache[iHash].v = vecOrigin;
		m_Cache[iHash].n = m_iNearest;
	}
	return m_iNearest;
}

//=========================================================
// CGraph - ShowNodeConnections - draws a line from the given node
// to all connected nodes
//=========================================================
void CGraph::ShowNodeConnections(int iNode)
{
	Vector	vecSpot;
	CNode* pNode;
	CNode* pLinkNode;
	int i;

	if (!m_fGraphPresent || !m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available or built
		ALERT(at_aiconsole, "Graph not ready!\n");
		return;
	}

	if (iNode < 0)
	{
		ALERT(at_aiconsole, "Can't show connections for node %d\n", iNode);
		return;
	}

	pNode = &m_pNodes[iNode];

	UTIL_ParticleEffect(pNode->m_vecOrigin, g_vecZero, 255, 20);// show node position

	if (pNode->m_cNumLinks <= 0)
	{// no connections!
		ALERT(at_aiconsole, "**No Connections!\n");
		//sheesh, just say it.
		easyPrintLine("***No connections out!***");
	}

	for (i = 0; i < pNode->m_cNumLinks; i++)
	{
		easyPrintLine("*CON #%d: to %d", i, NodeLink(iNode, i).m_iDestNode);

		pLinkNode = &Node(NodeLink(iNode, i).m_iDestNode);
		vecSpot = pLinkNode->m_vecOrigin;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SHOWLINE);

		WRITE_COORD(m_pNodes[iNode].m_vecOrigin.x);
		WRITE_COORD(m_pNodes[iNode].m_vecOrigin.y);
		WRITE_COORD(m_pNodes[iNode].m_vecOrigin.z + node_linktest_height);

		WRITE_COORD(vecSpot.x);
		WRITE_COORD(vecSpot.y);
		WRITE_COORD(vecSpot.z + node_linktest_height);
		MESSAGE_END();

	}
	easyPrintLine("***END OF CONNECTIONS***");
}




CGraph::CGraph(void) {
	scheduleNodeUpdate = FALSE;
	node_linktest_height = 8;
	node_hulltest_height = 8;
	node_hulltest_heightswap = FALSE;
	map_anyAirNodes = FALSE;

}//END OF CGraph constructor


//=========================================================
// CGraph - LinkVisibleNodes - the first, most basic
// function of node graph creation, this connects every
// node to every other node that it can see. Expects a 
// pointer to an empty connection pool and a file pointer 
// to write progress to. Returns the total number of initial
// links.
//
// If there's a problem with this process, the index
// of the offending node will be written to piBadNode
//=========================================================
int CGraph::LinkVisibleNodes(CLink* pLinkPool, FILE* file, int* piBadNode)
{
	int		i, j, z;
	edict_t* pTraceEnt;
	int		cTotalLinks, cLinksThisNode, cMaxInitialLinks;
	TraceResult	tr;

	// !!!BUGBUG - this function returns 0 if there is a problem in the middle of connecting the graph
	// it also returns 0 if none of the nodes in a level can see each other. piBadNode is ALWAYS read
	// by BuildNodeGraph() if this function returns a 0, so make sure that it doesn't get some random
	// number back.
	*piBadNode = 0;

	//MODDD - new.
	float linkVisibleExtraVertical;

	if (m_cNodes <= 0)
	{
		ALERT(at_aiconsole, "No Nodes!\n");
		return FALSE;
	}

	// if the file pointer is bad, don't blow up, just don't write the
	// file.
	if (!file)
	{
		ALERT(at_aiconsole, "**LinkVisibleNodes:\ncan't write to file.");
	}
	else
	{
		fprintf(file, "----------------------------------------------------------------------------\n");
		fprintf(file, "LinkVisibleNodes - Initial Connections\n");
		fprintf(file, "----------------------------------------------------------------------------\n");
	}

	cTotalLinks = 0;// start with no connections

	// to keep track of the maximum number of initial links any node had so far.
	// this lets us keep an eye on MAX_NODE_INITIAL_LINKS to ensure that we are
	// being generous enough.
	cMaxInitialLinks = 0;

	for (i = 0; i < m_cNodes; i++)
	{
		cLinksThisNode = 0;// reset this count for each node.

		if (file)
		{
			fprintf(file, "Node #%4d:\n\n", i);
		}

		for (z = 0; z < MAX_NODE_INITIAL_LINKS; z++)
		{// clear out the important fields in the link pool for this node
			pLinkPool[cTotalLinks + z].m_iSrcNode = i;// so each link knows which node it originates from
			pLinkPool[cTotalLinks + z].m_iDestNode = 0;
			pLinkPool[cTotalLinks + z].m_pLinkEnt = NULL;
		}

		m_pNodes[i].m_iFirstLink = cTotalLinks;

		// now build a list of every other node that this node can see
		for (j = 0; j < m_cNodes; j++)
		{
			if (j == i)
			{// don't connect to self!
				continue;
			}

#if 0

			if ((m_pNodes[i].m_afNodeInfo & bits_NODE_WATER) != (m_pNodes[j].m_afNodeInfo & bits_NODE_WATER))
			{
				// don't connect water nodes to air nodes or land nodes. It just wouldn't be prudent at this juncture.
				continue;
			}
#else
			if ((m_pNodes[i].m_afNodeInfo & bits_NODE_GROUP_REALM) != (m_pNodes[j].m_afNodeInfo & bits_NODE_GROUP_REALM))
			{
				// don't connect air nodes to water nodes to land nodes. It just wouldn't be prudent at this juncture.
				continue;
			}
#endif

			tr.pHit = NULL;// clear every time so we don't get stuck with last trace's hit ent
			pTraceEnt = 0;




			//Since nodeInfo must match between nodes i and j being compared, if the source is LAND assume it applies to both.
			if (m_pNodes[i].m_afNodeInfo & bits_NODE_LAND) {
				linkVisibleExtraVertical = node_linktest_height;
			}
			else {
				linkVisibleExtraVertical = 0;
			}

			//MODDD - allow for a vertical offset for raw line traces.
			UTIL_TraceLine(Vector(m_pNodes[i].m_vecOrigin.x, m_pNodes[i].m_vecOrigin.y, m_pNodes[i].m_vecOrigin.z + linkVisibleExtraVertical),
				Vector(m_pNodes[j].m_vecOrigin.x, m_pNodes[j].m_vecOrigin.y, m_pNodes[j].m_vecOrigin.z + linkVisibleExtraVertical),
				ignore_monsters,
				g_pBodyQueueHead,//!!!HACKHACK no real ent to supply here, using a global we don't care about
				&tr);


			if (tr.fStartSolid)
				continue;

			if (tr.flFraction != 1.0)
			{// trace hit a brush ent, trace backwards to make sure that this ent is the only thing in the way.

				pTraceEnt = tr.pHit;// store the ent that the trace hit, for comparison

				//MODDD - allow for a vertical offset for raw line traces.
				UTIL_TraceLine(Vector(m_pNodes[j].m_vecOrigin.x, m_pNodes[j].m_vecOrigin.y, m_pNodes[j].m_vecOrigin.z + linkVisibleExtraVertical),
					Vector(m_pNodes[i].m_vecOrigin.x, m_pNodes[i].m_vecOrigin.y, m_pNodes[i].m_vecOrigin.z + linkVisibleExtraVertical),
					ignore_monsters,
					g_pBodyQueueHead,//!!!HACKHACK no real ent to supply here, using a global we don't care about
					&tr);


				// there is a solid_bsp ent in the way of these two nodes, so we must record several things about in order to keep
				// track of it in the pathfinding code, as well as through save and restore of the node graph. ANY data that is manipulated 
				// as part of the process of adding a LINKENT to a connection here must also be done in CGraph::SetGraphPointers, where reloaded
				// graphs are prepared for use.
				if (tr.pHit == pTraceEnt && !FClassnameIs(tr.pHit, "worldspawn"))
				{

					if (EASY_CVAR_GET_DEBUGONLY(nodeDetailPrintout) == 1) {
						easyForcePrintLine("OOOOOHOOOOOOOOOOOOOOOOOOO: %s", STRING(tr.pHit->v.classname));
					}

					// get a pointer
					pLinkPool[cTotalLinks].m_pLinkEnt = VARS(tr.pHit);

					// record the modelname, so that we can save/load node trees
					memcpy(pLinkPool[cTotalLinks].m_szLinkEntModelname, STRING(VARS(tr.pHit)->model), 4);

					// set the flag for this ent that indicates that it is attached to the world graph
					// if this ent is removed from the world, it must also be removed from the connections
					// that it formerly blocked.
					if (!FBitSet(VARS(tr.pHit)->flags, FL_GRAPHED))
					{
						VARS(tr.pHit)->flags += FL_GRAPHED;
					}
				}
				else
				{// even if the ent wasn't there, these nodes couldn't be connected. Skip.
					continue;
				}
			}

			if (file)
			{
				fprintf(file, "%4d", j);

				if (!FNullEnt(pLinkPool[cTotalLinks].m_pLinkEnt))
				{// record info about the ent in the way, if any.
					fprintf(file, "  Entity on connection: %s, name: %s  Model: %s", STRING(VARS(pTraceEnt)->classname), STRING(VARS(pTraceEnt)->targetname), STRING(VARS(tr.pHit)->model));
				}

				//MODDD - minor mistake.
				//fprintf ( file, "\n", j );
				fprintf(file, "\n");
			}

			pLinkPool[cTotalLinks].m_iDestNode = j;
			cLinksThisNode++;
			cTotalLinks++;

			// If we hit this, either a level designer is placing too many nodes in the same area, or 
			// we need to allow for a larger initial link pool.
			if (cLinksThisNode == MAX_NODE_INITIAL_LINKS)
			{
				ALERT(at_aiconsole, "**LinkVisibleNodes:\nNode %d has NodeLinks > MAX_NODE_INITIAL_LINKS", i);
				fprintf(file, "** NODE %d HAS NodeLinks > MAX_NODE_INITIAL_LINKS **\n", i);
				*piBadNode = i;
				return	FALSE;
			}
			else if (cTotalLinks > MAX_NODE_INITIAL_LINKS* m_cNodes)
			{// this is paranoia
				ALERT(at_aiconsole, "**LinkVisibleNodes:\nTotalLinks > MAX_NODE_INITIAL_LINKS * NUMNODES");
				*piBadNode = i;
				return	FALSE;
			}

			if (cLinksThisNode == 0)
			{
				fprintf(file, "**NO INITIAL LINKS**\n");
			}

			// record the connection info in the link pool
			WorldGraph.m_pNodes[i].m_cNumLinks = cLinksThisNode;

			// keep track of the most initial links ANY node had, so we can figure out
			// if we have a large enough default link pool
			if (cLinksThisNode > cMaxInitialLinks)
			{
				cMaxInitialLinks = cLinksThisNode;
			}
		}


		if (file)
		{
			fprintf(file, "----------------------------------------------------------------------------\n");
		}
	}

	fprintf(file, "\n%4d Total Initial Connections - %4d Maximum connections for a single node.\n", cTotalLinks, cMaxInitialLinks);
	fprintf(file, "----------------------------------------------------------------------------\n\n\n");

	return cTotalLinks;
}

//=========================================================
// CGraph - RejectInlineLinks - expects a pointer to a link
// pool, and a pointer to and already-open file ( if you
// want status reports written to disk ). RETURNS the number
// of connections that were rejected
//=========================================================
int CGraph::RejectInlineLinks(CLink* pLinkPool, FILE* file)
{
	int	i, j, k;

	int	cRejectedLinks;

	BOOL	fRestartLoop;// have to restart the J loop if we eliminate a link.

	CNode* pSrcNode;
	CNode* pCheckNode;// the node we are testing for (one of pSrcNode's connections)
	CNode* pTestNode;// the node we are checking against ( also one of pSrcNode's connections)

	float flDistToTestNode, flDistToCheckNode;

	Vector2D	vec2DirToTestNode, vec2DirToCheckNode;

	if (file)
	{
		fprintf(file, "----------------------------------------------------------------------------\n");
		fprintf(file, "InLine Rejection:\n");
		fprintf(file, "----------------------------------------------------------------------------\n");
	}

	cRejectedLinks = 0;

	for (i = 0; i < m_cNodes; i++)
	{
		pSrcNode = &m_pNodes[i];

		if (file)
		{
			fprintf(file, "Node %3d:\n", i);
		}

		for (j = 0; j < pSrcNode->m_cNumLinks; j++)
		{
			pCheckNode = &m_pNodes[pLinkPool[pSrcNode->m_iFirstLink + j].m_iDestNode];

			vec2DirToCheckNode = (pCheckNode->m_vecOrigin - pSrcNode->m_vecOrigin).Make2D();
			flDistToCheckNode = vec2DirToCheckNode.Length();
			vec2DirToCheckNode = vec2DirToCheckNode.Normalize();

			pLinkPool[pSrcNode->m_iFirstLink + j].m_flWeight = flDistToCheckNode;

			fRestartLoop = FALSE;
			for (k = 0; k < pSrcNode->m_cNumLinks && !fRestartLoop; k++)
			{
				if (k == j)
				{// don't check against same node
					continue;
				}

				pTestNode = &m_pNodes[pLinkPool[pSrcNode->m_iFirstLink + k].m_iDestNode];

				vec2DirToTestNode = (pTestNode->m_vecOrigin - pSrcNode->m_vecOrigin).Make2D();

				flDistToTestNode = vec2DirToTestNode.Length();
				vec2DirToTestNode = vec2DirToTestNode.Normalize();

				if (DotProduct(vec2DirToCheckNode, vec2DirToTestNode) >= 0.998)
				{
					// there's a chance that TestNode intersects the line to CheckNode. If so, we should disconnect the link to CheckNode. 
					if (flDistToTestNode < flDistToCheckNode)
					{
						if (file)
						{
							fprintf(file, "REJECTED NODE %3d through Node %3d, Dot = %8f\n", pLinkPool[pSrcNode->m_iFirstLink + j].m_iDestNode, pLinkPool[pSrcNode->m_iFirstLink + k].m_iDestNode, DotProduct(vec2DirToCheckNode, vec2DirToTestNode));
						}

						pLinkPool[pSrcNode->m_iFirstLink + j] = pLinkPool[pSrcNode->m_iFirstLink + (pSrcNode->m_cNumLinks - 1)];
						pSrcNode->m_cNumLinks--;
						j--;

						cRejectedLinks++;// keeping track of how many links are cut, so that we can return that value.

						fRestartLoop = TRUE;
					}
				}
			}
		}

		if (file)
		{
			fprintf(file, "----------------------------------------------------------------------------\n\n");
		}
	}

	return cRejectedLinks;
}

//=========================================================
// TestHull is a modelless clip hull that verifies reachable
// nodes by walking from every node to each of it's connections
//=========================================================
class CTestHull : public CBaseMonster
{

public:
	BOOL isOrganic(void);
	void Spawn(entvars_t* pevMasterNode);
	virtual int ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void EXPORT CallBuildNodeGraph(void);
	void BuildNodeGraph(void);
	void EXPORT ShowBadNode(void);
	void EXPORT DropDelay(void);
	void EXPORT PathFind(void);

	Vector	vecBadNodeOrigin;
};

LINK_ENTITY_TO_CLASS(testhull, CTestHull);

//MODDD - just in case? Probably unimportant.
BOOL CTestHull::isOrganic(void) {
	return FALSE;
}

//=========================================================
// CTestHull::Spawn
//=========================================================
//MODDD NOTE - the supplied argument "pevMasterNode" is completley unused.
//             It's staying here for... reasons? guess it got left over.
void CTestHull::Spawn(entvars_t* pevMasterNode)
{
	SET_MODEL(ENT(pev), "models/player.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = 50;
	pev->yaw_speed = 8;

	if (WorldGraph.m_fGraphPresent)
	{// graph loaded from disk, so we don't need the test hull
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		SetThink(&CTestHull::DropDelay);
		pev->nextthink = gpGlobals->time + 1;
	}

	// Make this invisible
	// UNDONE: Shouldn't we just use EF_NODRAW?  This doesn't need to go to the client.
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;
}

//=========================================================
// TestHull::DropDelay - spawns TestHull on top of 
// the 0th node and drops it to the ground.
//=========================================================
void CTestHull::DropDelay(void)
{
	//MODDD - can hide this text.
	if (EASY_CVAR_GET_DEBUGONLY(hideNodeGraphRebuildNotice) != 1) {
		CenterPrintAll("Node Graph out of Date. Rebuilding...");
	}

	UTIL_SetOrigin(VARS(pev), WorldGraph.m_pNodes[0].m_vecOrigin);

	SetThink(&CTestHull::CallBuildNodeGraph);

	pev->nextthink = gpGlobals->time + 1;
}

//=========================================================
// nodes start out as ents in the world. As they are spawned,
// the node info is recorded then the ents are discarded.
//=========================================================
void CNodeEnt::KeyValue(KeyValueData* pkvd)
{

	//MODDD NOTE - is there one called "nodeid"?  Because it sure is ignored as of retail.
	//It may look kinda like this?  Should be checked to see it isn't bogus anyways if this
	//were ever done.
	/*
	if (FStrEq(pkvd->szKeyName, "nodeid"))
	{
		nodeid_suggestion = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	*/



	if (FStrEq(pkvd->szKeyName, "hinttype"))
	{
		m_sHintType = (short)atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	if (FStrEq(pkvd->szKeyName, "activity"))
	{
		m_sHintActivity = (short)atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

//=========================================================
//=========================================================
void CNodeEnt::Spawn(void)
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;// always solid_not 

	if (WorldGraph.m_fGraphPresent)
	{// graph loaded from disk, so discard all these node ents as soon as they spawn
		REMOVE_ENTITY(edict());
		return;
	}


	if (WorldGraph.m_cNodes == 0)
	{// this is the first node to spawn, spawn the test hull entity that builds and walks the node tree
		CTestHull* pHull = GetClassPtr((CTestHull*)NULL);
		pHull->Spawn(pev);
	}

	if (WorldGraph.m_cNodes >= MAX_NODES)
	{
		ALERT(at_aiconsole, "cNodes > MAX_NODES\n");
		return;
	}

	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_vecOriginPeek =
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_vecOrigin = pev->origin;

	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_flHintYaw = pev->angles.y;
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_sHintType = m_sHintType;
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_sHintActivity = m_sHintActivity;


	if (FClassnameIs(pev, "info_node_air")) {
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_afNodeInfo = bits_NODE_AIR;

		//MODDD - yes?
		map_anyAirNodes = TRUE;

	}
	else {
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_afNodeInfo = 0;
	}

	WorldGraph.m_cNodes++;

	REMOVE_ENTITY(edict());

}

//=========================================================
// CTestHull - ShowBadNode - makes a bad node fizzle. When
// there's a problem with node graph generation, the test 
// hull will be placed up the bad node's location and will generate
// particles
//=========================================================
void CTestHull::ShowBadNode(void)
{
	pev->movetype = MOVETYPE_FLY;
	pev->angles.y = pev->angles.y + 4;

	UTIL_MakeVectors(pev->angles);

	UTIL_ParticleEffect(pev->origin, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin + gpGlobals->v_forward * 64, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin - gpGlobals->v_forward * 64, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin + gpGlobals->v_right * 64, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin - gpGlobals->v_right * 64, g_vecZero, 255, 25);

	pev->nextthink = gpGlobals->time + 0.1;
}


void CTestHull::CallBuildNodeGraph(void)
{
	// TOUCH HACK -- Don't allow this entity to call anyone's "touch" function
	gTouchDisabled = TRUE;
	BuildNodeGraph();
	gTouchDisabled = FALSE;
	// Undo TOUCH HACK
}

//=========================================================
// BuildNodeGraph - think function called by the empty walk
// hull that is spawned by the first node to spawn. This
// function links all nodes that can see each other, then
// eliminates all inline links, then uses a monster-sized 
// hull that walks between each node and each of its links
// to ensure that a monster can actually fit through the space
//=========================================================
void CTestHull::BuildNodeGraph(void)
{
	FILE* file;

	char	szNrpFilename[MAX_PATH];// text node report filename

	CLink* pTempPool; // temporary link pool 

	CNode* pSrcNode;// node we're currently working with
	CNode* pDestNode;// the other node in comparison operations

	//MODDD - new variables to be the same as pSrcNode and pDestNode or swapped if necessary before doing a check.
	//Some WALK_MOVE calls on elevation / inclines / stairs may be better checking the higher one vs. the lower one.
	CNode* pHullTest_SrcNode;
	CNode* pHullTest_DestNode;

	BOOL	fSkipRemainingHulls;//if smallest hull can't fit, don't check any others
	BOOL	fPairsValid;// are all links in the graph evenly paired?

	int	i, j, hull;

	int	iBadNode;// this is the node that caused graph generation to fail

	int	cMaxInitialLinks = 0;
	int	cMaxValidLinks = 0;

	int	iPoolIndex = 0;
	int	cPoolLinks;// number of links in the pool.

	Vector	vecDirToCheckNode;
	Vector	vecDirToTestNode;
	Vector	vecStepCheckDir;
	Vector	vecTraceSpot;
	Vector  vecSpot;

	Vector2D	vec2DirToCheckNode;
	Vector2D	vec2DirToTestNode;
	Vector2D	vec2StepCheckDir;
	Vector2D	vec2TraceSpot;
	Vector2D	vec2Spot;

	float flYaw;// use this stuff to walk the hull between nodes
	float flDist;
	int	step;

	//MODDD - apply anotehr offset to land-based nodes.
	float hulltestExtraVertical;

	SetThink(&CBaseEntity::SUB_Remove);// no matter what happens, the hull gets rid of itself.
	pev->nextthink = gpGlobals->time;

	// 	malloc a swollen temporary connection pool that we trim down after we know exactly how many connections there are.
	pTempPool = (CLink*)calloc(sizeof(CLink), (WorldGraph.m_cNodes * MAX_NODE_INITIAL_LINKS));
	if (!pTempPool)
	{
		ALERT(at_aiconsole, "**Could not malloc TempPool!\n");
		return;
	}


	// make sure directories have been made
	GET_GAME_DIR(szNrpFilename);
	strcat(szNrpFilename, "/maps");
	CreateDirectory(szNrpFilename, NULL);
	strcat(szNrpFilename, "/graphs");
	CreateDirectory(szNrpFilename, NULL);

	strcat(szNrpFilename, "/");
	strcat(szNrpFilename, STRING(gpGlobals->mapname));
	strcat(szNrpFilename, ".nrp");

	file = fopen(szNrpFilename, "w+");

	if (!file)
	{// file error
		ALERT(at_aiconsole, "Couldn't create %s!\n", szNrpFilename);

		if (pTempPool)
		{
			free(pTempPool);
		}

		return;
	}

	fprintf(file, "Node Graph Report for map:  %s.bsp\n", STRING(gpGlobals->mapname));
	fprintf(file, "%d Total Nodes\n\n", WorldGraph.m_cNodes);

	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{// print all node numbers and their locations to the file.
		WorldGraph.m_pNodes[i].m_cNumLinks = 0;
		WorldGraph.m_pNodes[i].m_iFirstLink = 0;
		memset(WorldGraph.m_pNodes[i].m_pNextBestNode, 0, sizeof(WorldGraph.m_pNodes[i].m_pNextBestNode));

		fprintf(file, "Node#         %4d\n", i);
		fprintf(file, "Location      %4d,%4d,%4d\n", (int)WorldGraph.m_pNodes[i].m_vecOrigin.x, (int)WorldGraph.m_pNodes[i].m_vecOrigin.y, (int)WorldGraph.m_pNodes[i].m_vecOrigin.z);
		fprintf(file, "HintType:     %4d\n", WorldGraph.m_pNodes[i].m_sHintType);
		fprintf(file, "HintActivity: %4d\n", WorldGraph.m_pNodes[i].m_sHintActivity);
		fprintf(file, "HintYaw:      %4f\n", WorldGraph.m_pNodes[i].m_flHintYaw);
		fprintf(file, "-------------------------------------------------------------------------------\n");
	}
	fprintf(file, "\n\n");


	// Automatically recognize WATER nodes and drop the LAND nodes to the floor.
	//
	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		//MODDD - likely for this to come up at some point.
		int thisNodeContents = UTIL_PointContents(WorldGraph.m_pNodes[i].m_vecOrigin);

		if (WorldGraph.m_pNodes[i].m_afNodeInfo & bits_NODE_AIR)
		{
			// do nothing
			//MODDD - actually, do a POINT_CONTENTS check.  If this is stuck in the map,
			// it's not worth considering.  Remove its afNodeInfo.
			if(thisNodeContents == CONTENTS_SOLID){
				easyPrintLine("NodeType WARNING! Node #%d (air node), was stuck in the map!  NodeInfo stripped.", i);
				WorldGraph.m_pNodes[i].m_afNodeInfo = 0;
			}

		}
		else if (thisNodeContents == CONTENTS_WATER)
		{
			// Clearly to have point contents in the WATER, this is not stuck in the map.
			WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_WATER;
		}
		else
		{
			//MODDD - don't be so quick to assume this is even a valid LAND node!
			// That will come from either snapping to the ground or being so far high up
			// that it reaches the max '384' distance in the trace (retail went ahead and
			// called that a 'land' node?).
			//WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_LAND;


			// trace to the ground, then pop up 8 units and place node there to make it
			// easier for them to connect (think stairs, chairs, and bumps in the floor).
			// After the routing is done, push them back down.
			//
			TraceResult	tr;

			UTIL_TraceLine(WorldGraph.m_pNodes[i].m_vecOrigin,
				WorldGraph.m_pNodes[i].m_vecOrigin - Vector(0, 0, 384),
				ignore_monsters,
				g_pBodyQueueHead,//!!!HACKHACK no real ent to supply here, using a global we don't care about
				&tr);

			// This trace is ONLY used if we hit an entity flagged with FL_WORLDBRUSH
			TraceResult	trEnt;
			UTIL_TraceLine(WorldGraph.m_pNodes[i].m_vecOrigin,
				WorldGraph.m_pNodes[i].m_vecOrigin - Vector(0, 0, 384),
				dont_ignore_monsters,
				g_pBodyQueueHead,//!!!HACKHACK no real ent to supply here, using a global we don't care about
				&trEnt);


			// Did we hit something closer than the floor?
			if (trEnt.flFraction < tr.flFraction)
			{
				// If it was a world brush entity, copy the node location
				if (trEnt.pHit && (trEnt.pHit->v.flags & FL_WORLDBRUSH)){
					tr.vecEndPos = trEnt.vecEndPos;
					//MODDD - record the fraction too
					tr.flFraction = trEnt.flFraction;
				}
			}


			//MODDD - TODO?  Could there also be a check for rising out of the ground slightly to see if the
			// node was accidentally placed too deep, if it begins in solid space?  Unsure if that really ever
			// has a chance of happening, it would've failed to work with routing in retail anyway 
			// (mappers would have never done this).


			//MODDD - now another check.  Is the point reached from attempting to snap to the ground
			// even in valid space?  If it started in some odd place deep inside the map and only found
			// more map 384 below, it shouldn't have any nodeinfo.
			// But only bother if tr.flFraction is 1.  Ending any earlier is only possible from hitting
			// something it snapped to (found ground; valid).
			// Will consider ending mid-air also valid (assume this at a fraction of 1 that is still in
			// mid-air).
			if(tr.flFraction >= 1.0){
				int newContents = UTIL_PointContents(tr.vecEndPos);
				if(newContents != CONTENTS_SOLID){
					// in some space?  We'll count it then.
					// EHHHhhhhh call it mid-air.  It really is then, but give a warning, how could
					// this be intentional.
					// Note that nodes made 'air' from failing to snap to the ground don't count for
					// 'map_anyAirNodes', likely far too few to justify that.
					// Although loading a map would see that these are AIR nodes and still set map_anyAirNodes.
					// NEVERMIND.  Remove all nodeinfo here too, this clearly was a mistake.
					//WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_LAND;
					//WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_AIR;
					WorldGraph.m_pNodes[i].m_afNodeInfo = 0;

					if(thisNodeContents == CONTENTS_SOLID){
						// started at solid
						easyPrintLine("NodeType WARNING! Node #%d (ground node), began stuck in the map and lowered into valid space, but still could not snap to ground. NodeInfo stripped.", i);  // NodeInfo changed to 'air'."
					}else{
						easyPrintLine("NodeType WARNING! Node #%d (ground node) too high, could not snap to ground. NodeInfo stripped.", i);
					}

				}else{
					// Couldn't break out of solid geometry?  Nope.
					easyPrintLine("NodeType WARNING! Node #%d (ground node), started stuck in the map and could not lower out of it!  NodeInfo stripped.", i);
					WorldGraph.m_pNodes[i].m_afNodeInfo = 0;
				}
			}else{
				// Hit something?  This is indeed on the ground.
				WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_LAND;
				//MODDD - don't add node_linktest_height anymore. It's going to just get subtracted anyways.
				// Checking to see what is land based or not later is perfectly fine.  This height change gets reverted anyways.
				WorldGraph.m_pNodes[i].m_vecOriginPeek.z = WorldGraph.m_pNodes[i].m_vecOrigin.z = tr.vecEndPos.z;// + node_linktest_height;
			}
			
		}//node type determining

	}//for loop through nodes

	cPoolLinks = WorldGraph.LinkVisibleNodes(pTempPool, file, &iBadNode);

	if (!cPoolLinks)
	{
		ALERT(at_aiconsole, "**ConnectVisibleNodes FAILED!\n");

		SetThink(&CTestHull::ShowBadNode);// send the hull off to show the offending node.
		//pev->solid = SOLID_NOT;
		pev->origin = WorldGraph.m_pNodes[iBadNode].m_vecOrigin;

		if (pTempPool)
		{
			free(pTempPool);
		}

		if (file)
		{// close the file
			fclose(file);
		}

		return;
	}

	// send the walkhull to all of this node's connections now. We'll do this here since
	// so much of it relies on being able to control the test hull.
	fprintf(file, "----------------------------------------------------------------------------\n");
	fprintf(file, "Walk Rejection:\n");

	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		pSrcNode = &WorldGraph.m_pNodes[i];

		fprintf(file, "-------------------------------------------------------------------------------\n");
		fprintf(file, "Node %4d:\n\n", i);

		for (j = 0; j < pSrcNode->m_cNumLinks; j++)
		{
			// assume that all hulls can walk this link, then eliminate the ones that can't.
			pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo = bits_LINK_SMALL_HULL | bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL | bits_LINK_FLY_HULL;


			// do a check for each hull size.

			// if we can't fit a tiny hull through a connection, no other hulls with fit either, so we 
			// should just fall out of the loop. Do so by setting the SkipRemainingHulls flag.
			fSkipRemainingHulls = FALSE;
			for (hull = 0; hull < MAX_NODE_HULLS; hull++)
			{
				//MODDD NOTE - this skip may look a little odd since each check removes hull-passable flags, but
				//             it makes sense. The smallest (earliest) hull check that fails already removes all
				//             larger flags, making the bigger checks and even flag removals unnecessary.
				if (fSkipRemainingHulls && (hull == NODE_HUMAN_HULL || hull == NODE_LARGE_HULL)) // skip the remaining walk hulls
					continue;

				switch (hull)
				{
				case NODE_SMALL_HULL:
					UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
					break;
				case NODE_HUMAN_HULL:
					UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
					break;
				case NODE_LARGE_HULL:
					UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));
					break;
				case NODE_FLY_HULL:
					UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));
					// UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
					break;
				}



				//MODDD - new pDestNode assignment location.
				pDestNode = &WorldGraph.m_pNodes[pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode];



				if (node_hulltest_heightswap != TRUE) {
					//RETAIL BEHAVIOR - no swaps.
					pHullTest_SrcNode = pSrcNode;
					pHullTest_DestNode = pDestNode;
				}
				else {

					if (pSrcNode->m_vecOrigin.z >= pDestNode->m_vecOrigin.z) {
						//Swap to keep the start point of the WALK_MOVE check the lower of the two points.
						pHullTest_SrcNode = pDestNode;
						pHullTest_DestNode = pSrcNode;
					}
					else {
						//leave the way it is.
						pHullTest_SrcNode = pSrcNode;
						pHullTest_DestNode = pDestNode;
					}
				}

				if (pHullTest_SrcNode->m_afNodeInfo & bits_NODE_LAND) {
					hulltestExtraVertical = node_hulltest_height;
				}
				else {
					hulltestExtraVertical = 0;
				}

				//MODDD - can be moved up a little too.
				//UTIL_SetOrigin ( pev, pHullTest_SrcNode->m_vecOrigin );// place the hull on the node
				UTIL_SetOrigin(pev, Vector(pHullTest_SrcNode->m_vecOrigin.x, pHullTest_SrcNode->m_vecOrigin.y, pHullTest_SrcNode->m_vecOrigin.z + hulltestExtraVertical));// place the hull on the node

				if (!FBitSet(pev->flags, FL_ONGROUND))
				{
					ALERT(at_aiconsole, "OFFGROUND!\n");
				}

				// now build a yaw that points to the dest node, and get the distance.
				if (j < 0)
				{
					ALERT(at_aiconsole, "**** j = %d ****\n", j);
					if (pTempPool)
					{
						free(pTempPool);
					}

					if (file)
					{// close the file
						fclose(file);
					}
					return;
				}

				//MODDD - old pDestNode assignment location

				if (pHullTest_DestNode->m_afNodeInfo & bits_NODE_LAND) {
					hulltestExtraVertical = node_hulltest_height;
				}
				else {
					hulltestExtraVertical = 0;
				}

				//MODDD - can be moved up a little too.
				vecSpot = Vector(pHullTest_DestNode->m_vecOrigin.x, pHullTest_DestNode->m_vecOrigin.y, pHullTest_DestNode->m_vecOrigin.z + hulltestExtraVertical);
				//vecSpot.z = pev->origin.z;

				if (hull < NODE_FLY_HULL)
				{
					int SaveFlags = pev->flags;
					int MoveMode = WALKMOVE_WORLDONLY;
					if (pHullTest_SrcNode->m_afNodeInfo & bits_NODE_WATER)
					{
						pev->flags |= FL_SWIM;
						MoveMode = WALKMOVE_NORMAL;
					}

					flYaw = UTIL_VecToYaw(pHullTest_DestNode->m_vecOrigin - pev->origin);

					flDist = (vecSpot - pev->origin).Length2D();

					int fWalkFailed = FALSE;

					// in this loop we take tiny steps from the current node to the nodes that it links to, one at a time.
					// pev->angles.y = flYaw;
					for (step = 0; step < flDist && !fWalkFailed; step += HULL_STEP_SIZE)
					{
						float stepSize = HULL_STEP_SIZE;

						if ((step + stepSize) >= (flDist - 1))
							stepSize = (flDist - step) - 1;

						if (!WALK_MOVE(ENT(pev), flYaw, stepSize, MoveMode))
						{// can't take the next step

							fWalkFailed = TRUE;
							break;
						}
					}

					/*
					if(i == 18 || j == 18 || i == 30 || j == 30){
						//Ayy yo.
						int x = 4;
					}
					*/

					/*
					if( (pTempPool[ pSrcNode->m_iFirstLink + j ].m_iDestNode == 18 || pTempPool[ pSrcNode->m_iFirstLink + j ].m_iDestNode == 25) &&
						(pTempPool[ pSrcNode->m_iFirstLink + j ].m_iSrcNode == 18 || pTempPool[ pSrcNode->m_iFirstLink + j ].m_iSrcNode == 25) &&
						(i == 18 || i == 25) ){
							int x = 44;
							//OH SHIT SON
							fWalkFailed = FALSE;
					}
					*/


					//MODDD NOTE - ...  okay. what is this unfinished comment supposed to say?
					//All I see is the monster's origin is further away than 64 from the goal (vecSpot).
					//Yea this never happens it seems anyways.

					if (!fWalkFailed && (pev->origin - vecSpot).Length() > 64)
					{
						// ALERT( at_console, "bogus walk\n");
						// we thought we 
						fWalkFailed = TRUE;
					}

					//WARNING - HORRIBLY HACKY
					//fWalkFailed = FALSE;

					if (fWalkFailed)
					{
						//pTempPool[ pSrcNode->m_iFirstLink + j ] = pTempPool [ pSrcNode->m_iFirstLink + ( pSrcNode->m_cNumLinks - 1 ) ];

						// now me must eliminate the hull that couldn't walk this connection
						// MODDD - printing ints (step) with formatting symbol '%f', eh?
						// That's a paddlin'.
						switch (hull)
						{
						case NODE_SMALL_HULL:	// if this hull can't fit, nothing can, so drop the connection
							fprintf(file, "NODE_SMALL_HULL step %d\n", step);
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~(bits_LINK_SMALL_HULL | bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL);
							fSkipRemainingHulls = TRUE;// don't bother checking larger hulls
							break;
						case NODE_HUMAN_HULL:
							fprintf(file, "NODE_HUMAN_HULL step %d\n", step);
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~(bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL);
							fSkipRemainingHulls = TRUE;// don't bother checking larger hulls
							break;
						case NODE_LARGE_HULL:
							fprintf(file, "NODE_LARGE_HULL step %d\n", step);
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~bits_LINK_LARGE_HULL;
							break;
						}
					}
					else {
						//break point. did anything... not fail?
						int x = 4;
					}
					pev->flags = SaveFlags;
				}
				else
				{
					//MODDD - CHECK. Is this accurate?  Or will it lead to sending paths that break when tried the moment they begin?
					TraceResult tr;

					UTIL_TraceHull(pHullTest_SrcNode->m_vecOrigin + Vector(0, 0, 32), pHullTest_DestNode->m_vecOriginPeek + Vector(0, 0, 32), ignore_monsters, large_hull, ENT(pev), &tr);
					if (tr.fStartSolid || tr.flFraction < 1.0)
					{
						pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~bits_LINK_FLY_HULL;
					}
				}
			}

			if (pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo == 0)
			{
				fprintf(file, "Rejected Node %3d - Unreachable by ", pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode);
				pTempPool[pSrcNode->m_iFirstLink + j] = pTempPool[pSrcNode->m_iFirstLink + (pSrcNode->m_cNumLinks - 1)];
				fprintf(file, "Any Hull\n");

				pSrcNode->m_cNumLinks--;
				cPoolLinks--;// we just removed a link, so decrement the total number of links in the pool.
				j--;
			}

		}
	}
	fprintf(file, "-------------------------------------------------------------------------------\n\n\n");

	cPoolLinks -= WorldGraph.RejectInlineLinks(pTempPool, file);

	// now malloc a pool just large enough to hold the links that are actually used
	WorldGraph.m_pLinkPool = (CLink*)calloc(sizeof(CLink), cPoolLinks);

	if (!WorldGraph.m_pLinkPool)
	{// couldn't make the link pool!
		ALERT(at_aiconsole, "Couldn't malloc LinkPool!\n");
		if (pTempPool)
		{
			free(pTempPool);
		}
		if (file)
		{// close the file
			fclose(file);
		}

		return;
	}
	WorldGraph.m_cLinks = cPoolLinks;

	//copy only the used portions of the TempPool into the graph's link pool
	int iFinalPoolIndex = 0;
	int iOldFirstLink;

	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		iOldFirstLink = WorldGraph.m_pNodes[i].m_iFirstLink;// store this, because we have to re-assign it before entering the copy loop

		WorldGraph.m_pNodes[i].m_iFirstLink = iFinalPoolIndex;

		for (j = 0; j < WorldGraph.m_pNodes[i].m_cNumLinks; j++)
		{
			WorldGraph.m_pLinkPool[iFinalPoolIndex++] = pTempPool[iOldFirstLink + j];
		}
	}

	// Node sorting numbers linked nodes close to each other
	//
	WorldGraph.SortNodes();

	// This is used for HashSearch
	//
	WorldGraph.BuildLinkLookups();

	fPairsValid = TRUE; // assume that the connection pairs are all valid to start

	fprintf(file, "\n\n-------------------------------------------------------------------------------\n");
	fprintf(file, "Link Pairings:\n");

	// link integrity check. The idea here is that if Node A links to Node B, node B should
	// link to node A. If not, we have a situation that prevents us from using a basic 
	// optimization in the FindNearestLink function. 
	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		for (j = 0; j < WorldGraph.m_pNodes[i].m_cNumLinks; j++)
		{
			int iLink;
			WorldGraph.HashSearch(WorldGraph.INodeLink(i, j), i, iLink);
			if (iLink < 0)
			{
				fPairsValid = FALSE;// unmatched link pair.
				fprintf(file, "WARNING: Node %3d does not connect back to Node %3d\n", WorldGraph.INodeLink(i, j), i);
			}
		}
	}

	// !!!LATER - if all connections are properly paired, when can enable an optimization in the pathfinding code
	// (in the find nearest line function)
	if (fPairsValid)
	{
		fprintf(file, "\nAll Connections are Paired!\n");
	}

	fprintf(file, "-------------------------------------------------------------------------------\n");
	fprintf(file, "\n\n-------------------------------------------------------------------------------\n");
	fprintf(file, "Total Number of Connections in Pool: %d\n", cPoolLinks);
	fprintf(file, "-------------------------------------------------------------------------------\n");
	//MODDD - changed %d to %u because of a warning, got an unsigned from the 'sizeof'
	// but expected a signed from the %d.
	fprintf(file, "Connection Pool: %u bytes\n", sizeof(CLink) * cPoolLinks);
	fprintf(file, "-------------------------------------------------------------------------------\n");


	ALERT(at_aiconsole, "%d Nodes, %d Connections\n", WorldGraph.m_cNodes, cPoolLinks);

	// This is used for FindNearestNode
	//
	WorldGraph.BuildRegionTables();


	// Push all of the LAND nodes down to the ground now. Leave the water and air nodes alone.
	//
	//MODDD - no longer necessary, this offset is not applied to begin with anymore.
	/*
	for ( i = 0 ; i < WorldGraph.m_cNodes ; i++ )
	{
		if ((WorldGraph.m_pNodes[ i ].m_afNodeInfo & bits_NODE_LAND))
		{
			WorldGraph.m_pNodes[ i ].m_vecOrigin.z -= node_linktest_height;
		}
	}
	*/

	if (pTempPool)
	{// free the temp pool
		free(pTempPool);
	}

	if (file)
	{
		fclose(file);
	}

	// We now have some graphing capabilities.
	//
	WorldGraph.m_fGraphPresent = TRUE;//graph is in memory.
	WorldGraph.m_fGraphPointersSet = TRUE;// since the graph was generated, the pointers are ready
	WorldGraph.m_fRoutingComplete = FALSE; // Optimal routes aren't computed, yet.

	// Compute and compress the routing information.
	//
	WorldGraph.ComputeStaticRoutingTables();

	// save the node graph for this level	
	WorldGraph.FSaveGraph((char*)STRING(gpGlobals->mapname));
	ALERT(at_console, "Done.\n");
}


//=========================================================
// returns a hardcoded path.
//=========================================================
//MODDD NOTE - this method is never called.  Well ok then.
void CTestHull::PathFind(void)
{
	int iPath[50];
	int iPathSize;
	int i;
	CNode* pNode, * pNextNode;

	if (!WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available
		ALERT(at_aiconsole, "Graph not ready!\n");
		return;
	}

	iPathSize = WorldGraph.FindShortestPath(iPath, 0, 19, 0, 0); // UNDONE use hull constant

	if (!iPathSize)
	{
		ALERT(at_aiconsole, "No Path!\n");
		return;
	}

	ALERT(at_aiconsole, "%d\n", iPathSize);

	pNode = &WorldGraph.m_pNodes[iPath[0]];

	for (i = 0; i < iPathSize - 1; i++)
	{

		pNextNode = &WorldGraph.m_pNodes[iPath[i + 1]];

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SHOWLINE);

		WRITE_COORD(pNode->m_vecOrigin.x);
		WRITE_COORD(pNode->m_vecOrigin.y);
		WRITE_COORD(pNode->m_vecOrigin.z + node_linktest_height);

		WRITE_COORD(pNextNode->m_vecOrigin.x);
		WRITE_COORD(pNextNode->m_vecOrigin.y);
		WRITE_COORD(pNextNode->m_vecOrigin.z + node_linktest_height);
		MESSAGE_END();

		pNode = pNextNode;
	}

}


//=========================================================
// CStack Constructor
//=========================================================
CStack::CStack(void)
{
	m_level = 0;
}

//=========================================================
// pushes a value onto the stack
//=========================================================
void CStack::Push(int value)
{
	if (m_level >= MAX_STACK_NODES)
	{
		printf("Error!\n");
		return;
	}
	m_stack[m_level] = value;
	m_level++;
}

//=========================================================
// pops a value off of the stack
//=========================================================
int CStack::Pop(void)
{
	if (m_level <= 0)
		return -1;

	m_level--;
	return m_stack[m_level];
}

//=========================================================
// returns the value on the top of the stack
//=========================================================
int CStack::Top(void)
{
	return m_stack[m_level - 1];
}

//=========================================================
// copies every element on the stack into an array LIFO 
//=========================================================
void CStack::CopyToArray(int* piArray)
{
	int i;

	for (i = 0; i < m_level; i++)
	{
		piArray[i] = m_stack[i];
	}
}

//=========================================================
// CQueue constructor
//=========================================================
CQueue::CQueue(void)
{
	m_cSize = 0;
	m_head = 0;
	m_tail = -1;
}

//=========================================================
// inserts a value into the queue
//=========================================================
void CQueue::Insert(int iValue, float fPriority)
{

	if (Full())
	{
		printf("Queue is full!\n");
		return;
	}

	m_tail++;

	if (m_tail == MAX_STACK_NODES)
	{//wrap around
		m_tail = 0;
	}

	m_queue[m_tail].Id = iValue;
	m_queue[m_tail].Priority = fPriority;
	m_cSize++;
}

//=========================================================
// removes a value from the queue (FIFO)
//=========================================================
int CQueue::Remove(float& fPriority)
{
	if (m_head == MAX_STACK_NODES)
	{// wrap
		m_head = 0;
	}

	m_cSize--;
	fPriority = m_queue[m_head].Priority;
	return m_queue[m_head++].Id;
}

//=========================================================
// CQueue constructor
//=========================================================
CQueuePriority::CQueuePriority(void)
{
	m_cSize = 0;
}

//=========================================================
// inserts a value into the priority queue
//=========================================================
void CQueuePriority::Insert(int iValue, float fPriority)
{

	if (Full())
	{
		printf("Queue is full!\n");
		return;
	}

	m_heap[m_cSize].Priority = fPriority;
	m_heap[m_cSize].Id = iValue;
	m_cSize++;
	Heap_SiftUp();
}

//=========================================================
// removes the smallest item from the priority queue
//
//=========================================================
int CQueuePriority::Remove(float& fPriority)
{
	int iReturn = m_heap[0].Id;
	fPriority = m_heap[0].Priority;

	m_cSize--;

	m_heap[0] = m_heap[m_cSize];

	Heap_SiftDown(0);
	return iReturn;
}

#define HEAP_LEFT_CHILD(x) (2*(x)+1)
#define HEAP_RIGHT_CHILD(x) (2*(x)+2)
#define HEAP_PARENT(x) (((x)-1)/2)

void CQueuePriority::Heap_SiftDown(int iSubRoot)
{
	int parent = iSubRoot;
	int child = HEAP_LEFT_CHILD(parent);

	struct tag_HEAP_NODE Ref = m_heap[parent];

	while (child < m_cSize)
	{
		int rightchild = HEAP_RIGHT_CHILD(parent);
		if (rightchild < m_cSize)
		{
			if (m_heap[rightchild].Priority < m_heap[child].Priority)
			{
				child = rightchild;
			}
		}
		if (Ref.Priority <= m_heap[child].Priority)
			break;

		m_heap[parent] = m_heap[child];
		parent = child;
		child = HEAP_LEFT_CHILD(parent);
	}
	m_heap[parent] = Ref;
}

void CQueuePriority::Heap_SiftUp(void)
{
	int child = m_cSize - 1;
	while (child)
	{
		int parent = HEAP_PARENT(child);
		if (m_heap[parent].Priority <= m_heap[child].Priority)
			break;

		struct tag_HEAP_NODE Tmp;
		Tmp = m_heap[child];
		m_heap[child] = m_heap[parent];
		m_heap[parent] = Tmp;

		child = parent;
	}
}

//=========================================================
// CGraph - FLoadGraph - attempts to load a node graph from disk.
// if the current level is maps/snar.bsp, maps/graphs/snar.nod
// will be loaded. If file cannot be loaded, the node tree
// will be created and saved to disk.
//=========================================================
int CGraph::FLoadGraph(char* szMapName)
{
	char	szFilename[MAX_PATH];
	int	iVersion;
	int     length;
	byte* aMemFile;
	byte* pMemFile;

	// make sure the directories have been made
	char	szDirName[MAX_PATH];
	GET_GAME_DIR(szDirName);
	strcat(szDirName, "/maps");
	CreateDirectory(szDirName, NULL);
	strcat(szDirName, "/graphs");
	CreateDirectory(szDirName, NULL);

	strcpy(szFilename, "maps/graphs/");
	strcat(szFilename, szMapName);
	strcat(szFilename, ".nod");

	pMemFile = aMemFile = LOAD_FILE_FOR_ME(szFilename, &length);

	if (!aMemFile)
	{
		return FALSE;
	}
	else
	{
		int i;

		// Read the graph version number
		//
		length -= sizeof(int);
		if (length < 0) goto ShortFile;
		memcpy(&iVersion, pMemFile, sizeof(int));
		pMemFile += sizeof(int);

		if (iVersion != GRAPH_VERSION)
		{
			// This file was written by a different build of the dll!
			//
			ALERT(at_aiconsole, "**ERROR** Graph version is %d, expected %d\n", iVersion, GRAPH_VERSION);
			goto ShortFile;
		}

		// Read the graph class
		//
		length -= sizeof(CGraph);
		if (length < 0) goto ShortFile;
		memcpy(this, pMemFile, sizeof(CGraph));
		pMemFile += sizeof(CGraph);

		// Set the pointers to zero, just in case we run out of memory.
		//
		m_pNodes = NULL;
		m_pLinkPool = NULL;
		m_di = NULL;
		m_pRouteInfo = NULL;
		m_pHashLinks = NULL;


		// Malloc for the nodes
		//
		m_pNodes = (CNode*)calloc(sizeof(CNode), m_cNodes);

		if (!m_pNodes)
		{
			ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d nodes!\n", m_cNodes);
			goto NoMemory;
		}

		// Read in all the nodes
		//
		length -= sizeof(CNode) * m_cNodes;
		if (length < 0) goto ShortFile;
		memcpy(m_pNodes, pMemFile, sizeof(CNode) * m_cNodes);
		pMemFile += sizeof(CNode) * m_cNodes;


		// Malloc for the link pool
		//
		m_pLinkPool = (CLink*)calloc(sizeof(CLink), m_cLinks);

		if (!m_pLinkPool)
		{
			ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d link!\n", m_cLinks);
			goto NoMemory;
		}

		// Read in all the links
		//
		length -= sizeof(CLink) * m_cLinks;
		if (length < 0) goto ShortFile;
		memcpy(m_pLinkPool, pMemFile, sizeof(CLink) * m_cLinks);
		pMemFile += sizeof(CLink) * m_cLinks;

		// Malloc for the sorting info.
		//
		m_di = (DIST_INFO*)calloc(sizeof(DIST_INFO), m_cNodes);
		if (!m_di)
		{
			ALERT(at_aiconsole, "***ERROR**\nCouldn't malloc %d entries sorting nodes!\n", m_cNodes);
			goto NoMemory;
		}

		// Read it in.
		//
		length -= sizeof(DIST_INFO) * m_cNodes;
		if (length < 0) goto ShortFile;
		memcpy(m_di, pMemFile, sizeof(DIST_INFO) * m_cNodes);
		pMemFile += sizeof(DIST_INFO) * m_cNodes;

		// Malloc for the routing info.
		//
		m_fRoutingComplete = FALSE;
		m_pRouteInfo = (char*)calloc(sizeof(char), m_nRouteInfo);
		if (!m_pRouteInfo)
		{
			ALERT(at_aiconsole, "***ERROR**\nCounldn't malloc %d route bytes!\n", m_nRouteInfo);
			goto NoMemory;
		}
		m_CheckedCounter = 0;
		for (i = 0; i < m_cNodes; i++)
		{
			m_di[i].m_CheckedEvent = 0;
			
			//MODDD - record if at least one node is an AIR node.
			if (m_pNodes[i].m_afNodeInfo & bits_NODE_AIR) {
				//found one? we're good.
				map_anyAirNodes = TRUE;
				break;
			}
		}

		// Read in the route information.
		//
		length -= sizeof(char) * m_nRouteInfo;
		if (length < 0) goto ShortFile;
		memcpy(m_pRouteInfo, pMemFile, sizeof(char) * m_nRouteInfo);
		pMemFile += sizeof(char) * m_nRouteInfo;
		m_fRoutingComplete = TRUE;

		// malloc for the hash links
		//
		m_pHashLinks = (short*)calloc(sizeof(short), m_nHashLinks);
		if (!m_pHashLinks)
		{
			ALERT(at_aiconsole, "***ERROR**\nCounldn't malloc %d hash link bytes!\n", m_nHashLinks);
			goto NoMemory;
		}

		// Read in the hash link information
		//
		length -= sizeof(short) * m_nHashLinks;
		if (length < 0) goto ShortFile;
		memcpy(m_pHashLinks, pMemFile, sizeof(short) * m_nHashLinks);
		pMemFile += sizeof(short) * m_nHashLinks;

		// Set the graph present flag, clear the pointers set flag
		//
		m_fGraphPresent = TRUE;
		m_fGraphPointersSet = FALSE;


		FREE_FILE(aMemFile);

		if (length != 0)
		{
			ALERT(at_aiconsole, "***WARNING***:Node graph was longer than expected by %d bytes.!\n", length);
		}

		return TRUE;
	}

ShortFile:
NoMemory:
	FREE_FILE(aMemFile);
	return FALSE;
}

//=========================================================
// CGraph - FSaveGraph - It's not rocket science.
// this WILL overwrite existing files.
//=========================================================
int CGraph::FSaveGraph(char* szMapName)
{

	int	iVersion = GRAPH_VERSION;
	char	szFilename[MAX_PATH];
	FILE* file;

	if (!m_fGraphPresent || !m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available or built
		ALERT(at_aiconsole, "Graph not ready!\n");
		return FALSE;
	}

	// make sure directories have been made
	GET_GAME_DIR(szFilename);
	strcat(szFilename, "/maps");
	CreateDirectory(szFilename, NULL);
	strcat(szFilename, "/graphs");
	CreateDirectory(szFilename, NULL);

	strcat(szFilename, "/");
	strcat(szFilename, szMapName);
	strcat(szFilename, ".nod");

	file = fopen(szFilename, "wb");

	ALERT(at_aiconsole, "Created: %s\n", szFilename);

	if (!file)
	{// couldn't create
		ALERT(at_aiconsole, "Couldn't Create: %s\n", szFilename);
		return FALSE;
	}
	else
	{
		// write the version
		fwrite(&iVersion, sizeof(int), 1, file);

		// write the CGraph class
		fwrite(this, sizeof(CGraph), 1, file);

		// write the nodes
		fwrite(m_pNodes, sizeof(CNode), m_cNodes, file);

		// write the links
		fwrite(m_pLinkPool, sizeof(CLink), m_cLinks, file);

		fwrite(m_di, sizeof(DIST_INFO), m_cNodes, file);

		// Write the route info.
		//
		if (m_pRouteInfo && m_nRouteInfo)
		{
			fwrite(m_pRouteInfo, sizeof(char), m_nRouteInfo, file);
		}

		if (m_pHashLinks && m_nHashLinks)
		{
			fwrite(m_pHashLinks, sizeof(short), m_nHashLinks, file);
		}
		fclose(file);
		return TRUE;
	}
}

//=========================================================
// CGraph - FSetGraphPointers - Takes the modelnames of 
// all of the brush ents that block connections in the node
// graph and resolves them into pointers to those entities.
// this is done after loading the graph from disk, whereupon
// the pointers are not valid.
//=========================================================
int CGraph::FSetGraphPointers(void)
{
	int i;
	edict_t* pentLinkEnt;

	for (i = 0; i < m_cLinks; i++)
	{// go through all of the links

		if (m_pLinkPool[i].m_pLinkEnt != NULL)
		{
			char name[5];
			// when graphs are saved, any valid pointers are will be non-zero, signifying that we should
			// reset those pointers upon reloading. Any pointers that were NULL when the graph was saved
			// will be NULL when reloaded, and will ignored by this function.

			// m_szLinkEntModelname is not necessarily NULL terminated (so we can store it in a more alignment-friendly 4 bytes)
			memcpy(name, m_pLinkPool[i].m_szLinkEntModelname, 4);
			name[4] = 0;
			pentLinkEnt = FIND_ENTITY_BY_STRING(NULL, "model", name);

			if (FNullEnt(pentLinkEnt))
			{
				// the ent isn't around anymore? Either there is a major problem, or it was removed from the world
				// ( like a func_breakable that's been destroyed or something ). Make sure that LinkEnt is null.
				ALERT(at_aiconsole, "**Could not find model %s\n", name);
				m_pLinkPool[i].m_pLinkEnt = NULL;
			}
			else
			{
				m_pLinkPool[i].m_pLinkEnt = VARS(pentLinkEnt);

				if (!FBitSet(m_pLinkPool[i].m_pLinkEnt->flags, FL_GRAPHED))
				{
					m_pLinkPool[i].m_pLinkEnt->flags += FL_GRAPHED;
				}
			}
		}
	}

	// the pointers are now set.
	m_fGraphPointersSet = TRUE;
	return TRUE;
}

//=========================================================
// CGraph - CheckNODFile - this function checks the date of 
// the BSP file that was just loaded and the date of the a
// ssociated .NOD file. If the NOD file is not present, or 
// is older than the BSP file, we rebuild it.
//
// returns FALSE if the .NOD file doesn't qualify and needs
// to be rebuilt.
//
// !!!BUGBUG - the file times we get back are 20 hours ahead!
// since this happens consistantly, we can still correctly 
// determine which of the 2 files is newer. This needs fixed,
// though. ( I now suspect that we are getting GMT back from
// these functions and must compensate for local time ) (sjb)
//=========================================================
int CGraph::CheckNODFile(char* szMapName)
{
	int		retValue;

	char		szBspFilename[MAX_PATH];
	char		szGraphFilename[MAX_PATH];


	strcpy(szBspFilename, "maps/");
	strcat(szBspFilename, szMapName);
	strcat(szBspFilename, ".bsp");

	strcpy(szGraphFilename, "maps/graphs/");
	strcat(szGraphFilename, szMapName);
	strcat(szGraphFilename, ".nod");

	retValue = TRUE;

	int iCompare;
	if (COMPARE_FILE_TIME(szBspFilename, szGraphFilename, &iCompare))
	{
		if (iCompare > 0)
		{// BSP file is newer.
			ALERT(at_aiconsole, ".NOD File will be updated\n\n");
			retValue = FALSE;
		}
	}
	else
	{
		retValue = FALSE;
	}

	return retValue;
}




void CGraph::HashInsert(int iSrcNode, int iDestNode, int iKey)
{
	struct tagNodePair np;

	np.iSrc = iSrcNode;
	np.iDest = iDestNode;
	CRC32_t dwHash;
	CRC32_INIT(&dwHash);
	CRC32_PROCESS_BUFFER(&dwHash, &np, sizeof(np));
	dwHash = CRC32_FINAL(dwHash);

	int di = m_HashPrimes[dwHash & 15];
	int i = (dwHash >> 4) % m_nHashLinks;
	while (m_pHashLinks[i] != ENTRY_STATE_EMPTY)
	{
		i += di;
		if (i >= m_nHashLinks) i -= m_nHashLinks;
	}
	m_pHashLinks[i] = iKey;
}

void CGraph::HashSearch(int iSrcNode, int iDestNode, int& iKey)
{
	struct tagNodePair np;

	np.iSrc = iSrcNode;
	np.iDest = iDestNode;
	CRC32_t dwHash;
	CRC32_INIT(&dwHash);
	CRC32_PROCESS_BUFFER(&dwHash, &np, sizeof(np));
	dwHash = CRC32_FINAL(dwHash);

	int di = m_HashPrimes[dwHash & 15];
	int i = (dwHash >> 4) % m_nHashLinks;
	while (m_pHashLinks[i] != ENTRY_STATE_EMPTY)
	{
		CLink& link = Link(m_pHashLinks[i]);
		if (iSrcNode == link.m_iSrcNode && iDestNode == link.m_iDestNode)
		{
			break;
		}
		else
		{
			i += di;
			if (i >= m_nHashLinks) i -= m_nHashLinks;
		}
	}
	iKey = m_pHashLinks[i];
}


void CGraph::HashChoosePrimes(int TableSize)
{
	int LargestPrime = TableSize / 2;
	if (LargestPrime > Primes[NUMBER_OF_PRIMES - 2])
	{
		LargestPrime = Primes[NUMBER_OF_PRIMES - 2];
	}
	int Spacing = LargestPrime / 16;

	// Pick a set primes that are evenly spaced from (0 to LargestPrime)
	// We divide this interval into 16 equal sized zones. We want to find
	// one prime number that best represents that zone.
	//

	//MODDD - NOW DOES NOTHING EITHER WAY, left just to see the difference.
//#if COMPILE_FOR_VS_2008 == 1
	//int iPrime;
//#endif

	int iZone = 0;
	int iPrime = 0;

	//MODDD - NOTICE! the line below's first var, "iZone" used to have "int" in front, meaning, declare both.
	//Instead, we're going to rely on the vars declared ahead of time and just re-used / reset as needed throughout
	//the loops below, no re-declarations, just like C would have done (stops VS 06 from throwing a hissy fit)

	for (iZone = 1, iPrime = 0; iPrime < 16; iZone += Spacing)
	{
		// Search for a prime number that is less than the target zone
		// number given by iZone.
		//
		int Lower = Primes[0];
		for (int jPrime = 0; Primes[jPrime] != 0; jPrime++)
		{
			if (jPrime != 0 && TableSize % Primes[jPrime] == 0) continue;
			int Upper = Primes[jPrime];
			if (Lower <= iZone && iZone <= Upper)
			{
				// Choose the closest lower prime number.
				//
				if (iZone - Lower <= Upper - iZone)
				{
					m_HashPrimes[iPrime++] = Lower;
				}
				else
				{
					m_HashPrimes[iPrime++] = Upper;
				}
				break;
			}
			Lower = Upper;
		}
	}

	// Alternate negative and positive numbers
	//

	for (iPrime = 0; iPrime < 16; iPrime += 2)
	{
		m_HashPrimes[iPrime] = TableSize - m_HashPrimes[iPrime];
	}

	// Shuffle the set of primes to reduce correlation with bits in
	// hash key.
	//

	for (iPrime = 0; iPrime < 16 - 1; iPrime++)
	{
		int Pick = RANDOM_LONG(0, 15 - iPrime);
		int Temp = m_HashPrimes[Pick];
		m_HashPrimes[Pick] = m_HashPrimes[15 - iPrime];
		m_HashPrimes[15 - iPrime] = Temp;
	}
}

// Renumber nodes so that nodes that link together are together.
//
#define UNNUMBERED_NODE -1
void CGraph::SortNodes(void)
{
	// We are using m_iPreviousNode to be the new node number.
	// After assigning new node numbers to everything, we move
	// things and patchup the links.
	//
	int iNodeCnt = 0;
	m_pNodes[0].m_iPreviousNode = iNodeCnt++;
	int i;
	for (i = 1; i < m_cNodes; i++)
	{
		m_pNodes[i].m_iPreviousNode = UNNUMBERED_NODE;
	}

	for (i = 0; i < m_cNodes; i++)
	{
		// Run through all of this node's neighbors
		//
		for (int j = 0; j < m_pNodes[i].m_cNumLinks; j++)
		{
			int iDestNode = INodeLink(i, j);
			if (m_pNodes[iDestNode].m_iPreviousNode == UNNUMBERED_NODE)
			{
				m_pNodes[iDestNode].m_iPreviousNode = iNodeCnt++;
			}
		}
	}

	// Assign remaining node numbers to unlinked nodes.
	//
	for (i = 0; i < m_cNodes; i++)
	{
		if (m_pNodes[i].m_iPreviousNode == UNNUMBERED_NODE)
		{
			m_pNodes[i].m_iPreviousNode = iNodeCnt++;
		}
	}

	// Alter links to reflect new node numbers.
	//
	for (i = 0; i < m_cLinks; i++)
	{
		m_pLinkPool[i].m_iSrcNode = m_pNodes[m_pLinkPool[i].m_iSrcNode].m_iPreviousNode;
		m_pLinkPool[i].m_iDestNode = m_pNodes[m_pLinkPool[i].m_iDestNode].m_iPreviousNode;
	}

	// Rearrange nodes to reflect new node numbering.
	//
	for (i = 0; i < m_cNodes; i++)
	{
		while (m_pNodes[i].m_iPreviousNode != i)
		{
			// Move current node off to where it should be, and bring
			// that other node back into the current slot.
			//
			int iDestNode = m_pNodes[i].m_iPreviousNode;
			CNode TempNode = m_pNodes[iDestNode];
			m_pNodes[iDestNode] = m_pNodes[i];
			m_pNodes[i] = TempNode;
		}
	}
}

void CGraph::BuildLinkLookups(void)
{
	m_nHashLinks = 3 * m_cLinks / 2 + 3;

	HashChoosePrimes(m_nHashLinks);
	m_pHashLinks = (short*)calloc(sizeof(short), m_nHashLinks);
	if (!m_pHashLinks)
	{
		ALERT(at_aiconsole, "Couldn't allocated Link Lookup Table.\n");
		return;
	}
	int i;
	for (i = 0; i < m_nHashLinks; i++)
	{
		m_pHashLinks[i] = ENTRY_STATE_EMPTY;
	}

	for (i = 0; i < m_cLinks; i++)
	{
		CLink& link = Link(i);
		HashInsert(link.m_iSrcNode, link.m_iDestNode, i);
	}
#if 0
	for (i = 0; i < m_cLinks; i++)
	{
		CLink& link = Link(i);
		int iKey;
		HashSearch(link.m_iSrcNode, link.m_iDestNode, iKey);
		if (iKey != i)
		{
			ALERT(at_aiconsole, "HashLinks don't match (%d versus %d)\n", i, iKey);
		}
	}
#endif
}

void CGraph::BuildRegionTables(void)
{
	if (m_di) free(m_di);

	// Go ahead and setup for range searching the nodes for FindNearestNodes
	//
	m_di = (DIST_INFO*)calloc(sizeof(DIST_INFO), m_cNodes);
	if (!m_di)
	{
		ALERT(at_aiconsole, "Couldn't allocated node ordering array.\n");
		return;
	}

	// Calculate regions for all the nodes.
	//
	//
	int i;
	for (i = 0; i < 3; i++)
	{
		m_RegionMin[i] = 999999999.0; // just a big number out there;
		m_RegionMax[i] = -999999999.0; // just a big number out there;
	}
	for (i = 0; i < m_cNodes; i++)
	{
		if (m_pNodes[i].m_vecOrigin.x < m_RegionMin[0])
			m_RegionMin[0] = m_pNodes[i].m_vecOrigin.x;
		if (m_pNodes[i].m_vecOrigin.y < m_RegionMin[1])
			m_RegionMin[1] = m_pNodes[i].m_vecOrigin.y;
		if (m_pNodes[i].m_vecOrigin.z < m_RegionMin[2])
			m_RegionMin[2] = m_pNodes[i].m_vecOrigin.z;

		if (m_pNodes[i].m_vecOrigin.x > m_RegionMax[0])
			m_RegionMax[0] = m_pNodes[i].m_vecOrigin.x;
		if (m_pNodes[i].m_vecOrigin.y > m_RegionMax[1])
			m_RegionMax[1] = m_pNodes[i].m_vecOrigin.y;
		if (m_pNodes[i].m_vecOrigin.z > m_RegionMax[2])
			m_RegionMax[2] = m_pNodes[i].m_vecOrigin.z;
	}
	for (i = 0; i < m_cNodes; i++)
	{
		m_pNodes[i].m_Region[0] = CALC_RANGE(m_pNodes[i].m_vecOrigin.x, m_RegionMin[0], m_RegionMax[0]);
		m_pNodes[i].m_Region[1] = CALC_RANGE(m_pNodes[i].m_vecOrigin.y, m_RegionMin[1], m_RegionMax[1]);
		m_pNodes[i].m_Region[2] = CALC_RANGE(m_pNodes[i].m_vecOrigin.z, m_RegionMin[2], m_RegionMax[2]);
	}

	for (i = 0; i < 3; i++)
	{
		int j;
		for (j = 0; j < NUM_RANGES; j++)
		{
			m_RangeStart[i][j] = 255;
			m_RangeEnd[i][j] = 0;
		}
		for (j = 0; j < m_cNodes; j++)
		{
			m_di[j].m_SortedBy[i] = j;
		}

		for (j = 0; j < m_cNodes - 1; j++)
		{
			int jNode = m_di[j].m_SortedBy[i];
			int jCodeX = m_pNodes[jNode].m_Region[0];
			int jCodeY = m_pNodes[jNode].m_Region[1];
			int jCodeZ = m_pNodes[jNode].m_Region[2];
			int jCode;
			switch (i)
			{
			case 0:
				jCode = (jCodeX << 16) + (jCodeY << 8) + jCodeZ;
				break;
			case 1:
				jCode = (jCodeY << 16) + (jCodeZ << 8) + jCodeX;
				break;
			case 2:
				jCode = (jCodeZ << 16) + (jCodeX << 8) + jCodeY;
				break;
			}

			for (int k = j + 1; k < m_cNodes; k++)
			{
				int kNode = m_di[k].m_SortedBy[i];
				int kCodeX = m_pNodes[kNode].m_Region[0];
				int kCodeY = m_pNodes[kNode].m_Region[1];
				int kCodeZ = m_pNodes[kNode].m_Region[2];
				int kCode;
				switch (i)
				{
				case 0:
					kCode = (kCodeX << 16) + (kCodeY << 8) + kCodeZ;
					break;
				case 1:
					kCode = (kCodeY << 16) + (kCodeZ << 8) + kCodeX;
					break;
				case 2:
					kCode = (kCodeZ << 16) + (kCodeX << 8) + kCodeY;
					break;
				}

				if (kCode < jCode)
				{
					// Swap j and k entries.
					//
					int Tmp = m_di[j].m_SortedBy[i];
					m_di[j].m_SortedBy[i] = m_di[k].m_SortedBy[i];
					m_di[k].m_SortedBy[i] = Tmp;
				}
			}
		}
	}

	// Generate lookup tables.
	//
	for (i = 0; i < m_cNodes; i++)
	{
		int CodeX = m_pNodes[m_di[i].m_SortedBy[0]].m_Region[0];
		int CodeY = m_pNodes[m_di[i].m_SortedBy[1]].m_Region[1];
		int CodeZ = m_pNodes[m_di[i].m_SortedBy[2]].m_Region[2];

		if (i < m_RangeStart[0][CodeX])
		{
			m_RangeStart[0][CodeX] = i;
		}
		if (i < m_RangeStart[1][CodeY])
		{
			m_RangeStart[1][CodeY] = i;
		}
		if (i < m_RangeStart[2][CodeZ])
		{
			m_RangeStart[2][CodeZ] = i;
		}
		if (m_RangeEnd[0][CodeX] < i)
		{
			m_RangeEnd[0][CodeX] = i;
		}
		if (m_RangeEnd[1][CodeY] < i)
		{
			m_RangeEnd[1][CodeY] = i;
		}
		if (m_RangeEnd[2][CodeZ] < i)
		{
			m_RangeEnd[2][CodeZ] = i;
		}
	}

	// Initialize the cache.
	//
	memset(m_Cache, 0, sizeof(m_Cache));
}

void CGraph::ComputeStaticRoutingTables(void)
{
	int nRoutes = m_cNodes * m_cNodes;
#define FROM_TO(x,y) ((x)*m_cNodes+(y))
	short* Routes = new short[nRoutes];

	int* pMyPath = new int[m_cNodes];
	unsigned short* BestNextNodes = new unsigned short[m_cNodes];
	char* pRoute = new char[m_cNodes * 2];


	if (Routes && pMyPath && BestNextNodes && pRoute)
	{
		int nTotalCompressedSize = 0;
		for (int iHull = 0; iHull < MAX_NODE_HULLS; iHull++)
		{
			for (int iCap = 0; iCap < 2; iCap++)
			{
				int iCapMask;
				switch (iCap)
				{
				case 0:
					iCapMask = 0;
					break;

				case 1:
					iCapMask = bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;
					break;
				}


				// Initialize Routing table to uncalculated.
				//
				int iFrom;
				for (iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = 0; iTo < m_cNodes; iTo++)
					{
						Routes[FROM_TO(iFrom, iTo)] = -1;
					}
				}

				for (iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = m_cNodes - 1; iTo >= 0; iTo--)
					{
						if (Routes[FROM_TO(iFrom, iTo)] != -1) continue;

						int cPathSize = FindShortestPath(pMyPath, iFrom, iTo, iHull, iCapMask);

						// Use the computed path to update the routing table.
						//
						if (cPathSize > 1)
						{
							for (int iNode = 0; iNode < cPathSize - 1; iNode++)
							{
								int iStart = pMyPath[iNode];
								int iNext = pMyPath[iNode + 1];
								for (int iNode1 = iNode + 1; iNode1 < cPathSize; iNode1++)
								{
									int iEnd = pMyPath[iNode1];
									Routes[FROM_TO(iStart, iEnd)] = iNext;
								}
							}
#if 0
							// Well, at first glance, this should work, but actually it's safer
							// to be told explictly that you can take a series of node in a
							// particular direction. Some links don't appear to have links in
							// the opposite direction.
							//
							for (iNode = cPathSize - 1; iNode >= 1; iNode--)
							{
								int iStart = pMyPath[iNode];
								int iNext = pMyPath[iNode - 1];
								for (int iNode1 = iNode - 1; iNode1 >= 0; iNode1--)
								{
									int iEnd = pMyPath[iNode1];
									Routes[FROM_TO(iStart, iEnd)] = iNext;
								}
							}
#endif
						}
						else
						{
							Routes[FROM_TO(iFrom, iTo)] = iFrom;
							Routes[FROM_TO(iTo, iFrom)] = iTo;
						}
					}
				}

				for (iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = 0; iTo < m_cNodes; iTo++)
					{
						BestNextNodes[iTo] = Routes[FROM_TO(iFrom, iTo)];
					}

					// Compress this node's routing table.
					//
					int iLastNode = 9999999; // just really big.
					int cSequence = 0;
					int cRepeats = 0;
					int CompressedSize = 0;
					char* p = pRoute;
					for (int i = 0; i < m_cNodes; i++)
					{
						BOOL CanRepeat = ((BestNextNodes[i] == iLastNode) && cRepeats < 127);
						BOOL CanSequence = (BestNextNodes[i] == i && cSequence < 128);

						if (cRepeats)
						{
							if (CanRepeat)
							{
								cRepeats++;
							}
							else
							{
								// Emit the repeat phrase.
								//
								CompressedSize += 2; // (count-1, iLastNode-i)
								*p++ = cRepeats - 1;
								int a = iLastNode - iFrom;
								int b = iLastNode - iFrom + m_cNodes;
								int c = iLastNode - iFrom - m_cNodes;
								if (-128 <= a && a <= 127)
								{
									*p++ = a;
								}
								else if (-128 <= b && b <= 127)
								{
									*p++ = b;
								}
								else if (-128 <= c && c <= 127)
								{
									*p++ = c;
								}
								else
								{
									ALERT(at_aiconsole, "Nodes need sorting (%d,%d)!\n", iLastNode, iFrom);
								}
								cRepeats = 0;

								if (CanSequence)
								{
									// Start a sequence.
									//
									cSequence++;
								}
								else
								{
									// Start another repeat.
									//
									cRepeats++;
								}
							}
						}
						else if (cSequence)
						{
							if (CanSequence)
							{
								cSequence++;
							}
							else
							{
								// It may be advantageous to combine
								// a single-entry sequence phrase with the
								// next repeat phrase.
								//
								if (cSequence == 1 && CanRepeat)
								{
									// Combine with repeat phrase.
									//
									cRepeats = 2;
									cSequence = 0;
								}
								else
								{
									// Emit the sequence phrase.
									//
									CompressedSize += 1; // (-count)
									*p++ = -cSequence;
									cSequence = 0;

									// Start a repeat sequence.
									//
									cRepeats++;
								}
							}
						}
						else
						{
							if (CanSequence)
							{
								// Start a sequence phrase.
								//
								cSequence++;
							}
							else
							{
								// Start a repeat sequence.
								//
								cRepeats++;
							}
						}
						iLastNode = BestNextNodes[i];
					}
					if (cRepeats)
					{
						// Emit the repeat phrase.
						//
						CompressedSize += 2;
						*p++ = cRepeats - 1;
#if 0
						iLastNode = iFrom + *pRoute;
						if (iLastNode >= m_cNodes) iLastNode -= m_cNodes;
						else if (iLastNode < 0) iLastNode += m_cNodes;
#endif
						int a = iLastNode - iFrom;
						int b = iLastNode - iFrom + m_cNodes;
						int c = iLastNode - iFrom - m_cNodes;
						if (-128 <= a && a <= 127)
						{
							*p++ = a;
						}
						else if (-128 <= b && b <= 127)
						{
							*p++ = b;
						}
						else if (-128 <= c && c <= 127)
						{
							*p++ = c;
						}
						else
						{
							ALERT(at_aiconsole, "Nodes need sorting (%d,%d)!\n", iLastNode, iFrom);
						}
					}
					if (cSequence)
					{
						// Emit the Sequence phrase.
						//
						CompressedSize += 1;
						*p++ = -cSequence;
					}

					// Go find a place to store this thing and point to it.
					//
					int nRoute = p - pRoute;
					if (m_pRouteInfo)
					{
						int i;
						for (i = 0; i < m_nRouteInfo - nRoute; i++)
						{
							if (memcmp(m_pRouteInfo + i, pRoute, nRoute) == 0)
							{
								break;
							}
						}
						if (i < m_nRouteInfo - nRoute)
						{
							m_pNodes[iFrom].m_pNextBestNode[iHull][iCap] = i;
						}
						else
						{
							char* Tmp = (char*)calloc(sizeof(char), (m_nRouteInfo + nRoute));
							memcpy(Tmp, m_pRouteInfo, m_nRouteInfo);
							free(m_pRouteInfo);
							m_pRouteInfo = Tmp;
							memcpy(m_pRouteInfo + m_nRouteInfo, pRoute, nRoute);
							m_pNodes[iFrom].m_pNextBestNode[iHull][iCap] = m_nRouteInfo;
							m_nRouteInfo += nRoute;
							nTotalCompressedSize += CompressedSize;
						}
					}
					else
					{
						m_nRouteInfo = nRoute;
						m_pRouteInfo = (char*)calloc(sizeof(char), nRoute);
						memcpy(m_pRouteInfo, pRoute, nRoute);
						m_pNodes[iFrom].m_pNextBestNode[iHull][iCap] = 0;
						nTotalCompressedSize += CompressedSize;
					}
				}
			}
		}
		ALERT(at_aiconsole, "Size of Routes = %d\n", nTotalCompressedSize);
	}
	//MODDD - proper delete[] update
	if (Routes) delete[] Routes;
	if (BestNextNodes) delete[] BestNextNodes;
	if (pRoute) delete[] pRoute;
	if (pMyPath) delete[] pMyPath;
	Routes = 0;
	BestNextNodes = 0;
	pRoute = 0;
	pMyPath = 0;

#if 0
	TestRoutingTables();
#endif
	m_fRoutingComplete = TRUE;
}//END OF ComputeStaticRoutingTables


// Test those routing tables. Doesn't really work, yet.
//
void CGraph::TestRoutingTables(void)
{
	int* pMyPath = new int[m_cNodes];
	int* pMyPath2 = new int[m_cNodes];
	if (pMyPath && pMyPath2)
	{
		for (int iHull = 0; iHull < MAX_NODE_HULLS; iHull++)
		{
			for (int iCap = 0; iCap < 2; iCap++)
			{
				int iCapMask;
				switch (iCap)
				{
				case 0:
					iCapMask = 0;
					break;

				case 1:
					iCapMask = bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;
					break;
				}

				for (int iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = 0; iTo < m_cNodes; iTo++)
					{
						m_fRoutingComplete = FALSE;
						int cPathSize1 = FindShortestPath(pMyPath, iFrom, iTo, iHull, iCapMask);
						m_fRoutingComplete = TRUE;
						int cPathSize2 = FindShortestPath(pMyPath2, iFrom, iTo, iHull, iCapMask);

						// Unless we can look at the entire path, we can verify that it's correct.
						//
						if (cPathSize2 == MAX_PATH_SIZE) continue;

						// Compare distances.
						//
#if 1
						float flDistance1 = 0.0;
						int i;
						for (i = 0; i < cPathSize1 - 1; i++)
						{
							// Find the link from pMyPath[i] to pMyPath[i+1]
							//
							if (pMyPath[i] == pMyPath[i + 1]) continue;
							int iVisitNode;
							BOOL bFound = FALSE;
							for (int iLink = 0; iLink < m_pNodes[pMyPath[i]].m_cNumLinks; iLink++)
							{
								iVisitNode = INodeLink(pMyPath[i], iLink);
								if (iVisitNode == pMyPath[i + 1])
								{
									flDistance1 += m_pLinkPool[m_pNodes[pMyPath[i]].m_iFirstLink + iLink].m_flWeight;
									bFound = TRUE;
									break;
								}
							}
							if (!bFound)
							{
								ALERT(at_aiconsole, "No link.\n");
							}
						}

						float flDistance2 = 0.0;
						for (i = 0; i < cPathSize2 - 1; i++)
						{
							// Find the link from pMyPath2[i] to pMyPath2[i+1]
							//
							if (pMyPath2[i] == pMyPath2[i + 1]) continue;
							int iVisitNode;
							BOOL bFound = FALSE;
							for (int iLink = 0; iLink < m_pNodes[pMyPath2[i]].m_cNumLinks; iLink++)
							{
								iVisitNode = INodeLink(pMyPath2[i], iLink);
								if (iVisitNode == pMyPath2[i + 1])
								{
									flDistance2 += m_pLinkPool[m_pNodes[pMyPath2[i]].m_iFirstLink + iLink].m_flWeight;
									bFound = TRUE;
									break;
								}
							}
							if (!bFound)
							{
								ALERT(at_aiconsole, "No link.\n");
							}
						}
						if (fabs(flDistance1 - flDistance2) > 0.10)
						{
#else
						if (cPathSize1 != cPathSize2 || memcmp(pMyPath, pMyPath2, sizeof(int) * cPathSize1) != 0)
						{
#endif
							ALERT(at_aiconsole, "Routing is inconsistent!!!\n");
							ALERT(at_aiconsole, "(%d to %d |%d/%d)1:", iFrom, iTo, iHull, iCap);
							for (i = 0; i < cPathSize1; i++)
							{
								ALERT(at_aiconsole, "%d ", pMyPath[i]);
							}
							ALERT(at_aiconsole, "\n(%d to %d |%d/%d)2:", iFrom, iTo, iHull, iCap);
							for (i = 0; i < cPathSize2; i++)
							{
								ALERT(at_aiconsole, "%d ", pMyPath2[i]);
							}
							ALERT(at_aiconsole, "\n");
							m_fRoutingComplete = FALSE;
							cPathSize1 = FindShortestPath(pMyPath, iFrom, iTo, iHull, iCapMask);
							m_fRoutingComplete = TRUE;
							cPathSize2 = FindShortestPath(pMyPath2, iFrom, iTo, iHull, iCapMask);
							goto EnoughSaid;
						}
						}
					}
				}
			}
		}

EnoughSaid:

	//MODDD - proper delete[] update
	if (pMyPath) delete[] pMyPath;
	if (pMyPath2) delete[] pMyPath2;
	pMyPath = 0;
	pMyPath2 = 0;
	}




//MODDD - Well that was always there.  Nevermind me, I'm a moron.

//=========================================================
// CNodeViewer - Draws a graph of the shorted path from all nodes
// to current location (typically the player).  It then draws
// as many connects as it can per frame, trying not to overflow the buffer
//=========================================================
class CNodeViewer : public CBaseEntity
{
public:
	void Spawn(void);

	int m_iBaseNode;
	int m_iDraw;
	int m_nVisited;
	int m_aFrom[128];
	int m_aTo[128];
	int m_iHull;
	int m_afNodeType;
	Vector m_vecColor;

	void FindNodeConnections(int iNode);
	void AddNode(int iFrom, int iTo);
	void EXPORT DrawThink(void);

};
LINK_ENTITY_TO_CLASS(node_viewer, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_human, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_fly, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_large, CNodeViewer);

//MODDD - new.
// NO.  Don't do point ones.  It uses a number that's invalid in
//   the static route table array (-1).  Don't even try.
//LINK_ENTITY_TO_CLASS(node_viewer_point, CNodeViewer);
//LINK_ENTITY_TO_CLASS(node_viewer_point_air, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_small, CNodeViewer);


void CNodeViewer::Spawn()
{
	if (!WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available or built
		ALERT(at_console, "Graph not ready!\n");
		UTIL_Remove(this);
		return;
	}


	//MODDD - why wasn't there at least SMALL too??
	// CANNED.  The point ones at least
	/*
	if (FClassnameIs(pev, "node_viewer_point"))
	{
		m_iHull = NODE_POINT_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor = Vector(255, 150, 255);
	}
	else if (FClassnameIs(pev, "node_viewer_point_air"))
	{
		m_iHull = NODE_POINT_HULL;
		m_afNodeType = bits_NODE_AIR;
		m_vecColor = Vector(255, 255, 255);
	}
	else
	*/
	if (FClassnameIs(pev, "node_viewer_small"))
	{
		m_iHull = NODE_SMALL_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor = Vector(160, 255, 255);
	}
	else if (FClassnameIs(pev, "node_viewer_fly"))
	{
		m_iHull = NODE_FLY_HULL;
		m_afNodeType = bits_NODE_AIR;
		m_vecColor = Vector(160, 100, 255);
	}
	else if (FClassnameIs(pev, "node_viewer_large"))
	{
		m_iHull = NODE_LARGE_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor = Vector(100, 255, 160);
	}
	else
	{
		m_iHull = NODE_HUMAN_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor = Vector(255, 160, 100);
	}


	m_iBaseNode = WorldGraph.FindNearestNode(pev->origin, m_afNodeType);

	if (m_iBaseNode < 0)
	{
		ALERT(at_console, "No nearby node\n");
		return;
	}

	m_nVisited = 0;

	ALERT(at_aiconsole, "basenode %d\n", m_iBaseNode);

	if (WorldGraph.m_cNodes < 128)
	{
		for (int i = 0; i < WorldGraph.m_cNodes; i++)
		{
			AddNode(i, WorldGraph.NextNodeInRoute(i, m_iBaseNode, m_iHull, 0));
		}
	}
	else
	{
		// do a depth traversal
		FindNodeConnections(m_iBaseNode);

		int start = 0;
		int end;
		do {
			end = m_nVisited;
			// ALERT( at_console, "%d :", m_nVisited );
			for (end = m_nVisited; start < end; start++)
			{
				FindNodeConnections(m_aFrom[start]);
				FindNodeConnections(m_aTo[start]);
			}
		} while (end != m_nVisited);
	}

	ALERT(at_aiconsole, "%d nodes\n", m_nVisited);

	m_iDraw = 0;
	SetThink(&CNodeViewer::DrawThink);
	pev->nextthink = gpGlobals->time;
}


void CNodeViewer::FindNodeConnections(int iNode)
{
	AddNode(iNode, WorldGraph.NextNodeInRoute(iNode, m_iBaseNode, m_iHull, 0));
	for (int i = 0; i < WorldGraph.m_pNodes[iNode].m_cNumLinks; i++)
	{
		CLink* pToLink = &WorldGraph.NodeLink(iNode, i);
		AddNode(pToLink->m_iDestNode, WorldGraph.NextNodeInRoute(pToLink->m_iDestNode, m_iBaseNode, m_iHull, 0));
	}
}

void CNodeViewer::AddNode(int iFrom, int iTo)
{
	if (m_nVisited >= 128)
	{
		return;
	}
	else
	{
		if (iFrom == iTo)
			return;

		for (int i = 0; i < m_nVisited; i++)
		{
			if (m_aFrom[i] == iFrom && m_aTo[i] == iTo)
				return;
			if (m_aFrom[i] == iTo && m_aTo[i] == iFrom)
				return;
		}
		m_aFrom[m_nVisited] = iFrom;
		m_aTo[m_nVisited] = iTo;
		m_nVisited++;
	}
}


void CNodeViewer::DrawThink(void)
{
	pev->nextthink = gpGlobals->time;

	//for (int i = 0; i < 10; i++)
	//MODDD - no more limit!  or... uh-oh?
	while (TRUE)
	{
		if (m_iDraw == m_nVisited)
		{
			if(m_iDraw == 0){
				easyForcePrintLine("!!! Nothing could get to here in that hull size !!!");
			}

			UTIL_Remove(this);
			return;
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.x);
		WRITE_COORD(WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.y);
		WRITE_COORD(WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.z + node_linktest_height);

		WRITE_COORD(WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.x);
		WRITE_COORD(WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.y);
		WRITE_COORD(WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.z + node_linktest_height);
		WRITE_SHORT(g_sModelIndexLaser);
		WRITE_BYTE(0); // framestart
		WRITE_BYTE(0); // framerate
		//MODDD - more life.  Was 250.
		WRITE_BYTE(3000); // life
		WRITE_BYTE(40);  // width
		WRITE_BYTE(0);   // noise
		WRITE_BYTE(m_vecColor.x);   // r, g, b
		WRITE_BYTE(m_vecColor.y);   // r, g, b
		WRITE_BYTE(m_vecColor.z);   // r, g, b
		WRITE_BYTE(128);	// brightness
		WRITE_BYTE(0);		// speed
		MESSAGE_END();

		m_iDraw++;
	}
}


