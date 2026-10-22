#ifndef navPrefs_h
#define navPrefs_h

// **********************************************************************
//                            newApp template. 
//
// This is the starting point for creating a new "application" panel for
// a handheld device.
//
// First you must pick a name for your app. It can be 8 chars or less.
// no dots or slashes please.
//  
// The application name will typically be the name used for your app's
// .h and .cpp files. As well as your app's main class name.
//
// All your app's code files should be placed in a folder That can live 
// in your Arduino libraries folder. This folder's name is up to you.
//
// Now, if your App needs any data? files? artork? What have you. THIS
// ALL GOES IN A FOLDER WITH YOUR APP's NAME ON IT. Typically stored
// in the folder with all the source code.
//
// ON YOUR SD CARD, you should have a /system/appFiles/ folder. Drop a
// copy of your App's data folder into there. (The one with your App's
// name on it.) Then reinstall your SD card. Now your OS will know where
// to find your App's goodies when asked.
// 
// **********************************************************************



#include	<lilOS.h>
#include	<blockFile.h>
#include <fontLabel.h>
#include <bmpFlasher.h>
#include <dispTools.h>
//#include <navPrefsDB.h>

// **********************************************************************
// userPrefsFile
// **********************************************************************

extern blockFile*	prefsFile;
		
extern float		timeOffset;
extern float		magCorrect;
extern float		scopeValue;
extern float		anchorMargin;
extern bool			streaming;


// Set the prefs file. Creates new one if none available. This needs to be called by the
// main setup code. It's basically a global for anything than needs prefs.
extern void setPrefsFile(const char* filePath);


// Refresh the global prefs from the file.
extern void readPrefs(void);


// Save the global prefs to the file.
extern void savePrefs(void);


// **********************************************************************
// navPrefsValueField
// **********************************************************************


class navPrefsValueField :	public valueField {

	public:
				navPrefsValueField(rect* inRect,const char* inMsg);
	virtual	~navPrefsValueField(void);
	
				void		setVarPtr(float* inVar);
	virtual	void		doAction(void);
	virtual	void		handleCom(stdComs comID);
	
				float*	ourVarPtr;
};

	
	
// **********************************************************************
// navPrefs
// **********************************************************************


class navPrefs	: public panel {

	public:
					navPrefs(int newAppID);
	virtual		~navPrefs(void);
	
				void	setupEditField(navPrefsValueField** inLabel,float* inVar,const char* inMsg,int offset);
	virtual	void  setup(void);												// Run once. do your setup here.
				void	checkTimeOffset(void);
				void	checkMagCorrect(void);
				void	checkAnchorScope(void);
				void	checkAnchorMargin(void);
				void	checkStreaming(void);
	virtual	void  loop(void);													// Run over and over. Do stuff here.
	virtual	void  drawSelf(void);											// If you want to draw your background, do it here.
	virtual	void  closing(void);												// Need to save anything before closing shop? Do it here.
	
				timeObj*					checkTimer;
				navPrefsValueField*	timeOffsetLbl;
				navPrefsValueField*	magneticCorrectionLbl;
				navPrefsValueField*	anchorScopeLbl;
				navPrefsValueField*	anchorMarginLbl;
				bmpFlasher*				streamBtn;
				int						line;
				bool						editing;
};



#endif