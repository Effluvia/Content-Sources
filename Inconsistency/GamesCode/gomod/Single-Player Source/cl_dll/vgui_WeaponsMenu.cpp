#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CWeaponsMenu :: CWeaponsMenu(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CMenuPanel(iTrans, iRemoveMe, x,y,wide,tall)
{

	int lado;
	int alto;

	lado = 10;
	alto = 60;


   
    CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
    SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
 
   
    
    m_pPanel = new CTransparentPanel( 200, 80, 40, 530, 500);
    m_pPanel->setParent( this );
    m_pPanel->setBorder( new LineBorder( Color(255 * 0.7,170 * 0.7,0,0) ) );
 

	//Imagen Body
    m_pMyPicture = new CImageLabel( "body_hlweapons", 1, 1 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Body

    m_pCancelButton = new CommandButton( gHUD.m_TextMessage.BufferedLocaliseTextString( "X" ),
                                         490, 1, 30, 30);
    m_pCancelButton->setContentAlignment(vgui::Label::a_center);
    m_pCancelButton->setParent( m_pPanel );
    m_pCancelButton->addActionSignal( new CMenuHandler_TextWindow(HIDE_TEXTWINDOW) );
 

   

		///////////   START  PRIMERA FILA \\\\\\\\\\\\\\\\\\\\


	//Imagen 9mmhandgun
    m_pMyPicture = new CImageLabel( "handgun", 1, 90 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen 9mmhandgun

		 // Boton - 9mmhandgun Start
    m_pSpeak = new CommandButton( "", 3, 92, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "9mm_button" ) );
    // End - 9mmhandgun





	// Imagen 9mmAR
    m_pMyPicture = new CImageLabel( "9mmAR", 85+lado+10, 90 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen 9mmAR

	// Boton - 9mmAR Start
    m_pSpeak = new CommandButton( "", 87+lado+10, 92, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "9mmAR_button" ) );
    // End - 9mmAR 






	// Imagen Shotgun
    m_pMyPicture = new CImageLabel( "shotgun", 169+lado+30, 90 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Shotgun

	// Boton - Shotgun Start
    m_pSpeak = new CommandButton( "", 171+lado+30, 92, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "shotgun_button" ) );
    // End - Shotgun 







	// Imagen Gauss
    m_pMyPicture = new CImageLabel( "gauss", 252+lado+50, 90 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Gauss

	// Boton - Gauss Start
    m_pSpeak = new CommandButton( "", 254+lado+50, 92, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "gauss_button" ) );
    // End - Gauss








	// Imagen Grenade
    m_pMyPicture = new CImageLabel( "grenade", 336+lado+70,90 );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Grenade

	// Boton - Grenade Start
    m_pSpeak = new CommandButton( "", 338+lado+70, 92, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "grenade_button" ) );
    // End - Grenade











	// Imagen crowbar
    m_pMyPicture = new CImageLabel( "crowbar", 336+lado+70, 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen crowbar

	// Boton - crowbar Start
    m_pSpeak = new CommandButton( "", 338+lado+70, 125+alto, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "crowbar_button" ) );
    // End - crowbar



	///////////    FIN PRIMERA FILA \\\\\\\\\\\\\\\\\\\\





	///////////    START SEGUNDA FILA \\\\\\\\\\\\\\\\\\\\

      

		// Imagen Egon
	
    m_pMyPicture = new CImageLabel( "egon", 1, 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Egon

		// Boton - Egon Start
    m_pSpeak = new CommandButton( "", 3, 125+alto, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "egon_button" ) );
    // End - Egon










		// Imagen satchel
    m_pMyPicture = new CImageLabel( "satchel", 85+lado+10, 123+alto  );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen satchel

	// Boton - satchel Start
    m_pSpeak = new CommandButton( "", 87+lado+10, 125+alto , 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "satchel_button" ) );
    // End - satchel 










	// Imagen Rpg
    m_pMyPicture = new CImageLabel( "rpg", 169+lado+30 , 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Barney Rpg

	// Boton - Rpg Start
    m_pSpeak = new CommandButton( "", 171+lado+30, 125+alto , 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "rpg_button" ) );
    // End - Rpg 











	
	// Imagen 357
    m_pMyPicture = new CImageLabel( "357", 252+lado+50, 123+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen 357

	// Boton - 357 Start
    m_pSpeak = new CommandButton( "", 254+lado+50, 125+alto, 96, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "357_button" ) );

    // End - 357







		//Imagen RemoveTool
    m_pMyPicture = new CImageLabel( "removetool", 1, 213+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen RemoveTool

		 // Boton - RemoveTool Start
    m_pSpeak = new CommandButton( "", 3, 215+alto, 97, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "removetool_button" ) );
    // End - RemoveTool




	// Imagen CrossBow
    m_pMyPicture = new CImageLabel( "crossbow", 85+lado+10, 213+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen CrossBow

	// Boton - CrossBow Start
    m_pSpeak = new CommandButton( "", 87+lado+10, 215+alto, 95, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "crossbow_button" ) );
    // End - CrossBow






		// Imagen Physgun
    m_pMyPicture = new CImageLabel( "physgun", 169+lado+30, 213+alto );
    m_pMyPicture->setVisible( true );
    m_pMyPicture->setParent( m_pPanel );
    // End - Imagen Physgun

	// Boton - Physgun Start
    m_pSpeak = new CommandButton( "", 170+lado+30, 215+alto, 95, 85);
    m_pSpeak->setContentAlignment( vgui::Label::a_center );
    m_pSpeak->setParent( m_pPanel );
    m_pSpeak->addActionSignal( new CMenuHandler_StringCommand( "physgun" ) );
    // End - Physgun








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