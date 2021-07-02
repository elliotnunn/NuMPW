/*
	File:		GestaltEqu.h

	Copyright:	© 1984-1993 by Apple Computer, Inc., all rights reserved.

	WARNING
	This file was auto generated by the interfacer tool. Modifications
	must be made to the master file.

*/

#ifndef __GESTALTEQU__
#define __GESTALTEQU__

#ifndef __TYPES__
#include <Types.h>
/*	#include <ConditionalMacros.h>								*/
/*	#include <MixedMode.h>										*/
/*		#include <Traps.h>										*/
#endif

#define gestaltVersion 'vers'

#define gestaltAddressingModeAttr 'addr'

#define gestaltAliasMgrAttr 'alis'

#define gestaltAppleTalkVersion 'atlk'

#define gestaltAUXVersion 'a/ux'

#define gestaltCloseViewAttr 'BSDa'

#define gestaltConnMgrAttr 'conn'

#define gestaltComponentMgr 'cpnt'

#define gestaltTSMgrVersion 'tsmv'

#define gestaltNativeCPUtype 'cput'

#define gestaltCRMAttr 'crm '

#define gestaltCTBVersion 'ctbv'

#define gestaltDBAccessMgrAttr 'dbac'

#define gestaltDITLExtAttr 'ditl'

#define gestaltDisplayMgrAttr 'dply'

#define gestaltDisplayMgrVers 'dplv'

#define gestaltEasyAccessAttr 'easy'

enum  {
/**************************
 *    Environment Selectors
 **************************/
	gestalt32BitAddressing		= 0,							/* using 32-bit addressing mode */
	gestalt32BitSysZone			= 1,							/* 32-bit compatible system zone */
	gestalt32BitCapable			= 2,							/* Machine is 32-bit capable */
	gestaltAliasMgrPresent		= 0,							/* True if the Alias Mgr is present */
	gestaltAliasMgrSupportsRemoteAppletalk = 1,					/* True if the Alias Mgr knows about Remote Appletalk */
	gestaltCloseViewEnabled		= 0,							/* Closeview enabled (dynamic bit - returns current state) */
	gestaltCloseViewDisplayMgrFriendly = 1,						/* Closeview compatible with Display Manager (FUTURE) */
	gestaltConnMgrPresent		= 0,
	gestaltConnMgrCMSearchFix	= 1,							/* Fix to CMAddSearch?     */
	gestaltConnMgrErrorString	= 2,							/* has CMGetErrorString() */
	gestaltConnMgrMultiAsyncIO	= 3,							/* CMNewIOPB, CMDisposeIOPB, CMPBRead, CMPBWrite, CMPBIOKill */
/* Motorola Architecture */
	gestaltCPU68000				= 0x000,						/* Various 68k CPUs...	*/
	gestaltCPU68010				= 0x001,
	gestaltCPU68020				= 0x002,
	gestaltCPU68030				= 0x003,
	gestaltCPU68040				= 0x004,
/* PowerPC Architecture */
	gestaltCPU601				= 0x101,						/* IBM 601 														*/
	gestaltCPU603				= 0x103,
	gestaltCPU604				= 0x104,
	gestaltCRMPresent			= 0,
	gestaltCRMPersistentFix		= 1,							/* fix for persistent tools */
	gestaltCRMToolRsrcCalls		= 2,							/* has CRMGetToolResource/ReleaseToolResource */
	gestaltDBAccessMgrPresent	= 0,							/* True if Database Access Mgr present */
	gestaltDITLExtPresent		= 0,							/* True if calls are present */
	gestaltDisplayMgrPresent	= 0,							/* True if Display Mgr is present */
	gestaltEasyAccessOff		= 0,							/* if Easy Access present, but off (no icon) */
	gestaltEasyAccessOn			= 1								/* if Easy Access "On" */
};

#define gestaltEditionMgrAttr 'edtn'

#define gestaltAppleEventsAttr 'evnt'

#define gestaltFinderAttr 'fndr'

#define gestaltFindFolderAttr 'fold'

#define gestaltFontMgrAttr 'font'

#define gestaltFPUType 'fpu '

#define gestaltFSAttr 'fs  '

#define gestaltFXfrMgrAttr 'fxfr'

#define gestaltHardwareAttr 'hdwr'

