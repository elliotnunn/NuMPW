/*
	File:		MoreATSUnicodeFontMenu.cp

	Description:

	Author:		AD

	Copyright:	Copyright © 1998-2000 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):
				10/14/1999	AD				Added name finder
											Fixed the !USE_NEW_TOOLBOX_CALLS instance handling.
											Variation fonts should always set style variations.
											Truncate names properly.
											Add code to find a font's intrinsic style.
				 7/27/1999	KG				Updated for Metrowerks Codewarror Pro 2.1				
				 7/01/1998	AD				Created
				

*/

#include "MoreATSUnicodeInternal.h"

#include <stddef.h>

#ifdef __MWERKS__

// includes for MetroWerks CodeWarrior

#include <Errors.h>
#include <Fonts.h>
#include <MacMemory.h>
#include <Menus.h>
#include <Script.h>
#include <StringCompare.h>

#else

#ifdef __APPLE_CC__

// includes for ProjectBuilder

#include <Carbon/Carbon.h>

#else

// includes for MPW

#include <Carbon.h>
#include <CoreServices.h>

#endif

#endif

#include "MoreATSUnicode.h"

#if !TARGET_API_MAC_CARBON
#if USE_NEW_TOOLBOX_CALLS
#define SETMENUITEMREFCON2EXISTS 1
#else
#define SETMENUITEMREFCON2EXISTS 0
#endif
#else
#define SETMENUITEMREFCON2EXISTS 0
#endif

typedef struct FontNamesRecord {
	ATSUFontID	fontID;
	ScriptCode	familyScript, styleScript;
	ItemCount	familyLength, styleLength;
	atsuInstanceIndex		instanceIndex;
	ByteOffset	styleOffset;
	char		names[ 1 ];
} FontNamesRecord, *FontNamesPtr;


#if !SETMENUITEMREFCON2EXISTS

typedef struct FontMenuItemRecord {
	short		menuID;
	short		itemNum;
	ATSUFontID	fontID;
	atsuInstanceIndex		instanceIndex;
} FontMenuItemRecord, **FontMenuItemsHandle;

#endif


/*	Given a fontID, instance index and name code for the style name, find usable and hopefully
	matching strings for the family name and style name, and build up a FontNamesRecord structure
	from them. The FontNamesRecord packs both strings one after another. */

static FontNamesPtr
BuildFontNamesRecord( ATSUFontID fontID, atsuInstanceIndex instanceIndex, FontNameCode subNameCode )
{
	OSStatus status = noErr;
	ByteCount		familyNameLength = 0, styleNameLength = 0;
	ItemCount		familyIndex, styleIndex;
	FontPlatformCode	familyPlatform = kFontNoPlatform, stylePlatform = kFontNoPlatform;
	FontScriptCode		familyScript = kFontNoScript, styleScript;
	FontLanguageCode	familyLanguage = kFontNoLanguage, styleLanguage;
	FontNamesPtr	fontNames = NULL;
	
	// Find the font's family name.
	// @@@ If you want to get font names that match the font's (QD Text) script, get its
	// corresponding QD family ID, get the script for that family, and pass it to
	// atsuFindBestFontName(). That will force it to look for a name in that script first.
	status = atsuFindBestFontName( fontID, kFontFamilyName,
									&familyPlatform, &familyScript, &familyLanguage,
									0, NULL, &familyNameLength, &familyIndex );
									
	// @@@ If there was no Mac name, give up. We should try to convert the name instead.
	if ( familyPlatform != kFontMacintoshPlatform ) return NULL;
	
	// Find the font's style name. Try to match the family name's encoding and language.
	if ( status == noErr ) {
		stylePlatform = familyPlatform;
		styleScript = familyScript;
		styleLanguage = familyLanguage;
		
		status = atsuFindBestFontName( fontID, subNameCode,
										&stylePlatform, &styleScript, &styleLanguage,
										0, NULL, &styleNameLength, &styleIndex );
		
		// @@@ If there was no Mac name, give up. We should try to convert the name instead.
		if ( stylePlatform != kFontMacintoshPlatform ) return NULL;
	}
	
	if ( status == noErr ) {
		fontNames = (FontNamesPtr) NewPtrClear( offsetof(FontNamesRecord, names) + familyNameLength + styleNameLength );
		if ( fontNames != NULL ) {
			fontNames->fontID = fontID;
			fontNames->familyScript = familyScript;
			(void) ATSUGetIndFontName( fontID, familyIndex, familyNameLength,
										 &fontNames->names[ 0 ], &fontNames->familyLength,
										 NULL, NULL, NULL, NULL );
			fontNames->styleOffset = familyNameLength;
			fontNames->styleScript = styleScript;
			(void) ATSUGetIndFontName( fontID, styleIndex, styleNameLength,
										 &fontNames->names[ familyNameLength ], &fontNames->styleLength,
										 NULL, NULL, NULL, NULL );
			
			fontNames->instanceIndex = instanceIndex;
		}
	}
	
	// Filter out invisible fonts.
	if ( fontNames->names[ 0 ] == '.' || fontNames->names[ 0 ] == '%' )
		status = 1;
	
	if ( status != noErr && fontNames != NULL ) {
		DisposePtr( (Ptr) fontNames );
		fontNames = NULL;
	}
	
	return fontNames;
}



