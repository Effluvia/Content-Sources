//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Maintains memory statistics
//
// $NoKeywords: $
//=============================================================================

#include <malloc.h>
#include "tier0/Dbg.h"
#include "tier0/mem.h"
#include <map>


//-----------------------------------------------------------------------------
// At any given point in time, we want to keep track of 
//-----------------------------------------------------------------------------
class CMemStats
{
public:
	CMemStats();

	void RegisterAllocation( int nSize, const char *pFileName, float flTime );
	void RegisterDeallocation( int nSize, const char *pFileName, float flTime );
	
//	void SpewStats( );

private:
	struct MemInfo_t
	{
		// Size in bytes
		int m_nCurrentSize;
		int m_nPeakSize;
		int m_nTotalSize;
		int m_nOverheadSize;

		// Count in terms of # of allocations
		int m_nCurrentCount;
		int m_nPeakCount;
		int m_nTotalCount;

		// Time spent allocating + deallocating
		float m_flTime;
	};

	// NOTE: Deliberately using STL here because the UTL stuff
	// is a client of this library; want to avoid circular dependency

	// Maps file name to info
	typedef std::map< const char *, MemInfo_t > StatMap_t;
	typedef StatMap_t::iterator StatMapIter_t;
	typedef StatMap_t::value_type StatMapEntry_t;

private:
	// Finds the file in our map
	StatMapIter_t FindOrCreateEntry( const char *pFileName );

private:
	StatMap_t m_StatMap;
	MemInfo_t m_GlobalInfo;
};
				   

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CMemStats::CMemStats()
{
}


//-----------------------------------------------------------------------------
// Finds the file in our map
//-----------------------------------------------------------------------------
CMemStats::StatMapIter_t CMemStats::FindOrCreateEntry( const char *pFileName )
{
	/*
	StatMapIter_t iter = m_StatMap.find( pFileName );
	if (iter == m_StatMap.end() )
	{
		MemInfo_t newInfo;
		memset( &newInfo, 0, sizeof(MemInfo_t) );
		iter = m_StatMap.insert( StatMapEntry_t( pFileName, newInfo ) );
	}
	return iter;
	*/

	//MODDD - THE FUTURE HAS COME TO HAUNT YOU
	std::pair<StatMapIter_t, bool> retval;
	MemInfo_t newInfo;
	retval = m_StatMap.insert( StatMapEntry_t( StatMapEntry_t( pFileName, newInfo ), MemInfo_t() ) );
	//return retval.first->second;
	return retval.first;

}


//-----------------------------------------------------------------------------
// Registers allocations + deallocations
//-----------------------------------------------------------------------------
void RegisterAllocation( int nSize, const char *pFileName, float flTime )
{
	//MODDD - NEVERMIND.
	/*
	StatMapIter_t iter = FindOrCreateEntry( pFileName );
	MemInfo_t &info = iter->second;
	++info.m_nCurrentCount;
	++info.m_nTotalCount;
	//MODDD - found incomplete at this stage
	//if (info.m_nCurrentCount > info.m_n

	if (info.m_nCurrentCount > info.m_nTotalCount){
		info.m_nTotalCount = info.m_nCurrentCount;
	}
	*/
	//MODDD - CRITICAL.  Don't know what else the intent was here.
}

void RegisterDeallocation( int nSize, const char *pFileName, float flTime )
{
	//MODDD - CRITICAL.  What to do here.
}

