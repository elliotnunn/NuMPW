/*
	File:		Events.h

	Copyright:	© 1984-1993 by Apple Computer, Inc., all rights reserved.

	WARNING
	This file was auto generated by the interfacer tool. Modifications
	must be made to the master file.

*/

#ifndef __EVENTS__
#define __EVENTS__

#ifndef __TYPES__
#include <Types.h>
/*	#include <ConditionalMacros.h>								*/
/*	#include <MixedMode.h>										*/
/*		#include <Traps.h>										*/
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
/*	#include <QuickdrawText.h>									*/
/*		#include <IntlResources.h>								*/
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

enum  {
	nullEvent					= 0,
	mouseDown					= 1,
	mouseUp						= 2,
	keyDown						= 3,
	keyUp						= 4,
	autoKey						= 5,
	updateEvt					= 6,
	diskEvt						= 7,
	activateEvt					= 8,
	osEvt						= 15,
/* event mask equates */
	mDownMask					= 2,
	mUpMask						= 4,
	keyDownMask					= 8,
	keyUpMask					= 16,
	autoKeyMask					= 32,
	updateMask					= 64,
	diskMask					= 128,
	activMask					= 256,
	highLevelEventMask			= 1024,
	osMask						= -32768
};

enum  {
	everyEvent					= -1,
/* event message equates */
	charCodeMask				= 0x000000FF,
	keyCodeMask					= 0x0000FF00,
	adbAddrMask					= 0x00FF0000,
	osEvtMessageMask			= (long)0xFF000000,
/* OS event messages.  Event (sub)code is in the high byte of the message field. */
	mouseMovedMessage			= 0xFA,
	suspendResumeMessage		= 0x01,
	resumeFlag					= 1,							/* Bit 0 of message indicates resume vs suspend */
	convertClipboardFlag		= 2,							/* Bit 1 in resume message indicates clipboard change */
/* modifiers */
	activeFlag					= 1,							/* Bit 0 of modifiers for activateEvt and mouseDown events */
	btnState					= 128,							/* Bit 7 of low byte is mouse button state */
	cmdKey						= 256,							/* Bit 0 */
	shiftKey					= 512,							/* Bit 1 */
	alphaLock					= 1024,							/* Bit 2 */
	optionKey					= 2048,							/* Bit 3 of high byte */
	controlKey					= 4096,
/* obsolete equates */
	networkEvt					= 10,
	driverEvt					= 11,
	app1Evt						= 12,
	app2Evt						= 13
};

enum  {
	app3Evt						= 14,
	app4Evt						= 15,
	networkMask					= 1024,
	driverMask					= 2048,
	app1Mask					= 4096,
	app2Mask					= 8192,
	app3Mask					= 16384,
	app4Mask					= -32768
};

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct EventRecord {
	short						what;
	long						message;
	long						when;
	Point						where;
	short						modifiers;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct EventRecord EventRecord;

typedef long KeyMap[4];

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct EvQEl {
	QElemPtr					qLink;
	short						qType;
	short						evtQWhat;						/*this part is identical to the EventRecord as...*/
	long						evtQMessage;					/*defined in ToolIntf*/
	long						evtQWhen;
	Point						evtQWhere;
	short						evtQModifiers;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct EvQEl EvQEl;

typedef EvQEl *EvQElPtr;

enum  {
	uppGNEFilterProcInfo		= SPECIAL_CASE_PROCINFO(kSpecialCaseGNEFilterProc)
};

#if USESROUTINEDESCRIPTORS
typedef void (*GetNextEventFilterProcPtr)(EventRecord *theEvent, Boolean *result);

typedef UniversalProcPtr GNEFilterUPP;

#define CallGNEFilterProc(userRoutine, theEvent, result)  \
	CallUniversalProc((UniversalProcPtr)(userRoutine), uppGNEFilterProcInfo, theEvent, result)

#define NewGNEFilterProc(userRoutine)  \
	(GNEFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppGNEFilterProcInfo, GetCurrentISA())

#else
typedef ProcPtr GNEFilterUPP;

#define NewGNEFilterProc(userRoutine)  \
	(GNEFilterUPP)(userRoutine)

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern pascal Boolean GetNextEvent(short eventMask, EventRecord *theEvent)
 ONEWORDINLINE(0xA970);
extern pascal Boolean WaitNextEvent(short eventMask, EventRecord *theEvent, unsigned long sleep, RgnHandle mouseRgn)
 ONEWORDINLINE(0xA860);
extern pascal Boolean EventAvail(short eventMask, EventRecord *theEvent)
 ONEWORDINLINE(0xA971);
extern pascal void GetMouse(Point *mouseLoc)
 ONEWORDINLINE(0xA972);
extern pascal Boolean Button(void)
 ONEWORDINLINE(0xA974);
extern pascal Boolean StillDown(void)
 ONEWORDINLINE(0xA973);
extern pascal Boolean WaitMouseUp(void)
 ONEWORDINLINE(0xA977);
extern pascal void GetKeys(KeyMap theKeys)
 ONEWORDINLINE(0xA976);
extern pascal long KeyTranslate(const void *transData, short keycode, long *state)
 ONEWORDINLINE(0xA9C3);
extern pascal unsigned long TickCount(void)
 ONEWORDINLINE(0xA975);
#if USESCODEFRAGMENTS
extern pascal long GetDblTime(void);
extern pascal long GetCaretTime(void);
extern pascal QHdrPtr GetEvQHdr(void);
#else
#define GetDblTime() (* (unsigned long*) 0x02F0)

#define GetCaretTime() (* (unsigned long*) 0x02F4)

#define GetEvQHdr() ((QHdrPtr) 0x014A)

#endif


#if USES68KINLINES
#pragma parameter __D0 PostEvent(__A0, __D0)
#endif
extern pascal OSErr PostEvent(short eventNum, long eventMsg)
 ONEWORDINLINE(0xA02F);

#if USES68KINLINES
#pragma parameter __D0 PPostEvent(__A0, __D0, __A1)
#endif
extern pascal OSErr PPostEvent(short eventCode, long eventMsg, EvQElPtr *qEl)
 TWOWORDINLINE(0xA12F, 0x2288);

#if USES68KINLINES
#pragma parameter __D0 OSEventAvail(__D0, __A0)
#endif
extern pascal Boolean OSEventAvail(short mask, EventRecord *theEvent)
 TWOWORDINLINE(0xA030, 0x5240);

#if USES68KINLINES
#pragma parameter __D0 GetOSEvent(__D0, __A0)
#endif
extern pascal Boolean GetOSEvent(short mask, EventRecord *theEvent)
 TWOWORDINLINE(0xA031, 0x5240);
extern pascal void FlushEvents(short whichMask, short stopMask)
 TWOWORDINLINE(0x201F, 0xA032);

#if USES68KINLINES
#pragma parameter  SetEventMask(__D0)
#endif
extern pascal void SetEventMask(short theMask)
 TWOWORDINLINE(0x31C0, 0x0144);
#if OLDROUTINENAMES
#define KeyTrans(transData, keycode, state) KeyTranslate(transData, keycode, state)

#endif

#ifdef __cplusplus
}
#endif

#endif