enum  {
	gestaltEasyAccessSticky		= 2,							/* if Easy Access "Sticky" */
	gestaltEasyAccessLocked		= 3,							/* if Easy Access "Locked" */
	gestaltEditionMgrPresent	= 0,							/* True if Edition Mgr present */
	gestaltEditionMgrTranslationAware = 1,						/* True if edition manager is translation manager aware */
	gestaltAppleEventsPresent	= 0,							/* True if Apple Events present */
	gestaltScriptingSupport		= 1,
	gestaltOSLInSystem			= 2,							/* OSL is in system so don’t use the one linked in to app */
	gestaltFinderDropEvent		= 0,							/* Finder recognizes drop event */
	gestaltFinderMagicPlacement	= 1,							/* Finder supports magic icon placement */
	gestaltFinderCallsAEProcess	= 2,							/* Finder calls AEProcessAppleEvent */
	gestaltOSLCompliantFinder	= 3,							/* Finder is scriptable and recordable */
	gestaltFinderSupports4GBVolumes = 4,						/* Finder correctly handles 4GB volumes */
	gestaltFindFolderPresent	= 0,							/* True if Folder Mgr present */
	gestaltOutlineFonts			= 0,							/* True if Outline Fonts supported */
	gestaltNoFPU				= 0,							/* no FPU */
	gestalt68881				= 1,							/* 68881 FPU */
	gestalt68882				= 2,							/* 68882 FPU */
	gestalt68040FPU				= 3,							/* 68040 built-in FPU */
	gestaltFullExtFSDispatching	= 0,							/* has really cool new HFSDispatch dispatcher */
	gestaltHasFSSpecCalls		= 1,							/* has FSSpec calls    */
	gestaltHasFileSystemManager	= 2,							/* has a file system manager */
	gestaltFXfrMgrPresent		= 0,
	gestaltFXfrMgrMultiFile		= 1,							/* supports FTSend and FTReceive */
	gestaltFXfrMgrErrorString	= 2,							/* supports FTGetErrorString */
	gestaltHasVIA1				= 0,							/* VIA1 exists */
	gestaltHasVIA2				= 1,							/* VIA2 exists */
	gestaltHasASC				= 3,							/* Apple Sound Chip exists */
	gestaltHasSCC				= 4								/* SCC exists */
};

#define gestaltHelpMgrAttr 'help'

#define gestaltKeyboardType 'kbd '

#define gestaltLowMemorySize 'lmem'

#define gestaltLogicalRAMSize 'lram'

#define gestaltMiscAttr 'misc'

enum  {
	gestaltHasSCSI				= 7,							/* SCSI exists */
	gestaltHasSoftPowerOff		= 19,							/* Capable of software power off */
	gestaltHasSCSI961			= 21,							/* 53C96 SCSI controller on internal bus */
	gestaltHasSCSI962			= 22,							/* 53C96 SCSI controller on external bus */
	gestaltHasUniversalROM		= 24,							/* Do we have a Universal ROM? */
	gestaltHelpMgrPresent		= 0,							/* true if help mgr is present */
	gestaltHelpMgrExtensions	= 1,							/* true if help mgr extensions are installed */
	gestaltMacKbd				= 1,
	gestaltMacAndPad			= 2,
	gestaltMacPlusKbd			= 3,
	gestaltExtADBKbd			= 4,
	gestaltStdADBKbd			= 5,
	gestaltPrtblADBKbd			= 6,
	gestaltPrtblISOKbd			= 7,
	gestaltStdISOADBKbd			= 8,
	gestaltExtISOADBKbd			= 9,
	gestaltADBKbdII				= 10,
	gestaltADBISOKbdII			= 11,
	gestaltPwrBookADBKbd		= 12,
	gestaltPwrBookISOADBKbd		= 13,
	gestaltAppleAdjustKeypad	= 14,							/*<70>*/
	gestaltAppleAdjustADBKbd	= 15,							/*<70>*/
	gestaltAppleAdjustISOKbd	= 16,							/*<70>*/
	gestaltScrollingThrottle	= 0,							/* true if scrolling throttle on */
	gestaltSquareMenuBar		= 2								/* true if menu bar is square */
};

#define gestaltMMUType 'mmu '

#define gestaltStdNBPAttr 'nlup'

#define gestaltNotificationMgrAttr 'nmgr'

#define gestaltNuBusConnectors 'sltc'

#define gestaltNuBusSlotCount 'nubs'

#define gestaltOSAttr 'os  '

#define gestaltOSTable 'ostt'

#define gestaltToolboxTable 'tbtt'

#define gestaltExtToolboxTable 'xttt'

#define gestaltPCXAttr 'pcxg'

#define gestaltLogicalPageSize 'pgsz'

