
// NEW FILE.  Need a list of EHANDLE's of uncertain size?

#include "extdll.h"
#include "cbase.h"


#ifndef LINKEDLIST_EHANDLE_H
#define LINKEDLIST_EHANDLE_H

class LinkedListTempNode {
public:
	EHANDLE ent;
	LinkedListTempNode* next;
	BOOL hitThisFrame;

	LinkedListTempNode() {
		ent = NULL;
		next = NULL;
		hitThisFrame = FALSE;
	}
	~LinkedListTempNode() {
		if (next != NULL) {
			delete next;
			next = NULL;
		}
	}

};

class LinkedListTemp {
public:
	LinkedListTempNode* head;

	BOOL contains(CBaseEntity* argTest) {
		// safety?
		//if(argTest==NULL || argTest->edict() == NULL){ return FALSE; }

		LinkedListTempNode* thisNode = head;

		while (thisNode != NULL) {

			if (thisNode->ent != NULL && thisNode->ent.Get() == argTest->edict()) {
				return TRUE;
			}
			thisNode = thisNode->next;
		}

		return FALSE;

	}//END OF contains

	LinkedListTempNode* getNonEmpty(BOOL& mustAppend) {

		if (head == NULL)return NULL; //all we can do.

		LinkedListTempNode* thisNode = head;

		while (thisNode != NULL) {
			if (thisNode->ent == NULL) {
				//if this node's ent has expired 
				mustAppend = FALSE;
				return thisNode;
			}
			else if (thisNode->next == NULL) {
				//or if the next is null, this is good.
				mustAppend = TRUE;
				return thisNode;
			}
			else {
				//not null? go there next.
				thisNode = thisNode->next;
			}
		}

		return NULL; //???
	}//END OF getLastNode

	void append(CBaseEntity* argEnt) {

		if (contains(argEnt)) {
			//forget it.
			return;
		}

		BOOL mustAppend;
		LinkedListTempNode* someNode = getNonEmpty(mustAppend);


		if (someNode == NULL) {
			//first one. send to the head.
			head = new LinkedListTempNode();
			head->ent = argEnt;
		}
		else {
			//append.
			if (mustAppend) {
				someNode->next = new LinkedListTempNode();
				someNode->next->ent = argEnt;
			}
			else {
				someNode->ent = argEnt;
			}

		}
	}//END OF append


	LinkedListTemp() {
		head = NULL;
	}
	~LinkedListTemp() {
		if (head != NULL) {
			delete head;
			head = NULL;
		}
	}

};

#endif //LINKEDLIST_EHANDLE_H
