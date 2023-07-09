#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CGeneralMenu :: CGeneralMenu(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CMenuPanel(iTrans, iRemoveMe, x,y,wide,tall)
{
 
    CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
    SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
 
  
	int	lado = 10;

 
    m_pPanel = new CTransparentPanel( 200, 80, 40, 465, 510);
    m_pPanel->setParent( this );
    m_pPanel->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
 
	// Imagen Body
    m_pMyPicture = new CImageLabel( "body_monsters_menu", 1, 1);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Body

    m_pCancelButton = new CommandButton( gHUD.m_TextMessage.BufferedLocaliseTextString( "." ),
                                         428, 8, 20, 20);
    m_pCancelButton->setContentAlignment(vgui::Label::a_center);
    m_pCancelButton->setParent( m_pPanel );
    m_pCancelButton->addActionSignal( new CMenuHandler_TextWindow(HIDE_TEXTWINDOW) );
 




		///////////   START  PRIMERA FILA \\\\\\\\\\\\\\\\\\\\

	

		// Imagen Activate Grass
    m_pMyPicture = new CImageLabel( "grass_on", 1, 62);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Grass

	// Boton - Activate Grass
    m_pSpeak = new CommandButton( "", 3, 62, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "grass_on"));
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Activate Grass




	// Imagen Activate Grass
    m_pMyPicture = new CImageLabel( "grass_off", 85+lado+10, 62);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Grass

	// Boton - Activate Grass
    m_pSpeak = new CommandButton( "", 87+lado+10, 62 , 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "grass_off"));
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Activate Grass








	// Boton - DuplicateMode 
    m_pSpeak = new CommandButton( "", 3, 100, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "duplicatemode"));
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - DuplicateMode 




		// Boton - RenderMode 
    m_pSpeak = new CommandButton( "", 3, 150, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "rendermode"));
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - RenderMode 




	// Boton - RemoveMode 
    m_pSpeak = new CommandButton( "", 3, 200, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "removemode"));
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - RemoveMode 



}




void CGeneralMenu :: SetActiveInfo( int iShowText )
{
   
	iShowText = iShowText + 1;
}