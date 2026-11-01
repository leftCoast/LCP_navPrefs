#include <navPrefs.h>
#include <strTools.h>
#include <rectObj.h>
#include <dispTools.h>
#include <editLabel.h>
#include <resizeBuff.h>
#include <GPSReader.h>
#include <debug.h>


#define	LABEL_FONT		AFF_SANS_BOLD_9_OB
#define	LABEL_COLOR		&yellow

#define SETTINGS_X	60
#define SETTINGS_Y	16		


#define FIELD_W		70
#define FIELD_H		32

#define TZONE_X		17
#define TZONE_Y		82
#define TZONE_LBL		"Time offset"
#define TZONE_MSG		"Current time zone offset from Greenwich mean time in hours. Positive values to the east of Greenwich. Negative to the west."

#define MAG_X			17
#define MAG_Y			152
#define MAG_LBL		"Mag correct"
#define MAG_MSG		"Local difference between true north and magnetic north, in degrees. If magnetic is clockwise shifted, this value will be negative. Else it will be positive." 

#define SCOPE_X		17
#define SCOPE_Y		222
#define SCOPE_LBL		"Anchor scope"
#define SCOPE_MSG		"Anchor scope multiplier. For example : If your anchor scope is 4:1, then the value to enter here should be 4."

#define MARGIN_X		17
#define MARGIN_Y		292
#define MARGIN_LBL	"Anchor margin"
#define MARGIN_MSG	"Margin is the number of feet added to extend your calculated anchor swing radius."

#define SPEW_BTN_X	17
#define SPEW_BTN_Y	360
#define SPEW_BTN_ON	"spewOn.bmp"
#define SPEW_BTN_OFF	"spewOff.bmp"
#define SPEW_X			97
#define SPEW_Y			378



// These prefs tend to be kinda' hard coded. For right now this'll get things running.

blockFile*	prefsFile = NULL;
		
float	timeOffset;
float	magCorrect;
float	scopeValue;
float	anchorMargin;
bool	streaming;


// ***************************************************************************************
// Prefs file global functions.
// ***************************************************************************************	
	


void	setPrefsFile(const char* filePath) {
	
	unsigned long	rootBlockID;
	unsigned long	rootBlock[5];
	unsigned long	numBytes;
	
	prefsFile = new blockFile(filePath);									// Here's the path. Create the pref file Obj.
	if (prefsFile) {																// Got one? Good!
		rootBlockID = prefsFile->readRootBlockID();						// Grab rootID.
		if (!rootBlockID) {														// If this is 0, then the file is empty.
			rootBlockID = prefsFile->getNewBlockID();						// We grab an ID for the root block.
			for(int i=0;i<5;i++) {												// For every array item..
				rootBlock[i]	= prefsFile->getNewBlockID();				// Set ID for each.
			}																			//
			numBytes = sizeof(unsigned long)*5;								// Calc. the rootBlock size.
			prefsFile->writeBlock(rootBlockID,(uint8_t*)rootBlock,numBytes);	// Stuff in the data we wrote.
		}
	}		
}


// Refresh the global prefs from the file.
void readPrefs(void) {
	
	unsigned long	rootBlockID;
	unsigned long	rootBlock[5];
	unsigned long	numBytes;
	
	if (prefsFile) {																								// We got a pref's file..
		numBytes = sizeof(unsigned long)*5;																	// Calc. the rootBlock size.
		rootBlockID = prefsFile->readRootBlockID();														// Grab rootID.
		if (rootBlockID) {																						// Got an ID >0?
			if (prefsFile->getBlockSize(rootBlockID)==numBytes) {										// The size what we expect?
				if (prefsFile->getBlock(rootBlockID,(uint8_t*)rootBlock,numBytes)) {				// Able to grab it?			
					prefsFile->getBlock(rootBlock[0],(uint8_t*)&timeOffset,sizeof(float));		// Grab UTC offset.
					prefsFile->getBlock(rootBlock[1],(uint8_t*)&magCorrect,sizeof(float));		// Grab mag correction.
					prefsFile->getBlock(rootBlock[2],(uint8_t*)&scopeValue,sizeof(float));		// ..
					prefsFile->getBlock(rootBlock[3],(uint8_t*)&anchorMargin,sizeof(float));	// ..
					prefsFile->getBlock(rootBlock[4],(uint8_t*)&streaming,sizeof(bool));			//
				}																										//
			}																											//
		}																												//
	}																													//
}


