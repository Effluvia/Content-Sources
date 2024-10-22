//========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================

#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>

#include "UtlSymbol.h"
#include "UtlVector.h"

namespace vgui
{

	//MODDD - new enum loc

	//MODDD - wrapped in their own namespaces instead of named enum.
	namespace AnimCommandType_e{
	//enum AnimCommandType_e
	enum enumType
	{
		CMD_ANIMATE,
		CMD_RUNEVENT,
		CMD_STOPEVENT,
		CMD_STOPANIMATION,
		CMD_STOPPANELANIMATIONS,
		CMD_SETFONT,
		CMD_SETTEXTURE,
		CMD_SETSTRING,
	};
	}

	//MODDD - wrapped in their own namespaces instead of named enum.
	namespace RelativeAlignment{
	//enum RelativeAlignment
	enum enumType
	{
		a_northwest = 0,
		a_north,
		a_northeast,
		a_west,
		a_center,
		a_east,
		a_southwest,
		a_south,
		a_southeast,
	};
	}







//-----------------------------------------------------------------------------
// Purpose: Handles controlling panel animation
//			It is never visible, but needs to be a panel so that can receive messages
//-----------------------------------------------------------------------------
class AnimationController : public Panel
{
public:
	AnimationController(Panel *parent);
	~AnimationController();

	// sets which script file to use
	bool SetScriptFile(const char *fileName, bool wipeAll =false );

	// runs a frame of animation (time is passed in so slow motion, etc. works)
	void UpdateAnimations( float curtime );

	// starts an animation sequence script
	bool StartAnimationSequence(const char *sequenceName);

	// gets the length of an animation sequence, in seconds
	float GetAnimationSequenceLength(const char *sequenceName);

	// sets that the script file should be reloaded each time a script is ran
	// used for development
	void SetAutoReloadScript(bool state);

private:
	bool LoadScriptFile(const char *fileName);
	bool ParseScriptFile(char *pMem, int length);

	bool m_bAutoReloadScript;
	float m_flCurrentTime;

	//MODDD - old enum loc

	struct RelativeAlignmentLookup
	{
		//MODDD
		//RelativeAlignment align;
		RelativeAlignment::enumType align;
		char const *name;
	};

	// a single animatable value
	// some var types use 1, 2, 3 or all 4 of the values
	struct Value_t
	{
		float a, b, c, d;
	};

	struct AnimAlign_t
	{
		// For Position, Xpos, YPos
		bool				relativePosition;
		UtlSymId_t			alignPanel;
		//MODDD
		//RelativeAlignment	alignment;
		RelativeAlignment::enumType	alignment;
	};

	// info for the animate command
	struct AnimCmdAnimate_t
	{
		UtlSymId_t panel;
		UtlSymId_t variable;
		Value_t target;
		int interpolationFunction;
		float	interpolationParameter;
		float startTime;
		float duration;

		AnimAlign_t align;

	};

	// info for the run event command
	struct AnimCmdEvent_t
	{
		UtlSymId_t event;
		UtlSymId_t variable;
		UtlSymId_t variable2;
		float timeDelay;
	};

	// holds a single command from an animation sequence
	struct AnimCommand_t
	{
		//MODDD
		//AnimCommandType_e commandType;
		AnimCommandType_e::enumType commandType;
		union
		{
			AnimCmdAnimate_t animate;
			AnimCmdEvent_t runEvent;
		} cmdData;
	};

	// holds a full sequence
	struct AnimSequence_t
	{
		UtlSymId_t name;
		float duration;
		CUtlVector<AnimCommand_t> cmdList;
	};

	// holds the list of sequences
	CUtlVector<AnimSequence_t> m_Sequences;

	// list of active animations
	struct ActiveAnimation_t
	{
		PHandle panel;
		UtlSymId_t seqName;		// the sequence this belongs to
		UtlSymId_t variable;
		bool started;
		Value_t startValue;
		Value_t endValue;
		int interpolator;
		float interpolatorParam;
		float startTime;
		float endTime;

		AnimAlign_t align;
	};
	CUtlVector<ActiveAnimation_t> m_ActiveAnimations;

	// posted messages
	struct PostedMessage_t
	{
		//MODDD
		//AnimCommandType_e commandType;
		AnimCommandType_e::enumType commandType;

		UtlSymId_t seqName;
		UtlSymId_t event;
		UtlSymId_t variable;
		UtlSymId_t variable2;
		float startTime;
	};
	CUtlVector<PostedMessage_t> m_PostedMessages;

	// variable names
	UtlSymId_t m_sPosition, m_sSize, m_sFgColor, m_sBgColor;
	UtlSymId_t m_sXPos, m_sYPos, m_sWide, m_sTall;

	// file name
	UtlSymId_t m_sScriptFileName;

	// runs a single line of the script
	void ExecAnimationCommand(UtlSymId_t seqName, AnimCommand_t &animCommand);
	// removes all commands belonging to a script
	void RemoveQueuedAnimationCommands(UtlSymId_t seqName);
	// removes an existing instance of a command
	void RemoveQueuedAnimationByType(vgui::Panel *panel, UtlSymId_t variable, UtlSymId_t sequenceToIgnore);

	// handlers
	void StartCmd_Animate(UtlSymId_t seqName, AnimCmdAnimate_t &cmd);
	void RunCmd_RunEvent(PostedMessage_t &msg);
	void RunCmd_StopEvent(PostedMessage_t &msg);
	void RunCmd_StopPanelAnimations(PostedMessage_t &msg);
	void RunCmd_StopAnimation(PostedMessage_t &msg);
	void RunCmd_SetFont(PostedMessage_t &msg);
	void RunCmd_SetTexture(PostedMessage_t &msg);
	void RunCmd_SetString(PostedMessage_t &msg);

	// value access
	Value_t GetValue(ActiveAnimation_t& anim, Panel *panel, UtlSymId_t var);
	void SetValue(ActiveAnimation_t& anim, Panel *panel, UtlSymId_t var, Value_t &value);

	// interpolation
	Value_t GetInterpolatedValue(int interpolator, float interpolatorParam, float currentTime, float startTime, float endTime, Value_t &startValue, Value_t &endValue);

	void	SetupPosition( AnimCmdAnimate_t& cmd, float *output, char const *psz, int screendimension );
	
	//MODDD
	//static RelativeAlignment LookupAlignment( char const *token );
	static RelativeAlignment::enumType LookupAlignment( char const *token );

	static RelativeAlignmentLookup g_AlignmentLookup[];

	int		GetRelativeOffset( AnimAlign_t& cmd, bool xcoord );

	typedef Panel BaseClass;
};

} // namespace vgui

#endif // ANIMATIONCONTROLLER_H
