#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CRenderMenu :: CRenderMenu(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CMenuPanel(iTrans, iRemoveMe, x,y,wide,tall)
{

	int lado;
	int alto;

	lado = 10;
	alto = 15;


    CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
    SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
 
    int r, g, b, a;
   
 
    m_pPanel = new CTransparentPanel( 200, 80, 50, 480, 550);
    m_pPanel->setParent( this );
    m_pPanel->setBorder( new LineBorder( Color(255 * 0.7,170 * 0.7,0,0) ) );
 
	
		//Imagen Body
    m_pMyPicture = new CImageLabel( "body_render_menu", 1, 1 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Body

    m_pCancelButton = new CommandButton( gHUD.m_TextMessage.BufferedLocaliseTextString( "Close" ),
                                         50, 400, 120, 42);
    m_pCancelButton->setContentAlignment(vgui::Label::a_center);
    m_pCancelButton->setParent( m_pPanel );
    m_pCancelButton->addActionSignal( new CMenuHandler_TextWindow(HIDE_TEXTWINDOW) );
 








	


		///////////   START  PRIMERA FILA \\\\\\\\\\\\\\\\\\\\







	// Boton - Render color Red

	
	//RED:


	// +
    m_pSpeak = new CommandButton( "+", 90, 70, 30, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "red+" ) );
	// +



	// -
    m_pSpeak = new CommandButton( "-", 130, 70, 30, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "red-" ) );
	// -

    // End - Render color Red














	
	// Boton - Render color GReen



	// +
    m_pSpeak = new CommandButton( "+", 90, 100, 30, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "green+" ) );
	// +



	// -
    m_pSpeak = new CommandButton( "-", 130, 100, 30, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "green-" ) );
	// -

    // End - Render color Blue






















		// Boton - Render color Blue

	
	//Blue:

	// +
    m_pSpeak = new CommandButton( "+", 90, 150, 30, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "blue+" ) );
	// +



	// -
    m_pSpeak = new CommandButton( "-", 130, 150, 30, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "blue-" ) );
	// -

    // End - Render color Blue






	// START - RENDER MODE








	
	//Render MODE


	// kNormal
    m_pSpeak = new CommandButton( "", 90, 200, 80, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "knormal" ) );
	// kNormal



	// kGlow
    m_pSpeak = new CommandButton( "", 90, 220, 80, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "kglow" ) );
	// kGlow




	// kTransColor
    m_pSpeak = new CommandButton( "", 90, 240, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "ktranscolor" ) );
	// kTransColor





	// kTransAlpha
    m_pSpeak = new CommandButton( "", 90, 260, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "ktransalpha" ) );
	// kTransAlpha






	// kTransAdd
    m_pSpeak = new CommandButton( "", 90, 280, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "ktransadd" ) );
	// kTransAlpha









	// kTransTexture
    m_pSpeak = new CommandButton( "", 90, 300, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "ktranstext" ) );
	// kTransTexture




    // End - Render Type




	//Start - Render Mode

	//	kRenderFxNone

	m_pSpeak = new CommandButton( "", 280, 80, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krendernone" ) );

	//	kRenderFxNone






	//	kRenderFxPulseSlow

	m_pSpeak = new CommandButton( "", 280, 100, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderpulseslow" ) );

	//	kRenderFxPulseSlow







	//	kRenderFxPulseFast

	m_pSpeak = new CommandButton( "", 280, 120, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderpulsefast" ) );

	//	kRenderFxPulseFast






		//	kRenderFxPulseSlowWide

	m_pSpeak = new CommandButton( "", 280, 140, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderpulseslowwide" ) );

	//	kRenderFxPulseSlowWide







//	kRenderFxPulseFastWide

	m_pSpeak = new CommandButton( "", 280, 160, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderpulsefastwide" ) );

	//	kRenderFxPulseFastWide





	//	kRenderFxFadeSlow

	m_pSpeak = new CommandButton( "", 280, 180, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderfadeslow" ) );

	//	kRenderFxFadeSlow





	//		kRenderFxFadeFast

	m_pSpeak = new CommandButton( "", 280, 200, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderfadefast" ) );

	//		kRenderFxFadeFast



//		kRenderFxSolidSlow

	m_pSpeak = new CommandButton( "", 280, 220, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krendersolidslow" ) );

	//		kRenderFxSolidSlow







	//		kRenderFxSolidFast

	m_pSpeak = new CommandButton( "", 280, 240, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krendersolidfast" ) );

	//		kRenderFxSolidFast




		//		kRenderFxStrobeSlow

	m_pSpeak = new CommandButton( "", 280, 260, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderstrobeslow" ) );

	//		kRenderFxStrobeSlow






	//		kRenderFxStrobeFast

	m_pSpeak = new CommandButton( "", 280, 280, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderstrobefast" ) );

	//		kRenderFxStrobeFast
	



	
	//		kRenderFxFlickerSlow

	m_pSpeak = new CommandButton( "", 280, 300, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderflickerslow" ));

	//		kRenderFxFlickerSlow





		//		kRenderFxFlickerFast

	m_pSpeak = new CommandButton( "", 280, 320, 150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderflickerfast" ));

	//		kRenderFxFlickerFast










	//		kRenderFxNoDissipation

	m_pSpeak = new CommandButton( "", 280, 340,150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krendernodissipation"));

	//		kRenderFxNoDissipation










	//		kRenderFxDistort

	m_pSpeak = new CommandButton( "", 280, 360,150, 20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderdistort"));

	//		kRenderFxDistort






		//		kRenderFxHologram

	m_pSpeak = new CommandButton( "", 280, 380 ,150,20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderhologram"));

	//		kRenderFxHologram



		//		kRenderFxExplode

	m_pSpeak = new CommandButton( "", 280, 400,150,20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderexplode"));

	//		kRenderFxExplode

	

		
	//		kRenderFxGlowShell

	m_pSpeak = new CommandButton( "", 280, 420,150,20);
    m_pSpeak->setContentAlignment( vgui::Label::a_west );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "krenderglowshell"));

	//		kRenderFxGlowShell
}