// Save the global prefs to the file.
void savePrefs(void) {

	unsigned long	rootBlockID;
	unsigned long	rootBlock[5];
	unsigned long	numBytes;
	
	if (prefsFile) {																								// We got a pref's file..
		numBytes = sizeof(unsigned long)*5;																	// Calc. the rootBlock size.
		rootBlockID = prefsFile->readRootBlockID();														// Grab rootID.
		if (rootBlockID) {
			if (prefsFile->getBlockSize(rootBlockID)==numBytes) {										// The size what we expect?
				if (prefsFile->getBlock(rootBlockID,(uint8_t*)rootBlock,numBytes)) {				// Able to grab it?
					prefsFile->writeBlock(rootBlock[0],(uint8_t*)&timeOffset,sizeof(float));	// Grab timeOffset.
					prefsFile->writeBlock(rootBlock[1],(uint8_t*)&magCorrect,sizeof(float));	// Grab magCorrect.
					prefsFile->writeBlock(rootBlock[2],(uint8_t*)&scopeValue,sizeof(float));	// ..
					prefsFile->writeBlock(rootBlock[3],(uint8_t*)&anchorMargin,sizeof(float));	// ..
					prefsFile->writeBlock(rootBlock[4],(uint8_t*)&streaming,sizeof(bool));		// 
				}																										//
			}																											//
		}																												//
	}																													//
}																														


// We just toggle the global. Then let the main loop sort everything else out.
void toggleStreaming(void) {
	
	streaming = !streaming;	// User wants this changed.. DO so.
	savePrefs();				// And save the change.
}


// **********************************************************************
// prefsField
// **********************************************************************


prefsField::prefsField(rect* inRect,valueType inType,const char* inMsg)
	: valueField(inRect,inType,inMsg) { ourPrefType = noPref; }
	
	
prefsField::~prefsField(void) { }


void prefsField::setPref(prefChoice inPref) { ourPrefType = inPref; }


void prefsField::updateValue(const char* newValue) {

	valueField::updateValue(newValue);
	switch(ourPrefType) {
		case noPref				: 											break;	// Nothing we can do.
		case timeOffsetPref	: timeOffset = atof(newValue);	break;
		case magCorrectPref	: magCorrect = atof(newValue);	break;
		case anchScopePref	: scopeValue = atof(newValue);	break;
		case anchMarginPref	: anchorMargin = atof(newValue);	break;
		case streamingPref	:											break;	// Again. NOthing we can do.
	}
}


// **********************************************************************
// navPrefs
// **********************************************************************


// We create our navPrefs object and, at least, pass the newAppID into our
// base panel constructor. Then you are free to do whatever you need in here.
navPrefs::navPrefs(int newAppID)
	: panel(newAppID) {
 
  	checkTimer		= new timeObj(100);
	timeOffsetLbl	= NULL;
	magCorrectLbl	= NULL;
	anchScopeLbl	= NULL;
	anchMarginLbl	= NULL;
	streamBtn		= NULL;
}


// The world as you know it, is ending. Deallocate what you need to. Don't
// deallocate stuff you've handed to addObj(). Those items will be dealt
// with internally.
navPrefs::~navPrefs(void) { }


// They all look the same. This sets up the "decoratioin bits" and colors etc.
prefsField* navPrefs::setupField(int inX,int inY,const char* labelTxt,const char* inMsg) {
	
	rect			frame(inX,inY,FIELD_W,FIELD_H);
	rectObj*		ourRectObj;
	prefsField*	ourField;
	label*		itemlabel;
	
	ourRectObj = new rectObj(&frame);
	if (ourRectObj) {
		ourRectObj->setColor(&darkYellow);
		addObj(ourRectObj);
		itemlabel = new label(inX,inY+FIELD_H+2,FIELD_W,8);
		if (itemlabel) {
			itemlabel->setColors(&darkYellow);
			itemlabel->setValue(labelTxt);
			addObj(itemlabel);
			frame.insetRect(3);
			frame.moveRect(0,2);
			ourField = new prefsField(&frame,aNumber,inMsg);
			if (ourField) {
				ourTxtPallette.setTypeFace(ourField,smallValues);
				ourField->setEventSet(fullClick);
				addObj(ourField);
				return ourField;
			}
		}
	}
	return NULL;
}


