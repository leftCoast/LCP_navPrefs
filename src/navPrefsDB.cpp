#include <navPrefsDB.h>
#include <modalKbd.h>
#include <dispTools.h>
#include <rectObj.h>
//#include <debug.h>

#define	README_MSG	"Yeah, well.."

// **************************************************************
// navPrefsReadmeDBox
// **************************************************************


navPrefsReadmeDBox::navPrefsReadmeDBox(listener* inListener)
	: alertObj("default",inListener,noteAlert,true,false) {
	
		int	addedY;
		
		addedY = height * 2;
		height = height + addedY;
		okBtn->y = okBtn->y + addedY;
		theMsg->height = theMsg->height + addedY;
		theMsg->setText(README_MSG);
		theMsg->calculate();
	}
	
	
navPrefsReadmeDBox::~navPrefsReadmeDBox(void) {  }		
	

	
// **************************************************************
// navPrefsEditDbox
// **************************************************************


navPrefsEditDbox::navPrefsEditDbox(float inValue,const char* alertMsg,listener* inListener,screenTypes inType)
	: alertObj(alertMsg,inListener,noteAlert,false,false),
	kbdUser(this,inType) {
	
	rect			aRect;
	rectObj*		editFrame;
	
	setRect(40,100,240,130);								// Set our alertBox size and shape.
	theMsg->x = theMsg->x + 5;								// Tweaking message size and shape
	theMsg->y = 20;											// More tweaking.
	theMsg->height = 60;										// Kick the default message down a bit.
	theMsg->width = theMsg->width + 30;					// ..
	theMsg->calculate();										// All set? recalculate the line breaks.
	
	aRect.setRect(20,90,100,17);							// Setup location for the edit box.																				//
	editField = new editLabel(&aRect,"");				// Create it.
	ourTxtPallette.setTypeFace(editField,editText);	// Set the typeFace parameters.
	editField->setValue(inValue);							//
	editField->setEventSet(fullClick);					// Events when editing.
	addObj(editField);										// Hook it up.
	getKbd()->setEditField(editField);					// Tell the keyboard where it's edit field is.
	aRect.insetRect(-3);										// Expand it by 3 pixels per side.
	editFrame = new rectObj(&aRect);						// Use it to frame the editing text.
	editFrame->setColor(&black);							// St frame color.
	addObj(editFrame);										// Add it to our dialog box.	
	editField->beginEditing();								// Make our edit field active.
	hookup();													// Hook into the idler queue.
}


navPrefsEditDbox::~navPrefsEditDbox(void) {  }

	
float	navPrefsEditDbox::getValue(void) { return atof(editField->getTextBuff()); }