/* The font list is sorted first by family name and then by style. */
static int
CompareFontNames( FontNamesPtr names1, FontNamesPtr names2 )
{
	int order;
	
	order = TextOrder(
				&names1->names[ 0 ], &names2->names[ 0 ],
				names1->familyLength, names2->familyLength,
				names1->familyScript, names2->familyScript,
				currentDefLang, currentDefLang );
	if ( order == 0 )
		order = TextOrder(
					&names1->names[ names1->styleOffset ], &names2->names[ names2->styleOffset ],
					names1->styleLength, names2->styleLength,
					names1->styleScript, names2->styleScript,
					currentDefLang, currentDefLang );

	return order;
}



static void
InsertFontInSortedList( FontNamesPtr fontNames, FontNamesPtr sortedFontList[], ItemCount *listLength )
{
	int	high, low, insertIndex, order;
	
	if ( fontNames ) {
		// Find the insert position. Binary search.
		low = 0; high = *listLength; insertIndex = low + (high - low) / 2;
		while ( low < high ) {
			order = CompareFontNames( fontNames, sortedFontList[ insertIndex ] );
			if ( order < 0 ) {	// new font goes before probe value
				high = insertIndex;
			} else if ( order > 0 ) { // new font goes after probe value
				low = insertIndex + 1;
			} else { // same as probe value: don't enter duplicates in list
				insertIndex = -1;
				break;
			}
			insertIndex = low + (high - low) / 2;
		}
		
		if ( insertIndex >= 0 ) {
			// Add a new record.
			BlockMoveData( &sortedFontList[ insertIndex ], &sortedFontList[ insertIndex + 1 ], (*listLength - insertIndex) * sizeof(FontNamesPtr) );
			sortedFontList[ insertIndex ] = fontNames;
			*listLength += 1;
		}
	}
}


/* Create a new menu with the given ID and title */
static MenuHandle
NewMenuName( short menuID, long length, char title[] )
{
    Str255 buf;

    buf[ 0 ] = length < 255 ? length : 255;
    BlockMove( title, &buf[ 1 ], buf[ 0 ]);
    return NewMenu( menuID, buf );
}


/* Append the given string to the menu */
static SInt16
AppendMenuName( MenuHandle menu, long length, char name[], ScriptCode script )
{
	Str255	buf;
	SInt16	itemNum;
	
	if ( length <= 255 ) {
		buf[ 0 ] = length;
	} else {
		buf[ 0 ] = 255;
		if ( CharacterByteType( name, 255, script ) > smSingleByte )	// truncate properly for multi-byte.
			--buf[ 0 ];
	}
	BlockMoveData( name, &buf[ 1 ], buf[ 0 ] );

	AppendMenu( menu, "\psome text" );	// avoid metacharacter processing
	itemNum = CountMenuItems( menu );
	SetMenuItemText( menu, itemNum, buf );
#if USE_NEW_TOOLBOX_CALLS
	(void) SetMenuItemTextEncoding( menu, itemNum, script );
#else
	SetItemCmd( menu, itemNum, (char) 0x1c );			// show item has a script
	SetItemIcon( menu, itemNum, script );				// scripts go in icon field
#endif
	
	return itemNum;
}

/*	Take the sorted font ID list and pour it into a hierarchial menu (appearance-savvy!)
	If we're using Appearance 1.1 or later, we simply store the fontID and instance as menu item refCons
	and don't bother building the adjunct font list. */

