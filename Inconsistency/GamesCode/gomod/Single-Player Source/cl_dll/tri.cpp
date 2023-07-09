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

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "gl_bored.h"
#include "com_weapons.h"
#include "particle_header.h"
#include <windows.h>
#include <gl/gl.h>


#define DLLEXPORT __declspec( dllexport )
#define GL_TEXTURE_RECTANGLE_NV 0x84F5

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
};


#include "r_studioint.h"

extern engine_studio_api_t IEngineStudio;

// TEXTURES
unsigned int g_uiScreenTex = 0;
unsigned int g_uiGlowTex = 0;

// FUNCTIONS
bool InitScreenGlow(void);
void RenderScreenGlow(void);
void DrawQuad(int width, int height, int ofsX = 0, int ofsY = 0);



bool InitScreenGlow(void)
{
     // register the CVARs
     gEngfuncs.pfnRegisterVariable("glow_blur_steps", "4", 0);
     gEngfuncs.pfnRegisterVariable("glow_darken_steps", "3", 0);
     gEngfuncs.pfnRegisterVariable("glow_strength", "1", 0);

//These three CVARs will be used to control the glow. The first one controls how much the screen is blurred, the second one controls how much darker colours are decreased by before blurring, and the third one controls how strongly the glow is applied to the scene.

     // create a load of blank pixels to create textures with
     unsigned char* pBlankTex = new unsigned char[ScreenWidth*ScreenHeight*3];
     memset(pBlankTex, 0, ScreenWidth*ScreenHeight*3);

//We create a block of memory with which the textures we create will be initialised.

     // Create the SCREEN-HOLDING TEXTURE
     glGenTextures(1, &g_uiScreenTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

//This is the texture we use to grab the screen and store it so that we can re-combine it with the glow later on. Notice that we're using rectangular textures, as the screen is never a size that is a power of two in both dimensions.

     // Create the BLURRED TEXTURE
     glGenTextures(1, &g_uiGlowTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiGlowTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth/2, ScreenHeight/2, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

//This texture is used to store the blurred and glowing version of the scene. It's half the size of the screen to conserve fillrate.

     // free the memory
     delete[] pBlankTex;

     return true;
}


void DrawQuad(int width, int height, int ofsX, int ofsY)
{
     glTexCoord2f(ofsX,ofsY);
     glVertex3f(0, 1, -1);
     glTexCoord2f(ofsX,height+ofsY);
     glVertex3f(0, 0, -1);
     glTexCoord2f(width+ofsX,height+ofsY);
     glVertex3f(1, 0, -1);
     glTexCoord2f(width+ofsX,ofsY);
     glVertex3f(1, 1, -1);
}


void RenderScreenGlow(void)
{
     // check to see if (a) we can render it, and (b) we're meant to render it

     if (IEngineStudio.IsHardware() != 1)
          return;

     if ((int)gEngfuncs.pfnGetCvarFloat("glow_blur_steps") == 0 || (int)gEngfuncs.pfnGetCvarFloat("glow_strength") == 0)
          return;

     // enable some OpenGL stuff
     glEnable(GL_TEXTURE_RECTANGLE_NV);     
     glColor3f(1,1,1);
     glDisable(GL_DEPTH_TEST);


     // STEP 1: Grab the screen and put it into a texture

     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);


     // STEP 2: Set up an orthogonal projection

     glMatrixMode(GL_MODELVIEW);
     glPushMatrix();
     glLoadIdentity();
     
     glMatrixMode(GL_PROJECTION);
     glPushMatrix();
     glLoadIdentity();
     glOrtho(0, 1, 1, 0, 0.1, 100);


     // STEP 3: Render the current scene to a new, lower-res texture, darkening non-bright areas of the scene
     // by multiplying it with itself a few times.

     glViewport(0, 0, ScreenWidth/2, ScreenHeight/2);

     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);

     glBlendFunc(GL_DST_COLOR, GL_ZERO);
     
     glDisable(GL_BLEND);

     glBegin(GL_QUADS);
     DrawQuad(ScreenWidth, ScreenHeight);
     glEnd();

     glEnable(GL_BLEND);

     glBegin(GL_QUADS);
     for (int i = 0; i < (int)gEngfuncs.pfnGetCvarFloat("glow_darken_steps"); i++)
          DrawQuad(ScreenWidth, ScreenHeight);
     glEnd();

     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiGlowTex);
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth/2, ScreenHeight/2, 0);


     // STEP 4: Blur the now darkened scene in the horizontal direction.

     float blurAlpha = 1 / (gEngfuncs.pfnGetCvarFloat("glow_blur_steps")*2 + 1);

     glColor4f(1,1,1,blurAlpha);

     glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

     glBegin(GL_QUADS);
     DrawQuad(ScreenWidth/2, ScreenHeight/2);
     glEnd();

     glBlendFunc(GL_SRC_ALPHA, GL_ONE);

     glBegin(GL_QUADS);
     for (i = 1; i <= (int)gEngfuncs.pfnGetCvarFloat("glow_blur_steps"); i++) {
          DrawQuad(ScreenWidth/2, ScreenHeight/2, -i, 0);
          DrawQuad(ScreenWidth/2, ScreenHeight/2, i, 0);
     }
     glEnd();
     
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth/2, ScreenHeight/2, 0);


     // STEP 5: Blur the horizontally blurred image in the vertical direction.

     glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

     glBegin(GL_QUADS);
     DrawQuad(ScreenWidth/2, ScreenHeight/2);
     glEnd();

     glBlendFunc(GL_SRC_ALPHA, GL_ONE);

     glBegin(GL_QUADS);
     for (i = 1; i <= (int)gEngfuncs.pfnGetCvarFloat("glow_blur_steps"); i++) {
          DrawQuad(ScreenWidth/2, ScreenHeight/2, 0, -i);
          DrawQuad(ScreenWidth/2, ScreenHeight/2, 0, i);
     }
     glEnd();
     
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth/2, ScreenHeight/2, 0);


     // STEP 6: Combine the blur with the original image.

     glViewport(0, 0, ScreenWidth, ScreenHeight);

     glDisable(GL_BLEND);

     glBegin(GL_QUADS);
     DrawQuad(ScreenWidth/2, ScreenHeight/2);
     glEnd();

     glEnable(GL_BLEND);
     glBlendFunc(GL_ONE, GL_ONE);

     glBegin(GL_QUADS);
     for (i = 1; i < (int)gEngfuncs.pfnGetCvarFloat("glow_strength"); i++) {
          DrawQuad(ScreenWidth/2, ScreenHeight/2);
     }
     glEnd();

     glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_uiScreenTex);

     glBegin(GL_QUADS);
     DrawQuad(ScreenWidth, ScreenHeight);
     glEnd();


     // STEP 7: Restore the original projection and modelview matrices and disable rectangular textures.

     glMatrixMode(GL_PROJECTION);
     glPopMatrix();

     glMatrixMode(GL_MODELVIEW);
     glPopMatrix();

     glDisable(GL_TEXTURE_RECTANGLE_NV);
     glEnable(GL_DEPTH_TEST);
     glDisable(GL_BLEND);
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

	
		//Shader Water
		g_Effects.WaterPlane( ); //Hide old and boring boring water brushes

}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
class CException;
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{

#if defined( TEST_IT )
//	Draw_Triangles();
#endif

	try {
		pParticleManager->UpdateSystems();
	} catch( CException *e ) {
		e;
		e = NULL;
		gEngfuncs.Con_Printf("There was a serious error within the particle engine. Particles will return on map change\n");
		delete pParticleManager;
		pParticleManager = NULL;
	}

	//Shader Water
		if( gEngfuncs.pfnGetCvarFloat( "cg_water" ) != 0) //If enabled, render the new water
		{
			g_Effects.PreRender();
			g_Effects.Render();
			g_Effects.PostRender();
		}


}