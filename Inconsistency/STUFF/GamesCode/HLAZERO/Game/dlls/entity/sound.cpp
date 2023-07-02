/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//=========================================================
// sound.cpp 
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "talkmonster.h"
#include "gamerules.h"

//MODDD
EASY_CVAR_EXTERN_DEBUGONLY(sparksComputerHitMulti)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)

EASY_CVAR_EXTERN_DEBUGONLY(announcerIsAJerk)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)



#define CDPVPRESETMAX 27

//MODDD - IMPORTANT: changed.  How it do?
//#define CSENTENCE_LRU_MAX	32		// max number of elements per sentence group
#define CSENTENCE_LRU_MAX	64		// max number of elements per sentence group


//MODDD - IMPORTANT: TODO: could we maybe edit this...?
#define CSENTENCEG_MAX 200					// max number of sentence groups
// globals

//MODDD - less frequent, was 0.25 to 2.25 minutes.
#define ANNOUNCE_MINUTES_MIN	0.64
#define ANNOUNCE_MINUTES_MAX	2.38




static char *memfgets( byte *pMemFile, int fileSize, int &filePos, char *pBuffer, int bufferSize );


// ==================== GENERIC AMBIENT SOUND ======================================

// runtime pitch shift and volume fadein/out structure

// NOTE: IF YOU CHANGE THIS STRUCT YOU MUST CHANGE THE SAVE/RESTORE VERSION NUMBER
// SEE BELOW (in the typedescription for the class)
typedef struct dynpitchvol
{
	// NOTE: do not change the order of these parameters 
	// NOTE: unless you also change order of rgdpvpreset array elements!
	int preset;

	int pitchrun;		// pitch shift % when sound is running 0 - 255
	int pitchstart;		// pitch shift % when sound stops or starts 0 - 255
	int spinup;			// spinup time 0 - 100
	int spindown;		// spindown time 0 - 100

	int volrun;			// volume change % when sound is running 0 - 10
	int volstart;		// volume change % when sound stops or starts 0 - 10
	int fadein;			// volume fade in time 0 - 100
	int fadeout;		// volume fade out time 0 - 100

						// Low Frequency Oscillator
	int lfotype;		// 0) off 1) square 2) triangle 3) random
	int lforate;		// 0 - 1000, how fast lfo osciallates
	
	int lfomodpitch;	// 0-100 mod of current pitch. 0 is off.
	int lfomodvol;		// 0-100 mod of current volume. 0 is off.

	int cspinup;		// each trigger hit increments counter and spinup pitch


	int cspincount;

	int pitch;			
	int spinupsav;
	int spindownsav;
	int pitchfrac;

	int vol;
	int fadeinsav;
	int fadeoutsav;
	int volfrac;

	int lfofrac;
	int lfomult;


} dynpitchvol_t;


// presets for runtime pitch and vol modulation of ambient sounds