static void
FontListToHierarchialMenu( MenuHandle theMenu, short firstHierMenuID, const FontNamesPtr fontList[], ItemCount numFonts, short *oNumSubmenus, void **lookupCookie )
{
#if SETMENUITEMREFCON2EXISTS
#pragma unused (lookupCookie)
#endif

	int 					i = 0, j, k, thisItem;
	short					submenusCreated = 0;
	
#if !SETMENUITEMREFCON2EXISTS
	// Allocate the adjunct font list. This will be handed back as the "menu lookup cookie."
	FontMenuItemsHandle		fontMenuItems = NULL;
	int						recordNum = 0;
	
	if ( numFonts > 0 ) {
		fontMenuItems = (FontMenuItemsHandle) NewHandle( numFonts * sizeof(FontMenuItemRecord) );
		if ( MemError() ) return;
	}
#endif
	
	while ( i < numFonts ) {
		// Collect up the fonts with the same family name
		for ( j = i + 1; j < numFonts; j++ ) {
			int order = TextOrder(
				&fontList[ i ]->names[ 0 ], &fontList[ j ]->names[ 0 ],
				fontList[ i ]->familyLength, fontList[ j ]->familyLength,
				fontList[ i ]->familyScript, fontList[ j ]->familyScript,
				currentDefLang, currentDefLang );

			if ( order != 0 )
				break;
		}
		
		// Should the style name be added to the top level if there's only one style? How about using full name instead?
		thisItem = AppendMenuName(
					theMenu, fontList[ i ]->familyLength, &fontList[ i ]->names[ 0 ], fontList[ i ]->familyScript );

		// Add a submenu for the styles
		if ( j - i > 1 ) {
			short subID = firstHierMenuID + submenusCreated++;
			MenuHandle subMenu = NewMenuName( subID, fontList[ i ]->familyLength, &fontList[ i ]->names[ 0 ] );
			if ( subMenu != NULL ) {
				for ( k = i; k < j; k++ ) {
					int subItemNum = AppendMenuName(
							subMenu, fontList[ k ]->styleLength, &fontList[ k ]->names[ fontList[ k ]->styleOffset ], fontList[ k ]->styleScript );
					
					if ( FONT_INSTANCE_MENU_ITEM_STYLE != 0 && fontList[ k ]->instanceIndex != katsuDefaultInstance ) {
						SetItemStyle( subMenu, subItemNum, FONT_INSTANCE_MENU_ITEM_STYLE);
					}
					
					// leave a trail so we can recover the fontID.
#if SETMENUITEMREFCON2EXISTS
					(void) SetMenuItemRefCon( subMenu, subItemNum, (UInt32) fontList[ k ]->fontID );
					(void) SetMenuItemRefCon2( subMenu, subItemNum, (UInt32) fontList[ k ]->instanceIndex);
#else					
					(*fontMenuItems)[ recordNum ].fontID = fontList[ k ]->fontID;
					(*fontMenuItems)[ recordNum ].instanceIndex = fontList[ k ]->instanceIndex;
					(*fontMenuItems)[ recordNum ].menuID = subID;
					(*fontMenuItems)[ recordNum ].itemNum = subItemNum;
					recordNum += 1;
#endif
				}
				InsertMenu( subMenu, hierMenu );
#if USE_NEW_TOOLBOX_CALLS
				(void) SetMenuItemHierarchicalID( theMenu, thisItem, subID );
#else
				SetItemCmd( theMenu, thisItem, hMenuCmd );
				SetItemMark( theMenu, thisItem, subID );
#endif
			}
		} else {
			// leave a trail so we can recover the fontID.
#if SETMENUITEMREFCON2EXISTS
			(void) SetMenuItemRefCon( theMenu, thisItem, (UInt32) fontList[ i ]->fontID );
			(void) SetMenuItemRefCon2( theMenu, thisItem, (UInt32) fontList[ i ]->instanceIndex);
#else					
			(*fontMenuItems)[ recordNum ].fontID = fontList[ i ]->fontID;
			(*fontMenuItems)[ recordNum ].instanceIndex = fontList[ i ]->instanceIndex;
#if !TARGET_API_MAC_CARBON
			(*fontMenuItems)[ recordNum ].menuID = (**theMenu).menuID;
#else
			(*fontMenuItems)[ recordNum ].menuID = GetMenuID(theMenu);
#endif
			(*fontMenuItems)[ recordNum ].itemNum = thisItem;
			recordNum += 1;
#endif
		}

		i = j;	// next family name
	}
	
	if ( oNumSubmenus ) *oNumSubmenus = submenusCreated;
#if !SETMENUITEMREFCON2EXISTS
	*lookupCookie = fontMenuItems;
#endif
}


/*	Go through the font ID list and count up the total number of instances that they might have.
	If we get a "bad" font along the way, whack the font ID list so that we don't try that one again. */