#define gestaltPowerMgrAttr 'powr'

enum  {
	gestaltNoMMU				= 0,							/* no MMU */
	gestaltAMU					= 1,							/* address management unit */
	gestalt68851				= 2,							/* 68851 PMMU */
	gestalt68030MMU				= 3,							/* 68030 built-in MMU */
	gestalt68040MMU				= 4,							/* 68040 built-in MMU */
	gestaltEMMU1				= 5,							/* Emulated MMU type 1  <SM13> */
	gestaltStdNBPPresent		= 0,
	gestaltNotificationPresent	= 0,							/* notification manager exists */
	gestaltSysZoneGrowable		= 0,							/* system heap is growable */
	gestaltLaunchCanReturn		= 1,							/* can return from launch */
	gestaltLaunchFullFileSpec	= 2,							/* can launch from full file spec */
	gestaltLaunchControl		= 3,							/* launch control support available */
	gestaltTempMemSupport		= 4,							/* temp memory support */
	gestaltRealTempMemory		= 5,							/* temp memory handles are real */
	gestaltTempMemTracked		= 6,							/* temporary memory handles are tracked */
	gestaltIPCSupport			= 7,							/* IPC support is present */
	gestaltSysDebuggerSupport	= 8,							/* system debugger support is present */
/* gestaltSkiaGlobalsSwitched is #9. I don't know why it is not here */
	gestaltBgndMouseDownSupport	= 10,							/* Background mouse down support is present */
	gestaltPCXHas8and16BitFAT	= 0,							/* PC Exchange supports both 8 and 16 bit FATs */
	gestaltPCXHasProDOS			= 1,							/* PC Exchange supports ProDOS */
	gestaltPMgrExists			= 0,
	gestaltPMgrCPUIdle			= 1,
	gestaltPMgrSCC				= 2,
	gestaltPMgrSound			= 3
};

#define gestaltPPCToolboxAttr 'ppc '

#define gestaltProcessorType 'proc'

#define gestaltParityAttr 'prty'

#define gestaltQuickdrawVersion 'qd  '

#define gestaltQuickdrawFeatures 'qdrw'

enum  {
/*
 * PPC will return the combination of following bit fields.
 * e.g. gestaltPPCSupportsRealTime +gestaltPPCSupportsIncoming + gestaltPPCSupportsOutGoing
 * indicates PPC is cuurently is only supports real time delivery
 * and both incoming and outgoing network sessions are allowed.
 * By default local real time delivery is supported as long as PPCInit has been called.*/
	gestaltPPCToolboxPresent	= 0x0000,						/* PPC Toolbox is present  Requires PPCInit to be called */
	gestaltPPCSupportsRealTime	= 0x1000,						/* PPC Supports real-time delivery */
	gestaltPPCSupportsIncoming	= 0x0001,						/* PPC will deny incoming network requests */
	gestaltPPCSupportsOutGoing	= 0x0002,						/* PPC will deny outgoing network requests */
	gestalt68000				= 1,
	gestalt68010				= 2,
	gestalt68020				= 3,
	gestalt68030				= 4,
	gestalt68040				= 5,
	gestaltHasParityCapability	= 0,							/* has ability to check parity */
	gestaltParityEnabled		= 1,							/* parity checking enabled */
	gestaltOriginalQD			= 0x000,						/* original 1-bit QD */
	gestalt8BitQD				= 0x100,						/* 8-bit color QD */
	gestalt32BitQD				= 0x200,						/* 32-bit color QD */
	gestalt32BitQD11			= 0x210,						/* 32-bit color QDv1.1 */
	gestalt32BitQD12			= 0x220,						/* 32-bit color QDv1.2 */
	gestalt32BitQD13			= 0x230,						/* 32-bit color QDv1.3 */
	gestaltHasColor				= 0,							/* color quickdraw present */
	gestaltHasDeepGWorlds		= 1,							/* GWorlds can be deeper than 1-bit */
	gestaltHasDirectPixMaps		= 2								/* PixMaps can be direct (16 or 32 bit) */
};

#define gestaltPhysicalRAMSize 'ram '

#define gestaltRBVAddr 'rbv '

#define gestaltSCCReadAddr 'sccr'

#define gestaltSCCWriteAddr 'sccw'

#define gestaltPopupAttr 'pop!'

#define gestaltResourceMgrAttr 'rsrc'

#define gestaltScrapMgrAttr 'scra'

#define gestaltScriptMgrVersion 'scri'

#define gestaltScriptCount 'scr#'

#define gestaltSerialAttr 'ser '

