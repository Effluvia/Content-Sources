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

#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE

// SRC only
	#define PROTECTED_THINGS_DISABLE

#include <windows.h>
#include <zmouse.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <oleidl.h>
#include <stdio.h>

#include <basetypes.h>

#include <vgui/VGUI.h>
#include <vgui/Dar.h>
#include <vgui/IClientPanel.h>
#include <vgui/ISurface.h>
#include <vgui/IInputInternal.h>
#include <vgui/IPanel.h>
#include <vgui/ISystem.h>
#include <vgui/ILocalize.h>
#include <vgui/IHTML.h>
#include <vgui/IVGui.h>
#include <vgui/IPanel.h>

#include <vgui/Cursor.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include "vgui_internal.h"
#include "bitmap.h"
#include "VPanel.h"

#include "vgui/htmlwindow.h"

#include "FileSystem.h"
#include "SteamBootStrapper.h"

#include "vgui_surfacelib/Win32Font.h"
#include "vgui_surfacelib/FontManager.h"
#include "vgui_key_translation.h"


/*
// dummy test
void DevMsg( int level, char const* pMsgFormat, ... )
{
	
	if( !IsSpewActive( "developer", level ) )
		return;

	va_list args;
	va_start( args, pMsgFormat );
	_SpewMessage( SPEW_MESSAGE, pMsgFormat, args );
	va_end(args);
	
}
*/







#ifdef PlaySound
#undef PlaySound
#endif

#ifdef CreateFont
#undef CreateFont
#endif

using namespace vgui;

#define PLAT(vpanel) (((VPanel *)vpanel)->Plat())

class Texture;

namespace vgui
{
class SurfacePlat
{
public:
	HWND    hwnd;
	HDC     hdc;
	HDC     hwndDC;
	HDC	  textureDC;
	HGLRC   hglrc;
	HRGN    clipRgn;
	HBITMAP bitmap;
	int     bitmapSize[2];
	int     restoreInfo[4];
	bool    isFullscreen;
	bool	disabled; // whether the window can take user input
	int     fullscreenInfo[3];
	VPanel	*embeddedPanel;
	
	bool    isToolbar;				// whether it has an icon in the tool tray or not
	HICON   notifyIcon;
	VPANEL notifyPanel;

	HPanel lastKeyFocusIndex;		// index to the last panel to have the focus
};
}

//-----------------------------------------------------------------------------
// Purpose: Implementation of ISurface for use running under windows, renders using GDI
//			This is not used in the game, EngineSurface is used instead
//-----------------------------------------------------------------------------
class CWin32Surface : public ISurface
{
public:
	CWin32Surface();
	~CWin32Surface();
	virtual void Shutdown();
	virtual void RunFrame();
	virtual VPANEL GetEmbeddedPanel();
	virtual void SetEmbeddedPanel( VPANEL panel);

	// initializes the surface with the current window state
	virtual void SetCurrentContextPanel(VPANEL panel);
	virtual void PushMakeCurrent(VPANEL panel,bool useInsets);
	virtual void PopMakeCurrent(VPANEL panel);

	virtual void DrawSetColor(int r, int g, int b, int a);
	virtual void DrawSetColor(Color col);
	virtual void DrawFilledRect(int x0, int y0, int x1, int y1);
	virtual void DrawOutlinedRect(int x0, int y0, int x1, int y1);
	virtual void DrawLine(int x0, int y0, int x1, int y1);
	virtual void DrawPolyLine(int *px, int *py, int numPoints);
	virtual void DrawSetTextFont(HFont font);
	virtual void DrawSetTextColor(int r, int g, int b, int a);
	virtual void DrawSetTextColor(Color col);
	virtual void DrawSetTextPos(int x, int y);
	virtual void DrawPrintText(const wchar_t *, int textLen, FontDrawType_t drawType = FONT_DRAW_DEFAULT);
	virtual void DrawUnicodeChar(wchar_t wch, FontDrawType_t drawType = FONT_DRAW_DEFAULT );
	virtual void DrawGetTextPos(int& x,int& y);

	virtual bool DrawGetTextureFile(int id, char *filename, int maxlen );
	virtual int	 DrawGetTextureId( char const *filename );
	virtual void DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload = false);
	virtual void DrawSetTexture(int id);	
	virtual void DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload = false);
	virtual void DrawGetTextureSize(int id, int &wide, int &tall);
	virtual void DrawTexturedRect(int x0, int y0, int x1, int y1);
	virtual int CreateNewTextureID( bool procedural );
	virtual bool IsTextureIDValid(int id);
	virtual void DrawFlushText();
	virtual IHTML *CreateHTMLWindow(vgui::IHTMLEvents *events, VPANEL context);
	virtual void PaintHTMLWindow(IHTML *htmlwin);
	virtual void DeleteHTMLWindow(IHTML *htmlwin);

	virtual VPANEL GetNotifyPanel();
	virtual void SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char *text);

	virtual void GetScreenSize(int &wide, int &tall);

	virtual void SetAsTopMost(VPANEL panel, bool state);
	virtual void BringToFront(VPANEL panel);
	virtual void SetForegroundWindow(VPANEL panel);
	virtual void SetPanelVisible(VPANEL panel, bool visible);
	virtual void SetMinimized(VPANEL panel, bool state);
	virtual bool IsMinimized(VPANEL panel);
	virtual void FlashWindow(VPANEL panel, bool state);
	virtual void SetTitle(VPANEL panel, const wchar_t *title);
	virtual void SetAsToolBar(VPANEL panel, bool state);
	virtual bool SupportsFeature(SurfaceFeature_e feature);
	virtual void SetTopLevelFocus(VPANEL panel);

	virtual int GetPopupCount();
	virtual VPANEL GetPopup(int index);
	virtual void AddPanel(VPANEL panel);
	virtual void ReleasePanel(VPANEL panel);
	virtual void CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon, bool disabled, bool mouseInput, bool kbInput );
	virtual bool RecreateContext(VPANEL panel);
	virtual void EnableMouseCapture(VPANEL panel, bool state);
	virtual bool ShouldPaintChildPanel(VPANEL childPanel);
	virtual void MovePopupToFront(VPANEL panel);
	virtual void MovePopupToBack(VPANEL panel);
	virtual void SwapBuffers(VPANEL panel);
	virtual void Invalidate(VPANEL panel);
	virtual void SetCursor(HCursor cursor);
	virtual void ApplyChanges();
	virtual bool IsWithin(int x, int y);
	virtual bool HasFocus();
	virtual void GetWorkspaceBounds(int &x, int &y, int &wide, int &tall);
	virtual void SolveTraverse(VPANEL panel, bool forceApplySchemeSettings);
	virtual void PaintTraverse(VPANEL panel);

	virtual void RestrictPaintToSinglePanel(VPANEL panel);
	virtual void SetModalPanel(VPANEL );
	virtual VPANEL GetModalPanel();
	virtual void UnlockCursor();
	virtual void LockCursor();
	virtual void SetTranslateExtendedKeys(bool state);
	virtual VPANEL GetTopmostPopup();


	// sound
	virtual void PlaySound(const char *fileName);

	// fonts
	virtual HFont CreateFont();
	virtual bool AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange);
	virtual int GetFontTall(HFont font);
	virtual void GetCharABCwide(HFont font, int ch, int &a, int &b, int &c);
	virtual int GetCharacterWidth(HFont font, int ch);
	virtual void GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall);
	virtual bool AddCustomFontFile(const char *fontFileName);
	virtual bool IsFontAdditive(HFont font);

	virtual bool IsCursorVisible() { return true; }
	
	// gets the absolute coordinates of the screen (in screen space)
	void GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall);

	// methods
	void setFocus(VPANEL panel);

	int GetHTMLWindowCount() { return _htmlWindows.GetCount(); }
	HtmlWindow *GetHTMLWindow(int i) { return _htmlWindows[i]; }


	void GetProportionalBase( int &width, int &height ) { width = BASE_WIDTH; height = BASE_HEIGHT; }

	virtual void CalculateMouseVisible();
	virtual bool NeedKBInput();

	// we use the default IInput cursor functions
	virtual bool HasCursorPosFunctions() { return false; }
	virtual void SurfaceGetCursorPos(int &x, int &y) {}
	virtual void SurfaceSetCursorPos(int x, int y){}

		// SRC specific interfaces
	virtual void DrawTexturedLine( const Vertex_t &a, const Vertex_t &b );
	virtual void DrawOutlinedCircle(int x, int y, int radius, int segments) ;
	virtual void DrawTexturedPolyLine( const Vertex_t *p,int n ) ; // (Note: this connects the first and last points).
	virtual void DrawTexturedSubRect( int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1 );
	virtual void DrawTexturedPolygon(int n, Vertex_t *pVertices);
	virtual const wchar_t *GetTitle(VPANEL panel);
	virtual void LockCursor( bool state );
	virtual bool IsCursorLocked( void ) const;
	virtual void SetWorkspaceInsets( int left, int top, int right, int bottom );
	// Lower level char drawing code, call DrawGet then pass in info to DrawRender (NOT SUPPORTED BY DEFAULT SURFACE )!!!
	virtual bool DrawGetUnicodeCharRenderInfo( wchar_t ch, CharRenderInfo& info );
	virtual void DrawRenderCharFromInfo( const CharRenderInfo& info );


		// Here's where the app systems get to learn about each other 
	virtual bool Connect( CreateInterfaceFn factory );
	virtual void Disconnect();

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void *QueryInterface( const char *pInterfaceName );

	// Init, shutdown
	virtual InitReturnVal_t Init();
	//virtual void Shutdown();
	


private:

	enum { BASE_HEIGHT = 480, BASE_WIDTH = 640 };

	bool LoadTGA(Texture *texture, const char *filename);
	bool LoadBMP(Texture *texture, const char *filename);

	void InternalThinkTraverse(VPANEL panel);
	void InternalSolveTraverse(VPANEL panel);
	void InternalSchemeSettingsTraverse(VPANEL panel, bool forceApplySchemeSettings);

	VPANEL GetContextPanelForChildPanel(VPANEL childPanel);
	void initStaticData();

	// sets the current line drawing color, called by DrawSetTextColor
	void SetLineColor(Color col);

	VPANEL _embeddedPanel;				// main panel
	VPANEL _notifyPanel;
	VPANEL _currentContextPanel;
	HTexture _notifyIcon;
	Dar<VPANEL> _popupList;			// list of panels that have their own win32 window
	HICON _currentCursor;

	Dar<HtmlWindow *> _htmlWindows;

	OSVERSIONINFO m_WindowsVersion;
	bool m_bSupportsUnicode;

	// current font info
	HFont m_hCurrentFont;
	CWin32Font *m_pActiveFont;
	HPEN pen; // the pen used to draw lines

	bool _needKB;
	bool _needMouse;
	
	int		m_TextPos[2];
};


CWin32Surface g_Surface;

//MODDD - expanded
//EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CWin32Surface, ISurface, VGUI_SURFACE_INTERFACE_VERSION, g_Surface);
static void* __CreateCWin32SurfaceISurface_interface(){
	return (ISurface *)&g_Surface;
}
static InterfaceReg __g_CreateCWin32SurfaceISurface_reg(__CreateCWin32SurfaceISurface_interface, VGUI_SURFACE_INTERFACE_VERSION);




class Texture
{
public:
	int     _id;
	HBITMAP _bitmap;
	HBITMAP _maskBitmap;
	bool    _bMask;
	HICON	_icon;
	int     _wide;
	int     _tall;
	void   *_dib;
	void   *_maskDib;
	const char *_filename;
};

//!! these defines duplicated in Surface_Win32.cpp
#define WM_MY_TRAY_NOTIFICATION (WM_USER+1)

static UINT staticShutdownMsg = 0;
static HICON staticDefaultCursor[20];
static WNDCLASS staticWndclass = { NULL };
static ATOM staticWndclassAtom = 0;
static bool staticStaticDataInitialized = false;
//MODDD - why no 'bool' ???
static bool staticSurfaceAvailable;

// these functions defined below
LRESULT CALLBACK staticProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
void staticNotifyIconProc(HWND hwnd, WPARAM wparam, LPARAM lparam);

#ifdef DEBUG_TIMING

#define START_TIMER() \
float paintTime;\
static LARGE_INTEGER ticksPerSecond = { 0 };\
if (!ticksPerSecond.QuadPart)\
{\
	QueryPerformanceFrequency(&ticksPerSecond);\
}\
LARGE_INTEGER Start, end;\
QueryPerformanceCounter(&Start);

#define END_TIMER(x) \
QueryPerformanceCounter(&end);\
paintTime = (float)(((end.QuadPart - Start.QuadPart) * 1000000) / ticksPerSecond.QuadPart) / 1000;\
if (paintTime > 1.0f) ivgui()->DPrintf2(x, paintTime);


#else
#define START_TIMER()
#define END_TIMER(x)

#endif // DEBUG_TIMING

//-----------------------------------------------------------------------------
// Purpose: Handles drag and drop
//-----------------------------------------------------------------------------
class CSurfaceDragDropTarget : public IDropTarget
{
public:
	CSurfaceDragDropTarget()
	{
		_refCount = 0;
		_dragData = NULL;
		OleInitialize(NULL);
	}

private:
	unsigned int _refCount;
	KeyValues *_dragData;

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if (riid == IID_IDropTarget)
		{
			*ppvObject = (IDropTarget *)this;
			return S_OK;
		}