static ItemCount
LookForFontInstances( ATSUFontID *fontIDList, ItemCount numFonts )
{
	OSStatus	status = noErr;
	ItemCount	numInstances = 0;
	int			i;
	
	for ( i = 0; i < numFonts; ++i ) {
		ItemCount	thisFontsInstances;
		
		status = ATSUCountFontInstances( fontIDList[ i ], &thisFontsInstances );
		if ( status == noErr )
			numInstances += thisFontsInstances;
		else
			fontIDList[ i ] = kATSUInvalidFontID;
	}
	
	return numInstances;
}


/*	Go through a font's instances, if any, and see if any of them match the given variations.
	The input array must be complete and in canonical order. Returns katsuNotAnInstance if no match. */

static atsuInstanceIndex
InstanceMatchingVariations( ATSUFontID fontID, ItemCount numVariations, const ATSUFontVariationValue valuesToMatch[] )
{
	ATSUFontVariationValue	instanceValues[ MAX_VARIATIONS ];
	atsuInstanceIndex		instanceIndex = katsuNotAnInstance;
	ItemCount				numInstances = 0;
	OSStatus				status = noErr;
	int						i, j;
	
	(void) ATSUCountFontInstances( fontID, &numInstances );
	
	// Values in an instance are always in canonical order, so we can
	// just grab the values and compare them directly. */
	for ( i = 0; i < numInstances; ++i ) {
		ItemCount	thisVariationCount;
		status = ATSUGetFontInstance( fontID, i, numVariations, NULL, instanceValues, &thisVariationCount );
		if ( thisVariationCount != numVariations )
			// something's very wrong!
			continue;
			
		instanceIndex = i;
		for ( j = 0; j < numVariations && i < MAX_VARIATIONS; ++j ) {
			if ( valuesToMatch[ j ] != instanceValues[ j ] ) {
				instanceIndex = katsuNotAnInstance;
				break;
			}
		}
		
		if ( instanceIndex != katsuNotAnInstance) break;
	}
	
	return instanceIndex;
}


/*	Find the font instance, if any, that matches the default variation values.
	Returns katsuDefaultInstance if there's no match. */

static atsuInstanceIndex
InstanceMatchingDefaultVariations( ATSUFontID fontID )
{
	ATSUFontVariationValue	defaultValues[ MAX_VARIATIONS ];
	ItemCount				numInstances = 0, numVariations = 0;
	OSStatus				status = noErr;
	int						i;
	
	(void) ATSUCountFontInstances( fontID, &numInstances );
	if ( numInstances == 0 ) return katsuDefaultInstance;
	
	(void) ATSUCountFontVariations( fontID, &numVariations );
	if ( numVariations > 0 ) {
		atsuInstanceIndex		foundInstance;
		// Get the variations in canonical order. We only need the values.
		for ( i = 0; i < numVariations && i < MAX_VARIATIONS; ++i ) {
			status = ATSUGetIndFontVariation( fontID, i, NULL, NULL, NULL, &defaultValues[ i ] );
			if ( status != noErr ) return katsuDefaultInstance;
		}
		
		foundInstance = InstanceMatchingVariations( fontID, numVariations, defaultValues );
		if ( foundInstance != katsuNotAnInstance ) return foundInstance;
	}
	
	return katsuDefaultInstance;
}


/*	Build a standard font menu. The top level consists of font family names. If there's more than
	one style for a given family, a submenu is created for those styles. Font instances are
	treated as styles. */

// @@@ If there's only one representative of a family, we should stick its full name into the
// menu instead of the bare family name. This will probably require a bit of restructuring since
// we don't know if this is the case until we've sorted the font list.

// @@@ If there are multiple fonts with the same names, we drop all but the first one encountered.
// Something more sophisticated should be done in that case.

