#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CSettingsMenu :: CSettingsMenu(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CMenuPanel(iTrans, iRemoveMe, x,y,wide,tall)
{
 
    CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
    SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
 
    int r, g, b, a;
 
 
    m_pPanel = new CTransparentPanel( 200, XRES(80), YRES(80), XRES(250), YRES(329));
    m_pPanel->setParent( this );
    m_pPanel->setBorder( new LineBorder( Color(255 * 0.7,170 * 0.7,200,0) ) );
 
   



	m_pMyPicture = new CImageLabel( "body_gravgun", 1, 1 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
   

 
	// Boton - Close
	 m_pCancelButton = new CommandButton( gHUD.m_TextMessage.BufferedLocaliseTextString( "Close" ),
                                         XRES(50), YRES(279), XRES(120), YRES(42));
    m_pCancelButton->setContentAlignment(vgui::Label::a_center);
    m_pCancelButton->setParent( m_pPanel );
    m_pCancelButton->addActionSignal( new CMenuHandler_TextWindow(HIDE_TEXTWINDOW) );
   	// Boton - Close




	






	// SOLID UP
    m_pSpeak = new CommandButton( "", 245, 160, 30, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_up" ) );
	// SOLID UP





	// SOLID DOWN
    m_pSpeak = new CommandButton( "", 240, 220, 45, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_down" ) );
	// SOLID DOWN








	// SOLID LEFT
    m_pSpeak = new CommandButton( "", 200, 190, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_left" ) );
	// SOLID LEFT







	
	// SOLID RIGHT
    m_pSpeak = new CommandButton( "", 290, 190, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_right" ) );
	// SOLID RIGHT






	// SOLID ROTATE X +
    m_pSpeak = new CommandButton( "", 150, 400, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_rotate_x_+" ) );
	// SOLID ROTATE X +







	// SOLID ROTATE X -
    m_pSpeak = new CommandButton( "", 190, 400, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_rotate_x_-" ) );
	// SOLID ROTATE X -
















	// SOLID ROTATE Y +
    m_pSpeak = new CommandButton( "", 150, 435, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_rotate_y_+" ) );
	// SOLID ROTATE Y +







	// SOLID ROTATE Y -
    m_pSpeak = new CommandButton( "", 190, 435, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_rotate_y_-" ) );
	// SOLID ROTATE Y -













	
	// SOLID ROTATE Z +
    m_pSpeak = new CommandButton( "", 150, 465, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_rotate_z_+" ) );
	// SOLID ROTATE Z +







	// SOLID ROTATE Z -
    m_pSpeak = new CommandButton( "", 190, 465, 35, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "solid_rotate_z_-" ) );
	// SOLID ROTATE Z -





}