		return E_NOINTERFACE;
	}
    
    virtual ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return ++_refCount;
	}
    
    virtual ULONG STDMETHODCALLTYPE Release( void)
	{
		return --_refCount;
	}

	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		if (_dragData)
		{
			_dragData->deleteThis();
		}
		_dragData = calculateData(pDataObject);
		return DragOver(grfKeyState, pt, pdwEffect);
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_NONE;

		if (!_dragData || !ivgui()->IsRunning())
			return S_OK;

		// get the panel the mouse is over
		VPANEL mouseOver = input()->GetMouseOver();
		if (mouseOver)
		{
			// check to see if the panel will accept this message
			if (((VPanel *)mouseOver)->Client()->RequestInfo(_dragData))
			{
				*pdwEffect = DROPEFFECT_COPY;
			}
		}

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave()
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		*pdwEffect = DROPEFFECT_NONE;

		if (!_dragData || !ivgui()->IsRunning())
			return S_OK;


		// get the panel the mouse is over
		VPANEL target = (VPANEL)_dragData->GetPtr("AcceptPanel");
		if (target && _dragData)
		{
			// check to see if the panel will accept this message
			ivgui()->PostMessage(target, _dragData, NULL, 0);
			_dragData = NULL;
		}

		if (_dragData)
		{
			_dragData->deleteThis();
		}
		_dragData = NULL;

		return S_OK;
	}

	// internal methods
	virtual KeyValues *calculateData(IDataObject *pDataObject)
	{
		KeyValues *dragData = NULL;

		// check on the type of data
		FORMATETC format =
		{
			CF_TEXT,
			NULL,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
    		};
		STGMEDIUM storage;

		if (pDataObject->GetData(&format, &storage) == S_OK)
		{
			// we got some data
			if (storage.tymed == TYMED_HGLOBAL)
			{
				const char *buf = (const char *)GlobalLock(storage.hGlobal);
				dragData = new KeyValues("DragDrop", "type", "text", "text", buf);
				GlobalUnlock(storage.hGlobal);
			}

			ReleaseStgMedium(&storage);
		}
		else
		{
			// try getting a file
			format.cfFormat = CF_HDROP;
			if (pDataObject->GetData(&format, &storage) == S_OK && storage.tymed == TYMED_HGLOBAL)
			{
				dragData = new KeyValues("DragDrop", "type", "files");
				KeyValues *fileList = dragData->FindKey("list", true);

				// parse out the file list
				HDROP hdrop = (HDROP)GlobalLock(storage.hGlobal);
				char namebuf[32], buf[512];
				int count = DragQueryFile(hdrop, 0xFFFFFFFF, buf, 511);
				for (int i = 0; i < count; i++)
				{
					sprintf(namebuf, "%d", i);
					DragQueryFile(hdrop, i, buf, 511);
					fileList->SetString(namebuf, buf);
				}
				GlobalUnlock(storage.hGlobal);
			}
		}

		return dragData;
	}
};

static CSurfaceDragDropTarget staticDragDropTarget;


#define MAXTEXTURECOUNT 256
static int      staticTextureCount = 0;
static Texture  staticTexture[MAXTEXTURECOUNT];
static Texture *staticTextureCurrent = NULL;

