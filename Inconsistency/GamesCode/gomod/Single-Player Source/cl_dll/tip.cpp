/*
    Copyright 2001 to 2004. The Battle Grounds Team and Contributors

    This file is part of the Battle Grounds Modification for Half-Life.

    The Battle Grounds Modification for Half-Life is free software;
    you can redistribute it and/or modify it under the terms of the
    GNU Lesser General Public License as published by the Free
    Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    The Battle Grounds Modification for Half-Life is distributed in
    the hope that it will be useful, but WITHOUT ANY WARRANTY; without
    even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the GNU Lesser General Public License
    for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with The Battle Grounds Modification for Half-Life;
    if not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA  02111-1307  USA

    You must obey the GNU Lesser General Public License in all respects for
    all of the code used other than code distributed with the Half-Life
    SDK developed by Valve.  If you modify this file, you may extend this
    exception to your version of the file, but you are not obligated to do so.
    If you do not wish to do so, delete this exception statement from your
    version.
*/

//This file was created on the 22/6/2k+2 at 14:02 by Ben Banfield for the Battle Grounds modification for Half-Life.
//Its purpose is to define the class which draws the hud background to the screen
// BEN - What is this header doing here?  This is your module bp and the purpose is wrong...

#include "STDIO.H"
#include "STDLIB.H"
#include "MATH.H"

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>

#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ServerBrowser.h"



cvar_t *g_HintBoxLevel;
extern CHudHintbox		gHintBox;

//Hintboxes Times
const float MoveIn = 1.0;
const float Stay = 7.0;
const float MoveOut = 1.0;

int CHudHintbox::Init()
{
	for (int h = 0; h < 256; h++)
		m_iSettingPlayed[h] = 0;

	m_fLastTime = 0 ;
	return 1;
}

int CHudHintbox::VidInit()
{
	m_HUD_hintbox_back = gHUD.GetSpriteIndex( "hintboxback" );
	m_hSprite_back = gHUD.GetSprite( m_HUD_hintbox_back );
	m_SpriteArea_back = &gHUD.GetSpriteRect( m_HUD_hintbox_back );
	m_bvisible = false;
	return 1;
}

int CHudHintbox::Set(int preset)
{
	if (((preset == HINTBOX_CLASS_LIGHTA) || (preset == HINTBOX_CLASS_MEDA) 
		|| (preset == HINTBOX_CLASS_HEAVYA) || (preset == HINTBOX_CLASS_LIGHTB) 
		|| (preset == HINTBOX_CLASS_MEDB) ||  (preset == HINTBOX_CLASS_HEAVYB)) 
		&& (m_bvisible == false))
	{
		m_iSetting = preset;
		StartTime = 0;
		m_bvisible = true;
	}
	else if ((!m_iSettingPlayed[preset]) && (m_bvisible == false))
	{
		m_iSetting = preset;
		m_iSettingPlayed[preset] = 1;
		StartTime = 0;
		m_bvisible = true;
	} 
	else if (m_bvisible == true)
		return 1;

	else 
		m_iSetting = 0;

	return 1;
}

