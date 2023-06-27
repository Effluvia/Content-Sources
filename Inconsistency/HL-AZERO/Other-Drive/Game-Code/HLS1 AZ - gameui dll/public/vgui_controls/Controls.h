//========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
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

#ifndef CONTROLS_H
#define CONTROLS_H

#ifdef _WIN32
#pragma once
#endif

//#include <vgui/VGUI.h>

#include "interface.h"

class IFileSystem;
class IKeyValues;
class KeyValues;




namespace vgui
{


// handles the initialization of the vgui interfaces
// interfaces (listed below) are first attempted to be loaded from primaryProvider, then secondaryProvider
// moduleName should be the name of the module that this instance of the vgui_controls has been compiled into
bool VGui_InitInterfacesList( const char *moduleName, CreateInterfaceFn *factoryList, int numFactories );

// returns the name of the module as specified above
/*const*/ char *GetControlsModuleName();

// set of accessor functions to vgui interfaces
// the appropriate header file for each is listed above the item

// #include <vgui/IPanel.h>
class IPanel *ipanel();

// #include <vgui/IInput.h>
class IInput *input();

// #include <vgui/IScheme.h>
class ISchemeManager *scheme();

// #include <vgui/ISurface.h>
class ISurface *surface();

// #include <vgui/ISystem.h>
class ISystem *system();

// #include <vgui/IVGui.h>
class IVGui *ivgui();

// #include <vgui/ILocalize.h>
class ILocalize *localize();

// #include "FileSystem.h"
IFileSystem *filesystem();

//MODDD - Beware!  IKeyValues.h does not exist.  Unsure if remaking that is necessary.
// There is already vstdlib/KeyValuesSystem.cpp and its interface type (IKeyValuesSystem)
// Oh.  Not really compatible.  Oh well, nowhere I know of ever referred to 'keyvalues()' anyway.
// #include "IKeyValues.h"
//IKeyValues *keyvalues();

// predeclare all the vgui control class names
class AnimatingImagePanel;
class AnimationController;
class BuildModeDialog;
class Button;
class CheckButton;
class ComboBox;
class Divider;
class EditablePanel;
class FileOpenDialog;
class Frame;
class GraphPanel;
class HTML;
class ImagePanel;
class Label;
class LabelComboBox;
class ListPanel;
class Menu;
class MenuBar;
class MenuButton;
class MenuItem;
class MessageBox;
class Panel;
class PanelListPanel;
class ProgressBar;
class ProgressBox;
class PropertyDialog;
class PropertyPage;
class PropertySheet;
class QueryBox;
class RadioButton;
class RichText;
class ScrollBar;
class ScrollBarSlider;
class SectionedListPanel;
class Slider;
class TextEntry;
class ToggleButton;
class Tooltip;
class URLTextEntry;
class WizardPanel;
class WizardSubPanel;

// vgui controls helper classes
class BuildGroup;
class FocusNavGroup;
class IBorder;
class IImage;
class Image;
class ImageList;
class TextImage;

// vgui enumerations
enum KeyCode;
enum MouseCode;

} // namespace vgui

// hotkeys disabled until we work out exactly how we want to do them
#define VGUI_HOTKEYS_ENABLED
// #define VGUI_DRAW_HOTKEYS_ENABLED

#endif // CONTROLS_H