static Texture *staticGetTextureById(int id)
{
	if(staticTextureCurrent!=null)
	{
		if(staticTextureCurrent->_id==id)
		{
			return staticTextureCurrent;
		}
	}
	
	for(int i=0;i<staticTextureCount;i++)
	{
		if(staticTexture[i]._id==id)
		{
			return &staticTexture[i];
		}
	}
	return null;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void staticGenerateIconForTexture(Texture *texture, HDC hdc)
{

	return;

	// see if there is an iconic version of the texture file first
	char buf[256];
	sprintf(buf, "%s.ico", texture->_filename);
	texture->_icon = (HICON)::LoadImage(NULL, buf, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
	if (texture->_icon)
		return;

	if (texture->_wide > 32 || texture->_tall > 32)
		return;

	// generate an icon from the tga

	// generate the AND plane based on the alpha
	uchar planeAND[32 * 32 / 8];

	/* !! disabled until verified
	//
	// from the alpha of the bitmap, generate a bitmask
	//
	uchar *ptr = (uchar *)texture->_dib + 3;	// jump to alpha portion
	uchar *ptrEnd = (uchar *)texture->_dib + (texture->_wide * texture->_tall * 4);
	uchar *andPtr = planeAND;
	while (ptr < ptrEnd)
	{
		// Start all empty
		*andPtr = 0;

		// assume the number of pixel is a multiple of 8
		// if the alpha is high enough, then mask out the pixel

		// it's two bits per pixel, strangely enough
		if (*ptr > 127)	{ *andPtr |= 0x03; }	ptr += 4;
		if (*ptr > 127)	{ *andPtr |= 0x0C; }	ptr += 4;
		if (*ptr > 127)	{ *andPtr |= 0x30; }	ptr += 4;
		if (*ptr > 127)	{ *andPtr |= 0xC0; }	ptr += 4;

		andPtr++;
	}
	*/

	memset(planeAND, 0x00, sizeof(planeAND));
	HBITMAP mask = ::CreateBitmap(texture->_wide, texture->_tall, 1, 1, planeAND);

	// create the icon
	ICONINFO iconInfo;
	iconInfo.fIcon = TRUE;
	iconInfo.xHotspot = 8;
	iconInfo.yHotspot = 8;
	iconInfo.hbmMask = mask;
	iconInfo.hbmColor = texture->_bitmap;

	texture->_icon = ::CreateIconIndirect(&iconInfo);

	::DeleteObject(mask);
}


//-----------------------------------------------------------------------------
// Purpose: Constructor, basic variable initialization
//-----------------------------------------------------------------------------
CWin32Surface::CWin32Surface()
{
	_currentCursor = NULL;

	initStaticData();

	staticSurfaceAvailable = false;

	m_hCurrentFont = 0;

	pen = NULL;

	_needKB = true;
	_needMouse = true;

	HRESULT hr;

	// this step is IMPORTANT , it turns "on" COM
	if (FAILED(hr = CoInitialize(NULL)))
	{
		// failed
	}

	// get our version info
	m_WindowsVersion.dwOSVersionInfoSize = sizeof(m_WindowsVersion);
	GetVersionEx(&m_WindowsVersion);
	if (m_WindowsVersion.dwMajorVersion >= 5)
	{
		m_bSupportsUnicode = true;
	}
	else
	{
		m_bSupportsUnicode = false;
	}

	m_TextPos[0] = m_TextPos[1] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CWin32Surface::~CWin32Surface()
{
	// free all the textures
	for (int i = 0; i < staticTextureCount; i++)
	{
		Texture *texture = &staticTexture[i];

		if (texture->_bitmap)
		{
			::DeleteObject(texture->_bitmap);
		}

		if (texture->_maskBitmap)
		{
			::DeleteObject(texture->_maskBitmap);
		}

		if (texture->_icon)
		{
			::DestroyIcon(texture->_icon);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shuts down app
//-----------------------------------------------------------------------------
void CWin32Surface::Shutdown()
{
	staticWndclass.lpfnWndProc = NULL;

	// save user config
	system()->SaveUserConfigFile();

	// free the fonts
	FontManager().ClearAllFonts();

	// ensure we don't try and process windows messages during Shutdown
	staticSurfaceAvailable = false;

	// release any panels still with surfaces
	while (GetPopupCount())
	{
		ReleasePanel(GetPopup(0));
	}

	// kill our windows instance
	::UnregisterClass("Surface", ::GetModuleHandle(NULL));
	staticWndclassAtom = NULL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
VPANEL CWin32Surface::GetEmbeddedPanel()
{
	return _embeddedPanel;
}

	// SRC specific interfaces
 void CWin32Surface::DrawTexturedLine( const Vertex_t &a, const Vertex_t &b )
 {

 }
 void CWin32Surface::DrawOutlinedCircle(int x, int y, int radius, int segments) 
 {

 }
 void CWin32Surface::DrawTexturedPolyLine( const Vertex_t *p,int n )
 {
 }
 void CWin32Surface::DrawTexturedSubRect( int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1 )
 {
 }
 void CWin32Surface::DrawTexturedPolygon(int n, Vertex_t *pVertices)
 {
 }

 const wchar_t *CWin32Surface::GetTitle(VPANEL panel)
 {
	return L"";
 }
 void CWin32Surface::LockCursor( bool state )
 {
 }
 bool CWin32Surface::IsCursorLocked( void ) const
 {
	 return false;
 }
 void CWin32Surface::SetWorkspaceInsets( int left, int top, int right, int bottom )
 {
 }


 
 //-----------------------------------------------------------------------------
// Connect, disconnect...
//-----------------------------------------------------------------------------
bool CWin32Surface::Connect( CreateInterfaceFn factory )
{	
return true;
}

void CWin32Surface::Disconnect()
{
}


//-----------------------------------------------------------------------------
// Access to other interfaces...
//-----------------------------------------------------------------------------
void *CWin32Surface::QueryInterface( const char *pInterfaceName )
{
	return NULL;
}


//-----------------------------------------------------------------------------
// Initialization and shutdown...
//-----------------------------------------------------------------------------
InitReturnVal_t CWin32Surface::Init( void )
{
	// :L    ???????????????
	return INIT_FAILED;
}



//-----------------------------------------------------------------------------
// Purpose: Sets up the panel for use
// Input  : *embeddedPanel - Main panel that becomes the top of the hierarchy
//-----------------------------------------------------------------------------
void CWin32Surface::SetEmbeddedPanel( VPANEL panel )
{
	_embeddedPanel = panel;

	staticSurfaceAvailable = true;

	SetCurrentContextPanel(panel);
	CreatePopup(panel, false, true, false, true, true);

}

// Lower level char drawing code, call DrawGet then pass in info to DrawRender
bool CWin32Surface::DrawGetUnicodeCharRenderInfo( wchar_t ch, CharRenderInfo& info )
{
	// Only supported in engine renderer!
	Assert( 0 );
	info.valid = false;
	return false;
}

void CWin32Surface::DrawRenderCharFromInfo( const CharRenderInfo& info )
{
	Assert( 0 );
}


//-----------------------------------------------------------------------------
// Purpose: Sets the current drawing context
//-----------------------------------------------------------------------------
void CWin32Surface::SetCurrentContextPanel(VPANEL panel)
{
	_currentContextPanel = panel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
VPANEL CWin32Surface::GetContextPanelForChildPanel(VPANEL childPanel)
{
	VPANEL contextPanel = childPanel;
	while (contextPanel && !PLAT(contextPanel))
	{
		contextPanel = (VPANEL)((VPanel *)contextPanel)->GetParent();
	}
	return contextPanel;
}

// static currently used win32 Paint info
static ::PAINTSTRUCT s_CurrentPaintStruct;

void CWin32Surface::PushMakeCurrent(VPANEL panel, bool useInsets)
{
	// find the win32 window context from the hierarchy
	VPANEL currentContextPanel = GetContextPanelForChildPanel(panel);

	SetCurrentContextPanel(currentContextPanel);

	// clear the current active font so that it will be reset
	m_pActiveFont = NULL;
	
	if (!currentContextPanel)
	{
		// no drawing context found, something is seriously wrong
		ivgui()->DPrintf( "Warning: VPanel with no drawing context\n" );
	}

	int inset[4];

	//!! need to make the inset part of VPanel
	((VPanel *)panel)->Client()->GetInset(inset[0],inset[1],inset[2],inset[3]);

	if(!useInsets)
	{
		inset[0]=0;
		inset[1]=0;
		inset[2]=0;
		inset[3]=0;
	}

	int absThis[4];
	((VPanel *)_currentContextPanel)->GetAbsPos(absThis[0], absThis[1]);
	((VPanel *)_currentContextPanel)->GetSize(absThis[2], absThis[3]);
	absThis[2] += absThis[0];
	absThis[3] += absThis[1];

	int absPanel[4];
	((VPanel *)panel)->GetAbsPos(absPanel[0], absPanel[1]);
	((VPanel *)panel)->GetSize(absPanel[2], absPanel[3]);
	absPanel[2] += absPanel[0];
	absPanel[3] += absPanel[1];

	int clipRect[4];
	((VPanel *)panel)->Client()->GetClipRect(clipRect[0],clipRect[1],clipRect[2],clipRect[3]);

	if ( _currentContextPanel == panel )
	{
		// this panel has it's own window, so use screen space
		::SetViewportOrgEx(PLAT(_currentContextPanel)->hdc,0+inset[0],0+inset[1],null);
	}
	else
	{
		// child window, so set win32 up so all subsequent drawing calls are done in local space
		::SetViewportOrgEx(PLAT(_currentContextPanel)->hdc,(absPanel[0]+inset[0])-absThis[0],(absPanel[1]+inset[1])-absThis[1],null);
	}

	// setup clipping
	// get and translate clipRect into surface space, then factor in inset
	int x0 = clipRect[0] - absThis[0];
	int y0 = clipRect[1] - absThis[1];
	int x1 = (clipRect[2] - absThis[0]) - inset[2];
	int y1 = (clipRect[3] - absThis[1]) - inset[3];

	//set the rect and select to make it current
	::SetRectRgn(PLAT(_currentContextPanel)->clipRgn,x0,y0,x1,y1);
	::SelectObject(PLAT(_currentContextPanel)->hdc, PLAT(_currentContextPanel)->clipRgn);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::PopMakeCurrent(VPANEL panel)
{
	if (panel == _currentContextPanel)
	{
		// reset the current panel to be the main panel
		SetCurrentContextPanel(_embeddedPanel);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::GetScreenSize(int &wide, int &tall)
{
	VPANEL context = _embeddedPanel;


	if (context)
	{
		SurfacePlat* tempPlat = PLAT(context);
		if(tempPlat){
			wide = ::GetDeviceCaps(PLAT(context)->hdc, HORZRES);
			tall = ::GetDeviceCaps(PLAT(context)->hdc, VERTRES);
		}else{
			// ???????????
			int x = 666;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
VPANEL CWin32Surface::GetNotifyPanel()
{
	return _notifyPanel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::SetNotifyIcon(VPANEL context, HTexture iconID, VPANEL panelToReceiveMessages, const char *text)
{
	context = GetContextPanelForChildPanel(context);
	if (!context)
		return;

	if (!text)
	{
		text = "";
	}

	// things haven't changed so just update the tooltip
	if (_notifyIcon == iconID && _notifyPanel == panelToReceiveMessages && context && PLAT(context)->notifyIcon)
	{
		::NOTIFYICONDATA iconData = 
		{
			sizeof(iconData),
			PLAT(context)->hwnd,
			1,									// icon ID
			NIF_TIP,							// modify tooltip only
			WM_MY_TRAY_NOTIFICATION,			// callback message
			PLAT(context)->notifyIcon,  // icon handle
			"",									// tooltip text
		};

		strncpy(iconData.szTip, text, 63);
		iconData.szTip[63] = '\0';
		::Shell_NotifyIcon(NIM_MODIFY, &iconData);
		return;
	}

	_notifyIcon = iconID;
	_notifyPanel = panelToReceiveMessages;

	DWORD dwMessage = NIM_MODIFY;
	if (!iconID)
	{
		dwMessage = NIM_DELETE;
		PLAT(context)->notifyIcon = NULL;
	}
	else if (!PLAT(context)->notifyIcon)
	{
		dwMessage = NIM_ADD;
	}

	// make sure the icon has been loaded
	Texture *texture = NULL;
	if (iconID)
	{
		texture = staticGetTextureById(iconID);

		if (!texture->_icon)
		{
			// generate an icon for the texture
			staticGenerateIconForTexture(texture, PLAT(_currentContextPanel)->hdc);
		}
	}

	// get the icon
	if (texture)
	{
		PLAT(context)->notifyIcon = texture->_icon;
	}

	::NOTIFYICONDATA iconData = 
	{
		sizeof(iconData),
		PLAT(context)->hwnd,
		1,									// icon ID
		NIF_ICON | NIF_TIP | NIF_MESSAGE,	// items used
		WM_MY_TRAY_NOTIFICATION,			// callback message
		PLAT(context)->notifyIcon,  // icon handle
		"",									// tooltip text
	};

	strncpy(iconData.szTip, text, 63);
	iconData.szTip[63] = '\0';

	BOOL success = ::Shell_NotifyIcon(dwMessage, &iconData);

	if (!success)
	{
		DWORD err = GetLastError();
		ivgui()->DPrintf2("error: SetNotifyIcon(%d) failed\n", err);
	}
}

void CWin32Surface::DrawSetColor(int r,int g,int b,int a)
{

	if(_currentContextPanel == 0){
		VPanel* testRef = (VPanel*)GetEmbeddedPanel();
		int x = 45;
	}

	SetBkColor(PLAT(_currentContextPanel)->hdc,RGB(r,g,b));
}

void CWin32Surface::DrawSetColor(Color col)
{
	DrawSetColor(col[0], col[1], col[2], col[3]);
}

void CWin32Surface::DrawSetTextPos(int x, int y)
{
	MoveToEx(PLAT(_currentContextPanel)->hdc,x,y,null);	
	m_TextPos[0] = x;
	m_TextPos[1] = y;
}

void CWin32Surface::DrawGetTextPos(int& x,int& y)
{
	x = m_TextPos[0];
	y = m_TextPos[1];
}


void CWin32Surface::DrawSetTextFont(HFont font)
{
	//MODDD - assert linker error
	//assert(font);
	
	// make the font current
	m_hCurrentFont = font;

//	m_FontAmalgams[m_hCurrentFont].GetFontForChar('a')->SetAsActiveFont(_currentContextPanel->Plat()->hdc);
}

void CWin32Surface::DrawFilledRect(int x0,int y0,int x1,int y1)
{
	// trick to draw filled rectangles using current background color
	RECT rect = { x0, y0, x1, y1};
	BOOL res = ExtTextOut(PLAT(_currentContextPanel)->hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

void CWin32Surface::DrawOutlinedRect(int x0,int y0,int x1,int y1)
{
	// draw an outline of a rectangle using 4 filledRect
	DrawFilledRect(x0,y0,x1,y0+1);     // top
	DrawFilledRect(x0,y1-1,x1,y1);	   // bottom
	DrawFilledRect(x0,y0+1,x0+1,y1-1); // left
	DrawFilledRect(x1-1,y0+1,x1,y1-1); // right
}

void CWin32Surface::DrawLine(int x0,int y0,int x1,int y1)
{
	POINT pt[2];
	CONST POINT *p=pt;
	pt[0].x=x0;
	pt[0].y=y0;
	pt[1].x=x1;
	pt[1].y=y1;

//	MoveToEx(_currentContextPanel->Plat()->hdc,x0,y0,NULL);
//	LineTo(_currentContextPanel->Plat()->hdc,x1,y1);
	Polyline(PLAT(_currentContextPanel)->hdc,p , 2);
}


void CWin32Surface::DrawPolyLine(int *px, int *py, int numPoints)
{
	POINT *pt;
	
	pt = (POINT *)malloc(sizeof(POINT) * numPoints);
	if(pt) 
	{
		for(int i=0;i<numPoints;i++)
		{
			pt[i].x= px[i];
			pt[i].y= py[i];
		}

		Polyline(PLAT(_currentContextPanel)->hdc, pt , numPoints);
		free(pt);
	}
}

void CWin32Surface::DrawSetTextColor(int r,int g,int b,int a)
{
	//MODDD - how can this happen??
	if(_currentContextPanel == NULL){
		return;
	}
	// set the draw color for lines
	SetLineColor(Color(r,g,b,a));
	SetTextColor(PLAT(_currentContextPanel)->hdc,RGB(r,g,b));
}

void CWin32Surface::DrawSetTextColor(Color col)
{
	// set the draw color for lines
	SetLineColor(col);
	// end then for text
	DrawSetTextColor(col[0], col[1], col[2], col[3]);
}

void CWin32Surface::SetLineColor(Color col)
{
	//MODDD - how can this happen??
	if(_currentContextPanel == NULL){
		return;
	}

	HPEN tmp=pen;
	pen = CreatePen(PS_SOLID,0,RGB(col[0], col[1], col[2]));
	if(pen) 
	{
		SelectObject(PLAT(_currentContextPanel)->hdc, pen ); 
	}
	// you must delete the pen AFTER a new one is selected
	if(tmp) 
	{
		DeleteObject(tmp);
	}
}


void CWin32Surface::DrawPrintText(const wchar_t *text, int textLen, FontDrawType_t drawType /*= FONT_DRAW_DEFAULT*/)
{
	//MODDD - assert linker error
	//assert(text);
	if (!text)
		return;

	if (textLen < 1)
		return;

	for (int i = 0; i < textLen; i++)
	{
		DrawUnicodeChar(text[i]);
	}

//	ExtTextOut(_currentContextPanel->Plat()->hdc, 0, 0, 0, NULL, text, textLen, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: draws single unicode character at the current position with the
//			current font & color
//-----------------------------------------------------------------------------
void CWin32Surface::DrawUnicodeChar(wchar_t wch, FontDrawType_t drawType /*= FONT_DRAW_DEFAULT*/)
{
	// set the current font
	CWin32Font *winFont = FontManager().GetFontForChar(m_hCurrentFont, wch);

	if (!winFont)
		return;

	if (m_pActiveFont != winFont)
	{
		winFont->SetAsActiveFont(PLAT(_currentContextPanel)->hdc);
		m_pActiveFont = winFont;
	}

	//ExtTextOutW(PLAT(_currentContextPanel)->hdc, 0, 0, 0, NULL, &wch, 1, null);
}

//-----------------------------------------------------------------------------
// Purpose: allocates a new texture id
//-----------------------------------------------------------------------------
int CWin32Surface::CreateNewTextureID( bool procedural )
{
	//!! hack, arbitrary base
	static int staticBindIndex = 2700;
	return staticBindIndex++;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the texture id has a valid texture bound to it
//-----------------------------------------------------------------------------
bool CWin32Surface::IsTextureIDValid(int id)
{
	return (staticGetTextureById(id) != NULL);
}

//-----------------------------------------------------------------------------
// Purpose: does nothing, since we don't need this optimization in win32
//-----------------------------------------------------------------------------
void CWin32Surface::DrawFlushText()
{
}

IHTML *CWin32Surface::CreateHTMLWindow(vgui::IHTMLEvents *events, VPANEL context)
{

	// setup the _currentContextPanel 
	VPANEL parent = GetContextPanelForChildPanel(context);
	if (!parent)
		return NULL;

	// now make the control
	HtmlWindow *IE = new HtmlWindow(events, context, PLAT(_currentContextPanel)->hwnd);
	IE->Show(true);

	// add it to our list of controls
	_htmlWindows.AddElement(IE);
	return dynamic_cast<IHTML *>(IE);
}


void CWin32Surface::DeleteHTMLWindow(IHTML *htmlwin)
{
	HtmlWindow *IE = dynamic_cast<HtmlWindow *>(htmlwin);

	if(IE)
	{
		_htmlWindows.RemoveElement(IE);
		delete IE;
	}
}

void CWin32Surface::PaintHTMLWindow(IHTML *htmlwin)
{
	HtmlWindow *IE = dynamic_cast<HtmlWindow *>(htmlwin);

	if(IE)
	{
		int w,h;
		IE->GetSize(w,h);

		//MODDD - assert linker error
		//assert(PLAT(_currentContextPanel)->hdc != NULL);
		IE->OnPaint(PLAT(_currentContextPanel)->hdc);
		//BOOL r = BitBlt(_currentContextPanel->Plat()->hdc, 0, 0, w, h, bit, 0, 0, SRCCOPY);	
		// don't release the bitmap's HDC, its "cached" by the IE control
		//::ReleaseDC(_currentContextPanel->Plat()->hwnd,bit); 
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets the current active texture
//-----------------------------------------------------------------------------
void CWin32Surface::DrawSetTexture(int id)
{
	Texture *texture = staticGetTextureById(id);
	staticTextureCurrent = texture;
}

HBITMAP staticCreateBitmapHandle(int wide, int tall, HDC hdc, int bpp, void **dib);
//-----------------------------------------------------------------------------
// Purpose: maps a texture from memory to an id, and uploads it into the engine
//-----------------------------------------------------------------------------
void CWin32Surface::DrawSetTextureRGBA(int id,const unsigned char* rgba,int wide,int tall, int hardwareFilter, bool forceReload)
{
	Texture *texture = staticGetTextureById(id);

	if (texture)
	{
		if (texture->_bitmap)
		{
			::DeleteObject(texture->_bitmap);
		}

		if (texture->_maskBitmap)
		{
			::DeleteObject(texture->_maskBitmap);
		}
	}
	if (!texture)
	{
		// allocate a new texture
		texture = &staticTexture[staticTextureCount++];
		memset(texture, 0, sizeof(Texture));
	}

	{
		// no texture or forced  load the new texture
		texture->_id = id;
		texture->_filename =NULL;

		texture->_wide = wide;
		texture->_tall = tall;
		texture->_icon = NULL;
		texture->_dib = NULL;
		texture->_bitmap = staticCreateBitmapHandle(texture->_wide, 
			texture->_tall,	PLAT(_currentContextPanel)->hdc, 32, &texture->_dib );

		// copy over the texture data
		memcpy(texture->_dib,rgba,wide*tall*4);

	}

	staticTextureCurrent = texture;

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : id - 
//			*filename - 
//			maxlen - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWin32Surface::DrawGetTextureFile(int id, char *filename, int maxlen )
{
	Texture *texture = staticGetTextureById(id);
	if ( !texture )
		return false;

	Q_strncpy( filename, texture->_filename, maxlen );
	return true;
}

int	  CWin32Surface::DrawGetTextureId( char const *filename )
{
	int i;
	for ( i = 0; i < staticTextureCount; i++ )
	{
		Texture *texture = staticGetTextureById(i);
		if ( !texture )
			continue;

		if ( !Q_stricmp( filename, texture->_filename ) )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Maps a texture file to an id, and makes it the current drawing texture
//			tries to load as a .tga first, and if not found as a .bmp
//-----------------------------------------------------------------------------
void CWin32Surface::DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload /*= false*/)
{
	Texture *texture = staticGetTextureById(id);
	
	if (!texture || stricmp(filename, texture->_filename) || forceReload )
	{
		// no texture, or the filename is different;  load the new texture
		if (!texture)
		{
			// allocate a new texture
			texture = &staticTexture[staticTextureCount++];
			memset(texture, 0, sizeof(Texture));
		}
		if (texture)
		{
			if (texture->_bitmap)
			{
				::DeleteObject(texture->_bitmap);
			}

			if (texture->_maskBitmap)
			{
				::DeleteObject(texture->_maskBitmap);
			}
		}
		texture->_id = id;
		texture->_filename = filename;

		// try and find the file
		bool success = LoadTGA(texture, filename);

		if (!success)
		{
			ivgui()->DPrintf2("Error: texture file '%s' does not exist or is invalid\n", filename);
			return;
		}
	}

	staticTextureCurrent = texture;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::DrawTexturedRect(int x0,int y0,int x1,int y1)
{
	if (staticTextureCurrent == null)
	{
		return;
	}

	if (PLAT(_currentContextPanel)->textureDC == null)
	{
		return;
	}

	HBITMAP bitmap = staticTextureCurrent->_bitmap;
	int wide = staticTextureCurrent->_wide;
	int tall = staticTextureCurrent->_tall;

	HGDIOBJ oldObject;

	if (staticTextureCurrent->_bMask)
	{
		HBITMAP bitmap_mask = staticTextureCurrent->_maskBitmap;

		// draw the mask first to clear out the background to black 
		oldObject = ::SelectObject(PLAT(_currentContextPanel)->textureDC, bitmap_mask);
		::StretchBlt(PLAT(_currentContextPanel)->hdc,x0,y0,x1-x0,y1-y0,PLAT(_currentContextPanel)->textureDC,0,0,wide,tall,SRCAND);

		// draw over the 'black areas' with the bitmap
		::SelectObject(PLAT(_currentContextPanel)->textureDC, bitmap);
		::StretchBlt(PLAT(_currentContextPanel)->hdc,x0,y0,x1-x0,y1-y0,PLAT(_currentContextPanel)->textureDC,0,0,wide,tall,SRCPAINT);
	}
	else
	{
		oldObject = ::SelectObject(PLAT(_currentContextPanel)->textureDC, bitmap);
		::StretchBlt(PLAT(_currentContextPanel)->hdc,x0,y0,x1-x0,y1-y0,PLAT(_currentContextPanel)->textureDC,0,0,wide,tall,SRCCOPY);
	}
	::SelectObject(PLAT(_currentContextPanel)->textureDC, oldObject);

// test code, should be used in win98/win2k for true alpha
//	BLENDFUNCTION blendFunction = { AC_SRC_OVER, 0, 255, 0 };
//	::AlphaBlend(PLAT(_currentContextPanel)->hdc,x0,y0,x1-x0,y1-y0,PLAT(_currentContextPanel)->textureDC,0,0,wide,tall,blendFunction);
}

//-----------------------------------------------------------------------------
// Purpose: Called by vgui to get texture dimensions
//-----------------------------------------------------------------------------
void CWin32Surface::DrawGetTextureSize(int id, int &wide, int &tall)
{
	Texture *texture = staticGetTextureById(id);

	if (!texture)
		return;

	wide = texture->_wide;
	tall = texture->_tall;
}

#pragma pack(1)
typedef struct
{
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} tga_header_t;
#pragma pack()

HBITMAP staticCreateBitmapHandle(int wide, int tall, HDC hdc, int bpp, void **dib)
{
	BITMAPINFOHEADER bitmapInfoHeader;
	memset(&bitmapInfoHeader, 0, sizeof(bitmapInfoHeader));
	bitmapInfoHeader.biSize = sizeof(bitmapInfoHeader);
	bitmapInfoHeader.biWidth = wide;
	bitmapInfoHeader.biHeight = -tall;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = bpp;
	bitmapInfoHeader.biCompression = BI_RGB;

	return ::CreateDIBSection(hdc, (BITMAPINFO*)&bitmapInfoHeader, DIB_RGB_COLORS, dib, null, 0);
}

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWin32Surface::LoadBMP(Texture *texture, const char *filename)
{
	// try load the tga
	static char buf[1024];
	_snprintf(buf, sizeof(buf), "%s.bmp", filename);
	FileHandle_t file = filesystem()->Open(buf, "rb", NULL);
	if (!file)
		return false;

	bool success = false;

	// Parse bitmap
	BITMAPFILEHEADER bmfHeader;
	DWORD dwBitsSize, dwFileSize;
	LPBITMAPINFO lpbmi;

	dwFileSize = filesystem()->Size( file );

	filesystem()->Read( &bmfHeader, sizeof(bmfHeader), file );
	
	if (bmfHeader.bfType == DIB_HEADER_MARKER)
	{
		dwBitsSize = dwFileSize - sizeof(bmfHeader);

		HGLOBAL hDIB = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize );
		char *pDIB = (LPSTR)::GlobalLock((HGLOBAL)hDIB);
		{
			int i, j;

			filesystem()->Read(pDIB, dwBitsSize, file );

			lpbmi = (LPBITMAPINFO)pDIB;

			// we now have a block of memory, rgba
			// throw a bitmap header on it and register it in windows
			texture->_wide = lpbmi->bmiHeader.biWidth;
			texture->_tall = lpbmi->bmiHeader.biHeight;
			texture->_icon = NULL;
			texture->_bMask = false;
			texture->_bitmap = staticCreateBitmapHandle(
				texture->_wide, 
				texture->_tall, 
				PLAT(_currentContextPanel)->hdc, 
				32, &texture->_dib );

			unsigned char *rgba = (unsigned char *)( pDIB + sizeof( BITMAPINFOHEADER ) + 256 * sizeof( RGBQUAD ) );

			// Copy raw data
			for (j = 0; j < texture->_tall; j++)
			{ 
				for (i = 0; i <texture->_wide; i++)
				{
					int y = (texture->_tall - j - 1);

					int offs = ( y * texture->_wide + i);
					int offsdest = (j * texture->_wide + i) * 4;
					unsigned char *src = ((unsigned char *)rgba) + offs;
					char *dst = ((char*)texture->_dib) + offsdest;
					
					dst[0] = lpbmi->bmiColors[ *src ].rgbRed;
					dst[1] = lpbmi->bmiColors[ *src ].rgbGreen;
					dst[2] = lpbmi->bmiColors[ *src ].rgbBlue;
					dst[3] = (unsigned char)255;
				}
			}

			success = true;
		}

		::GlobalUnlock( hDIB);
		::GlobalFree((HGLOBAL) hDIB);
	}

	filesystem()->Close(file);

	return success;


}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWin32Surface::LoadTGA(Texture *texture, const char *filename)
{
	bool invertAlpha = false;

	// try load the tga
	static char buf[1024];
	_snprintf(buf, sizeof(buf), "%s.tga", filename);
	FileHandle_t file = filesystem()->Open(buf, "rb", NULL);
	if (!file)
	{
		return LoadBMP( texture, filename );
	}

	// read the header
	tga_header_t tgaHeader;
	vgui::filesystem()->Read(&tgaHeader, sizeof(tgaHeader), file);		

	if (tgaHeader.image_type != 2 && tgaHeader.image_type != 10)
	{
		ivgui()->DPrintf2("Error: texture file '%s' has invalid image_type %d\n", filename, tgaHeader.image_type);
		return false;
	}

	if (tgaHeader.colormap_type != 0)
	{
		return false;
	}

	if (tgaHeader.pixel_size != 24 && tgaHeader.pixel_size !=32)
	{
		return false;
	}

	if (tgaHeader.id_length != 0)
	{
		vgui::filesystem()->Seek( file, tgaHeader.id_length, FILESYSTEM_SEEK_CURRENT );
	}

	// allocate memory for the destination
	uchar *rgba = (unsigned char *)malloc(tgaHeader.width * tgaHeader.height * 4);

	int column, row;
	uchar *ptr;
	bool bMask = false;

	if (tgaHeader.image_type == 2)
	{
		for (row = tgaHeader.height - 1; row >= 0; row--)
		{
			ptr = ((uchar *)rgba) + (row * tgaHeader.width * 4);
			for (column = 0; column < tgaHeader.width; column++) 
			{
				switch (tgaHeader.pixel_size) 
				{
					case 24:
					{
						vgui::filesystem()->Read(ptr + 2, 1, file);
						vgui::filesystem()->Read(ptr + 1, 1, file);
						vgui::filesystem()->Read(ptr + 0, 1, file);
						ptr[3] = 255;

						if (invertAlpha)
						{
							ptr[3] = 0;
						}
						ptr += 4;
						break;
					}
					case 32:
					{
						vgui::filesystem()->Read(ptr + 2, 1, file);
						vgui::filesystem()->Read(ptr + 1, 1, file);
						vgui::filesystem()->Read(ptr + 0, 1, file);
						vgui::filesystem()->Read(ptr + 3, 1, file);
						
						if (!invertAlpha)
						{
							// if it's 0, then it's going to be zeroed out
							if (ptr[3] == 0)
							{
								ptr[3] = 255;
							}
							else
							{
								// anything besides 0 will be shown
								ptr[3] = 0;
							}
							//ptr[3] = 255 - ptr[3];
						}
						bMask = true;

						ptr += 4;
						break;
					}
				}
			}
		}
	}
	else
	{
		uchar packetHeader, packetSize, j, color[4];

		for(row = tgaHeader.height - 1; row >= 0; row--)
		{
			ptr = ((uchar*)rgba) + row * tgaHeader.width * 4;
			for(column=0;column<tgaHeader.width;) 
			{
				vgui::filesystem()->Read(&packetHeader, 1, file);
				packetSize = 1 + (packetHeader & 0x7f);

				if (packetHeader & 0x80)
				{
					switch (tgaHeader.pixel_size)
					{
						case 24:
						{
							vgui::filesystem()->Read(color + 2, 1, file);
							vgui::filesystem()->Read(color + 1, 1, file);
							vgui::filesystem()->Read(color + 0, 1, file);

							if (invertAlpha)
							{
								color[3] = 0;
							}
							else
							{
								color[3] = 255;
							}

							break;
						}
						case 32:
						{
							vgui::filesystem()->Read(color + 2, 1, file);
							vgui::filesystem()->Read(color + 1, 1, file);
							vgui::filesystem()->Read(color + 0, 1, file);
							vgui::filesystem()->Read(color + 3, 1, file);

							bMask = true;
							if (invertAlpha)
							{
								color[3] = 255 - color[3];
							}

							break;
						}
					}
					
					for (j = 0; j < packetSize; j++)
					{
						ptr[0] = color[0];
						ptr[1] = color[1];
						ptr[2] = color[2];
						ptr[3] = color[3];
						ptr += 4;
						column++;
						if (column == tgaHeader.width)
						{
							column = 0;
							if (row > 0)
							{
								row--;
							}
							else
							{
								goto breakOut;
							}
							ptr = ((uchar*)rgba) + (row * tgaHeader.width * 4);
						}
					}
				}
				else
				{
					for (j = 0; j < packetSize; j++) 
					{
						switch (tgaHeader.pixel_size)
						{
							case 24:
							{
								vgui::filesystem()->Read(ptr + 2, 1, file);
								vgui::filesystem()->Read(ptr + 1, 1, file);
								vgui::filesystem()->Read(ptr + 0, 1, file);

								ptr[3] = 255;

								if (invertAlpha)
								{
									ptr[3] = 0;
								}
								else
								{
									ptr[3] = 255;
								}

								ptr += 4;
								break;
							}
							case 32:
							{
								vgui::filesystem()->Read(ptr + 2, 1, file);
								vgui::filesystem()->Read(ptr + 1, 1, file);
								vgui::filesystem()->Read(ptr + 0, 1, file);
								vgui::filesystem()->Read(ptr + 3, 1, file);

								if (invertAlpha)
								{
									ptr[3]=255-ptr[3];
								}

								ptr += 4;
								break;
							}
						}
						column++;
						if (column == tgaHeader.width)
						{
							column = 0;
							if(row > 0)
							{
								row--;
							}
							else
							{
								goto breakOut;
							}
							ptr = ((uchar*)rgba) + row * tgaHeader.width * 4;
						}
					}
				}
			}
		}
		breakOut:;
	}

	vgui::filesystem()->Close(file);


	// we now have a block of memory, rgba
	// throw a bitmap header on it and register it in windows
	texture->_wide = tgaHeader.width;
	texture->_tall = tgaHeader.height;
	texture->_icon = NULL;
	texture->_bitmap = staticCreateBitmapHandle(tgaHeader.width, tgaHeader.height, PLAT(_currentContextPanel)->hdc, 32, &texture->_dib);
	texture->_bMask = bMask;
	if (bMask)
		texture->_maskBitmap = staticCreateBitmapHandle(tgaHeader.width, tgaHeader.height, PLAT(_currentContextPanel)->hdc, 32, &texture->_maskDib);
	else
		texture->_maskBitmap = NULL;

	for (int j = 0; j < texture->_tall; j++)
	{
		for (int i = 0; i < texture->_wide; i++)
		{
			int offs = (j * texture->_wide + i) * 4;
			char *src = ((char*)rgba) + offs;
			char *dst = ((char*)texture->_dib) + offs;
			
			if (bMask)
			{
				char *maskDst = ((char*) texture->_maskDib) + offs;

				// if there's value here, then it needs to be cleared
				if (src[3])
				{
					// mask will be & with background, so it needs to be 0xFF
					maskDst[0] = maskDst[1] = maskDst[2] = maskDst[3] = -1;;

					// clear the art in the bitmap because it's not going to display and will be | with background
					dst[0] = dst[1] = dst[2] = dst[3] = 0;
				}
				else
				{
					// this will clear the background before drawing this bitmap
					maskDst[0] = maskDst[1] = maskDst[2] = maskDst[3] = 0x00;

					// copy over the art
					dst[0] = src[2];
					dst[1] = src[1];
					dst[2] = src[0];
					dst[3] = src[3];
				}
			}
			else
			{
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				dst[3] = src[3];
			}
		}
	}

	free(rgba);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: brings the current surface to the foreground
//-----------------------------------------------------------------------------
void CWin32Surface::BringToFront(VPANEL panel)
{
	// force the panel to the top of the windows z-order
	panel = GetContextPanelForChildPanel(panel);
	if (panel && PLAT(panel))
	{
		// this should trigger a WM_SETFOCUS message which will move the window to the top
		::SetActiveWindow(PLAT(panel)->hwnd);
	}
}


//-----------------------------------------------------------------------------
// Purpose: puts the thread that created the specified window into the foreground 
//          and activates the window.
//-----------------------------------------------------------------------------
void CWin32Surface::SetForegroundWindow(VPANEL panel)
{
	if (panel && PLAT(panel))
	{
		::SetForegroundWindow(PLAT(panel)->hwnd);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::MovePopupToFront(VPANEL panel)
{
	_popupList.MoveElementToEnd(panel);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::MovePopupToBack(VPANEL panel)
{
}


//-----------------------------------------------------------------------------
// Purpose: 
// HWND_TOPMOST - Places the window above all non-topmost windows. 
// The window maintains its topmost position even when it is deactivated.
// HWND_NOTOPMOST - Places the window above all non-topmost windows (that is, behind 
// all topmost windows). This flag has no effect if the window is already a non-topmost window.
//-----------------------------------------------------------------------------
void CWin32Surface::SetAsTopMost(VPANEL panel, bool state)
{
	panel = GetContextPanelForChildPanel(panel);
	DWORD style=SWP_NOMOVE|SWP_NOSIZE;	
	
	if (PLAT(panel)->disabled)
	{
		style |= SWP_NOACTIVATE;
	}
		
	if (state)
	{
		SetFocus(PLAT(panel)->hwnd);
		SetWindowPos(PLAT(panel)->hwnd,HWND_TOPMOST,0,0,0,0,style  );
	}
	else
	{
		SetWindowPos(PLAT(panel)->hwnd,HWND_NOTOPMOST,0,0,0,0,style );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::SetPanelVisible(VPANEL panel, bool visible)
{
	// if the panel has an attached window we need to set it's style
	if (PLAT(panel))
	{
		if (visible)
		{
			// show the window
			::ShowWindow(PLAT(panel)->hwnd, SW_SHOWNA);

		}
		else
		{
			// hide the window
			::ShowWindow(PLAT(panel)->hwnd, SW_HIDE);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Flashes the window icon in the taskbar, to get the users attention
// Input  : flashCount - number of times to flash the window
//-----------------------------------------------------------------------------
void CWin32Surface::FlashWindow(VPANEL panel, bool state)
{
	if (!PLAT(panel))
		return;

	::FlashWindow(PLAT(panel)->hwnd, state);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::SetTopLevelFocus(VPANEL panel)
{
	// this is handled by WM_FOCUS messages instead of directly
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::SetMinimized(VPANEL panel, bool state)
{
	if (PLAT(panel))
	{
		if (state)
		{
			::ShowWindow(PLAT(panel)->hwnd, SW_MINIMIZE);
		}
		else
		{
			::ShowWindow(PLAT(panel)->hwnd, SW_SHOWNORMAL);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the window is minimized
//-----------------------------------------------------------------------------
bool CWin32Surface::IsMinimized(VPANEL panel)
{
	if (PLAT(panel)->hwnd)
	{
		return ::IsIconic(PLAT(panel)->hwnd);
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::SetTitle(VPANEL panel, const wchar_t *title)
{
	panel = GetContextPanelForChildPanel(panel);
	if (panel)
	{
		if (m_bSupportsUnicode)
		{
			SetWindowTextW(PLAT(panel)->hwnd, title);
		}
		else
		{
			//MODDD - using malloca instead to avoid a linker issue
			//char *ansi = (char *)_alloca(wcslen(title) + 5);
			char* ansi = (char*)malloc(wcslen(title) + 5);
			localize()->ConvertUnicodeToANSI(title, ansi, wcslen(title) + 5);
			SetWindowTextA(PLAT(panel)->hwnd, ansi);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::SetAsToolBar(VPANEL panel, bool state)
{
	panel = GetContextPanelForChildPanel(panel);
	if (panel && PLAT(panel))
	{
		if (state)
		{
			::SetWindowLong(PLAT(panel)->hwnd, GWL_EXSTYLE, ::GetWindowLong(PLAT(panel)->hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
		}
		else
		{
			::SetWindowLong(PLAT(panel)->hwnd, GWL_EXSTYLE, ::GetWindowLong(PLAT(panel)->hwnd, GWL_EXSTYLE) & ~WS_EX_TOOLWINDOW);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CWin32Surface::GetPopupCount()
{
	return _popupList.GetCount();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
VPANEL CWin32Surface::GetPopup(int index)
{
	return _popupList[index];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon, bool disabled, bool mouseInput, bool kbInput )
{
	//MODDD - how can this happen??
	if(_embeddedPanel == NULL){
		return;
	}
	return;

	if (((VPanel *)panel)->IsPopup() && PLAT(panel))
	{
		// it's already a popup so bail
		return;
	}

	// make sure it's in the hierarchy
	if (!((VPanel *)panel)->GetParent() && panel != _embeddedPanel)
	{
		((VPanel *)panel)->SetParent((VPanel *)_embeddedPanel);
	}

	int x,y,wide,tall;
	((VPanel *)panel)->GetPos(x, y);
	((VPanel *)panel)->GetSize(wide, tall);
	((VPanel *)panel)->SetPopup(true);
	((VPanel *)panel)->SetKeyBoardInputEnabled(kbInput);
	((VPanel *)panel)->SetMouseInputEnabled(mouseInput);

	// find our parent window if we have one
	HWND hwndParent = NULL;
	VPANEL pParent = GetContextPanelForChildPanel(panel);
	if (pParent && pParent != _embeddedPanel)
	{
		hwndParent = PLAT(pParent)->hwnd;
	}

	//create the window and initialize platform specific data
	//window is initial a popup and not visible
	//when ApplyChanges is called the window will be shown unless
	//it SetVisible(false) is called
	SurfacePlat *plat = new SurfacePlat();
	((VPanel *)panel)->SetPlat(plat);

	// WS_SYSMENU and WS_MINIMIZEBOX flags are there simply to make icon appear in taskbar
	// WS_EX_TOOLWINDOW hides the taskbar/ALT-TAB button
	DWORD style=0,style_ex=0;
	if (!showTaskbarIcon)
	{
		style = WS_POPUP ;
		style_ex= WS_EX_TOOLWINDOW;
		if (!hwndParent)
		{
			hwndParent = PLAT(_embeddedPanel)->hwnd;
		}
	}
	else
	{
		style = WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX;
	}
	if (panel != _embeddedPanel && ((VPanel *)panel)->IsVisible())
	{
		style |= WS_VISIBLE;
	}

	if(disabled)
	{
		style |= WS_DISABLED;
	}

	if ( minimised )
	{
		style |= WS_MINIMIZE;
	}


	//MODDD - is this bettah
	//plat->hwnd = CreateWindowEx(style_ex, "Surface", "", style, x, y, wide, tall, hwndParent, NULL, GetModuleHandle(NULL), NULL);
	HINSTANCE hInstance = GetModuleHandle(NULL);
    plat->hwnd = CreateWindow("Surface", "Surface",  
        WS_OVERLAPPEDWINDOW,  
        20,  
        20,  
        850,  
        550,  
        NULL,  
        NULL,  
        hInstance,  
        NULL  
    ); 



	plat->clipRgn = CreateRectRgn(0,0,64,64);
	plat->hdc = CreateCompatibleDC(NULL);
	plat->hwndDC = NULL;
	plat->bitmap = null;
	plat->bitmapSize[0] = 0;
	plat->bitmapSize[1] = 0;
	plat->isFullscreen = false;
	plat->embeddedPanel = (VPanel *)panel;
	plat->disabled=disabled;
	plat->notifyIcon = NULL;
	plat->textureDC = NULL;

	::SetBkMode(plat->hdc, TRANSPARENT);
	::SetWindowLong(plat->hwnd, GWL_USERDATA, (LONG)ivgui()->PanelToHandle(panel));
	::SetTextAlign(plat->hdc, TA_LEFT | TA_TOP | TA_UPDATECP);
	
	if (!((VPanel *)panel)->IsVisible() || panel == _embeddedPanel)
	{
		SetPanelVisible(panel, false);
	}

	// create the context
	RecreateContext(panel);

	::RegisterDragDrop(plat->hwnd, &staticDragDropTarget);

	// add the panel to the popup list
	if (!_popupList.HasElement(panel))
	{
		_popupList.AddElement(panel);
	}
	else
	{
		// somehow getting added twice, fundamental problem
		_asm int 3;
	}

	//!! hack to set a valid current context panel
	SetCurrentContextPanel(_embeddedPanel);

}

//-----------------------------------------------------------------------------
// Purpose: Called when a panel is created
//-----------------------------------------------------------------------------
void CWin32Surface::AddPanel(VPANEL panel)
{
}

//-----------------------------------------------------------------------------
// Purpose: Called when a panel gets deleted
//-----------------------------------------------------------------------------
void CWin32Surface::ReleasePanel(VPANEL panel)
{
	_popupList.RemoveElement(panel);

	if (panel == _currentContextPanel)
	{
		_currentContextPanel = _embeddedPanel;
	}

	if (PLAT(panel))
	{
		SurfacePlat *plat = PLAT(panel);

		// release drag/drop
		::RevokeDragDrop(plat->hwnd);

		// remove notify icons
		if (plat->notifyIcon)
		{
			SetNotifyIcon(panel, NULL, NULL, NULL);
		}

		// hide the panel
		SetPanelVisible(panel, false);

		// free all the windows/bitmap/DC handles we are using
		::SetWindowLong(plat->hwnd, GWL_USERDATA, (LONG)-1);
		::SetWindowPos(plat->hwnd, HWND_BOTTOM, 0, 0, 1, 1, SWP_NOREDRAW|SWP_HIDEWINDOW);

		// free the window context
		if ( plat->bitmap )
		{
			::DeleteObject( plat->bitmap );
		}

		if ( plat->textureDC )
		{
			::DeleteDC( plat->textureDC );
		}

		if (plat->hwndDC)
		{
			::ReleaseDC( plat->hwnd, plat->hwndDC );
		}
		::DeleteDC( plat->hdc );
		::DestroyWindow( plat->hwnd );
		::DeleteObject( plat->clipRgn );

		// don't free staticWndclassAtom, because it is shared amongst surfaces,
		// and is automatically freed when the application terminates

		delete plat;
		// TEST!!!
		((VPanel *)panel)->SetPlat(NULL);
		//((VPanel *)panel)->IsPopup();
		int x = 45;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Applies any changes to the panel into the underline wnidow
//-----------------------------------------------------------------------------
bool CWin32Surface::RecreateContext(VPANEL panel)
{
	if (panel && PLAT(panel))
	{
		SurfacePlat *plat = PLAT(panel);
		int wide,tall;
		((VPanel *)panel)->GetSize(wide,tall);

		// rebuild bitmap only if necessary
		// simple scheme to prevent excessive allocations by allocating only when
		// bigger. It also adds in 100 extra for subsequent sizings
		// it will also realloc if the size is 200 smaller to shrink memory usage
		if ((wide > plat->bitmapSize[0]) 
			|| (tall > plat->bitmapSize[1])
			|| (wide < (plat->bitmapSize[0] - 200)) 
			|| (tall < (plat->bitmapSize[1] - 200)))
		{
			if (plat->bitmap != null)
			{
				::DeleteObject(plat->bitmap);
			}

			plat->hwndDC = GetDC(plat->hwnd);

			plat->bitmap = ::CreateCompatibleBitmap(plat->hwndDC, wide + 100, tall + 100);
			plat->bitmapSize[0] = wide + 100;
			plat->bitmapSize[1] = tall + 100;

			if (plat->textureDC)
			{
				::DeleteDC(plat->textureDC);
			}

			::SelectObject(plat->hdc, plat->bitmap);
			plat->textureDC = ::CreateCompatibleDC(plat->hdc);

			::ReleaseDC(plat->hwnd, plat->hwndDC);
			plat->hwndDC = NULL;
		}
	}
 
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::EnableMouseCapture(VPANEL panel, bool state)
{
	VPANEL contextPanel = GetContextPanelForChildPanel(panel);
	if (state)
	{
		HWND wnd = ::SetCapture(PLAT(contextPanel)->hwnd);
	}
	else
	{
		::ReleaseCapture();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWin32Surface::ShouldPaintChildPanel(VPANEL childPanel)
{
	// don't Paint children as part of the normal process, handle them in with WM_PAINT messages instead
	return !((VPanel *)childPanel)->IsPopup();
}

//-----------------------------------------------------------------------------
// Purpose: Called after a Paint to display the new buffer
//-----------------------------------------------------------------------------
void CWin32Surface::SwapBuffers(VPANEL panel)
{
	if (PLAT(panel))
	{
		START_TIMER();
		
		SurfacePlat *plat = PLAT(panel);

		int wide,tall;
		((VPanel *)panel)->GetSize(wide,tall);
		
		plat->hwndDC = ::GetDC(plat->hwnd);

		// reset origin and clipping then blit
		::SetRectRgn(plat->clipRgn, 0, 0, wide, tall);
		::SelectObject(plat->hdc, plat->clipRgn);
		::SetViewportOrgEx(plat->hdc, 0, 0, NULL);
		::BitBlt(plat->hwndDC, 0, 0, wide, tall, plat->hdc, 0, 0, SRCCOPY);

		::ReleaseDC(plat->hwnd, plat->hwndDC);
		plat->hwndDC = NULL;

		END_TIMER("SwapBuffers time: %.2fms\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame to change to window state if necessary
//-----------------------------------------------------------------------------
void CWin32Surface::ApplyChanges()
{
	for (int i = 0; i < GetPopupCount(); i++)
	{
		VPANEL panel = GetPopup(i);

		if (!PLAT(panel))
			continue;

		SurfacePlat *Plat = PLAT(panel);

		// force the main panel to be invisible
		if (panel == GetEmbeddedPanel())
		{
			::ShowWindow(Plat->hwnd, SW_HIDE);
			continue;
		}

		// hide and skip by invisible panels
		if (!((VPanel *)panel)->IsVisible())
		{
			::ShowWindow(Plat->hwnd, SW_HIDE);
			continue;
		}

		RECT rect;
		int x, y, wide, tall, sx, sy, swide, stall;

		// get how big the win32 window
		::GetWindowRect(Plat->hwnd, &rect);
		sx = rect.left;
		sy = rect.top;
		swide = rect.right-rect.left;
		stall = rect.bottom-rect.top;

		// how big is the embedded VPanel
		((VPanel *)panel)->GetPos(x, y);
		((VPanel *)panel)->GetSize(wide, tall);

		// if they are not the same, then adjust the win32 window so it is
		if ((x != sx) || (y != sy) || (wide != swide) || (tall != stall))
		{
			int result = ::SetWindowPos(Plat->hwnd, null, x, y, wide, tall, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
		}

		// check to see if the win32 window is visible
		if (::GetWindowLong(Plat->hwnd, GWL_STYLE) & WS_VISIBLE)
		{
			//check to see if embedded VPanel is not visible, if so then hide the win32 window
			if (!((VPanel *)panel)->IsVisible())
			{
				::ShowWindow(Plat->hwnd, SW_HIDE);
			}
		}
		else // win32 window is hidden
		{
			//check to see if embedded VPanel is visible, if so then show the win32 window
			if (((VPanel *)panel)->IsVisible())
			{
				::ShowWindow(Plat->hwnd, SW_SHOWNA);
			}
		}

		//if the win32 window changed size and is visible, then the context needs to be recreated
		if (((wide != swide) || (tall != stall)) && ((VPanel *)panel)->IsVisible())
		{
			RecreateContext(panel);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: recurses the panels calculating absolute positions
//			parents must be solved before children
//-----------------------------------------------------------------------------
void CWin32Surface::InternalSolveTraverse(VPANEL panel)
{
	// solve the parent
	((VPanel *)panel)->Solve();
	
	// now we can solve the children
	for (int i = 0; i < ((VPanel *)panel)->GetChildCount(); i++)
	{
		VPanel *child = ((VPanel *)panel)->GetChild(i);
		if (child->IsVisible())
		{
			InternalSolveTraverse((VPANEL)child);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: recurses the panels giving them a chance to do a user-defined think,
//			PerformLayout and ApplySchemeSettings
//			must be done child before parent
//-----------------------------------------------------------------------------
void CWin32Surface::InternalThinkTraverse(VPANEL panel)
{
	// parent
	((VPanel *)panel)->Client()->Think();

	// then the children...
	for (int i = 0; i < ((VPanel *)panel)->GetChildCount(); i++)
	{
		VPanel *child = ((VPanel *)panel)->GetChild(i);
		if (child->IsVisible())
		{
			InternalThinkTraverse((VPANEL)child);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: recurses the panels giving them a chance to do a ApplySchemeSettings
//			must be done child before parent
//-----------------------------------------------------------------------------
void CWin32Surface::InternalSchemeSettingsTraverse(VPANEL panel, bool forceApplySchemeSettings)
{
	// think the children...
	for (int i = 0; i < ((VPanel *)panel)->GetChildCount(); i++)
	{
		VPanel *child = ((VPanel *)panel)->GetChild(i);
		if ( forceApplySchemeSettings || child->IsVisible() )
		{	
			InternalSchemeSettingsTraverse((VPANEL)child, forceApplySchemeSettings);
		}
	}

	// ... then the parent
	((VPanel *)panel)->Client()->PerformApplySchemeSettings();
}

//-----------------------------------------------------------------------------
// Purpose: Walks through the panel tree calling Solve() on them all, in order
//-----------------------------------------------------------------------------
void CWin32Surface::SolveTraverse(VPANEL panel, bool forceApplySchemeSettings)
{
	//MODDD - how can this happen??
	if(panel == NULL){
		return;
	}


	if (!((VPanel *)panel)->IsVisible())
		return;

	InternalSchemeSettingsTraverse(panel, forceApplySchemeSettings); // do apply scheme settings, child to parent
	InternalThinkTraverse(panel);
	InternalSolveTraverse(panel);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWin32Surface::PaintTraverse(VPANEL panel)
{
	START_TIMER();

	((VPanel *)panel)->Client()->PaintTraverse(false,true);

	END_TIMER("Paint time: %.2fms\n");
}

// FIXME: write these functions!
void CWin32Surface::RestrictPaintToSinglePanel(VPANEL panel)
{
}

void CWin32Surface::SetModalPanel(VPANEL )
{
}

VPANEL CWin32Surface::GetModalPanel()
{
return 0;
}

void CWin32Surface::UnlockCursor()
{
}

void CWin32Surface::LockCursor()
{
}

void CWin32Surface::SetTranslateExtendedKeys(bool state)
{
}

VPANEL CWin32Surface::GetTopmostPopup()
{
	return 0;
}



//-----------------------------------------------------------------------------
// Purpose: Checks to see if the mouse should be visible or not
//-----------------------------------------------------------------------------
void CWin32Surface::CalculateMouseVisible()
{
	int i;
	_needMouse = false;
	_needKB = false;

	for(i = 0 ; i < surface()->GetPopupCount() ; i++ )
	{
		VPanel *pop = (VPanel *)surface()->GetPopup( i ) ;
		
		bool isVisible=pop->IsVisible();
		VPanel *p= pop->GetParent();

		while(p && isVisible)
		{
			if( p->IsVisible()==false)
			{
				isVisible=false;
				break;
			}
			p=p->GetParent();
		}
	
		if(isVisible)
		{
			_needMouse |= pop->IsMouseInputEnabled();
			_needKB |= pop->IsKeyBoardInputEnabled();
		}
	}

	if(_needMouse)
	{
		SetCursor(vgui::dc_arrow);
		UnlockCursor();
	}
	else
	{
		SetCursor(vgui::dc_none);
		LockCursor();
		int wide,tall;
		GetScreenSize(wide, tall);
		SetCursorPos(wide/2, tall/2);
	}
}

bool CWin32Surface::NeedKBInput()
{
	return _needKB;
}


//-----------------------------------------------------------------------------
// Purpose: Sets the current cursor
//-----------------------------------------------------------------------------
void CWin32Surface::SetCursor(HCursor cursor)
{
	switch (cursor)
	{
		case dc_none:
		case dc_arrow:
		case dc_ibeam:
		case dc_hourglass:
		case dc_crosshair:
		case dc_up:
		case dc_sizenwse:
		case dc_sizenesw:
		case dc_sizewe:
		case dc_sizens:
		case dc_sizeall:
		case dc_no:
		case dc_hand:
		{
			_currentCursor = staticDefaultCursor[cursor];
			break;
		}
		case dc_user:
		{
			break;
		}
	}
	
	::SetCursor(_currentCursor);
}

//-----------------------------------------------------------------------------
// Purpose: Forces the window to be redrawn
//-----------------------------------------------------------------------------
void CWin32Surface::Invalidate(VPANEL panel)
{
	panel = GetContextPanelForChildPanel(panel);
	if (panel && PLAT(panel) && ((VPanel *)panel)->IsVisible())
	{
		// last parm must be false so WM_ERASEBKGND is not generated, that should
		// only be generated by windows
		InvalidateRect(PLAT(panel)->hwnd, NULL, false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if any of the windows have focus
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWin32Surface::HasFocus()
{
	HWND focus = ::GetFocus();
	// see if any of the windows on the surface have the focus
	for (int i = 0; i < GetPopupCount(); i++)
	{
		VPANEL panel = GetPopup(i); 
		if (focus == PLAT(panel)->hwnd)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the cursor is over this surface
//			Uses the windows call to do this, instead of doing it procedurally
//-----------------------------------------------------------------------------
bool CWin32Surface::IsWithin(int x,int y)
{
	POINT pnt={x,y};
	HWND hwnd = WindowFromPoint(pnt);

	for (int i = 0; i < GetPopupCount(); i++)
	{
		if (PLAT(GetPopup(i))->hwnd == hwnd)
		{
			return true;
		}
	}

 	return false;
}

//-----------------------------------------------------------------------------
// Purpose: handles a set focus message
//-----------------------------------------------------------------------------
void CWin32Surface::setFocus(VPANEL panel)
{
	// reset the focus to the last focused panel
	panel = GetContextPanelForChildPanel(panel);
	if (panel && PLAT(panel))
	{
		// make sure we have a valid panel
		VPANEL focus = panel;
		if (focus && ((VPanel *)focus)->HasParent((VPanel *)panel))
		{
			if (input()->GetAppModalSurface() && input()->GetAppModalSurface() != panel)
			{
				// trying to set focus to something that's not the modal panel, set it back
				SetForegroundWindow(input()->GetAppModalSurface());
			}
			else
			{
				// make sure focus is at top of list
				// the subfocus will be automatically calculated in IInput::RunFrame()
				((VPanel *)focus)->MoveToFront();
				return;
			}
		}
	}
		
	// let the main panel get focus as the default
	if (panel)
	{
		((VPanel *)panel)->Client()->RequestFocus();
	}
}

struct FontRangeItem_t
{
	const char *name;
	int lowerRange;
	int upperRange;
};

FontRangeItem_t g_FontRanges[] = 
{
	{ "Basic Latin",							0x0000, 0x007F },
	{ "Latin-1 Supplement",						0x0080, 0x00FF },
	{ "Latin Extended-A",						0x0100, 0x017F },
	{ "Latin Extended-B",						0x0180, 0x024F },
	{ "IPA Extensions",							0x0250, 0x02AF },
	{ "Spacing Modifier Letters",				0x02B0, 0x02FF },
	{ "Combining Diacritical Marks",			0x0300, 0x036F },
	{ "Greek",									0x0370, 0x03FF },
	{ "Cyrillic",								0x0400, 0x04FF },
	{ "Armenian",								0x0530, 0x058F },
	{ "Hebrew",									0x0590, 0x05FF },
	{ "Arabic",									0x0600, 0x06FF },
	{ "Syriac",									0x0700, 0x074F },
	{ "Thaana",									0x0780, 0x07BF },
	{ "Devanagari",								0x0900, 0x097F },
	{ "Bengali",								0x0980, 0x09FF },
	{ "Gurmukhi",								0x0A00, 0x0A7F },
	{ "Gujarati",								0x0A80, 0x0AFF },
	{ "Oriya",									0x0B00, 0x0B7F },
	{ "Tamil",									0x0B80, 0x0BFF },
	{ "Telugu",									0x0C00, 0x0C7F },
	{ "Kannada",								0x0C80, 0x0CFF },
	{ "Malayalam",								0x0D00, 0x0D7F },
	{ "Sinhala",								0x0D80, 0x0DFF },
	{ "Thai",									0x0E00, 0x0E7F },
	{ "Lao",									0x0E80, 0x0EFF },
	{ "Tibetan",								0x0F00, 0x0FBF },
	{ "Myanmar",								0x1000, 0x109F },
	{ "Georgian",								0x10A0, 0x10FF },
	{ "Hangul Jamo",							0x1100, 0x11FF },
	{ "Ethiopic",								0x1200, 0x137F },
	{ "Cherokee",								0x13A0, 0x13FF },
	{ "Unified Canadian Aboriginal Syllabics",	0x1400, 0x167F },
	{ "Ogham",									0x1680, 0x169F },
	{ "Runic",									0x16A0, 0x16FF },
	{ "Khmer",									0x1780, 0x17FF },
	{ "Mongolian",								0x1800, 0x18AF },
	{ "Latin Extended Additional",				0x1E00, 0x1EFF },
	{ "Greek Extended",							0x1F00, 0x1FFF },
	{ "General Punctuation",					0x2000, 0x206F },
	{ "Superscripts and Subscripts",			0x2070, 0x209F },
	{ "Currency Symbols",						0x20A0, 0x20CF },
	{ "Combining Diacritical Marks for Symbols",0x20D0, 0x20FF },
	{ "Letterlike Symbols",						0x2100, 0x214F },
	{ "Number Forms",							0x2150, 0x218F },
	{ "Arrows",									0x2190, 0x21FF },
	{ "Mathematical Operators",					0x2200, 0x22FF },
	{ "Miscellaneous Technical",				0x2300, 0x23FF },
	{ "Control Pictures",						0x2400, 0x243F },
	{ "Optical Character Recognition",			0x2440, 0x245F },
	{ "Enclosed Alphanumerics",					0x2460, 0x24FF },
	{ "Box Drawing",							0x2500, 0x257F },
	{ "Block Elements",							0x2580, 0x259F },
	{ "Geometric Shapes",						0x25A0, 0x25FF },
	{ "Miscellaneous Symbols",					0x2600, 0x26FF },
	{ "Dingbats",								0x2700, 0x27BF },
	{ "Braille Patterns",						0x2800, 0x28FF },
	{ "CJK Radicals Supplement",				0x2E80, 0x2EFF },
	{ "KangXi Radicals",						0x2F00, 0x2FDF },
	{ "Ideographic Description Characters",		0x2FF0, 0x2FFF },
	{ "CJK Symbols and Punctuation",			0x3000, 0x303F },
	{ "Hiragana",								0x3040, 0x309F },
	{ "Katakana",								0x30A0, 0x30FF },
	{ "Bopomofo",								0x3100, 0x312F },
	{ "Hangul Compatibility Jamo",				0x3130, 0x318F },
	{ "Kanbun",									0x3190, 0x319F },
	{ "Bopomofo Extended",						0x31A0, 0x31BF },
	{ "Enclosed CJK Letters and Months",		0x3200, 0x32FF },
	{ "CJK Compatibility",						0x3300, 0x33FF },
	{ "CJK Unified Ideographs Extension A",		0x3400, 0x4DB5 },
	{ "CJK Unified Ideographs",					0x4E00, 0x9FFF },
	{ "Yi Syllables",							0xA000, 0xA48F },
	{ "Yi Radicals",							0xA490, 0xA4CF },
	{ "Hangul Syllables",						0xAC00, 0xD7A3 },
	{ "-- surrogates --",						0xD800, 0xDBFF },
	{ "CJK Compatibility Ideographs",			0xF900, 0xFAFF },
	{ "Alphabetic Presentation Forms",			0xFB00, 0xFB4F },
	{ "Arabic Presentation Forms-A",			0xFB50, 0xFDFF },
	{ "Combining Half Marks",					0xFE20, 0xFE2F },
	{ "CJK Compatibility Forms",				0xFE30, 0xFE4F },
	{ "Small Form Variants",					0xFE50, 0xFE6F },
	{ "Arabic Presentation Forms-B",			0xFE70, 0xFEFF },
	{ "Halfwidth and Fullwidth Forms",			0xFF00, 0xFFEF },
	{ "Specials",								0xFFF0, 0xFFFF },
};


//-----------------------------------------------------------------------------
// Purpose: creates a new empty font
//-----------------------------------------------------------------------------
HFont CWin32Surface::CreateFont()
{
	return FontManager().CreateFont();
}

//-----------------------------------------------------------------------------
// Purpose: adds glyphs to a font created by CreateFont()
//-----------------------------------------------------------------------------
bool CWin32Surface::AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange)
{
	return FontManager().AddGlyphSetToFont(font, windowsFontName, tall, weight, blur, scanlines, flags, lowRange, highRange);
}

//-----------------------------------------------------------------------------
// Purpose: returns the max height of a font
//-----------------------------------------------------------------------------
int CWin32Surface::GetFontTall(HFont font)
{
	return FontManager().GetFontTall(font);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : font - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWin32Surface::IsFontAdditive(HFont font)
{
	return FontManager().IsFontAdditive(font);
}

//-----------------------------------------------------------------------------
// Purpose: returns the abc widths of a single character
//-----------------------------------------------------------------------------
void CWin32Surface::GetCharABCwide(HFont font, int ch, int &a, int &b, int &c)
{
	FontManager().GetCharABCwide(font, ch, a, b, c);
}

//-----------------------------------------------------------------------------
// Purpose: returns the pixel width of a single character
//-----------------------------------------------------------------------------
int CWin32Surface::GetCharacterWidth(HFont font, int ch)
{
	return FontManager().GetCharacterWidth(font, ch);
}

//-----------------------------------------------------------------------------
// Purpose: returns the area of a text string, including newlines
//-----------------------------------------------------------------------------
void CWin32Surface::GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall)
{
	FontManager().GetTextSize(font, text, wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: adds a custom font file (only supports true type font files (.ttf) for now)
//-----------------------------------------------------------------------------
bool CWin32Surface::AddCustomFontFile(const char *fontFileName)
{
//	char fullPath[ MAX_PATH ];
	//MODDD - avoiding alloca, linker issue
	//char *fullPath = (char *)_alloca(filesystem()->GetLocalPathLen(fontFileName) + 1);
	
	//MODDD - just be 512 for now.
	//int myBufferSize = filesystem()->GetLocalPathLen(fontFileName) + 1;
	int myBufferSize = 512;

	char *fullPath = (char *)malloc(myBufferSize);

	//MODDD - GetLocalPath LAZY DEFAULT, not so lazy now
	filesystem()->GetLocalPath(fontFileName, fullPath, myBufferSize);
#ifdef LATER
	if ( m_WindowsVersion.dwMajorVersion >= 5 )
	{
		return (::AddFontResourceEx( fullpath, FR_PRIVATE, 0 ) > 0 );
	}
#endif
	return (::AddFontResource(fullPath) > 0);
}

//-----------------------------------------------------------------------------
// Purpose: Returns the bounds of the usable workspace area
//-----------------------------------------------------------------------------
void CWin32Surface::GetWorkspaceBounds(int &x, int &y, int &wide, int &tall)
{
	RECT rcScreen;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

	x = rcScreen.left;
	y = rcScreen.top;
	wide = rcScreen.right - x;
	tall = rcScreen.bottom - y;
}

//-----------------------------------------------------------------------------
// Purpose: gets the absolute coordinates of the screen (in screen space)
//-----------------------------------------------------------------------------
void CWin32Surface::GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall)
{
	// always work in full window screen space
	x = 0;
	y = 0;
	GetScreenSize(wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: Plays a sound
// Input  : *fileName - name of the wav file
//-----------------------------------------------------------------------------
void CWin32Surface::PlaySound(const char *fileName)
{
	filesystem()->GetLocalCopy(fileName);
	::PlaySoundA(fileName, NULL, SND_FILENAME | SND_ASYNC);
}

//-----------------------------------------------------------------------------
// Purpose: Handles windows message pump
//-----------------------------------------------------------------------------
void CWin32Surface::RunFrame()
{

	//MODDD - NEW???
	if(_currentContextPanel != NULL){
		((VPanel*)_currentContextPanel)->Client()->Repaint();
	}

	::MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		BOOL ret = ::GetMessage(&msg, NULL, 0, 0);
		if (ret == 0)
			break;


		//ivgui()->DPrintf( "I am unsatisfied %d\n", msg.message );
		// why 0x0060 (aka 96 in decimal) ?   WHHHHHOOOOOO KKKNNNNNNOOOOWWWWWWWWSSSSSS.
		if( msg.message == WM_QUIT || msg.message == 0x0060 || msg.message == 0x00A0 )
		{
			ivgui()->Stop();
			break;
		}


		if (ret == -1)
		{
			ivgui()->Stop();
			break;
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

//-----------------------------------------------------------------------------
// Purpose: cap bits
//-----------------------------------------------------------------------------
bool CWin32Surface::SupportsFeature(SurfaceFeature_e feature)
{
	switch (feature)
	{
	case ISurface::ESCAPE_KEY:
	case ISurface::ANTIALIASED_FONTS:
	case ISurface::OPENING_NEW_HTML_WINDOWS:
	case ISurface::FRAME_MINIMIZE_MAXIMIZE:
		return true;

	case ISurface::DROPSHADOW_FONTS:
	case ISurface::OUTLINE_FONTS:
	default:
		return false;
	};
}

//-----------------------------------------------------------------------------
// Purpose: Initializes all the static data for the app
//			Should be only called during load time
//-----------------------------------------------------------------------------
void CWin32Surface::initStaticData()
{
	//load up all default cursors, this gets called everytime a Surface is created, but
	//who cares
	staticDefaultCursor[dc_none]     =null;
	staticDefaultCursor[dc_arrow]    =(HICON)LoadCursor(null,(LPCTSTR)OCR_NORMAL);
	staticDefaultCursor[dc_ibeam]    =(HICON)LoadCursor(null,(LPCTSTR)OCR_IBEAM);
	staticDefaultCursor[dc_hourglass]=(HICON)LoadCursor(null,(LPCTSTR)OCR_WAIT);
	staticDefaultCursor[dc_crosshair]=(HICON)LoadCursor(null,(LPCTSTR)OCR_CROSS);
	staticDefaultCursor[dc_up]       =(HICON)LoadCursor(null,(LPCTSTR)OCR_UP);
	staticDefaultCursor[dc_sizenwse] =(HICON)LoadCursor(null,(LPCTSTR)OCR_SIZENWSE);
	staticDefaultCursor[dc_sizenesw] =(HICON)LoadCursor(null,(LPCTSTR)OCR_SIZENESW);
	staticDefaultCursor[dc_sizewe]   =(HICON)LoadCursor(null,(LPCTSTR)OCR_SIZEWE);
	staticDefaultCursor[dc_sizens]   =(HICON)LoadCursor(null,(LPCTSTR)OCR_SIZENS);
	staticDefaultCursor[dc_sizeall]  =(HICON)LoadCursor(null,(LPCTSTR)OCR_SIZEALL);
	staticDefaultCursor[dc_no]       =(HICON)LoadCursor(null,(LPCTSTR)OCR_NO);
	staticDefaultCursor[dc_hand]     =(HICON)LoadCursor(null,(LPCTSTR)32649);

	// make and register a very simple Window Class
	memset( &staticWndclass,0,sizeof(staticWndclass) );
	staticWndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	staticWndclass.lpfnWndProc = staticProc;
	staticWndclass.hInstance = GetModuleHandle(NULL);

	// Get the resource ID of the icon group from the environment...default to 101.
	DWORD wIconID = 101;
	const char *sIconResourceID = getenv(szSteamBootStrapperIconIdEnvVar);
	if ( sIconResourceID )
	{
		DWORD wTmpIconID = 0;
		if ( sscanf(sIconResourceID, "%u", &wTmpIconID) == 1 && wTmpIconID > 101 )
		{
			wIconID = wTmpIconID;
		}
	}
	staticWndclass.hIcon = ::LoadIcon(staticWndclass.hInstance, MAKEINTRESOURCE(wIconID));

	staticWndclass.lpszClassName = "Surface";
	staticWndclassAtom = ::RegisterClass( &staticWndclass );

	// register our global Shutdown command
	staticShutdownMsg = ::RegisterWindowMessage("ShutdownValvePlatform");




	//MODD - I DONT KNOW MAN
	//staticWndclass.lpszClassName = TEXT("Win32 Basic Drawing");  
	staticWndclass.cbClsExtra = 0;  
	staticWndclass.cbWndExtra = 0;  
	staticWndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  
	staticWndclass.hCursor = LoadCursor(NULL, IDC_ARROW);  
	staticWndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);  
	//staticWndclass.lpfnWndProc = WndProc;  
	staticWndclass.lpszMenuName = NULL;  
	//staticWndclass.hInstance = hInstance;  
	staticWndclass.style = CS_HREDRAW | CS_VREDRAW;  





}

//-----------------------------------------------------------------------------
// Purpose: Handles windows messages sent to the notify tray icon
//-----------------------------------------------------------------------------
static void staticNotifyIconProc(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	switch (lparam)
	{
		case WM_LBUTTONDOWN:
		{
			// notify the window of the double click
			if (g_Surface.GetNotifyPanel())
			{
				ivgui()->PostMessage(surface()->GetNotifyPanel(), new KeyValues("NotifyIconMsg", "msg", "WM_LBUTTONDOWN"), NULL, 0);
			}
			break;
		}

		case WM_LBUTTONDBLCLK:
		{
			//HACK: always bring us to front if the user double clicks the icon
			::SetForegroundWindow(hwnd);

			// notify the window of the double click
			if (surface()->GetNotifyPanel())
			{
				ivgui()->PostMessage(surface()->GetNotifyPanel(), new KeyValues("NotifyIconMsg", "msg", "LBUTTONDBLCLK"), NULL, 0);
			}
			break;
		}

		case WM_RBUTTONUP:		// win95/98/NT
		case WM_CONTEXTMENU:	// win2k/ME
		{
			// display context menu
			if (surface()->GetNotifyPanel())
			{
				ivgui()->PostMessage(surface()->GetNotifyPanel(), new KeyValues("NotifyIconMsg", "msg", "CONTEXTMENU"), NULL, 0);
			}
			break;
		}
		default:
			break;
	}
}

static const int MS_WM_XBUTTONDOWN	= 0x020B;
static const int MS_WM_XBUTTONUP	= 0x020C;
static const int MS_MK_BUTTON4		= 0x0020;
static const int MS_MK_BUTTON5		= 0x0040;

static KeyCode g_iPreviousKeyCode = KEY_NONE;




static LRESULT CALLBACK staticProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static UINT s_uTaskbarRestart;

	LRESULT ret = TRUE;

	VPANEL panel = NULL;
	IClientPanel *client = NULL;

	//MODDD - WHAT?  Compare to as-is and 2007.  This section looks sorta important.
	if (staticSurfaceAvailable)
	{
		panel = ivgui()->HandleToPanel(::GetWindowLong(hwnd, GWL_USERDATA));

		if (panel)
		{
			client = ((VPanel *)panel)->Client();


			//vgui::surface()->SetEmbeddedPanel( vgui::surface()->Getpan );
			// nope......
			//vgui::surface()->SetEmbeddedPanel( panel );
		}
	}
	
	//SurfacePlat *plat = new SurfacePlat();
	//((VPanel *)panel)->SetPlat(plat);
	//PLAT(panel)->hwnd;


//
//	// special case msg handle
//	if (msg == staticShutdownMsg)
//	{
//		// we're being notified that we have to Shutdown
//		ivgui()->ShutdownMessage(lparam);
//		return ::DefWindowProc(hwnd,msg,wparam,lparam);
//	}
//
//	if (!panel)
//	{
//		return ::DefWindowProc(hwnd,msg,wparam,lparam);
//	}
//
//	switch (msg)
//	{
//		case MS_WM_XBUTTONDOWN:
//		{
//			if ( HIWORD( wparam ) == 1 )
//			{
//				input()->InternalMousePressed(MOUSE_4);
//			}
//			else
//			{
//				input()->InternalMousePressed(MOUSE_5);
//			}
//			break;
//		}
//		case MS_WM_XBUTTONUP:
//		{
//			if ( HIWORD( wparam ) == 1 )
//			{
//				input()->InternalMouseReleased(MOUSE_4);
//			}
//			else
//			{
//				input()->InternalMouseReleased(MOUSE_5);
//			}
//			break;
//		}
//
//		case WM_CREATE:
//		{
//	        s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
//			break;
//        }
//		case WM_CLOSE:
//		{
//			// tell the panel to close
//			ivgui()->PostMessage(panel, new KeyValues("Close"), NULL, 0);
//
//			// don't Run default message pump, as that destroys the window
//			return 0;
//		}
//		case WM_MY_TRAY_NOTIFICATION:
//		{
//			staticNotifyIconProc(hwnd, wparam, lparam);
//			break;
//		}
//		case WM_SETFOCUS:
//		{
//			g_Surface.setFocus(panel);
//			//ivgui()->DPrintf("Set Focus %s %p\n", client->GetName(), panel);
//			break;
//		}
//		case WM_KILLFOCUS:
//		{
//			// check who is killing our focus
//			int found=0;
//			for(int i=0;i<g_Surface.GetHTMLWindowCount();i++)
//			{
//				if(g_Surface.GetHTMLWindow(i)->GetIEHWND()==(HWND)wparam)
//				{
//					found=1;
//					break;
//				}
//			}
//
//			if(found==0)
//			{
//				g_Surface.setFocus(NULL);
//			}
//			else
//			{ // the damn HTML window is trying to steal focus...
//			//	g_Surface.setFocus(panel);
//				g_Surface.setFocus(NULL);
//
//				// so tell oursevles to grab it back :)
//				//::PostMessage(hwnd,WM_APP,0,0);
//				
//			}
//			break;
//		}
//		case WM_APP:
//		{
//			::SetFocus(hwnd);
//			break;
//		}
//		case WM_SETCURSOR:
//		{		
////!!			SetCursor(staticCurrentCursor);
//			break;
//		}
//		case WM_MOUSEMOVE:
//		{
//			POINT pt;
//			pt.x = (short)LOWORD(lparam);
//			pt.y = (short)HIWORD(lparam);
//			::ClientToScreen((HWND)hwnd, &pt);
//			input()->InternalCursorMoved(pt.x, pt.y);
//			break;
//		}
//		case WM_LBUTTONDOWN:
//		{
//			input()->InternalMousePressed(MOUSE_LEFT);
//			break;
//		}
//		case WM_RBUTTONDOWN:
//		{
//			input()->InternalMousePressed(MOUSE_RIGHT);
//			break;
//		}
//		case WM_MBUTTONDOWN:
//		{
//			input()->InternalMousePressed(MOUSE_MIDDLE);
//			break;
//		}
//		case WM_LBUTTONDBLCLK:
//		{
//			input()->InternalMouseDoublePressed(MOUSE_LEFT);
//			break;
//		}
//		case WM_RBUTTONDBLCLK:
//		{
//			input()->InternalMouseDoublePressed(MOUSE_RIGHT);
//			break;
//		}
//		case WM_MBUTTONDBLCLK:
//		{
//			input()->InternalMouseDoublePressed(MOUSE_MIDDLE);
//			break;
//		}
//		case WM_LBUTTONUP:
//		{
//			input()->InternalMouseReleased(MOUSE_LEFT);
//			break;
//		}
//		case WM_RBUTTONUP:
//		{
//			input()->InternalMouseReleased(MOUSE_RIGHT);
//			break;
//		}
//		case WM_MBUTTONUP:
//		{
//			input()->InternalMouseReleased(MOUSE_MIDDLE);
//			break;
//		}
//		case WM_MOUSEWHEEL:
//		{
//			input()->InternalMouseWheeled(((short)HIWORD(wparam))/WHEEL_DELTA);
//			break;
//		}
//		case WM_KEYDOWN:
//		case WM_SYSKEYDOWN:
//		{
//			int code = wparam;
//			g_iPreviousKeyCode = KeyCode_VirtualKeyToVGUI( code );
//			if (!(lparam & (1<<30)))
//			{
//				input()->InternalKeyCodePressed( g_iPreviousKeyCode );
//			}
//			input()->InternalKeyCodeTyped( g_iPreviousKeyCode );
//			break;
//		}
//		case WM_SYSCHAR:
//		case WM_CHAR:
//		{
//			int unichar = wparam;
//			input()->InternalKeyTyped(unichar);
//			break;
//		}
//		case WM_KEYUP:
//		case WM_SYSKEYUP:
//		{
//			int code=wparam;
//			input()->InternalKeyCodeReleased( KeyCode_VirtualKeyToVGUI( code ) );
//			break;
//		}
//		case WM_ERASEBKGND:
//		{
//			//since the vgui Invalidate call does not erase the background
//			//this will only be called when windows itselfs wants a Repaint.
//			//this is the desired behavior because this call will for the
//			//surface and all its children to end up being repainted, which
//			//is what you want when windows wants you to Repaint the surface,
//			//but not what you want when say a control wants to be painted
//			//VPanel::Repaint will always Invalidate the surface so it will
//			//get a WM_PAINT, but that does not necessarily mean you want
//			//the whole surface painted
//			//simply this means.. this call only happens when windows wants the Repaint
//			//and WM_PAINT gets called just after this to do the real painting
//
//			client->Repaint();
////			vgui::ivgui()->DPrintf( "WM_ERASEBKGND(%X)\n", panel );
//			break;
//		}
//		case WM_PAINT:
//		{
//			//surface was by repainted vgui or by windows itself, do the repainting all repainting 
//			//will goes through here and nowhere else
//
//			// post Paint messages to the que
////			panel->SolveTraverse();
//
//			// post a message to Paint the window
////			vgui::ivgui()->DPrintf( "WM_PAINT(%X)\n", panel );
////			input()->PostMessage( panel, new KeyValues("Paint"), NULL );
//
//			// this relies on platTick() being called BEFORE dispatchMessages(), so that painting occurs
//
//			// on the same frame that the WM_PAINT message is received
////			double startTime, solveTime, paintTime;
//			// OLD CODE
//
//			// check that the panel is visible all the way to the root
//
//			bool IsVisible=ipanel()->IsVisible(panel);
//			VPANEL p= ipanel()->GetParent(panel);
//			// drill down the heirachy checking that everything is visible
//			while(p && IsVisible)
//			{
//				if( ipanel()->IsVisible(p)==false)
//				{
//					IsVisible=false;
//					break;
//				}
//				p=ipanel()->GetParent(p);
//			}
//
//			if( IsVisible )
//			{
//				PAINTSTRUCT ps;
//				HDC hDC=::BeginPaint(hwnd,&ps);
//	//			startTime = system()->GetCurrentTime();
// 				g_Surface.SolveTraverse(panel, false);
//	//			solveTime = system()->GetCurrentTime();
//				g_Surface.PaintTraverse(panel);
//	//			paintTime = system()->GetCurrentTime();
//
//
//				/*
//				HBRUSH hbrush;  
//				HPEN hpen;
//				//draw a rectangle with color blue  
//				hpen = CreatePen(PS_SOLID, 4, RGB(0, 0, 255));  
//				SelectObject(hDC, hpen);  
//				Rectangle(hDC, 10, 10, 245 - 10, 245 - 10);  
//				DeleteObject(hpen);  
//
//				::EndPaint(hwnd,&ps);
//				*/
//			}
//
////			debug timing code
////			ivgui()->DPrintf2("Paint timings (%.3f %.3f)\n", (float)(solveTime - startTime), (float)(paintTime - solveTime));
//		
//		/*	char dbtxt[200];
//			_snprintf(dbtxt,200,"paint:%p -- %p\n",hwnd,g_Surface.GetHTMLWindow(0)->GetIEHWND());
//			OutputDebugString( dbtxt );
//		*/
//			//clear the update rectangle so it does not get another Repaint
//			::ValidateRect(hwnd, NULL);	
//
//			break;
//		}
//		default:
//		{
//			if (msg == s_uTaskbarRestart)
//			{
//				// clear old taskbar icons
//				g_Surface.SetNotifyIcon(panel, NULL, NULL, "");
//
//				// notify window to re-add taskbar icons
//				ivgui()->PostMessage(panel, new KeyValues("TaskbarRestart"), NULL, 0);
//			}
//
//			break;
//		}
//	}







	// NOTHING OKAY
	HDC hdc;  
    PAINTSTRUCT ps;  
    int width,height;  
    RECT rect = { 30, 20, 200, 100 };  
    POINT pt[4] = {800,400,400,300,600,400,600,400};  
    HWND hw;  
  
    switch (msg)  
    {  
    case WM_CREATE :  
          
        return 0;  
      
    case WM_SIZE :  
        width = LOWORD(lparam);  
        height = HIWORD(lparam);  
  
    case WM_PAINT :  
          
        hdc = BeginPaint(hwnd, &ps);  
  
        HBRUSH hbrush;  
        HPEN hpen;  
  
        //draw a rectangle with color blue  
        hpen = CreatePen(PS_SOLID, 4, RGB(0, 0, 255));  
        SelectObject(hdc, hpen);  
        Rectangle(hdc, 10, 10, width - 10, height - 10);  
        DeleteObject(hpen);  
  
        //fill a solid rectangle  
        hbrush = CreateSolidBrush(RGB(250, 0, 0));  
        FillRect(hdc, &rect, hbrush);  
  
        //fill a hatch rectangle  
        hbrush = CreateHatchBrush(HS_CROSS, RGB(0, 240, 0));  
        rect.left = 220;  
        rect.top = 20;  
        rect.right = 400;  
        rect.bottom = 100;  
        FillRect(hdc, &rect, hbrush);  
  
        //draw a ellipse  
        hpen = CreatePen(PS_SOLID, 2, RGB(140, 0, 0));  
        SelectObject(hdc, hpen);  
        Ellipse(hdc,30,200,200,110);  
        DeleteObject(hpen);  
  
        //fill an ellipse  
        hbrush = CreateHatchBrush(HS_DIAGCROSS, RGB(255, 0, 250));  
        SelectObject(hdc,hbrush);  
        Ellipse(hdc, 420, 200, 220, 110);  
        DeleteObject(hbrush);  
  
        //draw round rect  
        hpen = CreatePen(PS_SOLID, 2, RGB(140, 0, 0));  
        hbrush = CreateHatchBrush(HS_HORIZONTAL, RGB(0, 0, 150));  
        SelectObject(hdc, hpen);  
        SelectObject(hdc, hbrush);  
        RoundRect(hdc, 300, 210, 30, 300, 50, 50);  
        DeleteObject(hbrush);  
        DeleteObject(hpen);  
  
        //draw arc  
        hpen = CreatePen(PS_SOLID, 2, RGB(0, 100, 0));  
        SelectObject(hdc, hpen);  
        Arc(hdc, 400, 320, 30, 450, 30, 40, 300, 400);  
        DeleteObject(hpen);  
  
        //draw polygon  
        hpen = CreatePen(PS_SOLID, 2, RGB(0, 140, 140));  
        hbrush = CreateHatchBrush(HS_FDIAGONAL, RGB(0, 140, 140));  
        SelectObject(hdc, hpen);  
        SelectObject(hdc, hbrush);  
        Polygon(hdc, pt, 4);  
        DeleteObject(hbrush);  
        DeleteObject(hpen);  
  
        //draw a text  
        SetTextColor(hdc, RGB(0, 100, 0));  
        TextOut(hdc, 600, 70, TEXT("Win32 Basic Drawing"),19);  
          
  
        EndPaint(hwnd, &ps);  
          
        return 0;  
    case WM_DESTROY:  
    case WM_CLOSE :  
        PostQuitMessage(EXIT_SUCCESS);  
        return 0;  
    }  
    return DefWindowProc(hwnd, msg, wparam, lparam);  













	return DefWindowProc(hwnd,msg,wparam,lparam);
}

