#include "cbase.h"
#include <vgui/IVgui.h>
#include <vgui/ILocalize.h>
#include "VguiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/RichText.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


class LetterText : public vgui::RichText
{
public:
	DECLARE_CLASS_SIMPLE( LetterText, vgui::RichText );

	LetterText(Panel *parent, const char *panelName) : vgui::RichText(parent, panelName)
	{
		DevMsg("*** LetterText constructed hooray\n");
		SetVerticalScrollbar( false );

		const char *text = 
"I have managed to escape from the General's compound, \n"
"and am hiding out in this apartment block.  If the \n"
"General's men find me, I'm dead.  If the creatures \n"
"find me, I'm dead.  I've placed tripmines on the \n"
"entrances to the building, and the area is booby-\n"
"trapped.  It's a poor defense but it's better than\n"
"nothing.  \n"
"\n"
"I have found an old minibus near the General's\n"
"compound that still has some fuel in it.  I believe\n"
"it could come in useful somehow.  In order to prevent\n"
"anyone else from using it, I've removed the tyres\n"
"and hidden them in various places around the town.\n"
"I altered the axle to make sure only those tires\n"
"would fit it, and marked them with the symbols from\n"
"a pack of playing cards: Hearts, Diamonds, Clubs\n"
"and Spades.\n"
"\n"
"One is hidden on the top floor of this apartment,\n"
"another is in a stall in the market place, another \n"
"is next to the barricade where the prisoners are \n"
"hanged, and the last is in the old gas station.\n"
"\n"
"The gas station is barricaded shut, but I think the\n"
"creatures got in through the sewers; there's no sign\n"
"of life there anyway.  You can get anywhere through \n"
"the sewers; I even heard a couple of 'enemies of the \n"
"people' once escaped from the prison cells through \n"
"the sewers.\n";

		SetText( text );
	}

	void ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetFont( pScheme->GetFont("LetterText", IsProportional() ) );

		// FIXME: SetFgColor(GetSchemeColor("LetterText.TextColor", pScheme)); etc

		SetFgColor( Color(0,0,0,255) );
		SetBgColor( Color(255,255,255,0)); // "paintBackground 0" not working?
	}
};

using namespace vgui;
DECLARE_BUILD_FACTORY( LetterText );