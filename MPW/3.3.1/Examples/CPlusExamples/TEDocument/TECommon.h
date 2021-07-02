/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	MultiFinder-Aware Simple TextEdit Sample Application
#
#	CPlusTESample
#
#	TECommon.h	-	C++ and Rez source
#
#	Copyright © 1989 Apple Computer, Inc.
#	All rights reserved.
#
#	Versions:	
#			1.10 					07/89
#			1.00 					04/89
#
#	Components:
#			TEDocument.o.make		July 9, 1989
#			TDocument.h				July 9, 1989
#			TECommon.h				July 9, 1989
#			TESample.h				July 9, 1989
#			TEDocument.h			July 9, 1989
#			TEDocument.cp			July 9, 1989
#			TESampleGlue.a			July 9, 1989
#
#	CPlusTESample is an example application that demonstrates
#	how to initialize the commonly used toolbox managers,
#	operate successfully under MultiFinder, handle desk
#	accessories and create, grow, and zoom windows. The
#	fundamental TextEdit toolbox calls and TextEdit autoscroll
#	are demonstrated. It also shows how to create and maintain
#	scrollbar controls. 
#
#	This version of TESample has been substantially reworked in
#	C++ to show how a "typical" object oriented program could
#	be written. To this end, what was once a single source code
#	file has been restructured into a set of classes which
#	demonstrate the advantages of object-oriented programming.
#   
------------------------------------------------------------------------------*/

#ifndef TECommon_Defs
#define TECommon_Defs

/*
	These definitions are shared by Rez and C++. We use #define statements
	instead of constants in this file because Rez doesn't support constants,
	and since the 3.0b1 version of Rez doesn't like the C++ comments, we use
	C style comments in this file as well.
 */

/*	Determining an application's minimum size to request from MultiFinder depends
	on many things, each of which can be unique to an application's function,
	the anticipated environment, the developer's attitude of what constitutes
	reasonable functionality and performance, etc. Here is a list of some things to
	consider when determining the minimum size (and preferred size) for your
	application. The list is pretty much in order of importance, but by no means
	complete.
	
	1.	What is the minimum size needed to give almost 100 percent assurance
		that the application won't crash because it ran out of memory? This
		includes not only things that you do have direct control over such as
		checking for NIL handles and pointers, but also things that some
		feel are not so much under their control such as QuickDraw and the
		Segment Loader.
		
	2.	What kind of performance can a user expect from the application when
		it is running in the minimum memory configuration? Performance includes
		not only speed in handling data, but also things like how many documents
		can be opened, etc.
		
	3.	What are the typical sizes of scraps that a user might wish to work
		with when lauching or switching to your application? If the amount
		of memory is too small, the scrap will be purged from memory. This
		can be quite frustrating to the user.
		
	4.	The previous items have concentrated on topics that tend to cause an
		increase in the minimum size to request from MultiFinder. On the flip
		side, however, should be the consideration of what environments the
		application may be running in. There may be a high probability that
		many users with relatively small memory configurations will want to
		avail themselves of your application. Or, many users might want to use it
		while several other, possibly related/complementary applications are
		running. If that is the case, it would be helpful to have a fairly
		small minimum size.
	
	What we did for CPlusTESample:
	
		We determined the smallest heap size that CPlusTESample could have and
		still run (24K). For the preferred size we added enough space to permit:
			a. a maximum size TextEdit text handle (32000 characters)
			b. a maximum usable TextEdit scrap (32000 characters)
			b. a maximum scrap as a result of Copy (32000 characters)
			d. a little performance cushion (see 2, above) (10K)
		Result: 124K for preferred size
		
		For the minimum size we took the 22K and then scaled down our requirements
		for a,b, and c above. We thought that providing 16K more would be lean
		and mean (see 4, above).
		Result: 40K for minimum size
*/
 
#define kPrefSize				124
#define kMinSize				40
	
/* kMinHeap - This is the minimum result from the following
	 equation:
			
			ORD(GetApplLimit) - ORD(ApplicZone)
			
	 for the application to run. It will insure that enough memory will
	 be around for reasonable-sized scraps, FKEYs, etc. to exist with the
	 application, and still give the application some 'breathing room'.
	 To derive this number, we ran under a MultiFinder partition that was
	 our requested minimum size, as given in the 'SIZE' resource. */
	 
#define	kMinHeap				 (34 * 1024)
	
/* kMinSpace - This is the minimum result from PurgeSpace, when called
	 at initialization time, for the application to run. This number acts
	 as a double-check to insure that there really is enough memory for the
	 application to run, including what has been taken up already by
	 pre-loaded resources, the scrap, code, and other sundry memory blocks. */
	 
#define	kMinSpace				(20 * 1024)

/* id of our STR# for error strings */
#define kTEDocErrStrings	 129

/* The following are indicies into STR# resources. */
#define	eNoMemory				1
#define	eNoSpaceCut				2
#define	eNoCut					3
#define	eNoCopy					4
#define	eExceedPaste			5
#define	eNoSpacePaste			6
#define	eNoWindow				7
#define	eExceedChar				8
#define	eNoPaste				9

#define	rMenuBar	128				/* application's menu bar */
#define	rAboutAlert	128				/* about alert */
#define	rUserAlert	129				/* user error alert */
#define	rDocWindow	128				/* application's window */
#define	rVScroll	128				/* vertical scrollbar control */
#define	rHScroll	129				/* horizontal scrollbar control */

/* The following constants are used to identify menus and their items. The menu IDs
   have an "m" prefix and the item numbers within each menu have an "i" prefix. */
#define	mApple					128		/* Apple menu */
#define	iAbout					1

#define	mFile					129		/* File menu */
#define	iNew					1
#define	iClose					4
#define	iQuit					12

#define	mEdit					130		/* Edit menu */
#define	iUndo					1
#define	iCut					3
#define	iCopy					4
#define	iPaste					5
#define	iClear					6

#endif TECommon_Defs