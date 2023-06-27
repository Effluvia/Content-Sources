

//Things needed by both hud.h (host of the entire GUI itself, CHud, with an instance gHUD), and other various child classses of CHudBase
//without redefining things or causing conflicts.


#ifndef HUDBASE_H
#define HUDBASE_H


#include "wrect.h"
#include "cl_dll.h"

//MODDD - new. Is that a good idea?
#include "cl_util.h"


#define RGB_YELLOWISH 0x00FFA000 //255,160,0
#define RGB_REDISH 0x00FF1010 //255,160,0
//MODDD - the above is false.  It is actually 255, 16, 16.
//On a side note, the values should be read like this:
//0x <- identifies as hexidecimal
//00 - transparency part
//FF - (first two #) R
//10 - (mid two #  ) G
//10 - (last two # ) B
#define RGB_GREENISH 0x0000A000 //0,160,0

//MODDD - new constant.  This is the brightness of the Pre-E3's GUI (the green one), range: 0 - 255
#define COLOR_PRE_E3_BRIGHTNESS 125


#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
//MODDD - new
#define DHN_DRAWPLACE  8
#define DHN_EMPTYDIGITSUNFADED 16

#define MIN_ALPHA	 100	

#define HUDELEM_ACTIVE	1

#define HUD_ACTIVE	1
#define HUD_INTERMISSION 2

#define MAX_PLAYER_NAME_LENGTH		32

#define MAX_MOTD_LENGTH				1536

typedef struct {
	int x, y;
} POSITION;


enum 
{ 
	MAX_PLAYERS = 64,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16,
};

typedef struct {
	unsigned char r,g,b,a;
} RGBA;

typedef struct cvar_s cvar_t;



//
//-----------------------------------------------------
//

class CHudBase
{
public:
	POSITION  m_pos;
	int   m_type;
	int   m_iFlags; // active, moving, 
	virtual		~CHudBase() {}
	virtual int Init( void ) {return 0;}
	virtual int VidInit( void ) {return 0;}
	virtual int Draw(float flTime) {return 0;}
	virtual void Think(void) {return;}
	virtual void Reset(void) {return;}
	virtual void InitHUDData( void ) {}		// called every time a server is connected to

	//MODDD - method to get Health from any other CHudBase child, and the var that the data goes to.
	//void* getHealth();
	//int (*pt2ConstMember)(float, char, char);
	//void (*getHealth)(CHudBase*);
	//typedef void(*getHealth)(CHudBase*);

};


struct HUDLIST {
	CHudBase	*p;
	HUDLIST		*pNext;
};




#endif //END OF HUDBASE_H