OSStatus
atsuAppendFontMenu( MenuHandle theMenu, short firstHierMenuID, short *oSubmenuCount, void **lookupCookie )
{
	OSStatus		status = noErr;
	ItemCount		numFonts = 0, totalInstances;
	ATSUFontID		*fontIDList = NULL;
	FontNamesPtr	*sortedFontList = NULL;
	ItemCount		sortedListLength = 0;
	int				i;
	
	status = ATSUFontCount( &numFonts );
	if ( status == noErr && numFonts > 0 ) {
		fontIDList = (ATSUFontID*) NewPtrClear( numFonts * sizeof(ATSUFontID) );
		status = MemError();
		
		if ( status == noErr )
			status = ATSUGetFontIDs( fontIDList, numFonts, NULL );
		
		if ( status == noErr )
			totalInstances = LookForFontInstances( fontIDList, numFonts );
			
		if ( status == noErr ) {
			// This might be too much because of "bad" fonts, but it's guaranteed to be big enough.
			sortedFontList = (FontNamesPtr*) NewPtrClear( (numFonts + totalInstances) * sizeof(FontNamesPtr) );
			status = MemError();
		}
		
		if ( status == noErr ) {
			for ( i = 0; i < numFonts; i++ ) {
				if ( fontIDList[ i ] != kATSUInvalidFontID ) {
					ItemCount		numInstances = 0, thisInstance;
					FontNamesPtr	fontNames = NULL;
					
					// Add the font's instances to the name list, if any.
					status = ATSUCountFontInstances( fontIDList[ i ], &numInstances );
					for ( thisInstance = 0; thisInstance < numInstances; ++thisInstance ) {
						FontNameCode	instanceNameCode;
						status = ATSUGetFontInstanceNameCode( fontIDList[ i ], thisInstance, &instanceNameCode );
						if ( status == noErr ) {
							fontNames = BuildFontNamesRecord( fontIDList[ i ], thisInstance, instanceNameCode );
							if ( fontNames ) InsertFontInSortedList( fontNames, sortedFontList, &sortedListLength );
						}
					}
					
					// Add the font itself. If it happens to have the same name as an instance,
					// it'll be skipped until we figure out what to do about name clashes.
					if ( InstanceMatchingDefaultVariations( fontIDList[ i ] ) == katsuDefaultInstance ) {
						fontNames = BuildFontNamesRecord( fontIDList[ i ], katsuDefaultInstance, kFontStyleName );
						if ( fontNames ) InsertFontInSortedList( fontNames, sortedFontList, &sortedListLength );
					}
				}
			}
			
			FontListToHierarchialMenu( theMenu, firstHierMenuID, sortedFontList, sortedListLength, oSubmenuCount, lookupCookie );
		}
	}
	
	if ( fontIDList ) DisposePtr( (Ptr) fontIDList );
	if ( sortedFontList ) {
		for ( i = 0; i < sortedListLength; i++ )
			DisposePtr( (Ptr) sortedFontList[ i ] );
		DisposePtr( (Ptr) sortedFontList );
	}
	
	return status;
}


ATSUFontID
atsuGetFontForFontMenuItem( short menuID, short itemNum, void *menuLookupCookie, atsuInstanceIndex *fontInstance )
{
#if SETMENUITEMREFCON2EXISTS
#pragma unused( menuLookupCookie )

	// The font ID is stored as the menu item's refCon. Isn't that handy?
	MenuHandle	theMenu = GetMenuHandle( menuID );
	
	if ( theMenu != NULL ) {
		UInt32	refCon, refCon2;
		OSStatus status = GetMenuItemRefCon( theMenu, itemNum, &refCon );
		if ( status == noErr && fontInstance != NULL ) 
			status = GetMenuItemRefCon2( theMenu, itemNum, &refCon2 );
		if ( status == noErr ) {
			*fontInstance = refCon2;
			return (ATSUFontID) refCon;
		}
	}
	
	return kATSUInvalidFontID;
	
# else // SETMENUITEMREFCON2EXISTS
	// Have to search for it in the side data structure we built.
	
	FontMenuItemsHandle		fontMenuItems = (FontMenuItemsHandle) menuLookupCookie;
	short					numEntries = GetHandleSize( (Handle) fontMenuItems ) / sizeof(FontMenuItemRecord);

#if 0
// @@@ If we sorted the item list we could do a binary search.
	short					low = 0, high = numEntries, probe = low + (high - low) / 2;
	
	while ( low < high ) {
		if ( menuID < (*fontMenuItems)[ probe ].menuID )
			high = probe;
		else if ( menuID > (*fontMenuItems)[ probe ].menuID )
			low = probe + 1;
		else if ( itemNum < (*fontMenuItems)[ probe ].itemNum )
			high = probe;
		else if ( itemNum > (*fontMenuItems)[ probe ].itemNum )
			low = probe + 1;
		else
			// found it! @@@ fill in the instance index, too!
			return (*fontMenuItems)[ probe ].fontID;

		probe = low + (high - low) / 2;
	}
	
#else

	// brute-force search.
	short					i;
	
	for ( i = 0; i < numEntries; ++i ) {
		if ( menuID == (*fontMenuItems)[ i ].menuID && itemNum == (*fontMenuItems)[ i ].itemNum ) {
			if ( fontInstance != NULL ) *fontInstance = (*fontMenuItems)[ i ].instanceIndex;
			return (*fontMenuItems)[ i ].fontID;
		}
	}
	
#endif

	// If we got here, we didn't find it.
	return kATSUInvalidFontID;

#endif	// if SETMENUITEMREFCON2EXISTS
}


