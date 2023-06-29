//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Used to create a path that can be followed by NPCs and trains.
//
//=============================================================================//

#include "cbase.h"
#include "pathtrack.h"
#include "entitylist.h"
#include "ndebugoverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CPathTrack )

	DEFINE_FIELD( m_pnext,			FIELD_CLASSPTR ),
	DEFINE_FIELD( m_pprevious,		FIELD_CLASSPTR ),
	DEFINE_FIELD( m_paltpath,		FIELD_CLASSPTR ),

	DEFINE_KEYFIELD( m_flRadius,	FIELD_FLOAT, "radius" ),
	DEFINE_FIELD( m_length,			FIELD_FLOAT ),
	DEFINE_KEYFIELD( m_altName,		FIELD_STRING, "altpath" ),
	DEFINE_KEYFIELD( m_eOrientationType, FIELD_INTEGER, "orientationtype" ),
//	DEFINE_FIELD( m_nIterVal,		FIELD_INTEGER ),
#ifdef HOE_TRAIN_PITCH
	DEFINE_KEYFIELD( m_flPitch,		FIELD_FLOAT, "pitch" ),
#endif
#ifdef HOE_TRACK_SPLINE
	DEFINE_KEYFIELD( m_eConnectionType,	FIELD_INTEGER, "connectiontype" ),
#endif

	DEFINE_INPUTFUNC( FIELD_VOID, "InPass", InputPass ),
	DEFINE_INPUTFUNC( FIELD_VOID, "InTeleport",  InputTeleport ),
	
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableAlternatePath", InputEnableAlternatePath ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableAlternatePath", InputDisableAlternatePath ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ToggleAlternatePath", InputToggleAlternatePath ),

	DEFINE_INPUTFUNC( FIELD_VOID, "EnablePath", InputEnablePath ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisablePath", InputDisablePath ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TogglePath", InputTogglePath ),

	// Outputs
	DEFINE_OUTPUT(m_OnPass, "OnPass"),
	DEFINE_OUTPUT(m_OnTeleport,  "OnTeleport"),

END_DATADESC()

LINK_ENTITY_TO_CLASS( path_track, CPathTrack );


//-----------------------------------------------------------------------------
// Finds circular paths
//-----------------------------------------------------------------------------
int CPathTrack::s_nCurrIterVal = 0;
bool CPathTrack::s_bIsIterating = false;


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPathTrack::CPathTrack()
{
	m_nIterVal = -1;
	m_eOrientationType = TrackOrientation_FacePath;
}


//-----------------------------------------------------------------------------
// Spawn!
//-----------------------------------------------------------------------------
void CPathTrack::Spawn( void )
{
	SetSolid( SOLID_NONE );
	UTIL_SetSize(this, Vector(-8, -8, -8), Vector(8, 8, 8));
}


//-----------------------------------------------------------------------------
// Activate!
//-----------------------------------------------------------------------------
void CPathTrack::Activate( void )
{
	BaseClass::Activate();

	if ( GetEntityName() != NULL_STRING )		// Link to next, and back-link
	{
		Link();
	}
}


//-----------------------------------------------------------------------------
// Connects up the previous + next pointers 
//-----------------------------------------------------------------------------
void CPathTrack::Link( void  )
{
	CBaseEntity *pTarget;

	if ( m_target != NULL_STRING )
	{
		pTarget = gEntList.FindEntityByName( NULL, m_target );

		if ( pTarget == this)
		{
			Warning("ERROR: path_track (%s) refers to itself as a target!\n", GetDebugName());
			
			//FIXME: Why were we removing this?  If it was already connected to, we weren't updating the other linked
			//		 end, causing problems with walking through bogus memory links!  -- jdw

			//UTIL_Remove(this);
			//return;
		}
		else if ( pTarget )
		{
			m_pnext = dynamic_cast<CPathTrack*>( pTarget );

			if ( m_pnext )		// If no next pointer, this is the end of a path
			{
				m_pnext->SetPrevious( this );
			}
		}
		else
		{
			Warning("Dead end link: %s\n", STRING( m_target ) );
		}
	}

	// Find "alternate" path
	if ( m_altName != NULL_STRING )
	{
		pTarget = gEntList.FindEntityByName( NULL, m_altName );
		if ( pTarget )
		{
			m_paltpath = dynamic_cast<CPathTrack*>( pTarget );
			m_paltpath->SetPrevious( this );
		}
	}
}


//-----------------------------------------------------------------------------
// Circular path checking
//-----------------------------------------------------------------------------
void CPathTrack::BeginIteration()
{
	Assert( !s_bIsIterating );
	++s_nCurrIterVal;
	s_bIsIterating = true;
}

