//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "external_lib_include.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

//MODDD - yea.
//#include "windows.h"
#include "glInclude.h"

EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(fogNear)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(fogFar)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(fogTest)

//EASY_CVAR_EXTERN_CLIENT_MASS



extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
};



// Triangle rendering apis are in gEngfuncs.pTriAPI


float fogColor[3];
float nextFogColorChange = -1;

//THANK YOU SPIRIT OF HALF LIFE 1.9!!!
void RenderFog ( float currentTime )
{
	//float g_fFogColor[4] = { FogColor.x, FogColor.y, FogColor.z, 1.0 };
	//bool bFog = g_iWaterLevel < 2 && g_fStartDist > 0 && g_fEndDist > 0;
	float fogTestVar = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(fogTest);

	//easyPrintLine("CURRENTTIMEZ %.2f %,2f", currentTime, nextFogColorChange);

	if(fogTestVar == 1){
		
		fogColor[0] = 0.2f;
		fogColor[1] = 0.8f;
		fogColor[2] = 0.2f;
	}else if(fogTestVar == 2){

		BOOL canChangecolor = FALSE;
		if(currentTime > -1){
			canChangecolor = (currentTime >= nextFogColorChange);
		}else{
			canChangecolor = TRUE;
		}
		if(canChangecolor){
			generateColorDec(fogColor);

			//easyPrintLine("MY COLOR: %.2f %.2f %.2f", fogColor[0], fogColor[1], fogColor[2]);

			if(currentTime > -1){
				nextFogColorChange = currentTime + 0.37;
			}
		}
		
	}else{
		//no fog for you.
		return;
	}


	float g_fFogColor[4] = { fogColor[0], fogColor[1], fogColor[2], 1.0 };

	if (TRUE)
	{
		glEnable(GL_FOG);
		//glFogf(GL_FOG_DENSITY, 0.0025);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, g_fFogColor );
		glFogf(GL_FOG_START, EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(fogNear));
		glFogf(GL_FOG_END, EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(fogFar));
		glHint(GL_FOG_HINT, GL_NICEST);
	}
	
	/*
    glClear(GL_FOG_BIT);
	glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
    glClear(GL_FOG_BIT);
	glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
	*/


}
void RenderFog(){
	RenderFog(-1);
}



//#define TEST_IT
#if defined( TEST_IT )

/*
=================
Draw_Triangles

Example routine.  Draws a sprite offset from the player origin.
=================
*/
void Draw_Triangles( void )
{
	cl_entity_t *player;
	vec3_t org;

	// Load it up with some bogus data
	player = gEngfuncs.GetLocalPlayer();
	if ( !player )
		return;

	org = player->origin;

	org.x += 50;
	org.y += 50;

	if (gHUD.m_hsprCursor == 0)
	{
		char sz[256];
		sprintf( sz, "sprites/cursor.spr" );
		gHUD.m_hsprCursor = SPR_Load( sz );
	}

	if ( !gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_hsprCursor ), 0 ))
	{
		return;
	}
	
	// Create a triangle, sigh
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
	// Overload p->color with index into tracer palette, p->packedColor with brightness
	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );
	// UNDONE: This gouraud shading causes tracers to disappear on some cards (permedia2)
	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y, org.z );

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

#endif

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{

	gHUD.m_Spectator.DrawOverview();
	
#if defined( TEST_IT )
//	Draw_Triangles();
#endif
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{

#if defined( TEST_IT )
//	Draw_Triangles();
#endif
}