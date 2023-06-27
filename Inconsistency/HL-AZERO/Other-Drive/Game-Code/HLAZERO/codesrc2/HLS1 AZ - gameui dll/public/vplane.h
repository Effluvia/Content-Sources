//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef VPLANE_H
#define VPLANE_H
#pragma once

#include "vector.h"

typedef int SideType;

// Used to represent sides of things like planes.
#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2

#define VP_EPSILON	0.01f


class VPlane
{
public:

				VPlane();
				VPlane(const Vector &vNormal, vec_t dist);

	void		Init(const Vector &vNormal, vec_t dist);

	// Return the distance from the point to the plane.
	vec_t		DistTo(const Vector &vVec) const;

	// Flip the plane.
	VPlane		Flip();

	// Get a point on the plane (normal*dist).
	Vector		GetPointOnPlane() const;

	// Copy.
	VPlane&		operator=(const VPlane &thePlane);

	// Snap the specified point to the plane (along the plane's normal).
	Vector		SnapPointToPlane(const Vector &vPoint) const;

	// Returns SIDE_ON, SIDE_FRONT, or SIDE_BACK.
	// The epsilon for SIDE_ON can be passed in.
	SideType	GetPointSide(const Vector &vPoint, vec_t sideEpsilon=VP_EPSILON) const;

	// Returns SIDE_FRONT or SIDE_BACK.
	SideType	GetPointSideExact(const Vector &vPoint) const;

	// Classify the box with respect to the plane.
	// Returns SIDE_ON, SIDE_FRONT, or SIDE_BACK
	SideType	BoxOnPlaneSide(const Vector &vMin, const Vector &vMax) const;


public:

	Vector		m_Normal;
	vec_t		m_Dist;
};


// ------------------------------------------------------------------------------------------- //
// Inlines.
// ------------------------------------------------------------------------------------------- //

inline VPlane::VPlane()
{
}

inline VPlane::VPlane(const Vector &vNormal, vec_t dist)
{
	m_Normal = vNormal;
	m_Dist = dist;
}

inline void	VPlane::Init(const Vector &vNormal, vec_t dist)
{
	m_Normal = vNormal;
	m_Dist = dist;
}

inline vec_t VPlane::DistTo(const Vector &vVec) const
{
	return vVec.Dot(m_Normal) - m_Dist;
}

inline VPlane VPlane::Flip()
{
	return VPlane(-m_Normal, -m_Dist);
}

inline Vector VPlane::GetPointOnPlane() const
{
	return m_Normal * m_Dist;
}

inline VPlane& VPlane::operator=(const VPlane &thePlane)
{
	m_Normal = thePlane.m_Normal;
	m_Dist = thePlane.m_Dist;
	return *this;
}

inline Vector VPlane::SnapPointToPlane(const Vector &vPoint) const
{
	return vPoint - m_Normal * DistTo(vPoint);
}

inline SideType VPlane::GetPointSide(const Vector &vPoint, vec_t sideEpsilon) const
{
	vec_t fDist;

	fDist = DistTo(vPoint);
	if(fDist >= sideEpsilon)
		return SIDE_FRONT;
	else if(fDist <= -sideEpsilon)
		return SIDE_BACK;
	else
		return SIDE_ON;
}

inline SideType VPlane::GetPointSideExact(const Vector &vPoint) const
{
	return DistTo(vPoint) > 0.0f ? SIDE_FRONT : SIDE_BACK;
}


// BUGBUG: This should either simply use the implementation in mathlib or cease to exist.
// mathlib implementation is much more efficient.  Check to see that VPlane isn't used in
// performance critical code.
inline SideType VPlane::BoxOnPlaneSide(const Vector &vMin, const Vector &vMax) const
{
	int i, firstSide, side;
	Vector vPoints[8] = 
	{
		Vector(vMin.x, vMin.y, vMin.z),
		Vector(vMin.x, vMin.y, vMax.z),
		Vector(vMin.x, vMax.y, vMax.z),
		Vector(vMin.x, vMax.y, vMin.z),

		Vector(vMax.x, vMin.y, vMin.z),
		Vector(vMax.x, vMin.y, vMax.z),
		Vector(vMax.x, vMax.y, vMax.z),
		Vector(vMax.x, vMax.y, vMin.z),
	};

	firstSide = GetPointSideExact(vPoints[0]);
	for(i=1; i < 8; i++)
	{
		side = GetPointSideExact(vPoints[i]);

		// Does the box cross the plane?
		if(side != firstSide)
			return SIDE_ON;
	}

	// Ok, they're all on the same side, return that.
	return firstSide;
}




#endif // VPLANE_H
