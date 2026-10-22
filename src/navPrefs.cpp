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

#define SETTINGS_LBL_X	60	// 95 x 26 pix
#define SETTINGS_LBL_Y	16		

#define EDIT_BACK_X		17		// 70 x 32 pix
#define EDIT_BACK_Y		73

#define TZONE_LBL_X		97		// 154 x 26 pix
#define TZONE_LBL_Y		82
#define TIME_MSG			"Current time zone offset from Greenwich mean time in hours. Positive values to the east of Greenwich. Negative to the west."

#define MAG_LBL_X			97		// 176 x 25 pix
#define MAG_LBL_Y			152
#define MAG_MSG			"Local difference between true north and magnetic north, in degrees. If magnetic is clockwise shifted, this value will be negative. Else it will be positive." 

#define SCOPE_LBL_X		97		// 200 x 25 pix
#define SCOPE_LBL_Y		222
#define SCOPE_MSG			"Anchor scope multiplier. For example : If your anchor scope is 4:1, then the value to enter here should be 4."

#define MARGIN_LBL_X		97		// 204 x 25 pix
#define MARGIN_LBL_Y		292
#define MARGIN_MSG		"Margin is the number of feet to extend your calculated anchor swing radius before setting an alarm."

#define SPEW_BTN_X		17		// 60 x 60 pix
#define SPEW_BTN_Y		360
#define SPEW_BTN_ON		"spewOn.bmp"
#define SPEW_BTN_OFF		"spewOff.bmp"
#define SPEW_LBL_X		97		// 124 x 25 pix
#define SPEW_LBL_Y		378



// These prefs tend to be kinda' hard coded. For right now this'll get things running.

blockFile*	prefsFile = NULL;
		
float			timeOffset;
float			magCorrect;
float			scopeValue;
float			anchorMargin;
bool			streaming;


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
// navPrefsValueField
// **********************************************************************


navPrefsValueField::navPrefsValueField(rect* inRect,const char* inMsg)
	: valueField(inRect,inMsg) { ourVarPtr	= NULL; }
	
	
navPrefsValueField::~navPrefsValueField(void) { }


void navPrefsValueField::setVarPtr(float* inVar) { ourVarPtr = inVar; }


// Create and fire off the editing modal.	
void navPrefsValueField::doAction(void) {

	((navPrefs*)ourPanel)->editing = true;
	ourDbox = new valueFieldEditDbox(atof(buff),ourMsg,this,sType320x480);
}


void navPrefsValueField::handleCom(stdComs comID) {

	switch(comID) {
		case cancelCmd	:
			ourDbox = NULL;
			((navPrefs*)ourPanel)->editing = false;
		break;
		case okCmd	:
			if (ourVarPtr) {
				setValue(ourDbox->getValue());
				*ourVarPtr = ourDbox->getValue();
				savePrefs();
			}
			ourDbox = NULL;
			((navPrefs*)ourPanel)->editing = false;
		break;
	}
}


// **********************************************************************
// navPrefs
// **********************************************************************


// We create our navPrefs object and, at least, pass the newAppID into our
// base panel constructor. Then you are free to do whatever you need in here.
navPrefs::navPrefs(int newAppID)
	: panel(newAppID) {
  
  	checkTimer = new timeObj(200);
	timeOffsetLbl				= NULL;
	magneticCorrectionLbl	= NULL;
	anchorScopeLbl				= NULL;
	anchorMarginLbl			= NULL;
	streamBtn					= NULL;
	line							= 0;
	editing						= false;
}


// The world as you know it, is ending. Deallocate what you need to. Don't
// deallocate stuff you've handed to addObj(). Those items will be dealt
// with internally.
navPrefs::~navPrefs(void) { }


// Each edit field is pretty much the same. Lets just do them here.
void navPrefs::setupEditField(navPrefsValueField** inLabel,float* inVar,const char* inMsg,int offset) {

	rect			aFrame;
	bmpObj*		aBmp;
	int			EditY;

	EditY = EDIT_BACK_Y+offset;
	aFrame.setRect(EDIT_BACK_X,EditY,70,32);
	rectObj* aRectObj = new rectObj(&aFrame);
	if (aRectObj) {
		aRectObj->setColor(&darkYellow);
		addObj(aRectObj);
		aFrame.setRect(EDIT_BACK_X+10,EditY+10,55,28);
		*inLabel = new navPrefsValueField(&aFrame,inMsg);
		if (*inLabel) {
			ourTxtPallette.setTypeFace(*inLabel,smallValues);
			(*inLabel)->setVarPtr(inVar);
			(*inLabel)->setValue(*inVar);
			addObj(*inLabel);
		}
	}
}


