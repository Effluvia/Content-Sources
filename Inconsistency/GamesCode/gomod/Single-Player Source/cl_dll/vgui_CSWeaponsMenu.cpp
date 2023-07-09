#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CCSWeaponsMenu :: CCSWeaponsMenu(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CMenuPanel(iTrans, iRemoveMe, x,y,wide,tall)
{

	int lado;
	int alto;


	int x_lado;

	lado = 10;
	alto = 70;
	
	x_lado = 30;



    CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
    SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
 
    int r, g, b, a;
  
 
    m_pPanel = new CTransparentPanel( 100, 83, 43, 565, 470);
    m_pPanel->setParent( this );
    m_pPanel->setBorder( new LineBorder( Color(255 * 0.7,170 * 0.7,0,0) ) );
 
   
 
 
    m_pTitle = new Label( "", 1, 1, 478, 58 );
    m_pTitle->setParent( m_pPanel );
    m_pTitle->setFont( pSchemes->getFont(hTitleScheme) );
    pSchemes->getFgColor( hTitleScheme, r, g, b, a );
    m_pTitle->setFgColor( r, g, b, a );
    pSchemes->getBgColor( hTitleScheme, r, g, b, a );
    m_pTitle->setBgColor( r, g, b, a );
    m_pTitle->setContentAlignment( vgui::Label::a_center );
    m_pTitle->setText( "GO-MOD COUNTER-STRIKE WEAPONS MENU" );
 



	

	//Imagen TOP
    m_pMyPicture = new CImageLabel( "top", -2, -2 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen TOP


	//Imagen Body
    m_pMyPicture = new CImageLabel( "body", -2, 65 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Body



	//CLOSE BOTON
	 m_pCancelButton = new CommandButton( gHUD.m_TextMessage.BufferedLocaliseTextString( "X" ),
                                         525, 2, 20, 20);
    m_pCancelButton->setContentAlignment(vgui::Label::a_west);
    m_pCancelButton->setParent( m_pPanel );
    m_pCancelButton->addActionSignal( new CMenuHandler_TextWindow(HIDE_TEXTWINDOW) );


	//////

		///////////   START  PRIMERA FILA \\\\\\\\\\\\\\\\\\\\


	//Imagen MP5
    m_pMyPicture = new CImageLabel( "csmp5", 1, 60+30 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen MP5

		 // Boton - MP5 Start
    m_pSpeak = new CommandButton( "", 3, 62+30, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csmp5_button" ) );
    // End - MP5





	// Imagen P228
    m_pMyPicture = new CImageLabel( "csp228", 85+x_lado, 60+30 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen P228

	// Boton - P228 Start
    m_pSpeak = new CommandButton( "", 87+x_lado, 62+30, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csp228_button" ) );
    // End - P228 






	// Imagen M3
    m_pMyPicture = new CImageLabel( "csm3", 169+x_lado+30, 60+30);
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Shotgun

	// Boton - Shotgun Start
    m_pSpeak = new CommandButton( "", 171+x_lado+30, 62+30, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csm3_button" ) );
    // End - Shotgun 







	// Imagen Glock18
    m_pMyPicture = new CImageLabel( "csglock18", 252+x_lado+60, 60+30 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Glock18

	// Boton - Glock18 Start
    m_pSpeak = new CommandButton( "", 254+x_lado+60,62+30, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csglock18_button" ) );
    // End - Glock18








	// Imagen Mac10
    m_pMyPicture = new CImageLabel( "csmac10", 336+x_lado+90,60+30 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Mac10

	// Boton - Mac10 Start
    m_pSpeak = new CommandButton( "", 338+x_lado+90, 62+30, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csmac10_button" ) );
    // End - Mac10











	// Imagen P90
    m_pMyPicture = new CImageLabel( "csp90", 336+x_lado+90, 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen P90

	// Boton - crowbar Start
    m_pSpeak = new CommandButton( "", 338+x_lado+90, 125+alto, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csp90_button" ) );
    // End - crowbar



	///////////    FIN PRIMERA FILA \\\\\\\\\\\\\\\\\\\\





	///////////    START SEGUNDA FILA \\\\\\\\\\\\\\\\\\\\

      

	// Imagen AK47
	
    m_pMyPicture = new CImageLabel( "csak47", 1, 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen AK47

		// Boton - AK47 Start
    m_pSpeak = new CommandButton( "",3, 125+alto, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csak47_button" ) );
    // End - AK47










		// Imagen M4A1
    m_pMyPicture = new CImageLabel( "csm4a1", 85+x_lado, 123+alto  );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen M4A1

	// Boton - M4A1 Start
    m_pSpeak = new CommandButton( "", 87+x_lado, 125+alto , 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csm4a1_button" ) );
    // End - M4A1 









	// Imagen USP
    m_pMyPicture = new CImageLabel( "csusp", 169+x_lado+30 , 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End -  USP

	// Boton - USP 
    m_pSpeak = new CommandButton( "", 171+x_lado+30, 125+alto , 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csusp_button" ) );
    // End - USP 











	
	// Imagen FiveSeven
    m_pMyPicture = new CImageLabel( "csfiveseven", 252+x_lado+60, 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen FiveSeven

	// Boton - FiveSeven Start
    m_pSpeak = new CommandButton( "", 254+x_lado+60, 125+alto, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csfiveseven_button" ) );

    // End - FiveSeven







		//Imagen XM1014
    m_pMyPicture = new CImageLabel( "csxm1014", 1, 223+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen XM1014

		 // Boton - XM1014 Start
    m_pSpeak = new CommandButton( "", 3, 227+alto, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csxm1014_button" ) );
    // End - XM1014




		// Imagen UMP45
    m_pMyPicture = new CImageLabel( "csump45", 85+x_lado, 223+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen UMP45

	// Boton - UMP45 Start
	 m_pSpeak = new CommandButton( "", 87+x_lado, 227+alto , 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "csump_button" ) );
    // End - UMP45








/*

		// Imagen Houndeye
    m_pMyPicture = new CImageLabel( "houndeye", XRES(366), YRES(123) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Houndeye

	// Boton - Houndeye Start
    m_pSpeak = new CommandButton( "", XRES(368), YRES(125), 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "houndeye_button" ) );
    // End - Houndeye


	///////////    FIN SEGUNDA FILA \\\\\\\\\\\\\\\\\\\\


	///////////    START TERCERA  FILA \\\\\\\\\\\\\\\\\\\\







		// Imagen Gman
    m_pMyPicture = new CImageLabel( "gman", XRES(1), YRES(190) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Gman

		// Boton - Gman Start
    m_pSpeak = new CommandButton( "", XRES(3), YRES(190), 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "gman_button" ) );
    // End - Gman










		// Imagen Bigmomma
    m_pMyPicture = new CImageLabel( "bigmomma", XRES(75), YRES(190) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Bigmomma

	// Boton - Bigmomma Start
    m_pSpeak = new CommandButton( "", XRES(76), YRES(190), 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "bigmomma_button" ) );
    // End - Bigmomma 










	// Imagen Aliengrunt
    m_pMyPicture = new CImageLabel( "aliengrunt", XRES(150), YRES(190) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Aliengrunt Vortigaunt

	// Boton - Aliengrunt Start
    m_pSpeak = new CommandButton( "", XRES(151), YRES(190), 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "aliengrunt_button" ) );
    // End - Aliengrunt 











	
	// Imagen babycrab
    m_pMyPicture = new CImageLabel( "babycrab", XRES(221), YRES(190) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen babycrab

	// Boton - babycrab Start
    m_pSpeak = new CommandButton( "", XRES(223), YRES(190), 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "babycrab_button" ) );

    // End - babycrab











		// Imagen Aliencontroller
    m_pMyPicture = new CImageLabel( "aliencontroller", XRES(292), YRES(190) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Aliencontroller

	// Boton - Aliencontroller Start
    m_pSpeak = new CommandButton( "", XRES(295), YRES(190), 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "aliencontroller_button" ) );
    // End - Aliencontroller










		// Imagen Cockroach
    m_pMyPicture = new CImageLabel( "cockroach", XRES(366), YRES(190) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Cockroach

	// Boton - Cockroach Start
    m_pSpeak = new CommandButton( "", XRES(368), YRES(190), 70, 59);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "cockroach_button" ) );
    // End - Cockroach



	/*
	 m_pMyPicture = new CImageLabel( "sparta", XRES(50), YRES(80) );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
   */

}