
// FILE NOT PART OF COMPILE.

// Copy to some other file and run by calling 'custom_parent_class_demo_run' with a 
// monster pointer ('this' in MonsterThink is easiest to get it to run).
// Put a breakpoint at the last statement in the method, see what some vars are.

// Shows a simple way to make calls to the parent method of a class possible.
// CMockParent inherits from CBaseMonster.
// The 'MyMonsterPointer_Parent' method gets either 'CBaseEntity::MyMonsterPointer'
// or 'CBaseMonster::MyMonsterPointer', depending on what class the CMockParent pointer
// (asParent1 and asParent2 further down) are.  CBaseEntity::MyMonsterPointer returns
// NULL, CBaseMonster::MyMonsterPointer works normally.
// This lets the CHGrunt class call 'mySquadModule.SomeMethod', which can then
// look to its parent (a I_SquadModule_Parent pointer) for its own 'I_SquadModule_SomeMethod_Parent'
// call to let CHGrunt call whatever its parent class is (CBaseMonster::SomeMethod(),
// CCombatMonster::SomeMethod(), etc).


class CMockParent : public CBaseMonster {
public:
	virtual CBaseMonster* MyMonsterPointer_Parent(void);
};
CBaseMonster* CMockParent::MyMonsterPointer_Parent(void){
	return NULL;
}



class CMockChild1 : public CMockParent {
public:
	virtual CBaseMonster* MyMonsterPointer_Parent(void);
};
CBaseMonster* CMockChild1::MyMonsterPointer_Parent(void){
	return CBaseEntity::MyMonsterPointer();
}

class CMockChild2 : public CMockParent {
public:
	virtual CBaseMonster* MyMonsterPointer_Parent(void);
};
CBaseMonster* CMockChild2::MyMonsterPointer_Parent(void){
	return CBaseMonster::MyMonsterPointer();
}



void custom_parent_class_demo_run(CBaseMonster* arg_this){

	// WELP.   That works.
	CBaseMonster* someMonster = arg_this;
	CBaseMonster* selfo1 = someMonster->CBaseMonster::MyMonsterPointer();
	CBaseMonster* selfo2 = someMonster->CBaseEntity::MyMonsterPointer();



	// Works too?
	CMockChild1 theGuy1;
	CMockChild2 theGuy2;

	CMockParent* asParent1 = (CMockParent*)&theGuy1;
	CMockParent* asParent2 = (CMockParent*)&theGuy2;

	//CBaseMonster* test1 = theGuy1.MyMonsterPointer_Parent();
	//CBaseMonster* test2 = theGuy2.MyMonsterPointer_Parent();
	CBaseMonster* test1 = asParent1->MyMonsterPointer_Parent();
	CBaseMonster* test2 = asParent2->MyMonsterPointer_Parent();


	// breakpoint?
	int x = 45;

}







/*
// Way too complicated and still doesn't work.  Amazing.  See the other approach above.
///////////////////////////////////////////////////////////////////////////////////////////
class CMockParent {
public:
	MONSTERSTATE (CBaseMonster::*GetIdealState_ParentMethodRef(void)) (void);
	CBaseMonster* (CBaseEntity::*GetMyMonsterMethod(void))(void);
};


// 'function returning a function not allowed'
//MONSTERSTATE (CBaseMonster::*GetIdealState_ParentMethodRef)(void) (void){
// 'oh whoops nevermind jk'
// ...................................................... w.  h.  a.  t.
MONSTERSTATE (CBaseMonster::*CMockParent::GetIdealState_ParentMethodRef(void)) (void){
	return &CBaseMonster::GetIdealState;
}


CBaseMonster* (CBaseEntity::*CMockParent::GetMyMonsterMethod(void)) (void){
	return NULL;
}



class CMock1 : public CMockParent {
public:
	CBaseMonster* (CBaseEntity::*GetMyMonsterMethod(void))(void);
};
CBaseMonster* (CBaseEntity::*CMock1::GetMyMonsterMethod(void)) (void){
	return &CBaseEntity::MyMonsterPointer;
}

class CMock2 : public CMockParent {
public:
	CBaseMonster* (CBaseEntity::*GetMyMonsterMethod(void))(void);
};
CBaseMonster* (CBaseEntity::*CMock2::GetMyMonsterMethod(void)) (void){
	return static_cast <CBaseMonster* (CBaseEntity::*)(void)>(&CBaseMonster::MyMonsterPointer);
}
///////////////////////////////////////////////////////////////////////////////////////////
*/