OSStatus
atsuDisposeFontMenuLookupCookie( void *menuLookupCookie )
{
	if ( menuLookupCookie ) {
		DisposeHandle( (Handle) menuLookupCookie );
		return MemError();
	} else {
		return noErr;
	}
}



// @@@ We could use a private tag to record the actual instance index in the style to make retrieving
// it later simpler. We couldn't avoid the complex match if we wanted to match an set of variation
// settings that happened to match an instance, though -- the instance search would have to be done
// every time a variation was set. If you find yourself using the getter more than the setter,
// you should consider doing that.

OSStatus
atsuSetStyleFontInstance( ATSUStyle theStyle, ATSUFontID fontID, atsuInstanceIndex instanceIndex )
{
	OSStatus	status = noErr;
	ATSUFontVariationAxis	axes[ MAX_VARIATIONS ];
	ATSUFontVariationValue	values[ MAX_VARIATIONS ];
	ItemCount				numVariations;
	
	if ( instanceIndex == katsuDefaultInstance ) {
		(void) ATSUClearFontVariations( theStyle, kATSUClearAll, NULL );
		
		// If the font has variations, put the default values into the style.
		// This helps the glyph cache out a bit and makes dealing with variation fonts
		// more uniform for our callers.
		status = ATSUCountFontVariations( fontID, &numVariations );
		if ( status == noErr && 0 < numVariations && numVariations < MAX_VARIATIONS ) {
			int i;
			for ( i = 0; i < numVariations; ++i ) {
				status = ATSUGetIndFontVariation( fontID, i, &axes[ i ], NULL, NULL, &values[ i ]);
				if ( status != noErr ) return status;
			}
			status = ATSUSetVariations( theStyle, numVariations, axes, values );
		}
	} else if ( instanceIndex >= 0 ) {
		status = ATSUGetFontInstance( fontID, instanceIndex, MAX_VARIATIONS, axes, values, &numVariations );

		if ( status == noErr && numVariations < MAX_VARIATIONS) {
			(void) ATSUClearFontVariations( theStyle, kATSUClearAll, NULL );
			status = ATSUSetVariations( theStyle, numVariations, axes, values);
		}
	}
	
	if ( status == noErr ) status = atsuSetFont( theStyle, fontID );
	
	return status;
}


// @@@ This ignores variation settings that don't apply to the font. A stricter version would
// consider those, too.
// @@@ Need more error checking.
// @@@ Consider adding a tolerance for the matching so that exact matches aren't necessary.

OSStatus
atsuGetStyleFontInstance( ATSUStyle theStyle, ATSUFontID* oFontID, atsuInstanceIndex* oInstanceIndex )
{
	ATSUFontID			theFont = kATSUInvalidFontID;
	atsuInstanceIndex	instance;
	OSStatus			status = noErr;
	ItemCount			numInstances = 0, numFontVariations = 0;
	int					i;
	
	// Get our hands on the font and some counts that we'll need.
	status = atsuGetFont( theStyle, &theFont );
	if ( theFont != kATSUInvalidFontID ) {
		(void) ATSUCountFontVariations( theFont, &numFontVariations );
		(void) ATSUCountFontInstances( theFont, &numInstances );
	}
	
	if ( numFontVariations > 0 ) {
		ATSUFontVariationAxis	axes[ MAX_VARIATIONS ];
		ATSUFontVariationValue	styleValues[ MAX_VARIATIONS ];
		ItemCount				styleVariationCount = 0;
		Boolean					allDefaultValues = true;
		
		// First get the font's variation axes and default values. This will also give us
		// the canonical order for the variations.
		for ( i = 0; i < numFontVariations && i < MAX_VARIATIONS; ++i ) {
			(void) ATSUGetIndFontVariation( theFont, i, &axes[i], NULL, NULL, &styleValues[i] );
		}
		
		// Now fill in whatever's been set in the style. We also note whether or not all the
		// settings are defaults for the last check we have to make.
		// Don't bother with this loop if no variations have been set in the style.
		(void) atsuCountStyleFontVariations( theStyle, &styleVariationCount );
		if ( styleVariationCount > 0 ) {
			for ( i = 0; i < numFontVariations; ++i ) {
				ATSUFontVariationValue	theValue;
				OSStatus localStatus = ATSUGetFontVariationValue( theStyle, axes[i], &theValue );
				if ( localStatus == noErr && theValue != styleValues[i] ) {	// we treat *any* error as if the value's not been set.
					styleValues[i] = theValue;
					allDefaultValues = false;
				}
			}
		}
		
		// Do the search.
		instance = InstanceMatchingVariations( theFont, numFontVariations, styleValues );
		
		// One last thing to test for: the current settings may match the defaults, for which there
		// may not have been a named instance.
		if ( instance == katsuNotAnInstance && allDefaultValues ) {
			instance = katsuDefaultInstance;
		}
	} else {
		instance = katsuDefaultInstance;
	}
	
	if ( oFontID ) *oFontID = theFont;
	if ( oInstanceIndex ) *oInstanceIndex = instance;
	
	return status;
}

