//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Core implementation of vgui
//
// $NoKeywords: $
//=============================================================================

#include "vgui_internal.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include "FileSystem.h"
#include <vstdlib/IKeyValuesSystem.h>

#include <stdio.h>

namespace vgui
{

ISurface *g_pSurface = NULL;
ISurface *surface()
{
	return g_pSurface;
}

IFileSystem *g_pFileSystem = NULL;
IFileSystem *filesystem()
{
	return g_pFileSystem;
}

//MODDD - uncommented, modified a bit (original left here)
/*IKeyValues *g_pKeyValues = NULL;
IKeyValues *keyvalues()
{
	return g_pKeyValues;
}*/
IKeyValuesSystem* g_pKeyValues = NULL;
IKeyValuesSystem* keyvalues(){
	return g_pKeyValues;
}






ILocalize *g_pLocalize = NULL;
ILocalize *localize()
{
	return g_pLocalize;
}

IPanel *g_pIPanel = NULL;
IPanel *ipanel()
{
	return g_pIPanel;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void *InitializeInterface( char const *interfaceName, CreateInterfaceFn *factoryList, int numFactories )
{
	void *retval;

	for ( int i = 0; i < numFactories; i++ )
	{
		CreateInterfaceFn factory = factoryList[ i ];
		if ( !factory )
			continue;

		retval = factory( interfaceName, NULL );
		if ( retval )
			return retval;
	}

	// No provider for requested interface!!!
	// assert( !"No provider for requested interface!!!" );

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool VGui_InternalLoadInterfaces( CreateInterfaceFn *factoryList, int numFactories )
{

	/*
		FILE *fp;

		fp = fopen("testarr.txt", "w+");
		fprintf(fp, "This is testing for fprintf...\n");
		fputs("This is testing for fputs...\n", fp);
		fclose(fp);
	*/


	//MODDD - restoring KeyValues.  Because I don't know.
	// loads all the interfaces
	g_pSurface = (ISurface *)InitializeInterface(VGUI_SURFACE_INTERFACE_VERSION, factoryList, numFactories );
	g_pFileSystem = (IFileSystem *)InitializeInterface(FILESYSTEM_INTERFACE_VERSION, factoryList, numFactories );
	//MODDD - is 'IKeyValuesSystem' instead ok?
	// Unforuntately what happens in vstdlib files don't transfer over here, unsure if remaking the IKeyValues class/interface thing
	// (VGUIKeyValues?) is really worth it.  If only internal logic involved that (not hl.exe) we're fine ignoring that.
	//g_pKeyValues = (IKeyValues *)InitializeInterface(KEYVALUES_INTERFACE_VERSION, factoryList, numFactories );
	//g_pKeyValues = (IKeyValuesSystem*)InitializeInterface(KEYVALUES_INTERFACE_VERSION, factoryList, numFactories);

	g_pLocalize = (ILocalize *)InitializeInterface(VGUI_LOCALIZE_INTERFACE_VERSION, factoryList, numFactories );
	g_pIPanel = (IPanel *)InitializeInterface(VGUI_PANEL_INTERFACE_VERSION, factoryList, numFactories );

	if (g_pSurface && g_pFileSystem /*&& g_pKeyValues*/ && g_pLocalize && g_pIPanel)
		return true;

	return false;
}

} // namespace vgui


//MODDD - beware of including at the same time as vstdlib.dll!
// (more of a test to see if the game ever grabs onto either of these)
// Already in vstdlib\KeyValuesSystem.cpp
// (TODO: if ever called upon, might want to implement some CVguiKeyValues, see
//  decomp'd vgui2.so for its vtable, might be similar to that KeyValuesSystem,
//  confusing...)
///////////////////////////////////////////////////////////////////////////////////
static void* __CreateCVGuiKeyValuesIKeyValues_interface() {
	//return (IKeyValuesSystem *)&g_KeyValuesSystem;
	return NULL;
}
static InterfaceReg __g_CreateCVGuiKeyValuesIKeyValuesSystem_reg(__CreateCVGuiKeyValuesIKeyValues_interface, KEYVALUES_INTERFACE_VERSION);

//EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CKeyValuesSystem, IKeyValuesSystem, KEYVALUES_INTERFACE_VERSION, g_KeyValuesSystem);
static void* __CreateCKeyValuesSystemIKeyValuesSystem_interface() {
	//return (IKeyValuesSystem *)&g_KeyValuesSystem;
	return NULL;
}
static InterfaceReg __g_CreateCKeyValuesSystemIKeyValuesSystem_reg(__CreateCKeyValuesSystemIKeyValuesSystem_interface, KEYVALUES_INTERFACE_VERSION);
///////////////////////////////////////////////////////////////////////////////////

