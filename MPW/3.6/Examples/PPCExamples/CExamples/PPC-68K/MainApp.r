/*
	File:		MainApp.r

	Contains:	Resources for the "calling 68K code from PowerPC" demo

	Written by:	Richard Clark

	Copyright:	© 1994 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

				 2/15/94	RC		Released

	To Do:
*/

#include "Types.r"
#include "CodeFragmentTypes.r"

resource 'cfrg' (0) {
   {
		kPowerPC,
		kFullLib,
		kNoVersionNum,
		kNoVersionNum,
		0,
		0,
		kIsApp,
		kOnDiskFlat,
		kZeroOffset,
		kWholeFork,
	  "MainApp"		// "Library name" for this app
   }
};

resource 'STR#' (128, "Import stuff") {
	{
		"MainApp";
		"gCodePointers"
	}
};


include "Wrapper.rsrc";