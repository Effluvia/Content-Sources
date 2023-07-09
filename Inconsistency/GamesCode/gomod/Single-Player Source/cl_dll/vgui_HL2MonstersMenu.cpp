#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CHL2MonstersMenu :: CHL2MonstersMenu(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CMenuPanel(iTrans, iRemoveMe, x,y,wide,tall)
{
   
    CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
    SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
 
    int r, g, b, a;
 

 
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

	
	// Imagen Hl2_Zombie
    m_pMyPicture = new CImageLabel( "hl2_zombie", 1, 60);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Hl2_Zombie

		 // Boton - Hl2_Zombie Start
    m_pSpeak = new CommandButton( "", 3, 62, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "hl2_zombie_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Hl2_Zombie





	// Imagen Headcrab
    m_pMyPicture = new CImageLabel( "headcrab", 75, 60 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Headcrab

	// Boton - Headcrab Start
    m_pSpeak = new CommandButton( "",76, 62, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "hl2_headcrab_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Headcrab 



/*


	// Imagen Vortigaunt
    m_pMyPicture = new CImageLabel( "alienslave", 150, 60 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Vortigaunt

	// Boton - Vortigaunt Start
    m_pSpeak = new CommandButton( "", 151, 62, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "alienslave_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Vortigaunt 







	// Imagen Zombie
    m_pMyPicture = new CImageLabel( "zombie", 221, 60 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Zombie

	// Boton - Zombie Start
    m_pSpeak = new CommandButton( "", 223, 62, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "zombie_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Zombie








	// Imagen Bullchicken
    m_pMyPicture = new CImageLabel( "bullchicken", 292, 60 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Bullchicken

	// Boton - Bullchicken Start
    m_pSpeak = new CommandButton( "", 295, 62, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "bull_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Bullchicken











	// Imagen Leech
    m_pMyPicture = new CImageLabel( "leech", 366, 60 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Leech

	// Boton - Leech Start
    m_pSpeak = new CommandButton( "", 368, 62, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "leech_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Leech



	///////////    FIN PRIMERA FILA \\\\\\\\\\\\\\\\\\\\





	///////////    START SEGUNDA FILA \\\\\\\\\\\\\\\\\\\\



		// Imagen Sentry
    m_pMyPicture = new CImageLabel( "sentry", 1, 123 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Sentry

		// Boton - Sentry Start
    m_pSpeak = new CommandButton( "", 3, 125, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "sentry_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Sentry










		// Imagen Tentacle
    m_pMyPicture = new CImageLabel( "tentacle", 75, 123 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Tentacle

	// Boton - Tentacle Start
    m_pSpeak = new CommandButton( "", 76, 125, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "tentacle_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Tentacle 










	// Imagen Barney
    m_pMyPicture = new CImageLabel( "barney", 150, 123 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Barney Vortigaunt

	// Boton - Barney Start
    m_pSpeak = new CommandButton( "", 151, 125, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "barney_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Barney 











	
	// Imagen Nihilant
    m_pMyPicture = new CImageLabel( "nihilant", 221, 123 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Nihilant

	// Boton - Nihilant Start
    m_pSpeak = new CommandButton( "", 223, 125, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "nihilant_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );

    // End - Nihilant











		// Imagen Scientist
    m_pMyPicture = new CImageLabel( "scientist", 292, 123 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Scientist

	// Boton - Scientist Start
    m_pSpeak = new CommandButton( "", 295, 125, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "scientist_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Scientist










		// Imagen Houndeye
    m_pMyPicture = new CImageLabel( "houndeye", 366, 123 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Houndeye

	// Boton - Houndeye Start
    m_pSpeak = new CommandButton( "", 368, 125, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "houndeye_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Houndeye


	///////////    FIN SEGUNDA FILA \\\\\\\\\\\\\\\\\\\\


	///////////    START TERCERA  FILA \\\\\\\\\\\\\\\\\\\\







		// Imagen Gman
    m_pMyPicture = new CImageLabel( "gman", 1, 190 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Gman

		// Boton - Gman Start
    m_pSpeak = new CommandButton( "", 3, 190, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "gman_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Gman










		// Imagen Bigmomma
    m_pMyPicture = new CImageLabel( "bigmomma", 75, 190 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Bigmomma

	// Boton - Bigmomma Start
    m_pSpeak = new CommandButton( "", 76, 190, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "bigmomma_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Bigmomma 










	// Imagen Aliengrunt
    m_pMyPicture = new CImageLabel( "aliengrunt", 150, 190 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Aliengrunt Vortigaunt

	// Boton - Aliengrunt Start
    m_pSpeak = new CommandButton( "", 151, 190, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "aliengrunt_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Aliengrunt 











	
	// Imagen babycrab
    m_pMyPicture = new CImageLabel( "babycrab", 221, 190 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen babycrab

	// Boton - babycrab Start
    m_pSpeak = new CommandButton( "", 223, 190, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "babycrab_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - babycrab











		// Imagen Aliencontroller
    m_pMyPicture = new CImageLabel( "aliencontroller", 292, 190 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Aliencontroller

	// Boton - Aliencontroller Start
    m_pSpeak = new CommandButton( "", 295, 190, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "aliencontroller_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Aliencontroller










		// Imagen Cockroach
    m_pMyPicture = new CImageLabel( "cockroach", 366, 190 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Cockroach

	// Boton - Cockroach Start
    m_pSpeak = new CommandButton( "", 368, 190, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "cockroach_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Cockroach






	// Boton - ENABLE IA Start
    m_pSpeak = new CommandButton( "", 50, 390, 100, 42);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "enableia_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(250 * 0.7,50 * 0.7,50,0) ) );
    // End - ENABLE IA





	// Boton - DISABLE IA Start
    m_pSpeak = new CommandButton( "", 167, 390, 100, 42);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "disableia_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(250 * 0.7,50 * 0.7,50,0) ) );
    // End - DISABLE IA


	////////////// START CUARTA FILA \\\\\\\\\\\\\\\\\\\\\\\\\\

		
	// Imagen Gargantua
    m_pMyPicture = new CImageLabel( "gargantua", 1, 250);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Gargantua Hgrunt

		 // Boton - Gargantua Start
    m_pSpeak = new CommandButton( "", 3, 250, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "gargantua_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Gargantua










	// Imagen Hassassin
    m_pMyPicture = new CImageLabel( "hassassin", 75, 250);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Hassassin 

		 // Boton - Hassassin Start
    m_pSpeak = new CommandButton( "", 76, 250, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "hassassin_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Hassassin






	// Imagen Gonome
    m_pMyPicture = new CImageLabel( "gonome", 150, 250);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Gonome 

		 // Boton - Gonome Start
    m_pSpeak = new CommandButton( "", 151, 250, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "gonome_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Gonome






		// Imagen Massn
    m_pMyPicture = new CImageLabel( "massn", 221, 250);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Massn 

	// Boton - Massn Start
    m_pSpeak = new CommandButton( "", 223, 250, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "massn_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Massn






		// Imagen PitDrone
    m_pMyPicture = new CImageLabel( "pitdrone", 292, 250);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - PitDrone 

	// Boton - PitDrone Start
    m_pSpeak = new CommandButton( "", 295, 250, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "pitdrone_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - PitDrone










	// Imagen Ally Sentry
    m_pMyPicture = new CImageLabel( "ally_sentry", 366, 250);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Ally Sentry 


	// Boton - Ally Sentry Start
    m_pSpeak = new CommandButton( "", 368, 250, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "ally_sentry_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Ally Sentry











	// Imagen Ally Otis
    m_pMyPicture = new CImageLabel( "otis", 1, 310);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Ally Otis 


	// Boton - Ally Otis Start
    m_pSpeak = new CommandButton( "", 3, 310, 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "otis_button" ) );
	m_pSpeak->setBorder( new LineBorder( Color(50 * 0.7,50 * 0.7,200,0) ) );
    // End - Ally Otis



	/*
	 m_pMyPicture = new CImageLabel( "sparta", XRES(50), YRES(80) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
	*/
}




void CHL2MonstersMenu :: SetActiveInfo( int iShowText )
{
   
	iShowText = iShowText + 1;
}