void custom_parent_class_demo_run_SCRAP(CBaseMonster* arg_this) {

	
	// TEST.  SOOooooo that was mostly pointless.   Horray.
	// Pointers to methods really, really, reeeeeeaaaaaaallllllllyyyyyyyy insist on being to the method it wants to give you.
	// No... matter.  How... much you.   Direct.      It.          classy.
	//   But.     someInst->BaseClass::theMethod(...)     works.     fine.         okay.      yeah.                        sure

	// Anyway.   The point of this was to see if calling the base class of another object is do-able, for turning
	// SquadMonster into a compositional form (don't know what else to call that, but the point is a form that can be
	// included as a member var in houndeye, hgrunt, hassault without including SquadMonster, so they're free to have
	// other more relevant parent class choices like houndeye : CBaseMonster,  hgrunt : CCombatMonster  (new for what's
	// common b/w hgrunt and hassault),   etc.
	// So that's sorted out here, but only one issue.  Can't assume what the parent class of whatever owns this SquadMonster
	// object is.  Is it CBaseMonster?  Would it be that CCombatMonster?  No idea.
	// Probably best to be some overridable given by whatever 'I_SquadMonster_Parent' class to tell me, like:
	// <see the CMockParent + CMock1, CMock2 class demo above>

	/*
	//CBaseMonster* someMonsto = arg_this;
	//CBaseMonster* thisMon = someMonsto->GetMonsterPointer();

	//static_cast <void (CBaseEntity::*)(void)
	//CBaseMonster* (CBaseMonster::* eventMethoda)(void) = &MyMonsterPointer;

	CBaseMonster* (CBaseEntity::* eventMethod0)(void) = &CBaseEntity::MyMonsterPointer;
	CBaseMonster* (CBaseEntity::* eventMethod1)(void) = static_cast <CBaseMonster* (CBaseEntity::*)(void)>(&CBaseEntity::MyMonsterPointer);
	CBaseMonster* (CBaseEntity::* eventMethod2)(void) = static_cast <CBaseMonster* (CBaseEntity::*)(void)>(&CBaseMonster::MyMonsterPointer);
	CBaseMonster* (CBaseMonster::* eventMethod3)(void) = &CBaseMonster::MyMonsterPointer;

	//static_cast <void (CBaseEntity::*)(CBaseMonster*)>(&someMonsto->GetMonsterPointer)();

	//CBaseMonster* someMon = ;
	CBaseMonster* resulto1 = (arg_this->*eventMethod1)();
	//CBaseMonster* resulto2 = (arg_this->*eventMethod2)();
	CBaseMonster* resulto3 = (arg_this->*eventMethod3)();

	CBaseEntity* selfo = static_cast<CBaseEntity*>(arg_this);
	CBaseMonster* resulto1b = (selfo->*eventMethod1)();
	//CBaseMonster* resulto2b = (selfo->*eventMethod2)();
	//CBaseMonster* resulto3b = (selfo->*eventMethod3)();


	CBaseMonster* resulto4 = CBaseEntity::MyMonsterPointer();
	CBaseMonster* resulto5 = CBaseMonster::MyMonsterPointer();
	CBaseMonster* resulto6 = MyMonsterPointer();
	*/

	//arg_this->CBaseEntity::MyMonsterPointer();


	/*
	// disabled.
	CMock1 theGuy1;
	CMock2 theGuy2;
	CBaseMonster* (CBaseEntity::* theOne1)(void) = theGuy1.GetMyMonsterMethod();
	CBaseMonster* (CBaseEntity::* theOne2)(void) = theGuy2.GetMyMonsterMethod();
	*/


}// custom_parent_class_demo_run
