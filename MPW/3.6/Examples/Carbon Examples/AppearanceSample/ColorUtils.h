/*
	File:		ColorUtils.h

	Contains:	Macros to help drawing in color.

	Version:    CarbonLib 1.0.2 SDK

	You may incorporate this sample code into your applications without
	restriction, though the sample code has been provided "AS IS" and the
	responsibility for its operation is 100% yours.  However, what you are
	not permitted to do is to redistribute the source as "Apple Sample Code"
	after having made changes. If you're going to re-distribute the source,
	we require that you make it clear in the source that the code was
	descended from Apple Sample Code, but that you've made changes.

	© 1997-1999 by Apple Computer, Inc. All rights reserved.
*/

#ifndef __COLOR_UTILS__
#define __COLOR_UTILS__

#include "ColorPenState.h"

enum {
	kBlack			= 0,
	kGrayLevel1		= 0x1111,
	kGrayLevel2		= 0x2222,
	kGrayLevel3		= 0x3333,
	kGrayLevel4		= 0x4444,
	kGrayLevel5		= 0x5555,
	kGrayLevel6		= 0x6666,
	kGrayLevel7		= 0x7777,
	kGrayLevel8		= 0x8888,
	kGrayLevel9		= 0x9999,
	kGrayLevelA		= 0xAAAA,
	kGrayLevelB		= 0xBBBB,
	kGrayLevelC		= 0xCCCC,
	kGrayLevelD		= 0xDDDD,
	kGrayLevelE		= 0xEEEE,
	kWhite			= 0xFFFF
};

#if TARGET_CARBON
#define IsColorGrafPort( port )	true
#define GetCurrentDepth( port )	GetPixDepth( GetPortPixMap( port ) )
#else
#define IsColorGrafPort( port ) (((port)->portBits.rowBytes & 0xC000) == 0xC000)
#define GetCurrentDepth( port )	( IsColorGrafPort( port ) ? (**((CGrafPtr)port)->portPixMap).pixelSize : 1 )
#endif

#define MakeGray( color, gray ) ((color).red = (color).green = (color).blue = gray )
 
#define SetForeGray( gray )	\
	do { RGBColor temp; temp.red = temp.green = temp.blue = (gray); RGBForeColor( &temp ); } while( 0 )
#define SetBackGray( gray )	\
	do { RGBColor temp; temp.red = temp.green = temp.blue = (gray); RGBBackColor( &temp ); } while( 0 )

#endif // __COLOR_UTILS__