//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "VPanelWrapper.h"
//MODDD - VPanelWrapper class moved to VPanelWrapper.h



VPanelWrapper::VPanelWrapper(void){

}
VPanelWrapper::~VPanelWrapper(void){

}


bool VPanelWrapper::IsProportional(VPANEL vguiPanel){
	return FALSE;
}
bool VPanelWrapper::IsAutoDeleteSet(VPANEL vguiPanel){
	return FALSE;
}
void VPanelWrapper::DeletePanel(VPANEL vguiPanel){

}

bool VPanelWrapper::RequestFocusPrev(VPANEL vguiPanel, VPANEL existingPanel){
	return FALSE;
}
bool VPanelWrapper::RequestFocusNext(VPANEL vguiPanel, VPANEL existingPanel){
	return FALSE;
}

int VPanelWrapper::GetTabPosition(VPANEL vguiPanel){
	return 0;
}
bool VPanelWrapper::IsEnabled(VPANEL vguiPanel){
	return true;
}
void VPanelWrapper::SetEnabled(VPANEL vguiPanel, bool state){
	// ???
}

const char* VPanelWrapper::LOCAL_GetClassName(VPANEL vguiPanel){
	return ((VPanel *)vguiPanel)->LOCAL_GetClassName();
}


//MODDD - expanded
//EXPOSE_SINGLE_INTERFACE(VPanelWrapper, IPanel, VGUI_PANEL_INTERFACE_VERSION);

//MODDD - lost the 'static'
VPanelWrapper __g_VPanelWrapper_singleton;
//EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VPanelWrapper, IPanel, VGUI_PANEL_INTERFACE_VERSION, __g_VPanelWrapper_singleton)
static void* __CreateVPanelWrapperIPanel_interface() {
	return (IPanel *)&__g_VPanelWrapper_singleton;
}
static InterfaceReg __g_CreateVPanelWrapperIPanel_reg(__CreateVPanelWrapperIPanel_interface, VGUI_PANEL_INTERFACE_VERSION);

