/*
	File:		CommResources.h

	Copyright:	© 1984-1993 by Apple Computer, Inc., all rights reserved.

	WARNING
	This file was auto generated by the interfacer tool. Modifications
	must be made to the master file.

*/

#ifndef __COMMRESOURCES__
#define __COMMRESOURCES__

#ifndef __OSUTILS__
#include <OSUtils.h>
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*		#include <MixedMode.h>									*/
/*			#include <Traps.h>									*/
#endif


/*	tool classes (also the tool file types)	*/

#define classCM 'cbnd'

#define classFT 'fbnd'

#define classTM 'tbnd'

enum  {
/*	version of the Comm Resource Manager	*/
	curCRMVersion				= 2,
/* constants general to the use of the Communications Resource Manager */
	crmType						= 9,							/* queue type	*/
	crmRecVersion				= 1,							/* version of queue structure */
/*	error codes */
	crmGenericError				= -1,
	crmNoErr					= 0
};


/* data structures general to the use of the Communications Resource Manager */

typedef OSErr CRMErr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct CRMRec {
	QElemPtr					qLink;							/*reserved*/
	short						qType;							/*queue type -- ORD(crmType) = 9*/
	short						crmVersion;						/*version of queue element data structure*/
	long						crmPrivate;						/*reserved*/
	short						crmReserved;					/*reserved*/
	long						crmDeviceType;					/*type of device, assigned by DTS*/
	long						crmDeviceID;					/*device ID; assigned when CRMInstall is called*/
	long						crmAttributes;					/*pointer to attribute block*/
	long						crmStatus;						/*status variable - device specific*/
	long						crmRefCon;						/*for device private use*/
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct CRMRec CRMRec;

typedef CRMRec *CRMRecPtr;

#ifdef __cplusplus
extern "C" {
#endif

extern pascal CRMErr InitCRM(void);
extern pascal QHdrPtr CRMGetHeader(void);
extern pascal void CRMInstall(CRMRecPtr crmReqPtr);
extern pascal OSErr CRMRemove(CRMRecPtr crmReqPtr);
extern pascal CRMRecPtr CRMSearch(CRMRecPtr crmReqPtr);
extern pascal short CRMGetCRMVersion(void);
extern pascal Handle CRMGetResource(ResType theType, short theID);
extern pascal Handle CRMGet1Resource(ResType theType, short theID);
extern pascal Handle CRMGetIndResource(ResType theType, short index);
extern pascal Handle CRMGet1IndResource(ResType theType, short index);
extern pascal Handle CRMGetNamedResource(ResType theType, ConstStr255Param name);
extern pascal Handle CRMGet1NamedResource(ResType theType, ConstStr255Param name);
extern pascal void CRMReleaseResource(Handle theHandle);
extern pascal Handle CRMGetToolResource(short procID, ResType theType, short theID);
extern pascal Handle CRMGetToolNamedResource(short procID, ResType theType, ConstStr255Param name);
extern pascal void CRMReleaseToolResource(short procID, Handle theHandle);
extern pascal long CRMGetIndex(Handle theHandle);
extern pascal short CRMLocalToRealID(ResType bundleType, short toolID, ResType theType, short localID);
extern pascal short CRMRealToLocalID(ResType bundleType, short toolID, ResType theType, short realID);
extern pascal OSErr CRMGetIndToolName(OSType bundleType, short index, Str255 toolName);
extern pascal OSErr CRMFindCommunications(short *vRefNum, long *dirID);
extern pascal Boolean CRMIsDriverOpen(ConstStr255Param driverName);
extern pascal CRMErr CRMParseCAPSResource(Handle theHandle, ResType selector, unsigned long *value);
extern pascal OSErr CRMReserveRF(short refNum);
extern pascal OSErr CRMReleaseRF(short refNum);
#ifdef __cplusplus
}
#endif

#endif