// setup() & loop() panel style.
void navPrefs::setup(void) {

	rect			aFrame;
	bmpObj*		aBmp;
	fontLabel*	aLbl;
	bmpFlasher*	aFlasher;
	char*			onPath;
	char*			offPath;
	int			offset;
	
	onPath = NULL;
	offPath = NULL;
	readPrefs();														// Hope they did the setup..
		
	aFrame.setRect(SETTINGS_LBL_X,SETTINGS_LBL_Y,200,30);
	aLbl = new fontLabel(&aFrame);
	ourTxtPallette.setTypeFace(aLbl,panelTitle);
	aLbl->setValue("Settings");
	addObj(aLbl);
	
	offset = 0;
	setupEditField(&timeOffsetLbl,&timeOffset,TIME_MSG,offset);
	
	offset = offset + 70;
	setupEditField(&magneticCorrectionLbl,&magCorrect,MAG_MSG,offset);
	
	offset = offset + 70;
	setupEditField(&anchorScopeLbl,&scopeValue,SCOPE_MSG,offset);
	
	offset = offset + 70;
	setupEditField(&anchorMarginLbl,&anchorMargin,MARGIN_MSG,offset);
	
	aFrame.setRect(TZONE_LBL_X,TZONE_LBL_Y,154,26);
	aLbl = new fontLabel(&aFrame);
	ourTxtPallette.setTypeFace(aLbl,itemLabel);
	aLbl->setValue("Local time offset");
	addObj(aLbl);
	
	aFrame.setRect(MAG_LBL_X,MAG_LBL_Y,176,25);
	aLbl = new fontLabel(&aFrame);
	ourTxtPallette.setTypeFace(aLbl,itemLabel);
	aLbl->setValue("Magnetic correction");
	addObj(aLbl);
	
	aFrame.setRect(SCOPE_LBL_X,SCOPE_LBL_Y,200,25);
	aLbl = new fontLabel(&aFrame);
	ourTxtPallette.setTypeFace(aLbl,itemLabel);
	aLbl->setValue("Anchor scope");
	addObj(aLbl);
	
	aFrame.setRect(MARGIN_LBL_X,MARGIN_LBL_Y,204,25);
	aLbl = new fontLabel(&aFrame);
	ourTxtPallette.setTypeFace(aLbl,itemLabel);
	aLbl->setValue("Anchor margin");
	addObj(aLbl);
	
	aFrame.setRect(SPEW_LBL_X,SPEW_LBL_Y,124,25);
	aLbl = new fontLabel(&aFrame);
	ourTxtPallette.setTypeFace(aLbl,itemLabel);
	aLbl->setValue("GPS streaming");
	addObj(aLbl);
	
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
	
	dispValue = atof(timeOffsetLbl->getTextBuff());
	if (dispValue!=timeOffset) {
		timeOffsetLbl->setValue(timeOffset);
	}
}


void navPrefs::checkMagCorrect(void) {

	float dispValue;
	
	dispValue = atof(magneticCorrectionLbl->getTextBuff());
	if (dispValue!=magCorrect) {
		magneticCorrectionLbl->setValue(magCorrect);
	}
}


void navPrefs::checkAnchorScope(void) {
		
	float dispValue;
	
	dispValue = atof(anchorScopeLbl->getTextBuff());
	if (dispValue!=scopeValue) {
		anchorScopeLbl->setValue(scopeValue);
	}
}


void navPrefs::checkAnchorMargin(void) {
		
	float dispValue;
	
	dispValue = atof(anchorMarginLbl->getTextBuff());
	if (dispValue!=anchorMargin) {
		anchorMarginLbl->setValue(anchorMargin);
	}
}


void navPrefs::checkStreaming(void) {
	
	if (streaming!=ourGPS->spew) {				// If the saved value doesn't match what the machine is doing.
		ourGPS->setSpew(streaming);				// Change to the machine to match the saved state.
	}
	if (streaming!=streamBtn->pulseHiLow()) {	// If the saved value does not match the button..
		streamBtn->manualSetHiLow(streaming);	// Change the button.
	}
}


void navPrefs::loop(void) {

	if (checkTimer->ding()&&!editing) {
		switch(line) {										// The idea here is to get better touch
			case 0 : checkTimeOffset();	break;	// Response by doing these one at a time.
			case 1 : checkMagCorrect();	break;
			case 2 : checkAnchorScope();	break;
			case 3 : checkAnchorMargin();	break;
			case 4 : checkStreaming();		break;
		}
		line++;
		if (line>4) line = 0;
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
