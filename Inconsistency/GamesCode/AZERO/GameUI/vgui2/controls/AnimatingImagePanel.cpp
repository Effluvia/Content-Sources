//========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <stdio.h>
#define PROTECTED_THINGS_DISABLE

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IImage.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>

#include <vgui_controls/AnimatingImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Contructor
//-----------------------------------------------------------------------------
AnimatingImagePanel::AnimatingImagePanel(Panel *parent, const char *name) : Panel(parent, name)
{
	m_iCurrentImage = 0;
	m_iFrameTimeMillis = 100;	// 10Hz frame rate
	m_iNextFrameTime = 0;
	m_pImageName = NULL;
	m_bFiltered = false;
	ivgui()->AddTickSignal(GetVPanel(), 0);
}

//-----------------------------------------------------------------------------
// Purpose: Layout the panel for drawing.
//-----------------------------------------------------------------------------
void AnimatingImagePanel::PerformLayout()
{
	Panel::PerformLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Add an image to the end of the list of animations
//-----------------------------------------------------------------------------
void AnimatingImagePanel::AddImage(IImage *image)
{
	m_Frames.AddToTail(image);

	if (image != NULL)
	{
		int wide,tall;
		image->GetSize(wide,tall);
		SetSize(wide,tall);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load a set of animations by name.
// Input: 
//		baseName: is the name of the animations without thier frame number or 
//			file extension, (e.g. c1.tga becomes just c.)
//		framecount: number of frames in the animation
//-----------------------------------------------------------------------------
void AnimatingImagePanel::LoadAnimation(const char *baseName, int frameCount)
{
	m_Frames.RemoveAll();
	for (int i = 1; i <= frameCount; i++)
	{
		char imageName[512];
		sprintf(imageName, "%s%d", baseName, i);
		AddImage(scheme()->GetImage(imageName, m_bFiltered));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw the current image
//-----------------------------------------------------------------------------
void AnimatingImagePanel::PaintBackground()
{
	if (m_Frames.IsValidIndex(m_iCurrentImage) && m_Frames[m_iCurrentImage] !=NULL)
	{
		surface()->DrawSetColor(255, 255, 255, 255);
		m_Frames[m_iCurrentImage]->Paint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame the panel is visible
//-----------------------------------------------------------------------------
void AnimatingImagePanel::OnTick()
{
	if (m_bAnimating && system()->GetTimeMillis() >= m_iNextFrameTime)
	{
		m_iNextFrameTime = system()->GetTimeMillis() + 100;
		m_iCurrentImage++;
		if (!m_Frames.IsValidIndex(m_iCurrentImage))
		{
			m_iCurrentImage = 0;
		}
		Repaint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
// Output: outResourceData- a set of keyvalues of imagenames.
//-----------------------------------------------------------------------------
void AnimatingImagePanel::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);
	if (m_pImageName)
	{
		outResourceData->SetString("image", m_pImageName);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Applies resouce settings
//-----------------------------------------------------------------------------
void AnimatingImagePanel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	const char *imageName = inResourceData->GetString("image", NULL);
	if (imageName)
	{
		delete [] m_pImageName;
		m_pImageName = new char[strlen(imageName) + 1];
		strcpy(m_pImageName, imageName);

		// add in the command
		LoadAnimation(m_pImageName, inResourceData->GetInt("frames"));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get editing details
//-----------------------------------------------------------------------------
const char *AnimatingImagePanel::GetDescription()
{
	static char buf[1024];
	_snprintf(buf, sizeof(buf), "%s, string image", BaseClass::GetDescription());
	return buf;
}

//-----------------------------------------------------------------------------
// Purpose: Starts the image doing its animation
//-----------------------------------------------------------------------------
void AnimatingImagePanel::StartAnimation()
{
	m_bAnimating = true;
//	ivgui()->AddTickSignal(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: Stops the images animation
//-----------------------------------------------------------------------------
void AnimatingImagePanel::StopAnimation()
{
	m_bAnimating = false;
//	ivgui()->RemoveTickSignal(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: Resets the animation to the start of the sequence.
//-----------------------------------------------------------------------------
void AnimatingImagePanel::ResetAnimation(int frame)
{
	if(m_Frames.IsValidIndex(frame))
	{
		m_iCurrentImage = frame;
	}
	else
	{
		m_iCurrentImage = 0;
	}
	Repaint();
//	ivgui()->RemoveTickSignal(GetVPanel());
}


//-----------------------------------------------------------------------------
// Purpose: Message map
//-----------------------------------------------------------------------------
MessageMapItem_t AnimatingImagePanel::m_MessageMap[] =
{
	MAP_MESSAGE( AnimatingImagePanel, "Tick", OnTick ),
};
IMPLEMENT_PANELMAP(AnimatingImagePanel, Panel);