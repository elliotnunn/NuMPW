/*
	File:		CTBUtilities.h

	Copyright:	© 1984-1993 by Apple Computer, Inc., all rights reserved.

	WARNING
	This file was auto generated by the interfacer tool. Modifications
	must be made to the master file.

*/

#ifndef __CTBUTILITIES__
#define __CTBUTILITIES__

#ifndef	__MEMORY__
#include <Memory.h>
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*		#include <MixedMode.h>									*/
/*			#include <Traps.h>									*/
#endif

#ifndef __STANDARDFILE__
#include <StandardFile.h>
/*	#include <Dialogs.h>										*/
/*		#include <Windows.h>									*/
/*			#include <Quickdraw.h>								*/
/*				#include <QuickdrawText.h>						*/
/*					#include <IntlResources.h>					*/
/*			#include <Events.h>									*/
/*				#include <OSUtils.h>							*/
/*			#include <Controls.h>								*/
/*				#include <Menus.h>								*/
/*		#include <TextEdit.h>									*/
/*	#include <Files.h>											*/
/*		#include <SegLoad.h>									*/
#endif

#ifndef __APPLETALK__
#include <AppleTalk.h>
#endif

enum  {
/*	version of Comm Toolbox Utilities	*/
	curCTBUVersion				= 2,
/*    Error codes/types    */
	ctbuGenericError			= -1,
	ctbuNoErr					= 0
};

typedef OSErr CTBUErr;

enum  {
	chooseDisaster				= -2,
	chooseFailed,
	chooseAborted,
	chooseOKMinor,
	chooseOKMajor,
	chooseCancel
};

typedef unsigned short ChooseReturnCode;

enum  {
	nlOk,
	nlCancel,
	nlEject
};

typedef unsigned short NuLookupReturnCode;

enum  {
	nameInclude					= 1,
	nameDisable,
	nameReject
};

typedef unsigned short NameFilterReturnCode;

enum  {
	zoneInclude					= 1,
	zoneDisable,
	zoneReject
};

typedef unsigned short ZoneFilterReturnCode;

typedef pascal short (*DialogHookProcPtr)(short item, DialogPtr theDialog);

enum {
	uppDialogHookProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(DialogPtr)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr DialogHookUPP;

#define CallDialogHookProc(userRoutine, item, theDialog)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDialogHookProcInfo, (item), (theDialog))
#define NewDialogHookProc(userRoutine)		\
		(DialogHookUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDialogHookProcInfo, GetCurrentISA())
#else
typedef DialogHookProcPtr DialogHookUPP;

#define CallDialogHookProc(userRoutine, item, theDialog)		\
		(*(userRoutine))((item), (theDialog))
#define NewDialogHookProc(userRoutine)		\
		(DialogHookUPP)(userRoutine)
#endif

enum  {
/*	Values for hookProc items		*/
	hookOK						= 1,
	hookCancel					= 2,
	hookOutline					= 3,
	hookTitle					= 4,
	hookItemList				= 5,
	hookZoneTitle				= 6,
	hookZoneList				= 7,
	hookLine					= 8,
	hookVersion					= 9,
	hookReserved1				= 10,
	hookReserved2				= 11,
	hookReserved3				= 12,
	hookReserved4				= 13,
/*	"virtual" hookProc items	*/
	hookNull					= 100,
	hookItemRefresh				= 101,
	hookZoneRefresh				= 102,
	hookEject					= 103,
	hookPreflight				= 104,
	hookPostflight				= 105,
	hookKeyBase					= 1000
};


/*	NuLookup structures/constants	*/

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct NLTypeEntry {
	Handle						hIcon;
	Str32						typeStr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct NLTypeEntry NLTypeEntry;

typedef NLTypeEntry NLType[4];

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct NBPReply {
	EntityName					theEntity;
	AddrBlock					theAddr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct NBPReply NBPReply;

typedef pascal short (*NameFilterProcPtr)(const EntityName *theEntity);

enum {
	uppNameFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EntityName*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr NameFilterUPP;

#define CallNameFilterProc(userRoutine, theEntity)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppNameFilterProcInfo, (theEntity))
#define NewNameFilterProc(userRoutine)		\
		(NameFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppNameFilterProcInfo, GetCurrentISA())
#else
typedef NameFilterProcPtr NameFilterUPP;

#define CallNameFilterProc(userRoutine, theEntity)		\
		(*(userRoutine))((theEntity))
#define NewNameFilterProc(userRoutine)		\
		(NameFilterUPP)(userRoutine)
#endif

typedef pascal short (*ZoneFilterProcPtr)(Str32 theZone);

enum {
	uppZoneFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(void* /* is array */)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ZoneFilterUPP;

#define CallZoneFilterProc(userRoutine, theZone)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppZoneFilterProcInfo, (theZone))
#define NewZoneFilterProc(userRoutine)		\
		(ZoneFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppZoneFilterProcInfo, GetCurrentISA())
#else
typedef ZoneFilterProcPtr ZoneFilterUPP;

#define CallZoneFilterProc(userRoutine, theZone)		\
		(*(userRoutine))((theZone))
#define NewZoneFilterProc(userRoutine)		\
		(ZoneFilterUPP)(userRoutine)
#endif

typedef NameFilterProcPtr nameFilterProcPtr;

typedef ZoneFilterProcPtr zoneFilterProcPtr;

#ifdef __cplusplus
extern "C" {
#endif

extern pascal CTBUErr InitCTBUtilities(void);
extern pascal short CTBGetCTBVersion(void);
extern pascal short StandardNBP(Point where, ConstStr255Param prompt, short numTypes, NLType typeList, NameFilterUPP nameFilter, ZoneFilterUPP zoneFilter, DialogHookUPP hook, NBPReply *theReply);
extern pascal short CustomNBP(Point where, ConstStr255Param prompt, short numTypes, NLType typeList, NameFilterUPP nameFilter, ZoneFilterUPP zoneFilter, DialogHookUPP hook, long userData, short dialogID, ModalFilterUPP filter, NBPReply *theReply);
#if OLDROUTINENAMES
#define NuLookup(where, prompt, numTypes, typeList, nameFilter, zoneFilter, hook, theReply)  \
	StandardNBP(where, prompt, numTypes, typeList, nameFilter, zoneFilter,  \
	hook, theReply)

#define NuPLookup(where, prompt, numTypes, typeList, nameFilter, zoneFilter, hook, userData, dialogID, filter, theReply)  \
	CustomNBP(where, prompt, numTypes, typeList, nameFilter,  \
	zoneFilter, hook, userData, dialogID, filter, theReply)

#endif

#ifdef __cplusplus
}
#endif

#endif