#define gestaltSlotAttr 'slot'

#define gestaltSlotMgrExists 0

#define gestaltNuBusPresent 1

#define gestaltSESlotPresent 2

#define gestaltSE30SlotPresent 3

#define gestaltPortableSlotPresent 4

#define gestaltFirstSlotNumber 'slt1'

#define gestaltSoundAttr 'snd '

#define gestaltSpeechAttr 'ttsc'

#define gestaltStandardFileAttr 'stdf'

#define gestaltSysArchitecture 'sysa'

#define gestaltTextEditVersion 'te  '

#define gestaltTermMgrAttr 'term'

#define gestaltTimeMgrVersion 'tmgr'

enum  {
	gestaltHasGrayishTextOr		= 3,							/* supports text mode grayishTextOr */
	gestaltPopupPresent			= 0,
	gestaltPartialRsrcs			= 0,							/* True if partial resources exist */
	gestaltScrapMgrTranslationAware = 0,						/* True if scrap manager is translation aware */
	gestaltTranslationMgrHintOrder = 1,							/* True if hint order reversal in effect */
	gestaltHasGPIaToDCDa		= 0,							/* GPIa connected to DCDa*/
	gestaltHasGPIaToRTxCa		= 1,							/* GPIa connected to RTxCa clock input*/
	gestaltHasGPIbToDCDb		= 2,							/* GPIb connected to DCDb */
	gestaltStereoCapability		= 0,							/* sound hardware has stereo capability */
	gestaltStereoMixing			= 1,							/* stereo mixing on external speaker */
	gestaltSoundIOMgrPresent	= 3,							/* The Sound I/O Manager is present */
	gestaltBuiltInSoundInput	= 4,							/* built-in Sound Input hardware is present */
	gestaltHasSoundInputDevice	= 5,							/* Sound Input device available */
	gestaltPlayAndRecord		= 6,							/* built-in hardware can play and record simultaneously */
	gestalt16BitSoundIO			= 7,							/* sound hardware can play and record 16-bit samples */
	gestaltStereoInput			= 8,							/* sound hardware can record stereo */
	gestaltLineLevelInput		= 9,							/* sound input port requires line level */
/* the following bits are not defined prior to Sound Mgr 3.0 */
	gestaltSndPlayDoubleBuffer	= 10,							/* SndPlayDoubleBuffer available, set by Sound Mgr 3.0 and later */
	gestaltMultiChannels		= 11,							/* multiple channel support, set by Sound Mgr 3.0 and later */
	gestalt16BitAudioSupport	= 12,							/* 16 bit audio data supported, set by Sound Mgr 3.0 and later */
	gestaltSpeechMgrPresent		= 0,							/* bit set indicates that Speech Manager exists */
	gestaltStandardFile58		= 0,							/* True if selectors 5-8 (StandardPutFile-CustomGetFile) are supported */
	gestaltStandardFileTranslationAware = 1,					/* True if standard file is translation manager aware */
	gestaltStandardFileHasColorIcons = 2,						/* True if standard file has 16x16 color icons */
	gestalt68k					= 1,							/* Motorola MC68k architecture										*/
	gestaltPowerPC				= 2,							/* IBM PowerPC architecture											*/
	gestaltTE1					= 1,							/* TextEdit in MacIIci ROM <8Aug89smb> */
	gestaltTE2					= 2,							/* TextEdit with 6.0.4 Script Systems on MacIIci (Script bug fixes for MacIIci) <8Aug89smb> */
	gestaltTE3					= 3,							/* TextEdit with 6.0.4 Script Systems all but MacIIci <8Aug89smb> */
	gestaltTE4					= 4,							/* TextEdit in System 7.0 */
	gestaltTE5					= 5,							/* TextWidthHook available in TextEdit */
	gestaltTermMgrPresent		= 0,
	gestaltTermMgrErrorString	= 2,
	gestaltStandardTimeMgr		= 1								/* standard time mgr is present */
};

#define gestaltVIA1Addr 'via1'

#define gestaltVIA2Addr 'via2'

#define gestaltVMAttr 'vm  '

#define gestaltTranslationAttr 'xlat'

#define gestaltDictionaryMgrAttr 'dict'

#define gestaltIconUtilitiesAttr 'icon'

#define gestaltRealtimeMgrAttr 'rtmr'

#define gestaltCompressionMgr 'icmp'

#define gestaltQuickTime 'qtim'

#define gestaltQuickTimeFeatures 'qtrs'

#define gestaltMixedModeAttr 'mixd'

