//========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================
#pragma warning( disable : 4244 ) // conversion from 'double' to 'float', possible loss of data

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>
#include "FileSystem.h"
#include "FileSystem_Helpers.h"
//#include <mathlib.h>

#include <stdio.h>
#include <math.h>

//MODDD - FUCK OFF
#include "mempool.h"

#include "UtlDict.h"
#include "mathlib.h"
#include "characterset.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/dbg.h>
// for SRC
#include <vstdlib/random.h>
//#include <Random.h>
#include <tier0/memdbgon.h>



//MODDD - bunch of changes to enum naming (not in the class, type accessible through '::enumType';
// See AnimationController.h for the most integral changes.
// Further changes to enum in this file after a point undocumented, too many places.  Compare to original file.


using namespace vgui;

static CUtlSymbolTable g_ScriptSymbols(0, 128, true);

enum Interpolators_e
{
	INTERPOLATOR_LINEAR,
	INTERPOLATOR_ACCEL,
	INTERPOLATOR_DEACCEL,
	INTERPOLATOR_PULSE,
	INTERPOLATOR_FLICKER,
	INTERPOLATOR_SIMPLESPLINE, // ease in / out
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
AnimationController::AnimationController(Panel *parent) : BaseClass(parent, NULL)
{
	m_bAutoReloadScript = false;

	// always invisible
	SetVisible(false);

	// get the names of common types
	m_sPosition = g_ScriptSymbols.AddString("position");
	m_sSize = g_ScriptSymbols.AddString("size"); 
	m_sFgColor = g_ScriptSymbols.AddString("fgcolor"); 
	m_sBgColor = g_ScriptSymbols.AddString("bgcolor");

	m_sXPos = g_ScriptSymbols.AddString("xpos");
	m_sYPos = g_ScriptSymbols.AddString("ypos");
	m_sWide = g_ScriptSymbols.AddString("wide");
	m_sTall = g_ScriptSymbols.AddString("tall");

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
AnimationController::~AnimationController()
{
}

//-----------------------------------------------------------------------------
// Purpose: Sets which script file to use
//-----------------------------------------------------------------------------
bool AnimationController::SetScriptFile(const char *fileName, bool wipeAll /*=false*/ )
{
	m_sScriptFileName = g_ScriptSymbols.AddString(fileName);

	// clear the current script
	m_Sequences.RemoveAll();

	if ( wipeAll )
	{
		m_ActiveAnimations.RemoveAll();
		m_PostedMessages.RemoveAll();
	}

	// load the new script file
	return LoadScriptFile(fileName);
}

//-----------------------------------------------------------------------------
// Purpose: loads a script file from disk
//-----------------------------------------------------------------------------
bool AnimationController::LoadScriptFile(const char *fileName)
{
	FileHandle_t f = filesystem()->Open(fileName, "rt", NULL);
	if (!f)
	{
		Warning("Couldn't find script file %s\n", fileName);
		return false;
	}

	// read the whole thing into memory
	int size = filesystem()->Size(f);
	// read into temporary memory block
	char *pMem = (char *)malloc(size);
	int bytesRead = filesystem()->Read(pMem, size, f);

	//MODDD - assert linker error
	//assert(bytesRead < size);

	pMem[bytesRead] = 0;
	filesystem()->Close(f);
	// parse
	bool success = ParseScriptFile(pMem, bytesRead);
	free(pMem);
	return success;
}

//MODDD - enums changed to a namespace-specific reference and moved outside of the AnimationController class
// (see AnimationController.h) so that the references here can also be straight through the namespace instead.
// Avoids warnings and likely VS6 compile issues.

AnimationController::RelativeAlignmentLookup AnimationController::g_AlignmentLookup[] =
{
	{ vgui::RelativeAlignment::a_northwest	, "northwest" },
	{ vgui::RelativeAlignment::a_north		, "north" },
	{ vgui::RelativeAlignment::a_northeast	, "northeast" },
	{ vgui::RelativeAlignment::a_west		, "west" },
	{ vgui::RelativeAlignment::a_center		, "center" },
	{ vgui::RelativeAlignment::a_east		, "east" },
	{ vgui::RelativeAlignment::a_southwest	, "southwest" },
	{ vgui::RelativeAlignment::a_south		, "south" },
	{ vgui::RelativeAlignment::a_southeast	, "southeast" },

	{ vgui::RelativeAlignment::a_northwest	, "nw" },
	{ vgui::RelativeAlignment::a_north		, "n" },
	{ vgui::RelativeAlignment::a_northeast	, "ne" },
	{ vgui::RelativeAlignment::a_west		, "w" },
	{ vgui::RelativeAlignment::a_center		, "c" },
	{ vgui::RelativeAlignment::a_east		, "e" },
	{ vgui::RelativeAlignment::a_southwest	, "sw" },
	{ vgui::RelativeAlignment::a_south		, "s" },
	{ vgui::RelativeAlignment::a_southeast	, "se" },
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *token - 
// Output : static AnimationController::RelativeAlignment
//-----------------------------------------------------------------------------

//MODDD
//AnimationController::RelativeAlignment AnimationController::LookupAlignment( char const *token )
RelativeAlignment::enumType AnimationController::LookupAlignment( char const *token )
{
	int i;
	int c = ARRAYSIZE( g_AlignmentLookup );

	for ( i = 0; i < c; i++ )
	{
		if ( !Q_stricmp( token, g_AlignmentLookup[ i ].name ) )
		{
			return g_AlignmentLookup[ i ].align;
		}
	}

	//MODDD
	//return AnimationController::RelativeAlignment::a_northwest;
	return RelativeAlignment::a_northwest;
}

//-----------------------------------------------------------------------------
// Purpose: Parse position including right edge and center adjustment out of a 
//  token.  This is relative to the screen
// Input  : *output - 
//			*psz - 
//			screendimension - 
//-----------------------------------------------------------------------------
void AnimationController::SetupPosition( AnimCmdAnimate_t& cmd, float *output, char const *psz, int screendimension )
{
	bool r = false, c = false;
	int pos;
	if ( psz[0] == '(' )
	{
		psz++;

		if ( Q_strstr( psz, ")" ) )
		{
			char sz[ 256 ];
			Q_strncpy( sz, psz, sizeof( sz ) );

			const char *colon = Q_strstr( sz, ":" );
			if ( colon )
			{
				//MODDD
				//*colon = 0;
				int relativeTo = colon - sz;
				sz[relativeTo] = 0;
				

				RelativeAlignment::enumType ra = LookupAlignment( sz );

				colon++;

				const char *panelName = colon;
				const char *panelEnd = Q_strstr( panelName, ")" );
				if ( panelEnd )
				{
					//MODDD - hope this is ok
					//*panelEnd = 0;
					int relativeTo = panelEnd - sz;
					sz[relativeTo] = 0;


					if ( Q_strlen( panelName ) > 0 )
					{
						// 
						cmd.align.relativePosition	= true;
						cmd.align.alignPanel			= g_ScriptSymbols.AddString(panelName);
						cmd.align.alignment			= ra;
					}
				}
			}

			psz = Q_strstr( psz, ")" ) + 1;
		}
	}
	else if (psz[0] == 'r' || psz[0] == 'R')
	{
		r = true;
		psz++;
	}
	else if (psz[0] == 'c' || psz[0] == 'C')
	{
		c = true;
		psz++;
	}

	// get the number
	pos = atoi(psz);

	// scale the values
	pos = vgui::scheme()->GetProportionalScaledValue( pos );

	// adjust the positions
	if (r)
	{
		pos = screendimension - pos;
	}
	if (c)
	{
		pos = (screendimension / 2) + pos;
	}

	// set the value
	*output = static_cast<float>( pos );
}

//-----------------------------------------------------------------------------
// Purpose: parses a script into sequences
//-----------------------------------------------------------------------------
bool AnimationController::ParseScriptFile(char *pMem, int length)
{
	// get the scheme (for looking up color names)
	IScheme *scheme = vgui::scheme()->GetIScheme(GetScheme());

	// get our screen size (for left/right/center alignment)
	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);

	// start by getting the first token
	char token[512];
	pMem = ParseFile(pMem, token, NULL);
	while (token[0])
	{
		// should be 'event'
		if (stricmp(token, "event"))
		{
			Warning("Couldn't parse script file: expected 'event', found '%s'\n", token);
			return false;
		}

		// get the event name
		pMem = ParseFile(pMem, token, NULL);
		if (strlen(token) < 1)
		{
			Warning("Couldn't parse script file: expected <event name>, found nothing\n");
			return false;
		}
		// create a new sequence
		int seqIndex = m_Sequences.AddToTail();
		AnimSequence_t &seq = m_Sequences[seqIndex];
		seq.name = g_ScriptSymbols.AddString(token);
		seq.duration = 0.0f;

		// get the open brace
		pMem = ParseFile(pMem, token, NULL);
		if (stricmp(token, "{"))
		{
			Warning("Couldn't parse script sequence '%s': expected '{', found '%s'\n", g_ScriptSymbols.String(seq.name), token);
			return false;
		}

		// walk the commands
		while (token && token[0])
		{
			// get the command type
			pMem = ParseFile(pMem, token, NULL);

			// skip out when we hit the end of the sequence
			if (token[0] == '}')
				break;

			// create a new command
			int cmdIndex = seq.cmdList.AddToTail();
			AnimCommand_t &animCmd = seq.cmdList[cmdIndex];
			memset(&animCmd, 0, sizeof(animCmd));
			if (!stricmp(token, "animate"))
			{
				animCmd.commandType = AnimCommandType_e::CMD_ANIMATE;
				// parse out the animation commands
				AnimCmdAnimate_t &cmdAnimate = animCmd.cmdData.animate;
				// panel to manipulate
				pMem = ParseFile(pMem, token, NULL);
				cmdAnimate.panel = g_ScriptSymbols.AddString(token);
				// variable to change
				pMem = ParseFile(pMem, token, NULL);
				cmdAnimate.variable = g_ScriptSymbols.AddString(token);
				// target value
				pMem = ParseFile(pMem, token, NULL);
				if (cmdAnimate.variable == m_sPosition)
				{
					// Get first token
					SetupPosition( cmdAnimate, &cmdAnimate.target.a, token, screenWide );

					// Get second token from "token"
					char token2[32];
					char *psz = ParseFile(token, token2, NULL);
					psz = ParseFile(psz, token2, NULL);
					psz = token2;

					// Position Y goes into ".b"
					SetupPosition( cmdAnimate, &cmdAnimate.target.b, psz, screenTall );
				}
				else if ( cmdAnimate.variable == m_sXPos )
				{
					// XPos and YPos both use target ".a"
					SetupPosition( cmdAnimate, &cmdAnimate.target.a, token, screenWide );
				}
				else if ( cmdAnimate.variable == m_sYPos )
				{
					// XPos and YPos both use target ".a"
					SetupPosition( cmdAnimate, &cmdAnimate.target.a, token, screenTall );
				}
				else 
				{
					// parse the floating point values right out
					if (0 == sscanf(token, "%f %f %f %f", &cmdAnimate.target.a, &cmdAnimate.target.b, &cmdAnimate.target.c, &cmdAnimate.target.d))
					{
						// could be referencing a value in the scheme file, lookup
						Color col = scheme->GetColor(token, Color(0, 0, 0, 0));
						cmdAnimate.target.a = col[0];
						cmdAnimate.target.b = col[1];
						cmdAnimate.target.c = col[2];
						cmdAnimate.target.d = col[3];
					}
				}

				// fix up scale
				if (cmdAnimate.variable == m_sSize)
				{
					cmdAnimate.target.a = static_cast<float>( vgui::scheme()->GetProportionalScaledValue(cmdAnimate.target.a) );
					cmdAnimate.target.b = static_cast<float>( vgui::scheme()->GetProportionalScaledValue(cmdAnimate.target.b) );
				}
				else if (cmdAnimate.variable == m_sWide ||
					     cmdAnimate.variable == m_sTall )
				{
					// Wide and tall both use.a
					cmdAnimate.target.a = static_cast<float>( vgui::scheme()->GetProportionalScaledValue(cmdAnimate.target.a) );
				}
				
				// interpolation function
				pMem = ParseFile(pMem, token, NULL);
				if (!stricmp(token, "Accel"))
				{
					cmdAnimate.interpolationFunction = INTERPOLATOR_ACCEL;
				}
				else if (!stricmp(token, "Deaccel"))
				{
					cmdAnimate.interpolationFunction = INTERPOLATOR_DEACCEL;
				}
				else if ( !stricmp(token, "Spline"))
				{
					cmdAnimate.interpolationFunction = INTERPOLATOR_SIMPLESPLINE;
				}
				else if (!stricmp(token,"Pulse"))
				{
					cmdAnimate.interpolationFunction = INTERPOLATOR_PULSE;
					// frequencey
					pMem = ParseFile(pMem, token, NULL);
					cmdAnimate.interpolationParameter = (float)atof(token);
				}
				else if ( !stricmp( token, "Flicker"))
				{
					cmdAnimate.interpolationFunction = INTERPOLATOR_FLICKER;
					// noiseamount
					pMem = ParseFile(pMem, token, NULL);
					cmdAnimate.interpolationParameter = (float)atof(token);
				}
				else
				{
					cmdAnimate.interpolationFunction = INTERPOLATOR_LINEAR;
				}
				// start time
				pMem = ParseFile(pMem, token, NULL);
				cmdAnimate.startTime = (float)atof(token);
				// duration
				pMem = ParseFile(pMem, token, NULL);
				cmdAnimate.duration = (float)atof(token);
				// check max duration
				if (cmdAnimate.startTime + cmdAnimate.duration > seq.duration)
				{
					seq.duration = cmdAnimate.startTime + cmdAnimate.duration;
				}
			}
			else if (!stricmp(token, "runevent"))
			{
				animCmd.commandType = AnimCommandType_e::CMD_RUNEVENT;
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.event = g_ScriptSymbols.AddString(token);
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.timeDelay = (float)atof(token);
			}
			else if (!stricmp(token, "stopevent"))
			{
				animCmd.commandType = AnimCommandType_e::CMD_STOPEVENT;
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.event = g_ScriptSymbols.AddString(token);
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.timeDelay = (float)atof(token);
			}
			else if (!stricmp(token, "StopPanelAnimations"))
			{
				animCmd.commandType = AnimCommandType_e::CMD_STOPPANELANIMATIONS;
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.event = g_ScriptSymbols.AddString(token);
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.timeDelay = (float)atof(token);
			}
			else if (!stricmp(token, "stopanimation"))
			{
				animCmd.commandType = AnimCommandType_e::CMD_STOPANIMATION;
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.event = g_ScriptSymbols.AddString(token);
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.variable = g_ScriptSymbols.AddString(token);
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.timeDelay = (float)atof(token);
			}
			else if ( !stricmp( token, "SetFont" ))
			{
				animCmd.commandType = AnimCommandType_e::CMD_SETFONT;
				// Panel name
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.event = g_ScriptSymbols.AddString(token);
				// Font parameter
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.variable = g_ScriptSymbols.AddString(token);
				// Font name from scheme
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.variable2 = g_ScriptSymbols.AddString(token);

				// Set time
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.timeDelay = (float)atof(token);
			}
			else if ( !stricmp( token, "SetTexture" ))
			{
				animCmd.commandType = AnimCommandType_e::CMD_SETTEXTURE;
				// Panel name
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.event = g_ScriptSymbols.AddString(token);
				// Texture Id
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.variable = g_ScriptSymbols.AddString(token);
				// material name
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.variable2 = g_ScriptSymbols.AddString(token);

				// Set time
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.timeDelay = (float)atof(token);
			}
			else if ( !stricmp( token, "SetString" ))
			{
				animCmd.commandType = AnimCommandType_e::CMD_SETSTRING;
				// Panel name
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.event = g_ScriptSymbols.AddString(token);
				// String variable name
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.variable = g_ScriptSymbols.AddString(token);
				// String value to set
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.variable2 = g_ScriptSymbols.AddString(token);

				// Set time
				pMem = ParseFile(pMem, token, NULL);
				animCmd.cmdData.runEvent.timeDelay = (float)atof(token);
			}
			else
			{
				Warning("Couldn't parse script sequence '%s': expected <anim command>, found '%s'\n", g_ScriptSymbols.String(seq.name), token);
				return false;
			}
			
		}

		// get the next token, if any
		pMem = ParseFile(pMem, token, NULL);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: runs a frame of animation
//-----------------------------------------------------------------------------
void AnimationController::UpdateAnimations( float currentTime )
{
	m_flCurrentTime = currentTime;

	// check posted messages
	{for (int i = 0; i < m_PostedMessages.Count(); i++)
	{
		PostedMessage_t &msgRef = m_PostedMessages[i];
		if (currentTime < msgRef.startTime)
			continue;

		// take a copy of th message
		PostedMessage_t msg = msgRef;

		// remove the event
		// do this before handling the message because the message queue may be messed with
		m_PostedMessages.Remove(i);
		// reset the count, start the whole queue again
		i = -1;

		// handle the event
		switch (msg.commandType)
		{
		case AnimCommandType_e::CMD_RUNEVENT:
			RunCmd_RunEvent(msg);
			break;
		case AnimCommandType_e::CMD_STOPEVENT:
			RunCmd_StopEvent(msg);
			break;
		case AnimCommandType_e::CMD_STOPPANELANIMATIONS:
			RunCmd_StopPanelAnimations(msg);
			break;
		case AnimCommandType_e::CMD_STOPANIMATION:
			RunCmd_StopAnimation(msg);
			break;
		case AnimCommandType_e::CMD_SETFONT:
			RunCmd_SetFont(msg);
			break;
		case AnimCommandType_e::CMD_SETTEXTURE:
			RunCmd_SetTexture(msg);
			break;
		case AnimCommandType_e::CMD_SETSTRING:
			RunCmd_SetString( msg );
			break;
		}
	}}

	// iterate all the currently active animations
	for (int i = 0; i < m_ActiveAnimations.Count(); i++)
	{
		ActiveAnimation_t &anim = m_ActiveAnimations[i];

		// see if the anim is ready to start
		if (currentTime < anim.startTime)
			continue;

		if (!anim.started)
		{
			// start the animation from the current value
			anim.startValue = GetValue(anim, anim.panel, anim.variable);
			anim.started = true;
		}

		// get the interpolated value
		Value_t val;
		if (currentTime > anim.endTime)
		{
			// animation is done, use the last value
			val = anim.endValue;
		}
		else
		{
			// get the interpolated value
			val = GetInterpolatedValue(anim.interpolator, anim.interpolatorParam, currentTime, anim.startTime, anim.endTime, anim.startValue, anim.endValue);
		}

		// apply the new value to the panel
		SetValue(anim, anim.panel, anim.variable, val);

		// see if we can remove the animation
		if (currentTime > anim.endTime)
		{
			m_ActiveAnimations.Remove(i);
			--i;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: produces an interpolated value
//-----------------------------------------------------------------------------
AnimationController::Value_t AnimationController::GetInterpolatedValue(int interpolator, float interpolatorParam, float currentTime, float startTime, float endTime, Value_t &startValue, Value_t &endValue)
{
	// calculate how far we are into the animation
	float pos = (currentTime - startTime) / (endTime - startTime);

	// adjust the percentage through by the interpolation function
	switch (interpolator)
	{
	case INTERPOLATOR_ACCEL:
		pos *= pos;
		break;
	case INTERPOLATOR_DEACCEL:
		pos = sqrtf(pos);
		break;
	case INTERPOLATOR_SIMPLESPLINE:
		pos = SimpleSpline( pos );
		break;
	case INTERPOLATOR_PULSE:
		// Make sure we end at 1.0, so use cosine
		pos = 0.5f + 0.5f * ( cos( pos * 2.0f * M_PI * interpolatorParam ) );
		break;
	case INTERPOLATOR_FLICKER:
		if ( RandomFloat( 0.0f, 1.0f ) < interpolatorParam )
		{
			pos = 1.0f;
		}
		else
		{
			pos = 0.0f;
		}
		break;
	case INTERPOLATOR_LINEAR:
	default:
		break;
	}

	// calculate the value
	Value_t val;
	val.a = ((endValue.a - startValue.a) * pos) + startValue.a;
	val.b = ((endValue.b - startValue.b) * pos) + startValue.b;
	val.c = ((endValue.c - startValue.c) * pos) + startValue.c;
	val.d = ((endValue.d - startValue.d) * pos) + startValue.d;
	return val;
}

//-----------------------------------------------------------------------------
// Purpose: sets that the script file should be reloaded each time a script is ran
//			used for development
//-----------------------------------------------------------------------------
void AnimationController::SetAutoReloadScript(bool state)
{
	m_bAutoReloadScript = state;
}

//-----------------------------------------------------------------------------
// Purpose: starts an animation sequence script
//-----------------------------------------------------------------------------
bool AnimationController::StartAnimationSequence(const char *sequenceName)
{
	if (m_bAutoReloadScript)
	{
		// reload the script file
		SetScriptFile(g_ScriptSymbols.String(m_sScriptFileName));
	}

	// lookup the symbol for the name
	UtlSymId_t seqName = g_ScriptSymbols.Find(sequenceName);
	if (seqName == UTL_INVAL_SYMBOL)
		return false;

	// Msg("Starting animation sequence %s\n", sequenceName);

	// remove the existing command from the queue
	RemoveQueuedAnimationCommands(seqName);

	// look through for the sequence
	int i;
	for (i = 0; i < m_Sequences.Count(); i++)
	{
		if (m_Sequences[i].name == seqName)
			break;
	}
	if (i >= m_Sequences.Count())
		return false;

	// execute the sequence
	for (int cmdIndex = 0; cmdIndex < m_Sequences[i].cmdList.Count(); cmdIndex++)
	{
		ExecAnimationCommand(seqName, m_Sequences[i].cmdList[cmdIndex]);
	}

	return true;	
}

//-----------------------------------------------------------------------------
// Purpose: gets the length of an animation sequence, in seconds
//-----------------------------------------------------------------------------
float AnimationController::GetAnimationSequenceLength(const char *sequenceName)
{
	// lookup the symbol for the name
	UtlSymId_t seqName = g_ScriptSymbols.Find(sequenceName);
	if (seqName == UTL_INVAL_SYMBOL)
		return 0.0f;

	// look through for the sequence
	int i;
	for (i = 0; i < m_Sequences.Count(); i++)
	{
		if (m_Sequences[i].name == seqName)
			break;
	}
	if (i >= m_Sequences.Count())
		return 0.0f;

	// sequence found
	return m_Sequences[i].duration;
}

//-----------------------------------------------------------------------------
// Purpose: removes an existing set of commands from the queue
//-----------------------------------------------------------------------------
void AnimationController::RemoveQueuedAnimationCommands(UtlSymId_t seqName)
{
	// Msg("Removing queued anims for sequence %s\n", g_ScriptSymbols.String(seqName));

	// remove messages posted by this sequence
	{for (int i = 0; i < m_PostedMessages.Count(); i++)
	{
		if (m_PostedMessages[i].seqName == seqName)
		{
			m_PostedMessages.Remove(i);
			--i;
		}
	}}

	// remove all animations
	for (int i = 0; i < m_ActiveAnimations.Count(); i++)
	{
		if (m_ActiveAnimations[i].seqName == seqName)
		{
			m_ActiveAnimations.Remove(i);
			--i;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: removes the specified queued animation
//-----------------------------------------------------------------------------
void AnimationController::RemoveQueuedAnimationByType(vgui::Panel *panel, UtlSymId_t variable, UtlSymId_t sequenceToIgnore)
{
	for (int i = 0; i < m_ActiveAnimations.Count(); i++)
	{
		if (m_ActiveAnimations[i].panel == panel && m_ActiveAnimations[i].variable == variable && m_ActiveAnimations[i].seqName != sequenceToIgnore)
		{
			// Msg("Removing queued anim %s::%s::%s\n", g_ScriptSymbols.String(m_ActiveAnimations[i].seqName), panel->GetName(), g_ScriptSymbols.String(variable));
			m_ActiveAnimations.Remove(i);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: runs a single line of the script
//-----------------------------------------------------------------------------
void AnimationController::ExecAnimationCommand(UtlSymId_t seqName, AnimCommand_t &animCommand)
{
	if (animCommand.commandType == AnimCommandType_e::CMD_ANIMATE)
	{
		StartCmd_Animate(seqName, animCommand.cmdData.animate);
	}
	else
	{
		// post the command to happen at the specified time
		PostedMessage_t &msg = m_PostedMessages[m_PostedMessages.AddToTail()];
		msg.seqName = seqName;
		msg.commandType = animCommand.commandType;
		msg.event = animCommand.cmdData.runEvent.event;
		msg.variable = animCommand.cmdData.runEvent.variable;
		msg.variable2 = animCommand.cmdData.runEvent.variable2;
		msg.startTime = m_flCurrentTime + animCommand.cmdData.runEvent.timeDelay;
	}
}

//-----------------------------------------------------------------------------
// Purpose: starts a variable animation
//-----------------------------------------------------------------------------
void AnimationController::StartCmd_Animate(UtlSymId_t seqName, AnimCmdAnimate_t &cmd)
{
	// make sure the child exists
	Panel *panel = GetParent()->FindChildByName(g_ScriptSymbols.String(cmd.panel),true);
	if (!panel)
		return;

	// build a command to add to the animation queue
	ActiveAnimation_t &anim = m_ActiveAnimations[m_ActiveAnimations.AddToTail()];
	anim.panel = panel;
	anim.seqName = seqName;
	anim.variable = cmd.variable;
	anim.interpolator = cmd.interpolationFunction;
	anim.interpolatorParam = cmd.interpolationParameter;
	// timings
	anim.startTime = m_flCurrentTime + cmd.startTime;
	anim.endTime = anim.startTime + cmd.duration;
	// values
	anim.started = false;
	anim.endValue = cmd.target;

	anim.align = cmd.align;
}

//-----------------------------------------------------------------------------
// Purpose: a posted message to run another event
//-----------------------------------------------------------------------------
void AnimationController::RunCmd_RunEvent(PostedMessage_t &msg)
{
	StartAnimationSequence(g_ScriptSymbols.String(msg.event));
}

//-----------------------------------------------------------------------------
// Purpose: a posted message to stop another event
//-----------------------------------------------------------------------------
void AnimationController::RunCmd_StopEvent(PostedMessage_t &msg)
{
	RemoveQueuedAnimationCommands(msg.event);
}

//-----------------------------------------------------------------------------
// Purpose: a posted message to stop all animations relevant to a specified panel
//-----------------------------------------------------------------------------
void AnimationController::RunCmd_StopPanelAnimations(PostedMessage_t &msg)
{
	Panel *panel = GetParent()->FindChildByName(g_ScriptSymbols.String(msg.event), true);
	Assert(panel != NULL);
	if (!panel)
		return;

	// loop through all the active animations cancelling any that 
	// are operating on said panel,	except for the event specified
	for (int i = 0; i < m_ActiveAnimations.Count(); i++)
	{
		if (m_ActiveAnimations[i].panel == panel && m_ActiveAnimations[i].seqName != msg.seqName)
		{
			m_ActiveAnimations.Remove(i);
			--i;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: a posted message to stop animations of a specific type
//-----------------------------------------------------------------------------
void AnimationController::RunCmd_StopAnimation(PostedMessage_t &msg)
{
	Panel *panel = GetParent()->FindChildByName(g_ScriptSymbols.String(msg.event), true);
	Assert(panel != NULL);
	if (!panel)
		return;

	RemoveQueuedAnimationByType(panel, msg.variable, msg.seqName);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &msg - 
//-----------------------------------------------------------------------------
void AnimationController::RunCmd_SetFont( PostedMessage_t &msg )
{
	Panel *panel = GetParent()->FindChildByName(g_ScriptSymbols.String(msg.event), true);
	Assert(panel != NULL);
	if (!panel)
		return;

	KeyValues *inputData = new KeyValues(g_ScriptSymbols.String(msg.variable));
	inputData->SetString(g_ScriptSymbols.String(msg.variable), g_ScriptSymbols.String(msg.variable2));
	if (!panel->SetInfo(inputData))
	{
	//	assert(!("Unhandlable var in AnimationController::SetValue())"));
	}
	inputData->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &msg - 
//-----------------------------------------------------------------------------
void AnimationController::RunCmd_SetTexture( PostedMessage_t &msg )
{
	Panel *panel = GetParent()->FindChildByName(g_ScriptSymbols.String(msg.event), true);
	Assert(panel != NULL);
	if (!panel)
		return;

	KeyValues *inputData = new KeyValues(g_ScriptSymbols.String(msg.variable));
	inputData->SetString(g_ScriptSymbols.String(msg.variable), g_ScriptSymbols.String(msg.variable2));
	if (!panel->SetInfo(inputData))
	{
	//	assert(!("Unhandlable var in AnimationController::SetValue())"));
	}
	inputData->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &msg - 
//-----------------------------------------------------------------------------
void AnimationController::RunCmd_SetString( PostedMessage_t &msg )
{
	Panel *panel = GetParent()->FindChildByName(g_ScriptSymbols.String(msg.event), true);
	Assert(panel != NULL);
	if (!panel)
		return;

	KeyValues *inputData = new KeyValues(g_ScriptSymbols.String(msg.variable));
	inputData->SetString(g_ScriptSymbols.String(msg.variable), g_ScriptSymbols.String(msg.variable2));
	if (!panel->SetInfo(inputData))
	{
	//	assert(!("Unhandlable var in AnimationController::SetValue())"));
	}
	inputData->deleteThis();
}

int AnimationController::GetRelativeOffset( AnimAlign_t& align, bool xcoord )
{
	if ( !align.relativePosition )
		return 0;

	Panel *panel = GetParent()->FindChildByName(g_ScriptSymbols.String(align.alignPanel), true);
	if ( !panel )
		return 0;

	int x, y, w, h;
	panel->GetBounds( x, y, w, h );

	int offset =0;
	switch ( align.alignment )
	{
	default:
	case RelativeAlignment::a_northwest:
		offset = xcoord ? x : y;
		break;
	case RelativeAlignment::a_north:
		offset = xcoord ? ( x + w ) / 2 : y;
		break;
	case RelativeAlignment::a_northeast:
		offset = xcoord ? ( x + w ) : y;
		break;
	case RelativeAlignment::a_west:
		offset = xcoord ? x : ( y + h ) / 2;
		break;
	case RelativeAlignment::a_center:
		offset = xcoord ? ( x + w ) / 2 : ( y + h ) / 2;
		break;
	case RelativeAlignment::a_east:
		offset = xcoord ? ( x + w ) : ( y + h ) / 2;
		break;
	case RelativeAlignment::a_southwest:
		offset = xcoord ? x : ( y + h );
		break;
	case RelativeAlignment::a_south:
		offset = xcoord ? ( x + w ) / 2 : ( y + h );
		break;
	case RelativeAlignment::a_southeast:
		offset = xcoord ? ( x + w ) : ( y +  h );
		break;
	}

	return offset;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the specified value from a panel
//-----------------------------------------------------------------------------
AnimationController::Value_t AnimationController::GetValue(ActiveAnimation_t& anim, Panel *panel, UtlSymId_t var)
{
	Value_t val = { 0, 0, 0, 0 };
	if (var == m_sPosition)
	{
		int x, y;
		panel->GetPos(x, y);
		val.a = (float)(x - GetRelativeOffset( anim.align, true ) );
		val.b = (float)(y - GetRelativeOffset( anim.align, false ) );
	}
	else if (var == m_sSize)
	{
		int w, t;
		panel->GetSize(w, t);
		val.a = (float)w;
		val.b = (float)t;
	}
	else if (var == m_sFgColor)
	{
		Color col = panel->GetFgColor();
		val.a = col[0];
		val.b = col[1];
		val.c = col[2];
		val.d = col[3];
	}
	else if (var == m_sBgColor)
	{
		Color col = panel->GetBgColor();
		val.a = col[0];
		val.b = col[1];
		val.c = col[2];
		val.d = col[3];
	}
	else if ( var == m_sXPos )
	{
		int x, y;
		panel->GetPos(x, y);
		val.a = (float)( x - GetRelativeOffset( anim.align, true ) );
	}
	else if ( var == m_sYPos )
	{
		int x, y;
		panel->GetPos(x, y);
		val.a = (float)( y - GetRelativeOffset( anim.align, false ) );
	}
	else if ( var == m_sWide )
	{
		int w, h;
		panel->GetSize(w, h);
		val.a = (float)w;
	}
	else if ( var == m_sTall )
	{
		int w, h;
		panel->GetSize(w, h);
		val.a = (float)h;
	}
	else
	{
		KeyValues *outputData = new KeyValues(g_ScriptSymbols.String(var));
		if (panel->RequestInfo(outputData))
		{
			// find the var and lookup it's type
			KeyValues *kv = outputData->FindKey(g_ScriptSymbols.String(var));
			if (kv && kv->GetDataType() == KeyValues::TYPE_FLOAT)
			{
				val.a = kv->GetFloat();
				val.b = 0.0f;
				val.c = 0.0f;
				val.d = 0.0f;
			}
			else if (kv && kv->GetDataType() == KeyValues::TYPE_COLOR)
			{
				Color col = kv->GetColor();
				val.a = col[0];
				val.b = col[1];
				val.c = col[2];
				val.d = col[3];
			}
		}
		else
		{
		//	Assert(!("Unhandlable var in AnimationController::GetValue())"));
		}
		outputData->deleteThis();
	}
	return val;
}

//-----------------------------------------------------------------------------
// Purpose: Sets a value in a panel
//-----------------------------------------------------------------------------
void AnimationController::SetValue(ActiveAnimation_t& anim, Panel *panel, UtlSymId_t var, Value_t &value)
{
	if (var == m_sPosition)
	{
		int x = (int)value.a + GetRelativeOffset( anim.align, true );
		int y = (int)value.b + GetRelativeOffset( anim.align, false );
		panel->SetPos(x, y);
	}
	else if (var == m_sSize)
	{
		panel->SetSize((int)value.a, (int)value.b);
	}
	else if (var == m_sFgColor)
	{
		Color col = panel->GetFgColor();
		col[0] = (unsigned char)value.a;
		col[1] = (unsigned char)value.b;
		col[2] = (unsigned char)value.c;
		col[3] = (unsigned char)value.d;
		panel->SetFgColor(col);
	}
	else if (var == m_sBgColor)
	{
		Color col = panel->GetBgColor();
		col[0] = (unsigned char)value.a;
		col[1] = (unsigned char)value.b;
		col[2] = (unsigned char)value.c;
		col[3] = (unsigned char)value.d;
		panel->SetBgColor(col);
	}
	else if (var == m_sXPos)
	{
		int newx = (int)value.a + GetRelativeOffset( anim.align, true );
		int x, y;
		panel->GetPos( x, y );
		x = newx;
		panel->SetPos(x, y);
	}
	else if (var == m_sYPos)
	{
		int newy = (int)value.a + GetRelativeOffset( anim.align, false );
		int x, y;
		panel->GetPos( x, y );
		y = newy;
		panel->SetPos(x, y);
	}
	else if (var == m_sWide)
	{
		int neww = (int)value.a;
		int w, h;
		panel->GetSize( w, h );
		w = neww;
		panel->SetSize(w, h);
	}
	else if (var == m_sTall)
	{
		int newh = (int)value.a;
		int w, h;
		panel->GetSize( w, h );
		h = newh;
		panel->SetSize(w, h);
	}
	else
	{
		KeyValues *inputData = new KeyValues(g_ScriptSymbols.String(var));
		// set the custom value
		if (value.b == 0.0f && value.c == 0.0f && value.d == 0.0f)
		{
			// only the first value is non-zero, so probably just a float value
			inputData->SetFloat(g_ScriptSymbols.String(var), value.a);
		}
		else
		{
			// multivalue, set the color
			Color col((unsigned char)value.a, (unsigned char)value.b, (unsigned char)value.c, (unsigned char)value.d);
			inputData->SetColor(g_ScriptSymbols.String(var), col);
		}
		if (!panel->SetInfo(inputData))
		{
		//	assert(!("Unhandlable var in AnimationController::SetValue())"));
		}
		inputData->deleteThis();
	}
}
// Hooks between panels and  animation controller system

class CPanelAnimationDictionary
{
public:
	//MODDD - FUCK OFF
	CPanelAnimationDictionary() : m_PanelAnimationMapPool( 32 )
	{
	}

	PanelAnimationMap		*FindOrAddPanelAnimationMap( char const *className );
	PanelAnimationMap		*FindPanelAnimationMap( char const *className );
	void					PanelAnimationDumpVars( char const *className );
private:

	struct PanelAnimationMapDictionaryEntry
	{
		PanelAnimationMap *map;
	};

	char const *StripNamespace( char const *className );
	void PanelAnimationDumpMap( PanelAnimationMap *map, bool recursive );

	//MODDD - FUCK OFF
	CClassMemoryPool< PanelAnimationMap > m_PanelAnimationMapPool;
	CUtlDict< PanelAnimationMapDictionaryEntry, int > m_AnimationMaps;
};


char const *CPanelAnimationDictionary::StripNamespace( char const *className )
{
	if ( !Q_strnicmp( className, "vgui::", 6 ) )
	{
		return className + 6;
	}
	return className;
}

//-----------------------------------------------------------------------------
// Purpose: Find but don't add mapping
// Input  : *className - 
// Output : PanelAnimationMap
//-----------------------------------------------------------------------------
PanelAnimationMap *CPanelAnimationDictionary::FindPanelAnimationMap( char const *className )
{
	int lookup = m_AnimationMaps.Find( StripNamespace( className ) );
	if ( lookup != m_AnimationMaps.InvalidIndex() )
	{
		return m_AnimationMaps[ lookup ].map;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *className - 
// Output : PanelAnimationMap
//-----------------------------------------------------------------------------
PanelAnimationMap *CPanelAnimationDictionary::FindOrAddPanelAnimationMap( char const *className )
{
	//MODDD - CRITICAL CRITICAL CRITICAL.  And now ...MapPool.Alloc() causing some obscure
	// linker issue in gameui including vgui_controls.lib?  UGH
	// Or just include mempool.cpp in gameui's project?   No idea as it wasn't in the as-is dsp though...

	//MODDD - FUCK OFF
	
	PanelAnimationMap *map = FindPanelAnimationMap( className );
	if ( map )
		return map;

	Panel::InitPropertyConverters();

	PanelAnimationMapDictionaryEntry entry;
	entry.map = (PanelAnimationMap *)m_PanelAnimationMapPool.Alloc();
	m_AnimationMaps.Insert( StripNamespace( className ), entry );
	return entry.map;
	

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *map - 
//			recursive - 
//-----------------------------------------------------------------------------
void CPanelAnimationDictionary::PanelAnimationDumpMap( PanelAnimationMap *map, bool recursive )
{
	if ( map->pfnClassName )
	{
		Msg( "%s\n", (*map->pfnClassName)() );
	}
	int c = map->entries.Count();
	for ( int i = 0; i < c; i++ )
	{
		PanelAnimationMapEntry *e = &map->entries[ i ];
		Msg( "  %s %s\n", e->type(), e->name() );
	}

	if ( recursive && map->baseMap )
	{
		PanelAnimationDumpMap( map->baseMap, recursive );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *className - 
//-----------------------------------------------------------------------------
void CPanelAnimationDictionary::PanelAnimationDumpVars( char const *className )
{
	if ( className == NULL )
	{
		for ( int i = 0; i < (int)m_AnimationMaps.Count(); i++ )
		{
			PanelAnimationDumpMap( m_AnimationMaps[ i ].map, false );
		}
	}
	else
	{
		PanelAnimationMap *map = FindPanelAnimationMap( className );
		if ( map )
		{
			PanelAnimationDumpMap( map, true );
		}
		else
		{
			Msg( "No such Panel Animation class %s\n", className );
		}
	}
}

CPanelAnimationDictionary& GetPanelAnimationDictionary()
{
	static CPanelAnimationDictionary dictionary;
	return dictionary;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *className - 
// Output : PanelAnimationMap
//-----------------------------------------------------------------------------
PanelAnimationMap *FindOrAddPanelAnimationMap( char const *className )
{
	return GetPanelAnimationDictionary().FindOrAddPanelAnimationMap( className );
}

//-----------------------------------------------------------------------------
// Purpose: Find but don't add mapping
// Input  : *className - 
// Output : PanelAnimationMap
//-----------------------------------------------------------------------------
PanelAnimationMap *FindPanelAnimationMap( char const *className )
{
	return GetPanelAnimationDictionary().FindPanelAnimationMap( className );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *className - 
//-----------------------------------------------------------------------------
void PanelAnimationDumpVars( char const *className )
{
	GetPanelAnimationDictionary().PanelAnimationDumpVars( className );
}