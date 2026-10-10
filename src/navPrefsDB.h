#ifndef navPrefsDB_h
#define navPrefsDB_h

#include <editLabel.h>
#include <datafield.h>
#include <modalKbd.h>
#include <alertObj.h>



// **************************************************************
// navPrefsReadmeDBox
// **************************************************************


class navPrefsReadmeDBox :	public alertObj {

	public:
				navPrefsReadmeDBox(listener* inListener);
				~navPrefsReadmeDBox(void);			
};



// **************************************************************
// navPrefsEditDbox
// **************************************************************


class navPrefsEditDbox :	public alertObj,
									public kbdUser {

public:
			navPrefsEditDbox(float inValue,const char* alertMsg,listener* inListener,screenTypes inType=sType240x320);
virtual	~navPrefsEditDbox(void);
				
			float 		getValue(void);

			editLabel*	editField;
};

			
#endif