void CPathTrack::EndIteration()
{
	Assert( s_bIsIterating );
	s_bIsIterating = false;
}

void CPathTrack::Visit()
{
	m_nIterVal = s_nCurrIterVal;
}

bool CPathTrack::HasBeenVisited() const
{
	return ( m_nIterVal == s_nCurrIterVal );
}


//-----------------------------------------------------------------------------
// Do we have an alternate path?
//-----------------------------------------------------------------------------
bool CPathTrack::HasAlternathPath() const
{
	return ( m_paltpath != NULL ); 
}


//-----------------------------------------------------------------------------
// Purpose: Toggles the track to or from its alternate path
//-----------------------------------------------------------------------------
void CPathTrack::ToggleAlternatePath( void )
{
	// Use toggles between two paths
	if ( m_paltpath != NULL )
	{
		if ( FBitSet( m_spawnflags, SF_PATH_ALTERNATE ) == false )
		{
			EnableAlternatePath();
		}
		else
		{
			DisableAlternatePath();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::EnableAlternatePath( void )
{
	if ( m_paltpath != NULL )
	{
		SETBITS( m_spawnflags, SF_PATH_ALTERNATE );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::DisableAlternatePath( void )
{
	if ( m_paltpath != NULL )
	{
		CLEARBITS( m_spawnflags, SF_PATH_ALTERNATE );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CPathTrack::InputEnableAlternatePath( inputdata_t &inputdata )
{
	EnableAlternatePath();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CPathTrack::InputDisableAlternatePath( inputdata_t &inputdata )
{
	DisableAlternatePath();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CPathTrack::InputToggleAlternatePath( inputdata_t &inputdata )
{
	ToggleAlternatePath();
}

//-----------------------------------------------------------------------------
// Purpose: Toggles the track to or from its alternate path
//-----------------------------------------------------------------------------
void CPathTrack::TogglePath( void )
{
	// Use toggles between two paths
	if ( FBitSet( m_spawnflags, SF_PATH_DISABLED ) )
	{
		EnablePath();
	}
	else
	{
		DisablePath();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::EnablePath( void )
{
	CLEARBITS( m_spawnflags, SF_PATH_DISABLED );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::DisablePath( void )
{
	SETBITS( m_spawnflags, SF_PATH_DISABLED );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CPathTrack::InputEnablePath( inputdata_t &inputdata )
{
	EnablePath();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CPathTrack::InputDisablePath( inputdata_t &inputdata )
{
	DisablePath();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CPathTrack::InputTogglePath( inputdata_t &inputdata )
{
	TogglePath();
}


void CPathTrack::DrawDebugGeometryOverlays() 
{
	// ----------------------------------------------
	// Draw line to next target is bbox is selected
	// ----------------------------------------------
	if (m_debugOverlays & (OVERLAY_BBOX_BIT|OVERLAY_ABSBOX_BIT))
	{
		if (m_pnext)
		{
			NDebugOverlay::Line(GetAbsOrigin(),m_pnext->GetAbsOrigin(),255,100,100,true,0.0);
		}
#ifdef HOE_TRACK_SPLINE
		if ( GetConnectionType() == TrackConnection_Spline )
		{
			Vector endPoints[11];
			GetSplineSegments( endPoints );
			const int iDivs = 10;
			for ( int i = 1; i <= iDivs; i++ )
			{
				NDebugOverlay::Line(endPoints[i-1],endPoints[i],100,100,255,true,0.0);
			}
		}
#endif // HOE_DLL
	}
	BaseClass::DrawDebugGeometryOverlays();
}

CPathTrack	*CPathTrack::ValidPath( CPathTrack	*ppath, int testFlag )
{
	if ( !ppath )
		return NULL;

	if ( testFlag && FBitSet( ppath->m_spawnflags, SF_PATH_DISABLED ) )
		return NULL;

	return ppath;
}


void CPathTrack::Project( CPathTrack *pstart, CPathTrack *pend, Vector &origin, float dist )
{
	if ( pstart && pend )
	{
		Vector dir = (pend->GetLocalOrigin() - pstart->GetLocalOrigin());
		VectorNormalize( dir );
		origin = pend->GetLocalOrigin() + dir * dist;
	}
}

CPathTrack *CPathTrack::GetNext( void )
{
	if ( m_paltpath && FBitSet( m_spawnflags, SF_PATH_ALTERNATE ) && !FBitSet( m_spawnflags, SF_PATH_ALTREVERSE ) )
	{
		Assert( !m_paltpath.IsValid() || m_paltpath.Get() != NULL );
		return m_paltpath;
	}
	
	// The paths shouldn't normally be getting deleted so assert that if it was set, it's valid.
	Assert( !m_pnext.IsValid() || m_pnext.Get() != NULL );
	return m_pnext;
}



CPathTrack *CPathTrack::GetPrevious( void )
{
	if ( m_paltpath && FBitSet( m_spawnflags, SF_PATH_ALTERNATE ) && FBitSet( m_spawnflags, SF_PATH_ALTREVERSE ) )
	{
		Assert( !m_paltpath.IsValid() || m_paltpath.Get() != NULL );
		return m_paltpath;
	}
	
	Assert( !m_pprevious.IsValid() || m_pprevious.Get() != NULL );
	return m_pprevious;
}



void CPathTrack::SetPrevious( CPathTrack *pprev )
{
	// Only set previous if this isn't my alternate path
	if ( pprev && !FStrEq( STRING(pprev->GetEntityName()), STRING(m_altName) ) )
		m_pprevious = pprev;
}


CPathTrack *CPathTrack::GetNextInDir( bool bForward )
{
	if ( bForward )
		return GetNext();
	
	return GetPrevious();
}


//-----------------------------------------------------------------------------
// Purpose: Assumes this is ALWAYS enabled
// Input  : origin - position along path to look ahead from
//			dist - distance to look ahead, negative values look backward
//			move - 
// Output : Returns the track that we will be PAST in 'dist' units.
//-----------------------------------------------------------------------------
CPathTrack *CPathTrack::LookAhead( Vector &origin, float dist, int move, CPathTrack **pNextNext )
{
	CPathTrack *pcurrent = this;
	float originalDist = dist;
	Vector currentPos = origin;

	bool bForward = true;
	if ( dist < 0 )
	{
		// Travelling backwards along the path.
		dist = -dist;
		bForward = false;
	}

	// Move along the path, until we've gone 'dist' units or run out of path.
	while ( dist > 0 )
	{
		// If there is no next path track, or it's disabled, we're done.
		if ( !ValidPath( pcurrent->GetNextInDir( bForward ), move ) )
		{
			if ( !move )
			{
				Project( pcurrent->GetNextInDir( !bForward ), pcurrent, origin, dist );
			}

			return NULL;
		}

		// The next path track is valid. How far are we from it?
#ifdef HOE_TRACK_SPLINE
		Vector dir;
		float length;
		if ( pcurrent->GetConnectionType() == TrackConnection_Spline )
		{
			length = pcurrent->GetSplineLength() - pcurrent->CalcDistanceAlongSpline( currentPos );
		}
		else
		{
			dir = pcurrent->GetNextInDir( bForward )->GetLocalOrigin() - currentPos;
			length = dir.Length();
		}
#else
		Vector dir = pcurrent->GetNextInDir( bForward )->GetLocalOrigin() - currentPos;
		float length = dir.Length();
#endif

		// If we are at the next node and there isn't one beyond it, return the next node.
		if ( !length  && !ValidPath( pcurrent->GetNextInDir( bForward )->GetNextInDir( bForward ), move ) )
		{
			if ( pNextNext )
			{
				*pNextNext = NULL;
			}

			if ( dist == originalDist )
			{
				// Didn't move at all, must be in a dead end.
				return NULL;
			}

			return pcurrent->GetNextInDir( bForward );
		}

		// If we don't hit the next path track within the distance remaining, we're done.
		if ( length > dist )
		{
#ifdef HOE_TRACK_SPLINE
			if ( pcurrent->GetConnectionType() == TrackConnection_Spline )
			{
				pcurrent->AdvanceAlongSpline( currentPos, dist, origin );
			}
			else
			{
				origin = currentPos + ( dir * ( dist / length ) );
			}
#else
			origin = currentPos + ( dir * ( dist / length ) );
#endif
			if ( pNextNext )
			{
				*pNextNext = pcurrent->GetNextInDir( bForward );
			}

			return pcurrent;
		}

		// We hit the next path track, advance to it.
		dist -= length;
		currentPos = pcurrent->GetNextInDir( bForward )->GetLocalOrigin();
		pcurrent = pcurrent->GetNextInDir( bForward );
		origin = currentPos;
	}

	// We consumed all of the distance, and exactly landed on a path track.
	if ( pNextNext )
	{
		*pNextNext = pcurrent->GetNextInDir( bForward );
	}

	return pcurrent;
}


// Assumes this is ALWAYS enabled
CPathTrack *CPathTrack::Nearest( const Vector &origin )
{
	int			deadCount;
	float		minDist, dist;
	Vector		delta;
	CPathTrack	*ppath, *pnearest;


	delta = origin - GetLocalOrigin();
	delta.z = 0;
	minDist = delta.Length();
	pnearest = this;
	ppath = GetNext();

	// Hey, I could use the old 2 racing pointers solution to this, but I'm lazy :)
	deadCount = 0;
	while ( ppath && ppath != this )
	{
		deadCount++;
		if ( deadCount > 9999 )
		{
			Warning( "Bad sequence of path_tracks from %s\n", GetDebugName() );
			Assert(0);
			return NULL;
		}
		delta = origin - ppath->GetLocalOrigin();
		delta.z = 0;
		dist = delta.Length();
		if ( dist < minDist )
		{
			minDist = dist;
			pnearest = ppath;
		}
		ppath = ppath->GetNext();
	}
	return pnearest;
}


//-----------------------------------------------------------------------------
// Purpose: Returns how the path follower should orient itself when moving
//			through this path track.
//-----------------------------------------------------------------------------
TrackOrientationType_t CPathTrack::GetOrientationType()
{
	return m_eOrientationType;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
QAngle CPathTrack::GetOrientation( bool bForwardDir )
{
	TrackOrientationType_t eOrient = GetOrientationType();
	if ( eOrient == TrackOrientation_FacePathAngles )
	{
		return GetLocalAngles();
	}

	CPathTrack *pPrev = this;
	CPathTrack *pNext = GetNextInDir( bForwardDir );

	if ( !pNext )
	{	pPrev = GetNextInDir( !bForwardDir );
		pNext = this;
	}

	Vector vecDir = pNext->GetLocalOrigin() - pPrev->GetLocalOrigin();

	QAngle angDir;
	VectorAngles( vecDir, angDir );
	return angDir;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pent - 
// Output : CPathTrack
//-----------------------------------------------------------------------------
CPathTrack *CPathTrack::Instance( edict_t *pent )
{
	CBaseEntity *pEntity = CBaseEntity::Instance( pent );
	if ( FClassnameIs( pEntity, "path_track" ) )
		return (CPathTrack *)pEntity;
	return NULL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::InputPass( inputdata_t &inputdata )
{
	m_OnPass.FireOutput( inputdata.pActivator, this );

#ifdef TF_DLL
	IGameEvent * event = gameeventmanager->CreateEvent( "path_track_passed" );
	if ( event )
	{
		event->SetInt( "index", entindex() );
		gameeventmanager->FireEvent( event, true );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::InputTeleport( inputdata_t &inputdata )
{
	m_OnTeleport.FireOutput( inputdata.pActivator, this );
}

#ifdef HOE_TRACK_SPLINE


//-----------------------------------------------------------------------------
TrackConnectionType_t CPathTrack::GetConnectionType()
{
	return m_eConnectionType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::GetSplinePoints( Vector &p1, Vector &p2, Vector &p3, Vector &p4 )
{
	CPathTrack *pPrev = ValidPath( GetPrevious() );
	CPathTrack *pNext = ValidPath( GetNext() );
	CPathTrack *pNextNext = pNext ? ValidPath( pNext->GetNext() ) : NULL;

	p1 = p2 = p3 = p4 = GetAbsOrigin();
	if ( pPrev && (pPrev->GetConnectionType() == TrackConnection_Spline) )
	{
		p1 = pPrev->GetAbsOrigin();
	}
	if ( pNext )
	{
		p3 = p4 = pNext->GetAbsOrigin();
		if ( pNext->GetConnectionType() == TrackConnection_Spline )
		{
			if ( pNextNext )
				p4 = pNextNext->GetAbsOrigin();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::GetSplineSegments( Vector endPoints[NUM_SPLINE_SEGMENTS+1] )
{
	if ( m_flSplineTime > 0 && gpGlobals->curtime == m_flSplineTime )
	{
		memcpy( endPoints, m_vecSplineSegments, sizeof(m_vecSplineSegments) );
		return;
	}

	Vector splinePoints[4];
	GetSplinePoints( splinePoints[0], splinePoints[1], splinePoints[2], splinePoints[3] );

	endPoints[0] = splinePoints[1]; // GetAbsOrigin()

	Vector vecPrev = splinePoints[1];
	const int iDivs = NUM_SPLINE_SEGMENTS;
	for ( int i = 1; i <= iDivs; i++ )
	{
		Vector vecCurr;
		float flT = (float)i / (float)iDivs;
		Catmull_Rom_Spline( splinePoints[0], splinePoints[1], splinePoints[2], splinePoints[3], flT, vecCurr );
		endPoints[i] = vecCurr;
//		flSplineLength += (vecCurr - vecPrev).Length();
		vecPrev = vecCurr;
	}

	// Fixme: only recalculate if path_tracks move, alt-paths change etc
	memcpy( m_vecSplineSegments, endPoints, sizeof(m_vecSplineSegments) );
	m_flSplineTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CPathTrack::GetSplineLength( void )
{
	Vector endPoints[NUM_SPLINE_SEGMENTS+1];
	GetSplineSegments( endPoints );

	float flDist = 0;

	const int iDivs = NUM_SPLINE_SEGMENTS;
	for ( int i = 1; i <= iDivs; i++ )
	{
		flDist += (endPoints[i] - endPoints[i-1]).Length();
	}
	return flDist;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPathTrack::CalcClosestSplineSegment( const Vector &testPoint )
{
	Vector endPoints[NUM_SPLINE_SEGMENTS+1];
	GetSplineSegments( endPoints );

	int nClosestSegment = -1;
	float flClosestDist = 999999999;

	const int iDivs = NUM_SPLINE_SEGMENTS;
	for ( int i = 1; i <= iDivs; i++ )
	{
		Vector vClosest;
		float dist;
		dist = CalcDistanceToLineSegment( testPoint, endPoints[i-1], endPoints[i] );
		if ( dist < flClosestDist )
		{
			nClosestSegment = i;
			flClosestDist = dist;
		}
	}
	return nClosestSegment;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CPathTrack::CalcDistanceAlongSpline( const Vector &testPoint, Vector *pVecClosest/* = 0*/ )
{
	int segment = CalcClosestSplineSegment( testPoint );

	Vector endPoints[NUM_SPLINE_SEGMENTS+1];
	GetSplineSegments( endPoints );

	Vector vClosest;
	CalcClosestPointOnLineSegment( testPoint, endPoints[segment-1], endPoints[segment], vClosest );

	float flDist = 0;
//	const int iDivs = 10;
	for ( int i = 1; i < segment; i++ )
	{
		flDist += (endPoints[i] - endPoints[i-1]).Length();
	}
	flDist += (vClosest-endPoints[segment-1]).Length();

	if ( pVecClosest != NULL )
		*pVecClosest = vClosest;

	return flDist;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::AdvanceAlongSpline( const Vector &vStartPoint, float flDist, Vector &vOut )
{
	int segment = CalcClosestSplineSegment( vStartPoint );

	Vector endPoints[NUM_SPLINE_SEGMENTS+1];
	GetSplineSegments( endPoints );

	Vector vPointOnSegment;
	CalcClosestPointOnLineSegment( vStartPoint, endPoints[segment-1], endPoints[segment], vPointOnSegment );

	float flDistRemainingInSegment = (endPoints[segment] - vPointOnSegment).Length();
	Vector vSegDir;

	const int iDivs = NUM_SPLINE_SEGMENTS;
	while ( flDist > 0 )
	{
		if ( flDistRemainingInSegment >= flDist )
		{
			vSegDir = endPoints[segment] - endPoints[segment-1];
			vSegDir.NormalizeInPlace();
			vOut = vPointOnSegment + flDist * vSegDir;
			return;
		}
		if ( segment == iDivs )
		{
			vOut = endPoints[segment];
			return;
		}
		flDist -= flDistRemainingInSegment;
		segment += 1;
		flDistRemainingInSegment = (endPoints[segment] - endPoints[segment-1]).Length();
		vPointOnSegment = endPoints[segment-1];
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPathTrack::GetSplineTangent( const Vector &testPoint, Vector &vTangent, Vector *vTangentNext/* = 0 */ )
{
	int segment = CalcClosestSplineSegment( testPoint );

	Vector endPoints[NUM_SPLINE_SEGMENTS+1];
	GetSplineSegments( endPoints );

	vTangent = endPoints[segment] - endPoints[segment-1];
	vTangent.NormalizeInPlace();

	const int iDivs = NUM_SPLINE_SEGMENTS;
	if ( vTangentNext != NULL )
	{
		// FIXME: handle reverse direction
		if ( segment < iDivs )
		{
			segment++;
			*vTangentNext = endPoints[segment] - endPoints[segment-1];
			vTangentNext->NormalizeInPlace();
		}
		else
		{
			*vTangentNext = vec3_origin;
		}
	}
}

#endif // HOE_TRACK_SPLINE