// setup() & loop() panel style.
void navPrefs::setup(void) {

	rect			aFrame;
	bmpObj*		aBmp;
	fontLabel*	aLbl;
	bmpFlasher*	aFlasher;
	char*			onPath;
	char*			offPath;
	
	onPath = NULL;
	offPath = NULL;
	readPrefs();														// Hope they did the setup..
	
	aFrame.setRect(SETTINGS_X,SETTINGS_Y,200,30);
	aLbl = new fontLabel(&aFrame);
	ourTxtPallette.setTypeFace(aLbl,panelTitle);
	aLbl->setValue("Settings");
	addObj(aLbl);
	
	timeOffsetLbl = setupField(TZONE_X,TZONE_Y,TZONE_LBL,TZONE_MSG);
	if (timeOffsetLbl) {
		timeOffsetLbl->setPref(timeOffsetPref);
		timeOffsetLbl->setValue(timeOffset);
	}
	
	magCorrectLbl = setupField(MAG_X,MAG_Y,MAG_LBL,MAG_MSG);
	if (magCorrectLbl) {
		magCorrectLbl->setPref(magCorrectPref);
		magCorrectLbl->setValue(magCorrect);
	}
	
	anchScopeLbl = setupField(SCOPE_X,SCOPE_Y,SCOPE_LBL,SCOPE_MSG);
	if (anchScopeLbl) {
		anchScopeLbl->setPref(anchScopePref);
		anchScopeLbl->setValue(scopeValue);
	}
	
	anchMarginLbl = setupField(MARGIN_X,MARGIN_Y,MARGIN_LBL,MARGIN_MSG);
	if (anchMarginLbl) {
		anchMarginLbl->setPref(anchMarginPref);
		anchMarginLbl->setValue(anchorMargin);
	}
	
	aFrame.setRect(SPEW_BTN_X,SPEW_BTN_Y,60,60);
	setFilePath(SPEW_BTN_ON);
	heapStr(&onPath,mFilePath);
	setFilePath(SPEW_BTN_OFF);
	heapStr(&offPath,mFilePath);
	streamBtn = new bmpFlasher(&aFrame,onPath,offPath);
	streamBtn->setCallback(toggleStreaming);
	streamBtn->setEventSet(fullClick);
	addObj(streamBtn);
	freeStr(&onPath);
	freeStr(&offPath);
	
	readPrefs();														// This'll trigger everything to sort out.
}

void navPrefs::checkTimeOffset(void) {

	float dispValue;
	
	if (timeOffsetLbl) {
		dispValue = atof(timeOffsetLbl->getTextBuff());
		if (dispValue!=timeOffset) {
			timeOffsetLbl->setValue(timeOffset);
		}
	}
}


void navPrefs::checkMagCorrect(void) {

	float dispValue;
	
	if (magCorrectLbl) {
		dispValue = atof(magCorrectLbl->getTextBuff());
		if (dispValue!=magCorrect) {
			magCorrectLbl->setValue(magCorrect);
		}
	}
}


void navPrefs::checkAnchorScope(void) {
		
	float dispValue;
	
	if (anchScopeLbl) {
		dispValue = atof(anchScopeLbl->getTextBuff());
		if (dispValue!=scopeValue) {
			anchScopeLbl->setValue(scopeValue);
		}
	}
}


void navPrefs::checkAnchorMargin(void) {
		
	float dispValue;
	
	if (anchMarginLbl) {
		dispValue = atof(anchMarginLbl->getTextBuff());
		if (dispValue!=anchorMargin) {
			anchMarginLbl->setValue(anchorMargin);
		}
	}
}


void navPrefs::checkStreaming(void) {
	
	if (streamBtn) {
		if (streaming!=ourGPS->spew) {				// If the saved value doesn't match what the machine is doing.
			ourGPS->setSpew(streaming);				// Change to the machine to match the saved state.
		}
		if (streaming!=streamBtn->pulseHiLow()) {	// If the saved value does not match the button..
			streamBtn->manualSetHiLow(streaming);	// Change the button.
		}
	}
}


void navPrefs::loop(void) {

	if (checkTimer->ding()) {
		checkTimeOffset();
		checkMagCorrect();
		checkAnchorScope();
		checkAnchorMargin();
		checkStreaming();
		checkTimer->start();
	}
}


// The default here is to not draw ourselves. You can change that.
void navPrefs::drawSelf(void) { screen->fillScreen(&black); }


// Whereas you call close() when you are complete and want to close.. Sometimes
// you don't have control of that. So? No matter if you call close, or something
// else calls close on you, this gets called so you can clean up before being
// deleted.
void navPrefs::closing(void) {  }