dynpitchvol_t rgdpvpreset[CDPVPRESETMAX] = 
{
// pitch	pstart	spinup	spindwn	volrun	volstrt	fadein	fadeout	lfotype	lforate	modptch modvol	cspnup		
{1,	255,	 75,	95,		95,		10,		1,		50,		95, 	0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0}, 
{2,	255,	 85,	70,		88,		10,		1,		20,		88,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0}, 
{3,	255,	100,	50,		75,		10,		1,		10,		75,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{4,	100,	100,	0,		0,		10,		1,		90,		90,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{5,	100,	100,	0,		0,		10,		1,		80,		80,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{6,	100,	100,	0,		0,		10,		1,		50,		70,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{7,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		50,		0,		10,		0,		0,0,0,0,0,0,0,0,0,0},
{8,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		150,	0,		10,		0,		0,0,0,0,0,0,0,0,0,0},
{9,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		750,	0,		10,		0,		0,0,0,0,0,0,0,0,0,0},
{10,128,	100,	50,		75,		10,		1,		30,		40,		2,		 8,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{11,128,	100,	50,		75,		10,		1,		30,		40,		2,		25,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{12,128,	100,	50,		75,		10,		1,		30,		40,		2,		70,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{13,50,		 50,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{14,70,		 70,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{15,90,		 90,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{16,120,	120,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{17,180,	180,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{18,255,	255,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{19,200,	 75,	90,		90,		10,		1,		50,		90,		2,		100,	20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{20,255,	 75,	97,		90,		10,		1,		50,		90,		1,		40,		50,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{21,100,	100,	0,		0,		10,		1,		30,		50,		3,		15,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{22,160,	160,	0,		0,		10,		1,		50,		50,		3,		500,	25,		0,		0,		0,0,0,0,0,0,0,0,0,0},
{23,255,	 75,	88,		0,		10,		1,		40,		0,		0,		0,		0,		0,		5,		0,0,0,0,0,0,0,0,0,0}, 
{24,200,	 20,	95,	    70,		10,		1,		70,		70,		3,		20,		50,		0,		0,		0,0,0,0,0,0,0,0,0,0}, 
{25,180,	100,	50,		60,		10,		1,		40,		60,		2,		90,		100,	100,	0,		0,0,0,0,0,0,0,0,0,0}, 
{26,60,		 60,	0,		0,		10,		1,		40,		70,		3,		80,		20,		50,		0,		0,0,0,0,0,0,0,0,0,0}, 
{27,128,	 90,	10,		10,		10,		1,		20,		40,		1,		5,		10,		20,		0,		0,0,0,0,0,0,0,0,0,0}
};

class CAmbientGeneric : public CBaseEntity
{
public:
	void KeyValue( KeyValueData* pkvd);
	void Spawn( void );
	void Precache( void );
	void EXPORT ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT RampThink( void );
	void InitModulationParms(void);

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	virtual int ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	float m_flAttenuation;		// attenuation value
	dynpitchvol_t m_dpv;	

	BOOL	m_fActive;	// only TRUE when the entity is playing a looping sound
	BOOL	m_fLooping;	// TRUE when the sound played will loop
};

LINK_ENTITY_TO_CLASS( ambient_generic, CAmbientGeneric );
TYPEDESCRIPTION	CAmbientGeneric::m_SaveData[] = 
{
	DEFINE_FIELD( CAmbientGeneric, m_flAttenuation, FIELD_FLOAT ),
	DEFINE_FIELD( CAmbientGeneric, m_fActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( CAmbientGeneric, m_fLooping, FIELD_BOOLEAN ),

	// HACKHACK - This is not really in the spirit of the save/restore design, but save this
	// out as a binary data block.  If the dynpitchvol_t is changed, old saved games will NOT
	// load these correctly, so bump the save/restore version if you change the size of the struct
	// The right way to do this is to split the input parms (read in keyvalue) into members and re-init this
	// struct in Precache(), but it's unlikely that the struct will change, so it's not worth the time right now.
	DEFINE_ARRAY( CAmbientGeneric, m_dpv, FIELD_CHARACTER, sizeof(dynpitchvol_t) ),
};

IMPLEMENT_SAVERESTORE( CAmbientGeneric, CBaseEntity );

//
// ambient_generic - general-purpose user-defined static sound
//
void CAmbientGeneric::Spawn( void )
{
/*
		-1 : "Default"
		0  : "Everywhere"
		200 : "Small Radius"
		125 : "Medium Radius"
		80  : "Large Radius"
*/

	if ( FBitSet ( pev->spawnflags, AMBIENT_SOUND_EVERYWHERE) )
	{
		m_flAttenuation = ATTN_NONE;
	}
	else if ( FBitSet ( pev->spawnflags, AMBIENT_SOUND_SMALLRADIUS) )
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if ( FBitSet ( pev->spawnflags, AMBIENT_SOUND_MEDIUMRADIUS) )
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if ( FBitSet ( pev->spawnflags, AMBIENT_SOUND_LARGERADIUS) )
	{
		m_flAttenuation = ATTN_NORM;
	}
	else 
	{// if the designer didn't set a sound attenuation, default to one.
		m_flAttenuation = ATTN_STATIC;
	}
	
	char* szSoundFile = (char*) STRING(pev->message);

	if ( FStringNull( pev->message ) || strlen( szSoundFile ) < 1 )
	{
		ALERT( at_error, "EMPTY AMBIENT AT: %f, %f, %f\n", pev->origin.x, pev->origin.y, pev->origin.z );
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink( &CBaseEntity::SUB_Remove );
		return;
	}
    pev->solid		= SOLID_NOT;
    pev->movetype	= MOVETYPE_NONE;

	// Set up think function for dynamic modification 
	// of ambient sound's pitch or volume. Don't
	// start thinking yet.

	SetThink(&CAmbientGeneric::RampThink);
	pev->nextthink = 0;

	// allow on/off switching via 'use' function.

	SetUse ( &CAmbientGeneric::ToggleUse );
	
	m_fActive = FALSE;

	if ( FBitSet ( pev->spawnflags, AMBIENT_SOUND_NOT_LOOPING ) )
		m_fLooping = FALSE;
	else
		m_fLooping = TRUE;
	Precache( );
}


void CAmbientGeneric::Precache( void )
{
	char* szSoundFile = (char*) STRING(pev->message);

	if ( !FStringNull( pev->message ) && strlen( szSoundFile ) > 1 )
	{
		if (*szSoundFile != '!')
			PRECACHE_SOUND(szSoundFile);
	}
	// init all dynamic modulation parms
	InitModulationParms();

	if ( !FBitSet (pev->spawnflags, AMBIENT_SOUND_START_SILENT ) )
	{
		// start the sound ASAP
		if (m_fLooping)
			m_fActive = TRUE;
	}
	if ( m_fActive )
	{
		EMIT_AMBIENT_SOUND( ENT(pev), pev->origin, szSoundFile, 
				(m_dpv.vol * 0.01), m_flAttenuation, SND_SPAWNING, m_dpv.pitch);

		pev->nextthink = gpGlobals->time + 0.1;
	}
}

// RampThink - Think at 5hz if we are dynamically modifying 
// pitch or volume of the playing sound.  This function will
// ramp pitch and/or volume up or down, modify pitch/volume
// with lfo if active.

void CAmbientGeneric::RampThink( void )
{
	char* szSoundFile = (char*) STRING(pev->message);
	int pitch = m_dpv.pitch; 
	int vol = m_dpv.vol;
	int flags = 0;
	int fChanged = 0;		// FALSE if pitch and vol remain unchanged this round
	int prev;

	if (!m_dpv.spinup && !m_dpv.spindown && !m_dpv.fadein && !m_dpv.fadeout && !m_dpv.lfotype)
		return;						// no ramps or lfo, stop thinking

	// ==============
	// pitch envelope
	// ==============
	if (m_dpv.spinup || m_dpv.spindown)
	{
		prev = m_dpv.pitchfrac >> 8;

		if (m_dpv.spinup > 0)
			m_dpv.pitchfrac += m_dpv.spinup;
		else if (m_dpv.spindown > 0)
			m_dpv.pitchfrac -= m_dpv.spindown;

		pitch = m_dpv.pitchfrac >> 8;
		
		if (pitch > m_dpv.pitchrun)
		{
			pitch = m_dpv.pitchrun;
			m_dpv.spinup = 0;				// done with ramp up
		}

		if (pitch < m_dpv.pitchstart)
		{
			pitch = m_dpv.pitchstart;
			m_dpv.spindown = 0;				// done with ramp down

			// shut sound off
			EMIT_AMBIENT_SOUND(ENT(pev), pev->origin, szSoundFile, 
					0, 0, SND_STOP, 0);

			// return without setting nextthink
			return;
		}

		if (pitch > 255) pitch = 255;
		if (pitch < 1) pitch = 1;

		m_dpv.pitch = pitch;

		fChanged |= (prev != pitch);
		flags |= SND_CHANGE_PITCH;
	}

	// ==================
	// amplitude envelope
	// ==================
	if (m_dpv.fadein || m_dpv.fadeout)
	{
		prev = m_dpv.volfrac >> 8;

		if (m_dpv.fadein > 0)
			m_dpv.volfrac += m_dpv.fadein;
		else if (m_dpv.fadeout > 0)
			m_dpv.volfrac -= m_dpv.fadeout;

		vol = m_dpv.volfrac >> 8;

		if (vol > m_dpv.volrun)
		{
			vol = m_dpv.volrun;
			m_dpv.fadein = 0;				// done with ramp up
		}

		if (vol < m_dpv.volstart)
		{
			vol = m_dpv.volstart;
			m_dpv.fadeout = 0;				// done with ramp down
			
			// shut sound off
			EMIT_AMBIENT_SOUND(ENT(pev), pev->origin, szSoundFile, 
					0, 0, SND_STOP, 0);

			// return without setting nextthink
			return;
		}

		if (vol > 100) vol = 100;
		if (vol < 1) vol = 1;

		m_dpv.vol = vol;

		fChanged |= (prev != vol);
		flags |= SND_CHANGE_VOL;
	}

	// ===================
	// pitch/amplitude LFO
	// ===================
	if (m_dpv.lfotype)
	{
		int pos;

		if (m_dpv.lfofrac > 0x6fffffff)
			m_dpv.lfofrac = 0;

		// update lfo, lfofrac/255 makes a triangle wave 0-255
		m_dpv.lfofrac += m_dpv.lforate;
		pos = m_dpv.lfofrac >> 8;

		if (m_dpv.lfofrac < 0)
		{
			m_dpv.lfofrac = 0;
			m_dpv.lforate = abs(m_dpv.lforate);
			pos = 0;
		}
		else if (pos > 255)
		{
			pos = 255;
			m_dpv.lfofrac = (255 << 8);
			m_dpv.lforate = -abs(m_dpv.lforate);
		}

		switch(m_dpv.lfotype)
		{
		case LFO_SQUARE:
			if (pos < 128)
				m_dpv.lfomult = 255;
			else
				m_dpv.lfomult = 0;
			
			break;
		case LFO_RANDOM:
			if (pos == 255)
				m_dpv.lfomult = RANDOM_LONG(0, 255);
			break;
		case LFO_TRIANGLE:
		default: 
			m_dpv.lfomult = pos;
			break;
		}

		if (m_dpv.lfomodpitch)
		{
			prev = pitch;

			// pitch 0-255
			pitch += ((m_dpv.lfomult - 128) * m_dpv.lfomodpitch) / 100;

			if (pitch > 255) pitch = 255;
			if (pitch < 1) pitch = 1;

			
			fChanged |= (prev != pitch);
			flags |= SND_CHANGE_PITCH;
		}

		if (m_dpv.lfomodvol)
		{
			// vol 0-100
			prev = vol;

			vol += ((m_dpv.lfomult - 128) * m_dpv.lfomodvol) / 100;

			if (vol > 100) vol = 100;
			if (vol < 0) vol = 0;
			
			fChanged |= (prev != vol);
			flags |= SND_CHANGE_VOL;
		}

	}

	// Send update to playing sound only if we actually changed
	// pitch or volume in this routine.

	if (flags && fChanged) 
	{
		if (pitch == PITCH_NORM)
			pitch = PITCH_NORM + 1; // don't send 'no pitch' !

		EMIT_AMBIENT_SOUND(ENT(pev), pev->origin, szSoundFile, 
				(vol * 0.01), m_flAttenuation, flags, pitch);
	}

	// update ramps at 5hz
	pev->nextthink = gpGlobals->time + 0.2;
	return;
}

// Init all ramp params in preparation to 
// play a new sound

void CAmbientGeneric::InitModulationParms(void)
{
	int pitchinc;

	m_dpv.volrun = pev->health * 10;	// 0 - 100
	if (m_dpv.volrun > 100) m_dpv.volrun = 100;
	if (m_dpv.volrun < 0) m_dpv.volrun = 0;

	// get presets
	if (m_dpv.preset != 0 && m_dpv.preset <= CDPVPRESETMAX)
	{
		// load preset values
		m_dpv = rgdpvpreset[m_dpv.preset - 1];

		// fixup preset values, just like
		// fixups in KeyValue routine.
		if (m_dpv.spindown > 0)
			m_dpv.spindown = (101 - m_dpv.spindown) * 64;
		if (m_dpv.spinup > 0)
			m_dpv.spinup = (101 - m_dpv.spinup) * 64;

		m_dpv.volstart *= 10;
		m_dpv.volrun *= 10;

		if (m_dpv.fadein > 0)
			m_dpv.fadein = (101 - m_dpv.fadein) * 64;
		if (m_dpv.fadeout > 0)
			m_dpv.fadeout = (101 - m_dpv.fadeout) * 64;

		m_dpv.lforate *= 256;

		m_dpv.fadeinsav = m_dpv.fadein;
		m_dpv.fadeoutsav = m_dpv.fadeout;
		m_dpv.spinupsav = m_dpv.spinup;
		m_dpv.spindownsav = m_dpv.spindown;
	}

	m_dpv.fadein = m_dpv.fadeinsav;
	m_dpv.fadeout = 0; 
	
	if (m_dpv.fadein)
		m_dpv.vol = m_dpv.volstart;
	else
		m_dpv.vol = m_dpv.volrun;

	m_dpv.spinup = m_dpv.spinupsav;
	m_dpv.spindown = 0; 

	if (m_dpv.spinup)
		m_dpv.pitch = m_dpv.pitchstart;
	else
		m_dpv.pitch = m_dpv.pitchrun;

	if (m_dpv.pitch == 0)
		m_dpv.pitch = PITCH_NORM;

	m_dpv.pitchfrac = m_dpv.pitch << 8;
	m_dpv.volfrac = m_dpv.vol << 8;

	m_dpv.lfofrac = 0;
	m_dpv.lforate = abs(m_dpv.lforate);

	m_dpv.cspincount = 1;
	
	if (m_dpv.cspinup) 
	{
		pitchinc = (255 - m_dpv.pitchstart) / m_dpv.cspinup;

		m_dpv.pitchrun = m_dpv.pitchstart + pitchinc;
		if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;
	}

	if ((m_dpv.spinupsav || m_dpv.spindownsav || (m_dpv.lfotype && m_dpv.lfomodpitch))
		&& (m_dpv.pitch == PITCH_NORM))
		m_dpv.pitch = PITCH_NORM + 1; // must never send 'no pitch' as first pitch
									  // if we intend to pitch shift later!
}

//
// ToggleUse - turns an ambient sound on or off.  If the 
// ambient is a looping sound, mark sound as active (m_fActive)
// if it's playing, innactive if not.  If the sound is not
// a looping sound, never mark it as active.
//
void CAmbientGeneric::ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	char* szSoundFile = (char*) STRING(pev->message);
	float fraction;

	if ( useType != USE_TOGGLE )
	{
		if ( (m_fActive && useType == USE_ON) || (!m_fActive && useType == USE_OFF) )
			return;
	}
	// Directly change pitch if arg passed. Only works if sound is already playing.

	if (useType == USE_SET && m_fActive)		// Momentary buttons will pass down a float in here
	{

		fraction = value;
		
		if ( fraction > 1.0 )
			fraction = 1.0;
		if (fraction < 0.0)
			fraction = 0.01;

		m_dpv.pitch = fraction * 255;

		EMIT_AMBIENT_SOUND(ENT(pev), pev->origin, szSoundFile, 
					0, 0, SND_CHANGE_PITCH, m_dpv.pitch);

		return;
	}

	// Toggle

	// m_fActive is TRUE only if a looping sound is playing.
	
	if ( m_fActive )
	{// turn sound off

		if (m_dpv.cspinup)
		{
			// Don't actually shut off. Each toggle causes
			// incremental spinup to max pitch

			if (m_dpv.cspincount <= m_dpv.cspinup)
			{	
				int pitchinc;

				// start a new spinup
				m_dpv.cspincount++;
				
				pitchinc = (255 - m_dpv.pitchstart) / m_dpv.cspinup;

				m_dpv.spinup = m_dpv.spinupsav;
				m_dpv.spindown = 0;

				m_dpv.pitchrun = m_dpv.pitchstart + pitchinc * m_dpv.cspincount;
				if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;

				pev->nextthink = gpGlobals->time + 0.1;
			}
			
		}
		else
		{
			m_fActive = FALSE;
			
			// HACKHACK - this makes the code in Precache() work properly after a save/restore
			pev->spawnflags |= AMBIENT_SOUND_START_SILENT;

			if (m_dpv.spindownsav || m_dpv.fadeoutsav)
			{
				// spin it down (or fade it) before shutoff if spindown is set
				m_dpv.spindown = m_dpv.spindownsav;
				m_dpv.spinup = 0;

				m_dpv.fadeout = m_dpv.fadeoutsav;
				m_dpv.fadein = 0;
				pev->nextthink = gpGlobals->time + 0.1;
			}
			else
				EMIT_AMBIENT_SOUND(ENT(pev), pev->origin, szSoundFile, 
					0, 0, SND_STOP, 0);
		}
	}
	else 
	{// turn sound on

		// only toggle if this is a looping sound.  If not looping, each
		// trigger will cause the sound to play.  If the sound is still
		// playing from a previous trigger press, it will be shut off
		// and then restarted.

		if (m_fLooping)
			m_fActive = TRUE;
		else
			// shut sound off now - may be interrupting a long non-looping sound
			EMIT_AMBIENT_SOUND(ENT(pev), pev->origin, szSoundFile, 
				0, 0, SND_STOP, 0);
			
		// init all ramp params for startup

		InitModulationParms();

		EMIT_AMBIENT_SOUND(ENT(pev), pev->origin, szSoundFile, 
				(m_dpv.vol * 0.01), m_flAttenuation, 0, m_dpv.pitch);
		
		pev->nextthink = gpGlobals->time + 0.1;

	} 
}
// KeyValue - load keyvalue pairs into member data of the
// ambient generic. NOTE: called BEFORE spawn!

void CAmbientGeneric::KeyValue( KeyValueData *pkvd )
{
	// NOTE: changing any of the modifiers in this code
	// NOTE: also requires changing InitModulationParms code.

	// preset
	if (FStrEq(pkvd->szKeyName, "preset"))
	{
		m_dpv.preset = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	// pitchrun
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		m_dpv.pitchrun = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
		
		if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;
		if (m_dpv.pitchrun < 0) m_dpv.pitchrun = 0;
	}		

	// pitchstart
	else if (FStrEq(pkvd->szKeyName, "pitchstart"))
	{
		m_dpv.pitchstart = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;		
		
		if (m_dpv.pitchstart > 255) m_dpv.pitchstart = 255;
		if (m_dpv.pitchstart < 0) m_dpv.pitchstart = 0;
	}

	// spinup
	else if (FStrEq(pkvd->szKeyName, "spinup"))
	{
		m_dpv.spinup = atoi(pkvd->szValue);
		
		if (m_dpv.spinup > 100) m_dpv.spinup = 100;
		if (m_dpv.spinup < 0) m_dpv.spinup = 0;

		if (m_dpv.spinup > 0)
			m_dpv.spinup = (101 - m_dpv.spinup) * 64;
		m_dpv.spinupsav = m_dpv.spinup;
		pkvd->fHandled = TRUE;
	}		

	// spindown
	else if (FStrEq(pkvd->szKeyName, "spindown"))
	{
		m_dpv.spindown = atoi(pkvd->szValue);
		
		if (m_dpv.spindown > 100) m_dpv.spindown = 100;
		if (m_dpv.spindown < 0) m_dpv.spindown = 0;

		if (m_dpv.spindown > 0)
			m_dpv.spindown = (101 - m_dpv.spindown) * 64;
		m_dpv.spindownsav = m_dpv.spindown;
		pkvd->fHandled = TRUE;
	}

	// volstart
	else if (FStrEq(pkvd->szKeyName, "volstart"))
	{
		m_dpv.volstart = atoi(pkvd->szValue);

		if (m_dpv.volstart > 10) m_dpv.volstart = 10;
		if (m_dpv.volstart < 0) m_dpv.volstart = 0;
		
		m_dpv.volstart *= 10;	// 0 - 100

		pkvd->fHandled = TRUE;
	}

	// fadein
	else if (FStrEq(pkvd->szKeyName, "fadein"))
	{
		m_dpv.fadein = atoi(pkvd->szValue);
		
		if (m_dpv.fadein > 100) m_dpv.fadein = 100;
		if (m_dpv.fadein < 0) m_dpv.fadein = 0;

		if (m_dpv.fadein > 0)
			m_dpv.fadein = (101 - m_dpv.fadein) * 64;
		m_dpv.fadeinsav = m_dpv.fadein;
		pkvd->fHandled = TRUE;
	}

	// fadeout
	else if (FStrEq(pkvd->szKeyName, "fadeout"))
	{
		m_dpv.fadeout = atoi(pkvd->szValue);
		
		if (m_dpv.fadeout > 100) m_dpv.fadeout = 100;
		if (m_dpv.fadeout < 0) m_dpv.fadeout = 0;

		if (m_dpv.fadeout > 0)
			m_dpv.fadeout = (101 - m_dpv.fadeout) * 64;
		m_dpv.fadeoutsav = m_dpv.fadeout;
		pkvd->fHandled = TRUE;
	}

	// lfotype
	else if (FStrEq(pkvd->szKeyName, "lfotype"))
	{
		m_dpv.lfotype = atoi(pkvd->szValue);
		if (m_dpv.lfotype > 4) m_dpv.lfotype = LFO_TRIANGLE;
		pkvd->fHandled = TRUE;
	}

	// lforate
	else if (FStrEq(pkvd->szKeyName, "lforate"))
	{
		m_dpv.lforate = atoi(pkvd->szValue);
		
		if (m_dpv.lforate > 1000) m_dpv.lforate = 1000;
		if (m_dpv.lforate < 0) m_dpv.lforate = 0;

		m_dpv.lforate *= 256;

		pkvd->fHandled = TRUE;
	}
	// lfomodpitch
	else if (FStrEq(pkvd->szKeyName, "lfomodpitch"))
	{
		m_dpv.lfomodpitch = atoi(pkvd->szValue);
		if (m_dpv.lfomodpitch > 100) m_dpv.lfomodpitch = 100;
		if (m_dpv.lfomodpitch < 0) m_dpv.lfomodpitch = 0;
		

		pkvd->fHandled = TRUE;
	}

	// lfomodvol
	else if (FStrEq(pkvd->szKeyName, "lfomodvol"))
	{
		m_dpv.lfomodvol = atoi(pkvd->szValue);
		if (m_dpv.lfomodvol > 100) m_dpv.lfomodvol = 100;
		if (m_dpv.lfomodvol < 0) m_dpv.lfomodvol = 0;

		pkvd->fHandled = TRUE;
	}

	// cspinup
	else if (FStrEq(pkvd->szKeyName, "cspinup"))
	{
		m_dpv.cspinup = atoi(pkvd->szValue);
		if (m_dpv.cspinup > 100) m_dpv.cspinup = 100;
		if (m_dpv.cspinup < 0) m_dpv.cspinup = 0;

		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}


// =================== ROOM SOUND FX ==========================================

class CEnvSound : public CPointEntity
{
public:
	void KeyValue( KeyValueData* pkvd);
	void Spawn( void );

	void Think( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flRadius;
	float m_flRoomtype;
};

LINK_ENTITY_TO_CLASS( env_sound, CEnvSound );
TYPEDESCRIPTION	CEnvSound::m_SaveData[] = 
{
	DEFINE_FIELD( CEnvSound, m_flRadius, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvSound, m_flRoomtype, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CEnvSound, CBaseEntity );


void CEnvSound::KeyValue( KeyValueData *pkvd )
{
	
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "roomtype"))
	{
		m_flRoomtype = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

// returns TRUE if the given sound entity (pev) is in range 
// and can see the given player entity (pevTarget)

BOOL FEnvSoundInRange(entvars_t *pev, entvars_t *pevTarget, float *pflRange) 
{
	CEnvSound *pSound = GetClassPtr( (CEnvSound *)pev );
	Vector vecSpot1 = pev->origin + pev->view_ofs;
	Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
	Vector vecRange;
	float flRange;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);
	
	// check if line of sight crosses water boundary, or is blocked

	if ((tr.fInOpen && tr.fInWater) || tr.flFraction != 1)
		return FALSE;

	// calc range from sound entity to player

	vecRange = tr.vecEndPos - vecSpot1;
	flRange = vecRange.Length();

	if (pSound->m_flRadius < flRange)		
		return FALSE;
	
	if (pflRange)
		*pflRange = flRange;

	return TRUE;
}

//
// A client that is visible and in range of a sound entity will
// have its room_type set by that sound entity.  If two or more
// sound entities are contending for a client, then the nearest
// sound entity to the client will set the client's room_type.
// A client's room_type will remain set to its prior value until
// a new in-range, visible sound entity resets a new room_type.
//

// CONSIDER: if player in water state, autoset roomtype to 14,15 or 16. 

void CEnvSound::Think( void )
{
	// get pointer to client if visible; FIND_CLIENT_IN_PVS will
	// cycle through visible clients on consecutive calls.

	edict_t *pentPlayer = FIND_CLIENT_IN_PVS(edict());
	CBasePlayer *pPlayer = NULL;

	if (FNullEnt(pentPlayer))
		goto env_sound_Think_slow; // no player in pvs of sound entity, slow it down
 
	pPlayer = GetClassPtr( (CBasePlayer *)VARS(pentPlayer));
	float flRange;

	// check to see if this is the sound entity that is 
	// currently affecting this player

	if(!FNullEnt(pPlayer->m_pentSndLast) && (pPlayer->m_pentSndLast == ENT(pev))) {

		// this is the entity currently affecting player, check
		// for validity

		if (pPlayer->m_flSndRoomtype != 0 && pPlayer->m_flSndRange != 0) {
		
			// we're looking at a valid sound entity affecting
			// player, make sure it's still valid, update range

			if (FEnvSoundInRange(pev, VARS(pentPlayer), &flRange)) {
				pPlayer->m_flSndRange = flRange;
				goto env_sound_Think_fast;
			} else {
				
				// current sound entity affecting player is no longer valid,
				// flag this state by clearing room_type and range.
				// NOTE: we do not actually change the player's room_type
				// NOTE: until we have a new valid room_type to change it to.

				pPlayer->m_flSndRange = 0;
				pPlayer->m_flSndRoomtype = 0;
				goto env_sound_Think_slow;
			}
		} else {
			// entity is affecting player but is out of range,
			// wait passively for another entity to usurp it...
			goto env_sound_Think_slow;
		}
	}

	// if we got this far, we're looking at an entity that is contending
	// for current player sound. the closest entity to player wins.

	if (FEnvSoundInRange(pev, VARS(pentPlayer), &flRange)) 
	{
		if (flRange < pPlayer->m_flSndRange || pPlayer->m_flSndRange == 0) 
		{
			// new entity is closer to player, so it wins.
			pPlayer->m_pentSndLast = ENT(pev);
			pPlayer->m_flSndRoomtype = m_flRoomtype;
			pPlayer->m_flSndRange = flRange;
			
			// send room_type command to player's server.
			// this should be a rare event - once per change of room_type
			// only!

			//CLIENT_COMMAND(pentPlayer, "room_type %f", m_flRoomtype);
			
			MESSAGE_BEGIN( MSG_ONE, SVC_ROOMTYPE, NULL, pentPlayer );		// use the magic #1 for "one client"
				WRITE_SHORT( (short)m_flRoomtype );					// sequence number
			MESSAGE_END();

			// crank up nextthink rate for new active sound entity
			// by falling through to think_fast...
		}
		// player is not closer to the contending sound entity,
		// just fall through to think_fast. this effectively
		// cranks up the think_rate of entities near the player.
	} 

	// player is in pvs of sound entity, but either not visible or
	// not in range. do nothing, fall through to think_fast...

env_sound_Think_fast:
	pev->nextthink = gpGlobals->time + 0.25;
	return;

env_sound_Think_slow:
	pev->nextthink = gpGlobals->time + 0.75;
	return;
}

//
// env_sound - spawn a sound entity that will set player roomtype
// when player moves in range and sight.
//
//
void CEnvSound::Spawn( )
{
	// spread think times
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5); 
}

// ==================== SENTENCE GROUPS, UTILITY FUNCTIONS  ======================================

// group of related sentences

typedef struct sentenceg
{
	char szgroupname[CBSENTENCENAME_MAX];
	int count;
	unsigned char rgblru[CSENTENCE_LRU_MAX];

} SENTENCEG;


SENTENCEG rgsentenceg[CSENTENCEG_MAX];
int fSentencesInit = FALSE;

char gszallsentencenames[CVOXFILESENTENCEMAX][CBSENTENCENAME_MAX];
int gcallsentences = 0;

// randomize list of sentence name indices

void USENTENCEG_InitLRU(unsigned char *plru, int count)
{
	int i, j, k;
	unsigned char temp;
	
	if (!fSentencesInit)
		return;

	if (count > CSENTENCE_LRU_MAX)
		count = CSENTENCE_LRU_MAX;

	for (i = 0; i < count; i++)
		plru[i] = (unsigned char) i;

	// randomize array
	for (i = 0; i < (count * 4); i++)
	{
		j = RANDOM_LONG(0,count-1);
		k = RANDOM_LONG(0,count-1);
		temp = plru[j];
		plru[j] = plru[k];
		plru[k] = temp;
	}
}

// ignore lru. pick next sentence from sentence group. Go in order until we hit the last sentence, 
// then repeat list if freset is true.  If freset is false, then repeat last sentence.
// ipick is passed in as the requested sentence ordinal.
// ipick 'next' is returned.  
// return of -1 indicates an error.

int USENTENCEG_PickSequential(int isentenceg, char *szfound, int ipick, int freset)
{
	char *szgroupname;
	unsigned char count;
	char sznum[8];
	
	if (!fSentencesInit)
		return -1;

	if (isentenceg < 0)
		return -1;

	szgroupname = rgsentenceg[isentenceg].szgroupname;
	count = rgsentenceg[isentenceg].count;
	
	if (count == 0)
		return -1;

	if (ipick >= count)
		ipick = count-1;

	strcpy(szfound, "!");
	strcat(szfound, szgroupname);
	sprintf(sznum, "%d", ipick);
	strcat(szfound, sznum);
	
	if (ipick >= count)
	{
		if (freset)
			// reset at end of list
			return 0;
		else
			return count;
	}

	return ipick + 1;
}



// pick a random sentence from rootname0 to rootnameX.
// picks from the rgsentenceg[isentenceg] least
// recently used, modifies lru array. returns the sentencename.
// note, lru must be seeded with 0-n randomized sentence numbers, with the
// rest of the lru filled with -1. The first integer in the lru is
// actually the size of the list.  Returns ipick, the ordinal
// of the picked sentence within the group.

int USENTENCEG_Pick(int isentenceg, char *szfound)
{
	char *szgroupname;
	unsigned char *plru;
	unsigned char i;
	unsigned char count;
	char sznum[8];
	unsigned char ipick;
	int ffound = FALSE;
	
	if (!fSentencesInit)
		return -1;

	if (isentenceg < 0)
		return -1;

	szgroupname = rgsentenceg[isentenceg].szgroupname;
	count = rgsentenceg[isentenceg].count;
	plru = rgsentenceg[isentenceg].rgblru;

	while (!ffound)
	{
		for (i = 0; i < count; i++)
			if (plru[i] != 0xFF)
			{
				ipick = plru[i];
				plru[i] = 0xFF;
				ffound = TRUE;
				break;
			}

		if (!ffound)
			USENTENCEG_InitLRU(plru, count);
		else
		{
			strcpy(szfound, "!");
			strcat(szfound, szgroupname);
			//MODDD - why not %u?   ipick is unsigned, less fitting for %d
			sprintf(sznum, "%u", ipick);
			strcat(szfound, sznum);

			//easyPrintLine("WO HO HO HO SENTENCE:::%s : count: %d choice: %d", szgroupname, count, ipick);
			return ipick;
		}
	}
	return -1;
}

// ===================== SENTENCE GROUPS, MAIN ROUTINES ========================

// Given sentence group rootname (name without number suffix),
// get sentence group index (isentenceg). Returns -1 if no such name.

int SENTENCEG_GetIndex(const char *szgroupname)
{
	int i;

	if (!fSentencesInit || !szgroupname)
		return -1;

	// search rgsentenceg for match on szgroupname

	i = 0;
	while (rgsentenceg[i].count)
	{
		//easyForcePrintLine("YOU honorable fellow %d %s : %s", i, szgroupname, rgsentenceg[i].szgroupname);
		if (!strcmp(szgroupname, rgsentenceg[i].szgroupname))
			return i;
	i++;
	}

	return -1;
}

// given sentence group index, play random sentence for given entity.
// returns ipick - which sentence was picked to 
// play from the group. Ipick is only needed if you plan on stopping
// the sound before playback is done (see SENTENCEG_Stop).

int SENTENCEG_PlayRndI(edict_t *entity, int isentenceg, float volume, float attenuation, int flags, int pitch)
{
	char name[64];
	int ipick;

	if (!fSentencesInit)
		return -1;

	name[0] = 0;

	ipick = USENTENCEG_Pick(isentenceg, name);
	if (ipick > 0 && name)
		EMIT_SOUND_DYN(entity, CHAN_VOICE, name, volume, attenuation, flags, pitch);
	return ipick;
}

// same as above, but takes sentence group name instead of index
int SENTENCEG_PlayRndSz(edict_t *entity, const char *szgroupname, float volume, float attenuation, int flags, int pitch)
{
	char name[64];
	int ipick;
	int isentenceg;

	if (!fSentencesInit)
		return -1;

	name[0] = 0;

	isentenceg = SENTENCEG_GetIndex(szgroupname);
	if (isentenceg < 0)
	{
		ALERT( at_console, "No such sentence group %s\n", szgroupname );
		return -1;
	}

	ipick = USENTENCEG_Pick(isentenceg, name);
	//easyPrintLine("SENTENCE PLAYED: %s", name);
	if (ipick >= 0 && name[0])
		EMIT_SOUND_DYN(entity, CHAN_VOICE, name, volume, attenuation, flags, pitch);

	return ipick;
}


//MODDD - new. same as SENTENCEG_PlayRndSz, but can do ambient sounds.
int SENTENCEG_PlayRndSz_Ambient(edict_t *entity, Vector vOrigin, const char *szgroupname, float volume, float attenuation, int flags, int pitch)
{
	char name[64];
	int ipick;
	int isentenceg;

	//float rgfl[3];
	//vOrigin.CopyToArray(rgfl);

	if (!fSentencesInit)
		return -1;

	name[0] = 0;

	isentenceg = SENTENCEG_GetIndex(szgroupname);
	if (isentenceg < 0)
	{
		ALERT( at_console, "No such sentence group %s\n", szgroupname );
		return -1;
	}

	ipick = USENTENCEG_Pick(isentenceg, name);
	//easyPrintLine("SENTENCE PLAYED: %s", name);
	if (ipick >= 0 && name[0]){
		EMIT_AMBIENT_SOUND(entity, vOrigin, name, volume, attenuation, flags, pitch);
	}

	return ipick;
}


// play sentences in sequential order from sentence group.  Reset after last sentence.

int SENTENCEG_PlaySequentialSz(edict_t *entity, const char *szgroupname, float volume, float attenuation, int flags, int pitch, int ipick, int freset)
{
	char name[64];
	int ipicknext;
	int isentenceg;

	if (!fSentencesInit)
		return -1;

	name[0] = 0;

	isentenceg = SENTENCEG_GetIndex(szgroupname);
	if (isentenceg < 0)
		return -1;

	ipicknext = USENTENCEG_PickSequential(isentenceg, name, ipick, freset);
	if (ipicknext >= 0 && name[0])
		EMIT_SOUND_DYN(entity, CHAN_VOICE, name, volume, attenuation, flags, pitch);
	return ipicknext;
}


// for this entity, for the given sentence within the sentence group, stop
// the sentence.

void SENTENCEG_Stop(edict_t *entity, int isentenceg, int ipick)
{
	char buffer[64];
	char sznum[8];
	
	if (!fSentencesInit)
		return;

	if (isentenceg < 0 || ipick < 0)
		return;
	
	strcpy(buffer, "!");
	strcat(buffer, rgsentenceg[isentenceg].szgroupname);
	sprintf(sznum, "%d", ipick);
	strcat(buffer, sznum);

	//MODDD NOTE - nah, don't filter, this is always a sentence anyways.
	// (I think this method in particular is never used?)
	UTIL_StopSound(entity, CHAN_VOICE, buffer, FALSE);
}

// open sentences.txt, scan for groups, build rgsentenceg
// Should be called from world spawn, only works on the
// first call and is ignored subsequently.


void SENTENCEG_Init()
{
	//MODDD
	BOOL stopReadingGroups = FALSE;

	char buffer[512];
	char szgroup[64];
	int i, j;
	int isentencegs;

	if (fSentencesInit)
		return;

	memset(gszallsentencenames, 0, CVOXFILESENTENCEMAX * CBSENTENCENAME_MAX);
	gcallsentences = 0;

	memset(rgsentenceg, 0, CSENTENCEG_MAX * sizeof(SENTENCEG));
	memset(buffer, 0, 512);
	memset(szgroup, 0, 64);
	isentencegs = -1;

	
	int filePos = 0, fileSize;
	byte *pMemFile = g_engfuncs.pfnLoadFileForMe( "sound/sentences.txt", &fileSize );
	if ( !pMemFile )
		return;

	// for each line in the file...
	while ( memfgets(pMemFile, fileSize, filePos, buffer, 511) != NULL )
	{
		// skip whitespace
		i = 0;
		while(buffer[i] && buffer[i] == ' ')
			i++;
		
		if (!buffer[i])
			continue;
		
		if(::stringStartsWith(buffer, "/###")){
			// Why this wonky-ass identity string in front? At least a single slash marks this line as a comment so the engine won't
			// try to count this line as a sentence, offsetting all sentence names and accompanying sound lists by 1.
			// This can lead to some stupid stuff like so much as commented out lines immediately after the flag causing the offset issue to show up.
			// Just less stable unless you use a comment character "/" in it. The "###" is so it won't get mistaken for other comments not meant to be flags.
			int readIndex = 4; //start at the end of the identify string.

			int writeIndex = 0;
			char temp[50];
			BOOL reading = TRUE;

			while(reading){
				char thisChar = buffer[readIndex];
				switch(thisChar){

				case '\0':
				case '\n':
				case '\r':
				case ' ':
					temp[writeIndex] = '\0';
					reading = FALSE;
				break;
				default:
					
					temp[writeIndex] = thisChar;
					writeIndex++;
				break;

				}
				readIndex++;
			}
			//easyPrintLine("???????????? |||%s|||", temp);


			//easyPrintLine("%s", buffer + i);

			if(FStrEq(temp, "STOPREADINGGROUPS") ){
				easyPrintLine("Sentences: SAW FLAG \"%s\".", temp);
				stopReadingGroups = TRUE;
				//gcallsentences++; //skip one? not needed, now understood as just a comment for the purposes of reading sentences as far as the engine is concerned.
			}else{
				easyPrintLine("Sentences: WARNING. FLAG \"%s\" UNRECOGNIZED, IGNORED.", temp);
			}

			continue;
		}//END OF buffer[i] == '#'



		if (buffer[i] == '/' || (!isalpha(buffer[i]) ) )
			continue;


		// get sentence name
		j = i;
		while (buffer[j] && buffer[j] != ' ')
			j++;

		if (!buffer[j])
			continue;

		if (gcallsentences > CVOXFILESENTENCEMAX)
		{
			//ALERT (at_error, "Too many sentences in sentences.txt! %d\n", gcallsentences);
			easyPrintLine("Sentences: ERROR: Too many sentences: %d. Last okay sentence: %s", gcallsentences, gszallsentencenames[gcallsentences - 2]);
			break;
		}

		// null-terminate name and save in sentences array
		buffer[j] = 0;
		const char *pString = buffer + i;



		//easyPrintLine("Sentences: found sentence title: %s ID:%d", pString, gcallsentences);
		if ( strlen( pString ) >= CBSENTENCENAME_MAX ){
			//ALERT( at_warning, "Sentence %s longer than %d letters\n", pString, CBSENTENCENAME_MAX-1 );
			easyPrintLine("Sentences: WARNING: Sentence %s longer than %d letters.", pString, CBSENTENCENAME_MAX-1 );
		}

		strcpy( gszallsentencenames[gcallsentences++], pString );


		j--;
		if (j <= i)
			continue;

		//If the first non-space character is a digit, block this line.
		if (!isdigit(buffer[j]))
			continue;

		// cut out suffix numbers
		while (j > i && isdigit(buffer[j]))
			j--;

		if (j <= i)
			continue;

		buffer[j+1] = 0;
		
		// if new name doesn't match previous group name, 
		// make a new group.
		
		

		if(stopReadingGroups){
			continue;
		}


		//This means (intentionally)... if it doesn't match. strcmp returns 0, or passes with "!strcmp(...)" if there is a match. Yes, confusing.
		if (strcmp(szgroup, &(buffer[i])))
		{
			// name doesn't match with prev name,
			// copy name into group, init count to 1
			isentencegs++;
			if (isentencegs >= CSENTENCEG_MAX)
			{
				//ALERT (at_error, "Too many sentence groups in sentences.txt!\n");
				easyPrintLine("Sentences: ERROR. TOO MANY SENTENCE GROUPS!");
				break;
			}

			strcpy(rgsentenceg[isentencegs].szgroupname, &(buffer[i]));
			rgsentenceg[isentencegs].count = 1;

			strcpy(szgroup, &(buffer[i]));

			continue;
		}
		else
		{
			//name matches with previous, increment group count
			if (isentencegs >= 0)
				rgsentenceg[isentencegs].count++;
		}
	}

	//MODDD
	easyPrintLine("Sentences: DONE. LOADED %d SENTENCES!  MAX POSSIBLE: %d", gcallsentences, CVOXFILESENTENCEMAX );

	g_engfuncs.pfnFreeFile( pMemFile );
	
	fSentencesInit = TRUE;

	// init lru lists

	i = 0;

	while (rgsentenceg[i].count && i < CSENTENCEG_MAX)
	{
		USENTENCEG_InitLRU(&(rgsentenceg[i].rgblru[0]), rgsentenceg[i].count);
		i++;
	}

}

// convert sentence (sample) name to !sentencenum, return !sentencenum

int SENTENCEG_Lookup(const char *sample, char *sentencenum)
{
	char sznum[8];

	int i;
	// this is a sentence name; lookup sentence number
	// and give to engine as string.
	//MODDD NOTE - this is a tiny bit inaccurate. This method call alone won't give the number (stopping "i") to the engine. That's up to whatever
	//             calls this method from elsewhere, like EMIT_SOUND_DYN or UTIL_EmitAmbientSound. They send just the index with an exclamation mark in front.
	for (i = 0; i < gcallsentences; i++)
		if (!stricmp(gszallsentencenames[i], sample+1))
		{
			if (sentencenum)
			{
				strcpy(sentencenum, "!");
				sprintf(sznum, "%d", i);
				strcat(sentencenum, sznum);
			}
			return i;
		}
	// sentence name not found!
	return -1;
}




//MODDD - these are new.  Available for all entities as utilities.
void SENTENCEG_PlaySingular(entvars_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation ){
	SENTENCEG_PlaySingular(ENT(entity), arg_channel, pszSentence, volume, attenuation, 0, 100);
}
void SENTENCEG_PlaySingular(edict_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation ){
	SENTENCEG_PlaySingular(entity, arg_channel, pszSentence, volume, attenuation, 0, 100);
}

void SENTENCEG_PlaySingular(entvars_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag ){
	SENTENCEG_PlaySingular(ENT(entity), arg_channel, pszSentence, volume, attenuation, flag, 100);
}
void SENTENCEG_PlaySingular(edict_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag ){
	SENTENCEG_PlaySingular(entity, arg_channel, pszSentence, volume, attenuation, flag, 100);
}

void SENTENCEG_PlaySingular(entvars_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag, int pitch ){
	SENTENCEG_PlaySingular(ENT(entity), arg_channel, pszSentence, volume, attenuation, flag, pitch);
}
void SENTENCEG_PlaySingular(edict_t* entity, int arg_channel, const char *pszSentence, float volume, float attenuation, int flag, int pitch ){
	//Just play the sentence given. But the system does require an exclamation mark for whatever reason.
	static char sentenceToPlay[127];

	sentenceToPlay[0] = '!';
	copyString(pszSentence, &sentenceToPlay[1], 127 - 1);
	
	//CHAN_VOICE ?
	EMIT_SOUND_DYN(entity, arg_channel, sentenceToPlay, volume, attenuation, flag, pitch);
}//END OF SENTENCEG_PlaySingular




//MODDD - UTIL_PlaySound method implementations (used to be EMIT_SOUND and EMIT_SOUND_DYN directly) have been moved to util.cpp.
//        No reason to be separate from the rest of the sound-handling stuff ultimately in util.cpp.


// ===================== MATERIAL TYPE DETECTION, MAIN ROUTINES ========================
// 
// Used to detect the texture the player is standing on, map the
// texture name to a material type.  Play footstep sound based
// on material type.

int fTextureTypeInit = FALSE;


int gcTextures = 0;
char grgszTextureName[CTEXTURESMAX][CBTEXTURENAMEMAX];	// texture names
char grgchTextureType[CTEXTURESMAX];						// parallel array of texture types

// open materials.txt,  get size, alloc space, 
// save in array.  Only works first time called, 
// ignored on subsequent calls.

static char *memfgets( byte *pMemFile, int fileSize, int &filePos, char *pBuffer, int bufferSize )
{
	// Bullet-proofing
	if ( !pMemFile || !pBuffer )
		return NULL;

	if ( filePos >= fileSize )
		return NULL;

	int i = filePos;
	int last = fileSize;

	// fgets always NULL terminates, so only read bufferSize-1 characters
	if ( last - filePos > (bufferSize-1) )
		last = filePos + (bufferSize-1);

	int stop = 0;

	// Stop at the next newline (inclusive) or end of buffer
	while ( i < last && !stop )
	{
		if ( pMemFile[i] == '\n' )
			stop = 1;
		i++;
	}


	// If we actually advanced the pointer, copy it over
	if ( i != filePos )
	{
		// We read in size bytes
		int size = i - filePos;
		// copy it out
		memcpy( pBuffer, pMemFile + filePos, sizeof(byte)*size );
		
		// If the buffer isn't full, terminate (this is always true)
		if ( size < bufferSize )
			pBuffer[size] = 0;

		// Update file pointer
		filePos = i;
		return pBuffer;
	}

	// No data read, bail
	return NULL;
}


void TEXTURETYPE_Init()
{
	char buffer[512];
	int i, j;
	byte *pMemFile;
	int fileSize, filePos;

	if (fTextureTypeInit)
		return;

	memset(&(grgszTextureName[0][0]), 0, CTEXTURESMAX * CBTEXTURENAMEMAX);
	memset(grgchTextureType, 0, CTEXTURESMAX);

	gcTextures = 0;
	memset(buffer, 0, 512);

	pMemFile = g_engfuncs.pfnLoadFileForMe( "sound/materials.txt", &fileSize );
	if ( !pMemFile )
		return;

	// for each line in the file...
	while (memfgets(pMemFile, fileSize, filePos, buffer, 511) != NULL && (gcTextures < CTEXTURESMAX))
	{
		// skip whitespace
		i = 0;
		while(buffer[i] && isspace(buffer[i]))
			i++;
		
		if (!buffer[i])
			continue;

		// skip comment lines
		if (buffer[i] == '/' || !isalpha(buffer[i]))
			continue;

		// get texture type
		grgchTextureType[gcTextures] = toupper(buffer[i++]);

		// skip whitespace
		while(buffer[i] && isspace(buffer[i]))
			i++;
		
		if (!buffer[i])
			continue;

		// get sentence name
		j = i;
		while (buffer[j] && !isspace(buffer[j]))
			j++;

		if (!buffer[j])
			continue;

		// null-terminate name and save in sentences array
		j = min (j, CBTEXTURENAMEMAX-1+i);
		buffer[j] = 0;
		strcpy(&(grgszTextureName[gcTextures++][0]), &(buffer[i]));
	}

	g_engfuncs.pfnFreeFile( pMemFile );
	
	fTextureTypeInit = TRUE;
}

// given texture name, find texture type
// if not found, return type 'concrete'

// NOTE: this routine should ONLY be called if the 
// current texture under the player changes!

char TEXTURETYPE_Find(char *name)
{
	// CONSIDER: pre-sort texture names and perform faster binary search here

	for (int i = 0; i < gcTextures; i++)
	{
		if (!strnicmp(name, &(grgszTextureName[i][0]), CBTEXTURENAMEMAX-1))
			return (grgchTextureType[i]);
	}

	return CHAR_TEX_CONCRETE;
}

// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play

float TEXTURETYPE_PlaySound(TraceResult *ptr,  Vector vecSrc, Vector vecEnd, int iBulletType)
{
	// hit the world, try to play sound based on texture material type
	char chTextureType;
	float fvol;
	float fvolbar;
	char szbuffer[64];
	const char *pTextureName;
	float rgfl1[3];
	float rgfl2[3];
	char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds) == 1){
		return 0;
	}

	//MODDD - if CVar forceAllowServersideTextureSounds is 1, deny this early termination.
	//MODDD - different approach altogheter.  Sometimes we still rely on the "fvolbar" returned regardless
	//        of whether we mean to play a sound or not.  So handle these at the end instead.
	//if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds) < 1 && !g_pGameRules->PlayTextureSounds() )
	//	return 0.0;

	chTextureType = 0;

	CBaseEntity *pEntity = CBaseEntity::Instance(ptr->pHit);
	BOOL isEntityWorld;
	
	
	// just in case short-circuit doesn't work.
	if (pEntity == NULL) {
		isEntityWorld = TRUE;  // just go ahead and say 'yes'.
	}
	else {
		// more to work with.
		BOOL temp_isWorld = pEntity->IsWorld();
		int temp_class = pEntity->Classify();

		isEntityWorld = (temp_isWorld || temp_class == CLASS_NONE);
	}

	//MODDD - why are we making such a strong assumption here?
	//        Why not call pEntity->IsWorld or pEntity->IsWorldAffiliated?
	//        uhh.. it's meant to work a certain way, check out combat.cpp's FireBulletsPlayer, around "doDefaultBulletHitEffectCheck && useBulletHitSound".
	
	//MODDD - differnet check.
	//if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
	if (!isEntityWorld) {
		// can be a little more specific.

		// !!!
		// possible to check for ptr->iHitgroup?  Doubt it's necessary.

		// Neither section should atch breakables, being not 'isEntityWorld' should do that.
		if (pEntity->isOrganic()) {
			// hit body
			chTextureType = CHAR_TEX_FLESH;
		}
		else {
			// oh, metal.
			// Note that as-is behavior is to do TRACE_TEXTURE on the ent anyway,
			// but that doesn't seem to work.  If you want an attempt anyway...
			//   pTextureName = TRACE_TEXTURE(ENT(pEntity->pev), rgfl1, rgfl2);
			// ...and turn determining 'chTextureType' from that into a method (further below).
			// For now, just assuming METAL texture.

			// CHANGE, custom sound behavior.  Use all the metal sounds, some are used by func_breakable vents, seems fitting for hitting
			// a sentry.
			//chTextureType = CHAR_TEX_METAL;


			// NOTE - is sending the entity instead of the world  ( ENT(0) ) ok?  No idea.
			// organic sounds will still use it from relying on normal texture logic below though.
			UTIL_playMetalTextureHitSound(pEntity->pev, ptr->vecEndPos);

			fvolbar = 0.45;
			return fvolbar;  //and done.
		}
	}
	else
	{
		// hit world

		// find texture under strike, get material type

		// copy trace vector into array for trace_texture

		vecSrc.CopyToArray(rgfl1);
		vecEnd.CopyToArray(rgfl2);

		// get texture from entity or world (world is ent(0))
		//MODDD - although metal entities are taken earlier now, that does leave heavily map-specific entities (breakables)
		// that still reach here.   Probably.
		if (pEntity) {
			pTextureName = TRACE_TEXTURE(ENT(pEntity->pev), rgfl1, rgfl2);
		}else {
			pTextureName = TRACE_TEXTURE(ENT(0), rgfl1, rgfl2);
		}
			
		if ( pTextureName )
		{
			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+'){
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' '){
				pTextureName++;
			}

			// '}}'
			strcpy(szbuffer, pTextureName);
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;

			// ALERT ( at_console, "texture hit: %s\n", szbuffer);

			// get texture type
			chTextureType = TEXTURETYPE_Find(szbuffer);	
		}
	}//END OF world check

	
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)==1)easyForcePrintLine("SND PLAY TEXTURE SOUND: %d, %c", (int)chTextureType, chTextureType);

	switch (chTextureType)
	{
	//MODDD - default noise? nah, doesn't need to be one.
	//default:
	case CHAR_TEX_CONCRETE: fvol = 0.9;	fvolbar = 0.6;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL: fvol = 0.9; fvolbar = 0.3;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:	fvol = 0.9; fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:	fvol = 0.5; fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE: fvol = 0.9; fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:	fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH: fvol = 0.9; fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD: fvol = 0.9; fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:

		if (iBulletType == BULLET_PLAYER_CROWBAR)
			return 0.0; // crowbar already makes this sound
		//MODDD NOTE - actually it plays "cbar_hitbod#", but it is similar. Don't need to play the usual anyways.

		fvol = 1.0;	fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	default:
		//MODDD - no default case?
		fvol = 0;
		fvolbar = 0;
		//fattn is already defaulted
		cnt = 0;
	}

	// did we hit a breakable?

	if (pEntity && FClassnameIs(pEntity->pev, "func_breakable"))
	{
		// drop volumes, the object will already play a damaged sound
		fvol /= 1.5;
		fvolbar /= 2.0;	
	}
	else if (chTextureType == CHAR_TEX_COMPUTER)
	{
		// play random spark if computer

		if ( ptr->flFraction != 1.0 && RANDOM_LONG(0,1))
		{
			//Again?  see func_break.cpp too.

			//MODDD!
			//UTIL_Sparks( ptr->vecEndPos );
			UTIL_Sparks( ptr->vecEndPos, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksComputerHitMulti) );


			float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range
			switch ( RANDOM_LONG(0,1) )
			{
				case 0: EMIT_AMBIENT_SOUND(ENT(0), ptr->vecEndPos, "buttons/spark5.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 1: EMIT_AMBIENT_SOUND(ENT(0), ptr->vecEndPos, "buttons/spark6.wav", flVolume, ATTN_NORM, 0, 100); break;
				// case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM);	break;
				// case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM);	break;
			}
		}
	}
	
	//MODDD - down here, actually play the sound if we want to.
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds) >= 1 || g_pGameRules->PlayTextureSounds() ){
		if(cnt > 0){
			// play material hit sound
			EMIT_AMBIENT_SOUND(ENT(0), ptr->vecEndPos, rgsz[RANDOM_LONG(0,cnt-1)], fvol, fattn, 0, 96 + RANDOM_LONG(0,0xf));
			//EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_WEAPON, rgsz[RANDOM_LONG(0,cnt-1)], fvol, ATTN_NORM, 0, 96 + RANDOM_LONG(0,0xf));
		}
	}
	

	return fvolbar;
}

// ===================================================================================
//
// Speaker class. Used for announcements per level, for door lock/unlock spoken voice. 
//

class CSpeaker : public CBaseEntity
{
public:
	void KeyValue( KeyValueData* pkvd);
	void Spawn( void );
	void Precache( void );
	void EXPORT ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT SpeakerThink( void );
	
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	
	int m_preset;			// preset number
};

LINK_ENTITY_TO_CLASS( speaker, CSpeaker );
TYPEDESCRIPTION	CSpeaker::m_SaveData[] = 
{
	DEFINE_FIELD( CSpeaker, m_preset, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CSpeaker, CBaseEntity );

//
// ambient_generic - general-purpose user-defined static sound
//
void CSpeaker::Spawn( void )
{
	char* szSoundFile = (char*) STRING(pev->message);

	//if this CVar is on, don't care about this early stop.
	if ( EASY_CVAR_GET_DEBUGONLY(announcerIsAJerk) <= 0 &&     (!m_preset && (FStringNull( pev->message ) || strlen( szSoundFile ) < 1 ))   )
	{
		ALERT( at_error, "SPEAKER with no Level/Sentence! at: %f, %f, %f\n", pev->origin.x, pev->origin.y, pev->origin.z );
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink( &CBaseEntity::SUB_Remove );
		return;
	}
    pev->solid		= SOLID_NOT;
    pev->movetype	= MOVETYPE_NONE;

	
	SetThink(&CSpeaker::SpeakerThink);
	if(EASY_CVAR_GET_DEBUGONLY(announcerIsAJerk) > 0){
		pev->nextthink = gpGlobals->time + 5;
	}else{
		pev->nextthink = 0.0;
	}

	// allow on/off switching via 'use' function.

	SetUse ( &CSpeaker::ToggleUse );

	Precache( );
}


void CSpeaker::Precache( void )
{

	if( EASY_CVAR_GET_DEBUGONLY(announcerIsAJerk) > 0){
		pev->nextthink = gpGlobals->time + 5;
		return;
	}

	if ( !FBitSet (pev->spawnflags, SPEAKER_START_SILENT ) )
		// set first announcement time for random n second
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(5.0, 15.0);
}
void CSpeaker::SpeakerThink( void )
{
	char* szSoundFile;
	float flvolume = pev->health * 0.1;
	float flattenuation = 0.3;
	int flags = 0;
	int pitch = 100;


	if( EASY_CVAR_GET_DEBUGONLY(announcerIsAJerk) > 0){
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(11, 16);
		flvolume = 1.0;
		SENTENCEG_PlayRndSz_Ambient ( ENT(pev), pev->origin, "voxjerk", flvolume, flattenuation, flags, pitch);
		CTalkMonster::g_talkWaitTime = gpGlobals->time + 10;
		return;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////


	// Wait for the talkmonster to finish first.
	if (gpGlobals->time <= CTalkMonster::g_talkWaitTime)
	{
		pev->nextthink = CTalkMonster::g_talkWaitTime + RANDOM_FLOAT( 5, 10 );
		return;
	}
	
	if (m_preset)
	{
		// go lookup preset text, assign szSoundFile
		switch (m_preset)
		{
		case 1: szSoundFile =  "C1A0_"; break;
		case 2: szSoundFile =  "C1A1_"; break;
		case 3: szSoundFile =  "C1A2_"; break;
		case 4: szSoundFile =  "C1A3_"; break;
		case 5: szSoundFile =  "C1A4_"; break; 
		case 6: szSoundFile =  "C2A1_"; break;
		case 7: szSoundFile =  "C2A2_"; break;
		case 8: szSoundFile =  "C2A3_"; break;
		case 9: szSoundFile =  "C2A4_"; break;
		case 10: szSoundFile = "C2A5_"; break;
		case 11: szSoundFile = "C3A1_"; break;
		case 12: szSoundFile = "C3A2_"; break;
		}
	} else
		szSoundFile = (char*) STRING(pev->message);
	
	if (szSoundFile[0] == '!')
	{
		// play single sentence, one shot
		EMIT_AMBIENT_SOUND( ENT(pev), pev->origin, szSoundFile, flvolume, flattenuation, flags, pitch);

		// shut off and reset
		pev->nextthink = 0.0;
	}
	else
	{
		// make random announcement from sentence group

		//MODDD - is this better? Plays through UTIL_EmitAmbientSound instead.
		//if (SENTENCEG_PlayRndSz(ENT(pev), szSoundFile, flvolume, flattenuation, flags, pitch) < 0)
		if (SENTENCEG_PlayRndSz_Ambient(ENT(pev), pev->origin, szSoundFile, flvolume, flattenuation, flags, pitch) < 0)
			ALERT(at_console, "Level Design Error!\nSPEAKER has bad sentence group name: %s\n",szSoundFile); 

		// set next announcement time for random 5 to 10 minute delay
		pev->nextthink = gpGlobals->time + 
						RANDOM_FLOAT(ANNOUNCE_MINUTES_MIN * 60.0, ANNOUNCE_MINUTES_MAX * 60.0);

		//MODDD - changed delay from 5 to 9. Some messages may take a little longer than 5 seconds to say.
		CTalkMonster::g_talkWaitTime = gpGlobals->time + 9;		// time delay until it's ok to speak: used so that two NPCs don't talk at once
	}

	return;
}


//
// ToggleUse - if an announcement is pending, cancel it.  If no announcement is pending, start one.
//
void CSpeaker::ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int fActive = (pev->nextthink > 0.0);


	if(EASY_CVAR_GET_DEBUGONLY(announcerIsAJerk) > 0){
		//return. No damns given.
		return;
	}


	// fActive is TRUE only if an announcement is pending
	
	if ( useType != USE_TOGGLE )
	{
		// ignore if we're just turning something on that's already on, or
		// turning something off that's already off.
		if ( (fActive && useType == USE_ON) || (!fActive && useType == USE_OFF) )
			return;
	}

	if ( useType == USE_ON )
	{
		// turn on announcements
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	if ( useType == USE_OFF )
	{
		// turn off announcements
		pev->nextthink = 0.0;
		return;
	
	}

	// Toggle announcements

	
	if ( fActive )
	{
		// turn off announcements
		pev->nextthink = 0.0;
	}
	else 
	{
		// turn on announcements
		pev->nextthink = gpGlobals->time + 0.1;
	} 
}

// KeyValue - load keyvalue pairs into member data
// NOTE: called BEFORE spawn!

void CSpeaker::KeyValue( KeyValueData *pkvd )
{
	
	// preset
	if (FStrEq(pkvd->szKeyName, "preset"))
	{
		m_preset = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}