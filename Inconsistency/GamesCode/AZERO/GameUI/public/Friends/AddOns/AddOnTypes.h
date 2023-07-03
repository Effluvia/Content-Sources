//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
//=============================================================================

#ifndef ADDONTYPES_H
#define ADDONTYPES_H
#pragma once

#ifndef WIN32
  typedef unsigned long long SessionInt64;
#else
  typedef unsigned __int64 SessionInt64;
#endif

#endif // ADDONTYPES_H