int CHudHintbox::Draw(float flTime)
{
	/*
	if (g_HintBoxLevel->value == 0)
		return 1;

//	if ((m_iSetting < HINTBOX_CLASS_LIGHTA) && !(g_HintBoxLevel->value >= 1))
//		return 1;

//	if ((m_iSetting > HINTBOX_CLASS_HEAVYB) && !(g_HintBoxLevel->value >= 2))
		return 1;

	if (((int)(flTime) % 60) == 0)
		gHintBox.Set(HINTBOX_TEAMSCORE);

	if (((int)(flTime) % 60) == 0)
		gHintBox.Set(HINTBOX_PERSONALSCORE);

	if (((int)(flTime) % 60) == 0)
		gHintBox.Set(HINTBOX_MUTE);

	if (((int)(flTime) % 60) == 0)
		gHintBox.Set(HINTBOX_PROGSPAWNING);

	if (((int)(flTime) % 60) == 0)
		gHintBox.Set(HINTBOX_CAMPING);

	if (((int)(flTime) % 60) == 0)
		gHintBox.Set(HINTBOX_INACCURATE);

	if (((int)(flTime) % 60) == 0)
		gHintBox.Set(HINTBOX_HINTBOX);

	switch (gHintBox.displaymode)
	{
	case 0: //software
		gHintBox.Set(HINTBOX_DISPLAYMODE);
		break;
	case 1: // GL
		break;
	case 2: // D3D
		gHintBox.Set(HINTBOX_DISPLAYMODE);
		break;
	}
*/
		m_fLastTime = flTime;
	
	if (!StartTime)
		StartTime = flTime;
	
	if (m_iSetting)
	{
		int r_back = 0, g_back = 0, b_back = 0, x_back = 0, y_back = 0;
		int x_length_back = 0, y_length_back = 0;      // positions of the base hintbox

		y_length_back = gHUD.GetSpriteRect(m_HUD_hintbox_back).bottom - gHUD.GetSpriteRect(m_HUD_hintbox_back).top;
		x_length_back = gHUD.GetSpriteRect(m_HUD_hintbox_back).right - gHUD.GetSpriteRect(m_HUD_hintbox_back).left;

		UnpackRGB(r_back, g_back, b_back, RGB_WHITEISH);
		ScaleColors(r_back, g_back, b_back, 255);
		
		if ((flTime - StartTime) < MoveIn)
		{
			x_back = ScreenWidth - (x_length_back * (1 - exp((-(flTime - StartTime) * 6) * MoveIn))) ;
			y_back = ScreenHeight - y_length_back ;
		}
		else
		{

			if (((flTime - StartTime) < Stay) && (flTime - StartTime) > MoveOut)
			{
				x_back = ScreenWidth - x_length_back ;
				y_back = ScreenHeight - y_length_back ;
			}
			else
			{
				if (((flTime - StartTime) > Stay) && ((flTime - StartTime) < (Stay + MoveOut)))
				{
					x_back = ScreenWidth - (x_length_back * (1 - exp((-((Stay + MoveOut) - (flTime - StartTime))*6) *	MoveOut)));
					y_back = ScreenHeight - y_length_back ;
				}
				else 
				{
					x_back = ScreenWidth;
					y_back = ScreenHeight;
					m_bvisible = false;
				}
			}
		}


		// position the sprite in the lower right corner of display...
		SPR_Set(gHUD.GetSprite(m_HUD_hintbox_back), r_back, g_back, b_back);
		SPR_DrawHoles(0, x_back, y_back, m_SpriteArea_back);

		/*--------------------------
		g_HintBoxLevel = cvar indicating the hintbox level
		possible hintbox levels: (default = 1)
			0 = off
			1 = newbie
			2 = advanced
		*/

		int r = 0, g = 0, b = 0, x = 2, y = -2;
		int x_length = 0, y_length = 0; 

		char szHintbox[32];
		_snprintf(szHintbox, sizeof(szHintbox) - 1, "hintbox%i", m_iSetting); 
		m_HUD_hintbox = gHUD.GetSpriteIndex( szHintbox );

		if(!m_HUD_hintbox)
			return 1;

		m_hSprite = gHUD.GetSprite( m_HUD_hintbox );
		m_SpriteArea = &gHUD.GetSpriteRect( m_HUD_hintbox );

		y_length = gHUD.GetSpriteRect(m_HUD_hintbox).bottom - gHUD.GetSpriteRect(m_HUD_hintbox).top;
		x_length = gHUD.GetSpriteRect(m_HUD_hintbox).right - gHUD.GetSpriteRect(m_HUD_hintbox).left;

		UnpackRGB(r, g, b, RGB_WHITEISH);
		ScaleColors(r, g, b, 255);
		
		// position the sprite in the lower right corner of display...
		x = x_back + x;
		y = y_back + y ;

		SPR_Set(gHUD.GetSprite(m_HUD_hintbox), r, g, b);
		SPR_DrawHoles(0, x, y, m_SpriteArea);
	}
	return 1;
}

/*

Newcomer Hints:

jump:
Jumping takes lots of stamina thus slowing you down.
Try to jump only when it's absolutely necessary
-

crouching:
Crouching takes stamina. Musket and equipment are heavy.
But you have slightly better aim while crouching as long as you stand still.
-

low stamina:
You are low on stamina. It recharges slowly when you are moving.
Try to stand still and it will be full again in a matter of seconds.
-

reload:
While you are reloading your weapon you can only move very slowly.
In this time your are an easy target for melee attacks. 
To stop the reload process press the alterantive fire <default: right mouse button>.
-

flag capture:
If you die this flag goes 
neutral again. So stay alive
(team mates can overload the flag to secure it.)
-


overload flag:
You just overloaded a flag. This means the flag will still be held by your team if the 
player who originally captured it dies. Be carefull if all overloaders die it goes neutral again.
-

Random:
If these hints start to annoy you you can turn them off or switch to hints for advanced players
in the commandmenu <default: F>.
-

___Advanced Hints:


Melee fighting:
Strafing in melee fighting might safe your butt, but you are also likely to loose overview.
Try running in cycles keeping the enemy in view all the time.


Random:
Sometimes its better to keep enemies busy than to kill them.
This could give your team the opportunity to sneak around and break their lines from behind


In most BG maps a flag capture unlocks a spawn area for your team.
if you manage to break thru the enemy lines and reach a flag you might spawn your whole 
team behind the enemy leaving them no chance.


crouching takes stamina, but it can safe your life if you 
dodge a bayonet attack in the right moment



Ideas:
easter egg hints
*/
