


#ifndef CUSTOMMESSAGE_H
#define CUSTOMMESSAGE_H

//#include "hud.h"
//#include "hudbase.h"

// This file is for messages that are not particular to any CHud subclass.
// Methods from hud_msg.cpp moved here too.  


// No longer needs to be a subclass of CHud!
// No need for any class at all, this ended up happening in the background anyway.


extern int flag_apply_m_flTimeWeaponIdle;
extern float stored_m_flTimeWeaponIdle;
extern int flag_apply_m_fJustThrown;
extern int stored_m_fJustThrown;





extern void Init_CustomMessage(void);

// NOTE - no need to extern the messages in custom_messages.cpp.
// They're called by the client's engine on receiving events.


#endif //END OF CUSTOMMESSAGE_H