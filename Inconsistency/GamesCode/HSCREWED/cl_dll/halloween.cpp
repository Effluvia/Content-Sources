//
// Halloween.cpp
//
// implementation of the secret of Halloween
//

#include "STDIO.H"
#include "STDLIB.H"
#include "MATH.H"

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>

char const * billymc [] = {
	"Victim #1",
	"Victim #2. Although he was still alive when he\nwas found starving in an\nabandoned apartment, he\ndied from blood loss on\nthe way to the hospital.\nThe place has apparently\nno tenant or owner",
	"The Three is The Terrible Treat for Those who Think",
	"YOU ENJOYTHIS DO YOU NOT?",
	"Does anyone suffer BEFORE?",
    "FEED ME A STRAY CAT",
    "EVERYONE IS GIYGAS, DONT BELIEVE THEIR LIES. YOU ARE RIGHT.",
    "Right?!!/",
    "I THouGHT Y/OU, LIKE,D, HALLOWEEN, ?!",
    "The pain reflects onto you. We all project pain onto others. To make them understand.",
    "GET BACK TO WHERE YOU BELONG? YOU HAVE MADE SUFFERING ENOUGH!",
    "THIS IS EVIL, STOP THIS THIS IS EVIL.",
    "HE HE HE HE HE :)",
    "AOBTD, I feel great about this! Is it not so?",
    "T0ggTVkgR09E\\\\STOP",
    "TONIGHT’S THE NIGHT; WHY HERE?",
    "17 Mothers without sons, Are yoU haPpy?",
    "I like scissors! 61!",
    "[Picture of Mario without eyes appears] MA MA  MI YA",
    "The Black Wind Howls!",
    "why iS There yOu ProteCting what yOU Never ThINk is right aGain??",
    "WHY DO YOU NOT LEAVE AND LET LIVE?",
    "Game Over!",
    "614122876039093081593306193923589",
    "Do you realise??",
    ":)",
    ":(",
    "[Playername] was banned by the admin?",
    "RG8geW91IHJlYWxseSBlbmpveSB0aGlzIHNlbnNlbGVzcyBraWxsaW5nPyBUaGVzZSBwbGF5ZXJzIGFyZSByZWFsOyB0aGUgYm90cyBhcmUgYWxzbyByZWFsIHRvZGF5LiBUaGV5IGxpdmUgYW5kIGRpZSB3aXRoaW4gdGhlIGdhbWUgYW5kIGdldCBxdWlja2x5IHJlcGxhY2VkIHdoZW4geW91IGtpbGwgdGhlbS4gT25seSBvbiB0b2RheS4gVGhpcyBpcyBhbGwgdHJ1ZS4gVGhhdCBib3QgaXMgcmVhbC4gSSBob3BlIHlvdSBlbmpveSB5b3Vyc2VsZiwgS2lsbGVyLiAyIDMgMSA0IElTIFRIRSBXQVk=",
	"K!" };

DECLARE_MESSAGE( m_Halloween, Halloween )

int CHudHalloween::Init(void)
{
	HOOK_MESSAGE(Halloween);

	gHUD.AddHudElem(this);
	return 1;
}

void CHudHalloween::Reset( void )
{
//	insanity = 0; //reset the player's sanity
}

int CHudHalloween::VidInit(void)
{
//	insanity = 0;
	return 1;
}

int CHudHalloween::MsgFunc_Halloween(const char *pszName,  int iSize, void *pbuf )
{
	// TODO: update local health data
	BEGIN_READ( pbuf, iSize );
	int x = READ_BYTE();

	m_iFlags |= HUD_ACTIVE;

	insanity = x;

	return 1;
}

int CHudHalloween::Draw(float flTime)
{
	if (!insanity)
		return 1;

	if (insanity >= 1)
	{
		m_cChatter = "TEST";

	gHUD.DrawHudString(ScreenWidth/2, ScreenHeight/2, ScreenWidth, m_cChatter, 0xFF, 0xFF, 0xFF); //s.o. 
	}

	return 1;
}