typedef struct FontNameEncoding {
	FontPlatformCode	platform;
	FontScriptCode		script;
	FontLanguageCode	language;
} FontNameEncoding;

// Note that this evaluates pref twice.
#define ElementMatches( elem, pref ) \
	((pref) == -1 || (elem) == (pref))

#define EncodingMatchesPrefs( enc, prefs ) \
	( ElementMatches((enc).platform, (prefs).platform) \
	&& ElementMatches((enc).script, (prefs).script) \
	&& ElementMatches((enc).language, (prefs).language) )

/* Determines if the first encoding is a better choice than the second one.
   If you want to change the priority of font name encodings, this is the
   place to do it: atsuFindBestName() itself shouldn't need to change when your heuristics do.
 */
 
static Boolean
BetterEncoding( const FontNameEncoding *newName, const FontNameEncoding *oldName, const FontNameEncoding *namePrefs )
{
	Boolean	checkPlatform, checkScript, checkLanguage;
	
	if ( EncodingMatchesPrefs( *newName, *namePrefs ) && EncodingMatchesPrefs( *oldName, *namePrefs ) ) {
		// If both names match the prefs, figure out which elements need to be compared
		checkPlatform = namePrefs->platform == kFontNoPlatform;
		checkScript = namePrefs->script == kFontNoScript;
		checkLanguage = namePrefs->language == kFontNoLanguage;
	} else if ( EncodingMatchesPrefs( *oldName, *namePrefs ) ) {
		// If only one name matches the prefs, we like that one better
		return false;
	} else if ( EncodingMatchesPrefs( *newName, *namePrefs ) ) {
		return true;
	} else {
		// If neither matches the prefs, we'll have to compare everything.
		checkPlatform = checkScript = checkLanguage = true;
	}
	
	// If we've gotten this far, we have to compare individual elements.
	// @@@ Should language have priority over platform/script???
	
	if ( checkPlatform && newName->platform != oldName->platform ) {
		if ( oldName->platform == kFontMacintoshPlatform )
			// We like Macintosh names best
			return false;
		else if ( newName->platform == kFontMacintoshPlatform )
			return true;
		else if ( oldName->platform == kFontUnicodePlatform )
			// Unicode names are next best
			return false;
		else if ( newName->platform == kFontUnicodePlatform )
			return true;
		// If it's neither, we don't really care.
	}
	
	// Script is only meaningful w/r to a particular platform, so we can only compare them
	// if the platforms are the same.
	if ( checkScript && newName->platform == oldName->platform && newName->script != oldName->script ) {
		if ( newName->platform == kFontMacintoshPlatform ) {
			// For Macintosh scripts, we use the system script order.
			// Since we know the scripts are different, we can return directly.
			return( ScriptOrder( newName->script, oldName->script ) < 0);
		}
		// No preference otherwise.
	}
	
	// Font language codes are (so far) identical to the standard Mac language codes,
	// so we can just use LanguageOrder to determine a preference regardless of the encoding.
	if ( checkLanguage && newName->language != oldName->language ) {
		// Since the languages are different, we can return directly.
		return( LanguageOrder( newName->language, oldName->language ) < 0 );
	}
	
	// If we have no reason to prefer one over the other, return the earlier one.
	return false;
}


/*	Given a particular font and name code, and a preference for platform, script and language
	(any may be "don't care" values), find the best available for the current system. 
*/

