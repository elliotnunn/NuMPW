/*
	File:		PPCGlue.c

	Contains:	Glue necessary to call AppendDialogItemList.

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

#include "AppearanceSamplePrefix.h"

#include "AppearanceHelpers.h"

#define RESULT_OFFSET(type) ((sizeof(type) == 1) ? 3 : ((sizeof(type) == 2) ? 1 : 0))

pascal OSErr
AppendDialogItemList( DialogPtr dialog, SInt16 ditlID, DITLMethod method )
{
	long	private_result;
	
	private_result = CallUniversalProc((UniversalProcPtr)GetToolTrapAddress(0xAA68),
		kD0DispatchedPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kTwoByteCode)
		 | DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(dialog)))
		 | DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ditlID)))
		 | DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(method))),
		0x0412,
		dialog,
		ditlID,
		method );
		
	return *(((OSErr*)&private_result) + RESULT_OFFSET(OSErr));
}