#define gestaltCFMAttr 'cfrg'

#define gestaltMachineType 'mach'

enum  {
	gestaltRevisedTimeMgr		= 2,							/* revised time mgr is present */
	gestaltExtendedTimeMgr		= 3,							/* extended time mgr is present */
	gestaltVMPresent			= 0,							/* true if virtual memory is present */
	gestaltVMNotInstalled		= 0,							/* <SM3> */
	gestaltTranslationMgrExists	= 0,							/* True if translation manager exists */
	gestaltDictionaryMgrPresent	= 0,							/* Dictionary Manager attributes */
	gestaltIconUtilitiesPresent	= 0,							/* true if icon utilities are present <SM4> CSS*/
	gestaltRealtimeMgrPresent	= 0,							/* true if the Realtime manger is present <SM8> WS 	*/
	gestaltPPCQuickTimeLibPresent = 0,							/* PowerPC QuickTime glue library is present */
	gestaltPowerPCAware			= 0,							/* true if Mixed Mode supports PowerPC parameter passing conventions */
	gestaltCFMPresent			= 0,							/* true if the Code Fragment Manager is present */
/*************************
 *    Info-only selectors
 ************************/
	kMachineNameStrID			= -16395,
	gestaltClassic				= 1,
	gestaltMacXL				= 2,
	gestaltMac512KE				= 3,
	gestaltMacPlus				= 4,
	gestaltMacSE				= 5,
	gestaltMacII				= 6,
	gestaltMacIIx				= 7,
	gestaltMacIIcx				= 8,
	gestaltMacSE030				= 9,
	gestaltPortable				= 10,
	gestaltMacIIci				= 11,
	gestaltMacIIfx				= 13,
	gestaltMacClassic			= 17,
	gestaltMacIIsi				= 18,
	gestaltMacLC				= 19
};

enum  {
	gestaltQuadra900			= 20,
	gestaltPowerBook170			= 21,
	gestaltQuadra700			= 22,
	gestaltClassicII			= 23,
	gestaltPowerBook100			= 24,
	gestaltPowerBook140			= 25,
	gestaltQuadra950			= 26,
	gestaltMacLCII				= 37,
	gestaltPowerBook145			= 54
};

#define gestaltMachineIcon 'micn'

#define gestaltROMSize 'rom '

#define gestaltROMVersion 'romv'

#define gestaltSystemVersion 'sysv'

typedef pascal OSErr (*SelectorFunctionProcPtr)(OSType selector, long *response);

enum {
	uppSelectorFunctionProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(OSType)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr SelectorFunctionUPP;

#define CallSelectorFunctionProc(userRoutine, selector, response)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppSelectorFunctionProcInfo, (selector), (response))
#define NewSelectorFunctionProc(userRoutine)		\
		(SelectorFunctionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppSelectorFunctionProcInfo, GetCurrentISA())
#else
typedef SelectorFunctionProcPtr SelectorFunctionUPP;

#define CallSelectorFunctionProc(userRoutine, selector, response)		\
		(*(userRoutine))((selector), (response))
#define NewSelectorFunctionProc(userRoutine)		\
		(SelectorFunctionUPP)(userRoutine)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if SystemSevenOrLater

#if USES68KINLINES
#pragma parameter __D0 Gestalt(__D0, __A1)
#endif
extern pascal OSErr Gestalt(OSType selector, long *response)
 TWOWORDINLINE(0xA1AD, 0x2288);

#if USES68KINLINES
#pragma parameter __D0 NewGestalt(__D0, __A0)
#endif
extern pascal OSErr NewGestalt(OSType selector, SelectorFunctionUPP gestaltFunction)
 ONEWORDINLINE(0xA3AD);

#if USES68KINLINES
#pragma parameter __D0 ReplaceGestalt(__D0, __A0, __A1)
#endif
extern pascal OSErr ReplaceGestalt(OSType selector, SelectorFunctionUPP gestaltFunction, SelectorFunctionUPP *oldGestaltFunction)
 FOURWORDINLINE(0x2F09, 0xA5AD, 0x225F, 0x2288);
#else
extern pascal OSErr Gestalt(OSType selector, long *response);
extern pascal OSErr NewGestalt(OSType selector, SelectorFunctionUPP gestaltFunction);
extern pascal OSErr ReplaceGestalt(OSType selector, SelectorFunctionUPP gestaltFunction, SelectorFunctionUPP *oldGestaltFunction);
#endif

#ifdef __cplusplus
}
#endif

#endif