OSStatus
atsuFindBestFontName( ATSUFontID theFont,
					  FontNameCode iNameCode,
					  FontPlatformCode *ioPlatform,
					  FontScriptCode *ioScript,
					  FontLanguageCode *ioLanguage,
					  ByteCount iMaxNameLength,
					  Ptr oName,
					  ByteCount *oNameLength,
					  ItemCount *oNameIndex )
{

	OSStatus status = -1;
	FontNameEncoding namePrefs = { *ioPlatform, *ioScript, *ioLanguage };
	
	// Script is meaningless without a platform, so make sure one is provided if needed.
	nrequire( *ioScript != kFontNoScript && *ioPlatform == kFontNoPlatform, PARAM_ERR );
	
	// @@@ It would be nice if there was something like ATSUValidateFontID() here.
	
	// If a specific name was requested, try for that one first.
	if ( *ioPlatform != kFontNoPlatform && *ioScript != kFontNoScript && *ioLanguage != kFontNoLanguage ) {
		status = ATSUFindFontName( theFont, iNameCode, *ioPlatform, *ioScript, *ioLanguage,
								   iMaxNameLength, oName, oNameLength, oNameIndex );
	}
	
	// If we didn't find that one (or didn't look for it), examine the name table in detail.
	if ( status != noErr ) {
		FontNameEncoding bestInfo;
		ItemCount bestIndex = -1, nextIndex, numNames;
		
		// First see if there are any names at all with the given code.
		// Remember the info for that name - it'll be our first guess.
		status = ATSUFindFontName( theFont, iNameCode, kFontNoPlatform, kFontNoScript, kFontNoLanguage,
								   0, NULL, NULL, &bestIndex );
		if ( status == noErr ) {
			nextIndex = bestIndex + 1;
			status = ATSUGetIndFontName( theFont, bestIndex, 0, NULL, NULL,
										NULL, &bestInfo.platform, &bestInfo.script, &bestInfo.language );
		}
		
		// Now go through the rest of the names comparing them with the best one so far.
		// The above probe is guaranteed to give us the first possible name, so we can start
		// looking starting with the next index.
		status = ATSUCountFontNames( theFont, &numNames );
		while ( status == noErr && nextIndex < numNames ) {
			FontNameEncoding nextInfo;
			FontNameCode nextCode;
			status = ATSUGetIndFontName( theFont, nextIndex, 0, NULL, NULL,
										&nextCode, &nextInfo.platform, &nextInfo.script, &nextInfo.language );
			if ( status == noErr && nextCode == iNameCode && BetterEncoding( &nextInfo, &bestInfo, &namePrefs ) ) {
				bestIndex = nextIndex;
				bestInfo = nextInfo;
			}
			
			++nextIndex;
		}
		
		if ( status == noErr ) {
			// The best name is in our candidate slot:
			*oNameIndex = bestIndex;
			*ioPlatform = bestInfo.platform;
			*ioScript = bestInfo.script;
			*ioLanguage = bestInfo.language;
			if ( oName || oNameLength )
				status = ATSUGetIndFontName( theFont, bestIndex,
											iMaxNameLength, oName, oNameLength,
											NULL, NULL, NULL, NULL );
		}
	}
		
	return status;

PARAM_ERR:
	return paramErr;
}


/*
	This is just like ATSUFONDtoFontID except that it also returns the intrinsic
	style of the selected font. This information is needed to correctly adjust the
	QD style bits. See the implementation of atsuSetStyleFromGrafPtr for an example.
	
	NB: On Mac OS 9 or later, just call through to FMGetFontFromFontFamilyInstance,
	which is the preferred function.
*/

OSStatus
atsuFONDtoFontID( short 			iFONDNumber,
				  StyleParameter	iFONDStyle,
				  ATSUFontID *		oFontID,
			 	  StyleParameter *	oIntrinsicStyle )
{
	OSStatus status;

//#if (ATSU_TARG_VERSION >= ATSU_1_2)
	// On Mac OS 9 or later, just call through to the Font Mgr.
	// Check to see if the function truly exists, in case we are on Mac OS 8.6 with CarbonLib
	if ( (Ptr) FMGetFontFromFontFamilyInstance != NULL )
		return FMGetFontFromFontFamilyInstance( iFONDNumber, iFONDStyle, oFontID, oIntrinsicStyle );
	else
	{
//#else
		if ( oIntrinsicStyle ) *oIntrinsicStyle = normal; // need some sort of fallback.
	
		status = ATSUFONDtoFontID( iFONDNumber, iFONDStyle, oFontID );
		if ( oIntrinsicStyle && (status == noErr || status == kATSUNoCorrespondingFontErr) ) {
			FMInput		fmInput = {	iFONDNumber, 1, iFONDStyle, false, 0, {1, 1}, {1, 1} };
			FMOutPtr	fmOutput;

			fmOutput = FMSwapFont( &fmInput );
			if ( fmOutput->errNum == noErr ) {
				// curStyle gives the left-over style bits.
				*oIntrinsicStyle = iFONDStyle & ~fmOutput->curStyle;
			}

			status = noErr;
		}
	}
	
	return status;
//#endif	
}