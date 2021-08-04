package main

import (
	"embed"
	"flag"
	"fmt"
	"io/fs"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

//#######################################################################
// The Macintosh Toolbox. Wish me luck...
//#######################################################################

// Trap Manager OS traps

// For historical reasons, unflagged Get/SetTrapAddress need to guess between OS/TB traps
func trapTableEntry(requested_trap uint16, requesting_trap uint16) uint32 {
	if requesting_trap&0x200 != 0 { // newOS/newTool trap
		if requesting_trap&0x400 != 0 {
			return kToolTable + 4*uint32(requested_trap&0x3ff)
		} else {
			return kOSTable + 4*uint32(requested_trap&0xff)
		}
	} else { // guess, using traditional trap numbering
		requested_trap &= 0x1ff // 64k ROM had a single 512-entry trap table
		if requested_trap <= 0x4f || requested_trap == 0x54 || requested_trap == 0x57 {
			return kOSTable + 4*uint32(requested_trap)
		} else {
			return kToolTable + 4*uint32(requested_trap)
		}
	}
}

func tGetTrapAddress() {
	tableEntry := trapTableEntry(readw(d0ptr+2), readw(d1ptr+2))
	writel(a0ptr, readl(tableEntry))
}

func tSetTrapAddress() {
	tableEntry := trapTableEntry(readw(d0ptr+2), readw(d1ptr+2))
	writel(tableEntry, readl(a0ptr))
}

// Segment Loader Toolbox traps

var gSegmentOffsets []uint32
var gSegmentNames []string

func tLoadSeg() {
	save := readRegs()

	segNum := popw()
	pushl(0)
	pushl(0x434f4445) // CODE
	pushw(segNum)
	call_m68k(executable_atrap(0xada0)) // _GetResource ,autoPop
	segPtr := readl(popl())

	gSegmentOffsets = append(gSegmentOffsets, segPtr)
	gSegmentNames = append(gSegmentNames, fmt.Sprintf("seg%d", segNum))

	offset := uint32(readw(segPtr))    // offset of first entry within jump table
	count := uint32(readw(segPtr + 2)) // number of jump table entries

	for i := uint32(0); i < count; i++ {
		jtEntry := readl(a5ptr) + 0x20 + offset + 8*i

		offsetInSegment := readw(jtEntry)
		writew(jtEntry, segNum)
		writew(jtEntry+2, 0x4ef9) // jmp
		writel(jtEntry+4, segPtr+4+uint32(offsetInSegment))
	}

	pc -= 6

	writeRegs(save, a0ptr, a1ptr, d0ptr, d1ptr, d2ptr) // ??? really need to do this?
}

// Package Toolbox traps

// QuickDraw Toolbox traps
func tBitAnd() {
	result := popl() & popl()
	writel(readl(spptr), result)
}
func tBitXor() {
	result := popl() ^ popl()
	writel(readl(spptr), result)
}

func tBitNot() {
	result := ^popl()
	writel(readl(spptr), result)
}

func tBitOr() {
	result := popl() | popl()
	writel(readl(spptr), result)
}

func tBitShift() {
	by := int16(popw())
	result := popl()
	if by > 0 {
		result <<= by % 32
	} else {
		result >>= (-by) % 32
	}
	writel(readl(spptr), result)
}

func tBitTst() {
	bitNum := popl()
	bytePtr := popl()
	bytePtr += bitNum / 8
	bitNum %= 8
	result := readb(bytePtr)&(0x80>>bitNum) != 0

	if result {
		writew(readl(spptr), 0x0100) // bool true
	} else {
		writew(readl(spptr), 0)
	}
}

func tBitSet() {
	bitNum := popl()
	bytePtr := popl()
	bytePtr += bitNum / 8
	bitNum %= 8
	writeb(bytePtr, readb(bytePtr)|(0x80>>bitNum))
}

func tBitClr() {
	bitNum := popl()
	bytePtr := popl()
	bytePtr += bitNum / 8
	bitNum %= 8
	writeb(bytePtr, readb(bytePtr) & ^(0x80>>bitNum))
}

// func tRandom() {
//     writew(readl(spptr), random.randint(0x10000))
// }

func tHiWord() {
	val := popw()
	popw()
	writew(readl(spptr), val)
}

func tLoWord() {
	popw()
	val := popw()
	writew(readl(spptr), val)
}

func tInitGraf() {
	a5 := readl(a5ptr)
	qd := popl()
	writel(a5, qd)
	writel(qd, 0xf8f8f8f8) // illegal thePort address
}

func tSetPort() {
	a5 := readl(a5ptr)
	qd := readl(a5)
	port := popl()
	writel(qd, port)
}

func tGetPort() {
	a5 := readl(a5ptr)
	qd := readl(a5)
	port := readl(qd)
	retaddr := popl()
	writel(retaddr, port)
}

func tSetRect() {
	botRight := popl()
	topLeft := popl()
	rectPtr := popl()
	writel(rectPtr, topLeft)
	writel(rectPtr+4, botRight)
}

func tOffsetRect() {
	dv := popw()
	dh := popw()
	rectPtr := popl()

	writew(rectPtr+0, readw(rectPtr+0)+dv) // top
	writew(rectPtr+2, readw(rectPtr+2)+dh) // left
	writew(rectPtr+4, readw(rectPtr+4)+dv) // bottom
	writew(rectPtr+6, readw(rectPtr+6)+dh) // right
}

// OS and Toolbox Event Manager traps

// Misc traps

func tDebugStr() {
	string := readPstring(popl())
	fmt.Println(macToUnicode(string))
}

// trap is in toolbox table but actually uses registers
func tSysError() {
	err := int16(readw(d0ptr + 2))
	panicstr := fmt.Sprintf("_SysError %d", err)
	if err == -491 { // display string on stack
		panicstr += macToUnicode(readPstring(popl()))
	}
	panic(panicstr)
}

func tSysEnvirons() {
	block := readl(a0ptr)
	write(16, block, 0)     // wipe
	writew(block, 2)        // environsVersion = 2
	writew(block+2, 3)      // machineType = SE
	writew(block+4, 0x0700) // systemVersion = seven
	writew(block+6, 3)      // processor = 68020
	writel(d0ptr, 0)        // noErr
}

func tGestalt() {
	trap := readw(d1ptr + 2)
	selector := string(mem[d0ptr:][:4])

	if trap&0x600 == 0 { // ab=00
		var reply uint32
		var err int
		switch selector {
		case "sysv":
			reply = 0x09228000 // highest possible
		case "fs  ":
			reply = 2 // FSSpec calls, not much else
		case "fold":
			reply = 1 // Folder Manager present
		default:
			err = -5551 // gestaltUndefSelectorErr
		}

		writel(a0ptr, reply)
		writel(d0ptr, uint32(err))

	} else if trap&0x600 == 0x200 { // ab=01
		panic("NewGestalt unimplemented")

	} else if trap&0x600 == 0x400 { // ab=10
		panic("ReplaceGestalt unimplemented")

	} else { // ab=11
		panic("GetGestaltProcPtr unimplemented")
	}
}

func tAliasDispatch() {
	if readl(d0ptr) == 0 { // FindFolder
		foundDirID := popl()
		foundVRefNum := popl()
		popb() // ignore createFolder
		folderType := string(mem[readl(spptr):][:4])
		popl()
		popw() // ignore vRefNum

		var path string
		switch folderType {
		case "pref", "sprf":
			path = filepath.Join(systemFolder, "Preferences")
		case "desk", "sdsk":
			path = filepath.Join(systemFolder, "Desktop Folder")
		case "trsh", "strs", "empt":
			path = filepath.Join(systemFolder, "Trash")
		case "temp":
			path = filepath.Join(systemFolder, "Temporary Items")
		default:
			path = systemFolder
		}

		writew(foundVRefNum, 2)
		writel(foundDirID, uint32(get_macos_dnum(path)))
		writew(readl(spptr), 0) // noErr
	} else {
		panic("Unimplemented _AliasDispatch selector")
	}
}

func tCmpString() {
	aptr := readl(a0ptr)
	bptr := readl(a1ptr)
	alen := readw(d0ptr)
	blen := readw(d0ptr + 2)
	diacSens := readw(d1ptr)&0x200 == 0 // ,MARKS
	caseSens := readw(d1ptr)&0x400 != 0 // ,CASE

	if gDebug >= 2 {
		fmt.Printf("compare %s %s = ", mem[aptr:][:alen], mem[bptr:][:blen])
	}

	if relString(macstring(mem[aptr:][:alen]), macstring(mem[bptr:][:blen]), caseSens, diacSens) == 0 {
		writel(d0ptr, 0)
	} else {
		writel(d0ptr, 1)
	}

	if gDebug >= 2 {
		fmt.Printf("%d\n", readl(d0ptr))
	}
}

func tGetOSEvent() {
	write(16, readl(a0ptr), 0) // null event
	writel(d0ptr, 0xffffffff)
}

// ToolServer-specific code
func tMenuKey() {
	keycode := popw() & 0xff
	if keycode == 12 { // cmd-Q
		writel(readl(spptr), 0x00810005) // quit menu item, hardcoded sadly
		return
	}
	writel(readl(spptr), 0) // return nothing
}

func tGetNextEvent() {
	eventrecord := popl()
	write(16, eventrecord, 0)
	mask := popw()

	// ToolServer-specific code: if accepting high-level events then do cmd-Q
	if mask&0x400 != 0 {
		writew(eventrecord, 3)        // keyDown
		writel(eventrecord+2, 12)     // Q
		writew(eventrecord+14, 0x100) // command
		writew(readl(spptr), 0xffff)  // return true
		return

	}
	writew(readl(spptr), 0) // return false
}

func tWaitNextEvent() {
	pop(8) // ignore sleep and mouseRgn args
	tGetNextEvent()
}

func tExitToShell() {
	panic("ExitToShell")

}

// Trivial do-nothing traps

func tUnimplemented() {
	panic("Unimplemented trap")
}

func tNop() {
}

func tClrD0() {
	writel(d0ptr, 0)
}

func tClrD0A0() {
	writel(d0ptr, 0)
	writel(a0ptr, 0)
}

func tPop2() {
	writel(spptr, readl(spptr)+2)
}

func tPop4() {
	writel(spptr, readl(spptr)+4)
}

func tPop6() {
	writel(spptr, readl(spptr)+6)
}

func tPop8() {
	writel(spptr, readl(spptr)+8)
}

func tPop10() {
	writel(spptr, readl(spptr)+10)
}

func tRetZero() {
	writel(readl(spptr), 0)
}

func tPop2RetZero() {
	sp := readl(spptr) + 2
	writel(spptr, sp)
	writel(sp, 0)
}

func tPop4RetZero() {
	sp := readl(spptr) + 4
	writel(spptr, sp)
	writel(sp, 0)
}

func tPop6RetZero() {
	sp := readl(spptr) + 6
	writel(spptr, sp)
	writel(sp, 0)
}

func tPop8RetZero() {
	sp := readl(spptr) + 8
	writel(spptr, sp)
	writel(sp, 0)
}

func tPop10RetZero() {
	sp := readl(spptr) + 10
	writel(spptr, sp)
	writel(sp, 0)
}

func tParamBlkNop() {
	pb := readl(a0ptr)
	writew(pb+16, 0)
	writel(d0ptr, 0)
}

const os_base = 0
const tb_base = 0x100

var my_traps [0x500]func()

func lineA(inst uint16) {
	if inst&0x800 != 0 { // Toolbox trap
		// Push a return address unless autoPop is used
		if inst&0x400 == 0 {
			pushl(pc)
		}

		pc = readl(kToolTable + 4*(uint32(inst)&0x3ff))
	} else { // OS trap
		writew(d1ptr+2, inst)

		pushl(readl(a2ptr))
		pushl(readl(d2ptr))
		pushl(readl(d1ptr))
		pushl(readl(a1ptr))
		if inst&0x100 == 0 {
			pushl(readl(a0ptr))
		}

		call_m68k(readl(kOSTable + 4*(uint32(inst)&0xff)))

		if inst&0x100 == 0 {
			writel(a0ptr, popl())
		}
		writel(a1ptr, popl())
		writel(d1ptr, popl())
		writel(d2ptr, popl())
		writel(a2ptr, popl())
	}
}

var gCurToolTrapNum int

func lineF(inst uint16) {
	pc = popl()
	check_for_lurkers()
	if inst&0x800 != 0 { // Go implementation of Toolbox trap
		gCurToolTrapNum = int(inst & 0x3ff)
		my_traps[tb_base+(inst&0x3ff)]()
	} else { // Go implementation of OS trap
		my_traps[os_base+(inst&0xff)]()
	}
}

// Set up the Toolbox and launch ToolServer

const (
	kOSTable        = 0x400
	kToolTable      = 0xe00   // up to 0x1e00
	kStackBase      = 0x40000 // extends down, note that registers are here too!
	kA5World        = 0x58000 // 0x8000 below and 0x8000 above, so 5xxxx is in A5 world
	kFakeHeapHeader = 0x90000 // very short
	kATrapTable     = 0xa0000 // 0x10000 above
	kFCBTable       = 0xb0000 // 0x8000 above
	kDQE            = 0xb9000 // 0x4 below and 0x10 above
	kVCB            = 0xba000 // ????
	kMenuList       = 0xbb000
	kPACKs          = 0xc0000
	kFTrapTable     = 0xf0000  // 0x10000 above
	kHeap           = 0x100000 // extends up
)

//go:embed "PACKs"
var embedPACKs embed.FS

func check_for_lurkers() {
	// we might do more involved things here, like check for heap corruption
	write(64, 0, 0)
}

// Get an address to jump to
func executable_atrap(trap uint16) (addr uint32) {
	addr = kATrapTable + (uint32(trap)&0xfff)*16

	writew(addr, trap)     // consider using autoPop instead?
	writew(addr+2, 0x4e75) // RTS

	return
}

// Get an address to jump to
func executable_ftrap(trap uint16) (addr uint32) {
	addr = kFTrapTable + (uint32(trap)&0xfff)*16

	writew(addr, trap)

	return
}

// Conservatively convert words in a string that look like Unix paths
func cmdPathCvt(s string) string {
	words := strings.Split(s, " ")

	for i := range words {
		if strings.HasPrefix(words[i], "/") {
			ok := true
			for _, c := range words[i] {
				if !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'A') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '/') {
					ok = false
					break
				}
				if ok {
					words[i] = pathCvt(words[i])
				}
			}
		}
	}

	return strings.Join(words, " ")
}

// Convert a Unix to a Mac path
func pathCvt(s string) string {
	ss := []byte(s)
	for i := range ss {
		if ss[i] == ':' {
			ss[i] = '/'
		} else if ss[i] == '/' {
			ss[i] = ':'
		}
	}
	return string(onlyvolname) + string(ss)
}

// Quote a string for MPW (but leave it in unicode)
func quote(s string) string {
	return "'" + s + "'" // TODO: quote correctly
}

var gDebug int

//go:embed "ToolServer 1.1.1/*"
var embedMPW embed.FS

var gFS UnionFS // use this for ReadFile, etc

func main() {
	my_traps = [...]func(){
		os_base + 0x00:  tOpen,                // _Open
		os_base + 0x01:  tClose,               // _Close
		os_base + 0x02:  tReadWrite,           // _Read
		os_base + 0x03:  tReadWrite,           // _Write
		os_base + 0x07:  tGetVInfo,            // _GetVInfo
		os_base + 0x08:  tCreate,              // _Create
		os_base + 0x09:  tDelete,              // _Delete
		os_base + 0x0a:  tOpen,                // _OpenRF
		os_base + 0x0c:  tGetFInfo,            // _GetFInfo
		os_base + 0x0d:  tSetFInfo,            // _SetFInfo
		os_base + 0x11:  tGetEOF,              // _GetEOF
		os_base + 0x12:  tSetEOF,              // _SetEOF
		os_base + 0x13:  tParamBlkNop,         // _FlushVol
		os_base + 0x14:  tGetVol,              // _GetVol
		os_base + 0x15:  tSetVol,              // _SetVol
		os_base + 0x18:  tGetFPos,             // _GetFPos
		os_base + 0x1a:  tGetZone,             // _GetZone
		os_base + 0x1b:  tClrD0A0,             // _SetZone
		os_base + 0x1c:  tFreeMem,             // _FreeMem
		os_base + 0x1d:  tFreeMem,             // _MaxMem
		os_base + 0x1e:  tNewPtr,              // _NewPtr
		os_base + 0x1f:  tDisposPtr,           // _DisposPtr
		os_base + 0x20:  tSetPtrSize,          // _SetPtrSize
		os_base + 0x21:  tGetPtrSize,          // _GetPtrSize
		os_base + 0x22:  tNewHandle,           // _NewHandle
		os_base + 0x23:  tDisposHandle,        // _DisposHandle
		os_base + 0x24:  tSetHandleSize,       // _SetHandleSize
		os_base + 0x25:  tGetHandleSize,       // _GetHandleSize
		os_base + 0x26:  tGetZone,             // _HandleZone
		os_base + 0x27:  tReallocHandle,       // _ReallocHandle
		os_base + 0x28:  tRecoverHandle,       // _RecoverHandle
		os_base + 0x29:  tClrD0,               // _HLock
		os_base + 0x2a:  tClrD0,               // _HUnlock
		os_base + 0x2b:  tEmptyHandle,         // _EmptyHandle
		os_base + 0x2c:  tClrD0A0,             // _InitApplZone
		os_base + 0x2d:  tClrD0A0,             // _SetApplLimit
		os_base + 0x2e:  tBlockMove,           // _BlockMove
		os_base + 0x30:  tGetOSEvent,          // _OSEventAvail
		os_base + 0x31:  tGetOSEvent,          // _GetOSEvent
		os_base + 0x32:  tClrD0A0,             // _FlushEvents
		os_base + 0x36:  tClrD0A0,             // _MoreMasters
		os_base + 0x3c:  tCmpString,           // _CmpString
		os_base + 0x40:  tClrD0A0,             // _ResrvMem
		os_base + 0x44:  tSetFPos,             // _SetFPos
		os_base + 0x46:  tGetTrapAddress,      // _GetTrapAddress
		os_base + 0x47:  tSetTrapAddress,      // _SetTrapAddress
		os_base + 0x48:  tGetZone,             // _PtrZone
		os_base + 0x49:  tClrD0,               // _HPurge
		os_base + 0x4a:  tClrD0,               // _HNoPurge
		os_base + 0x4b:  tClrD0,               // _SetGrowZone
		os_base + 0x4c:  tFreeMem,             // _CompactMem
		os_base + 0x4d:  tClrD0A0,             // _PurgeMem
		os_base + 0x55:  tNop,                 // _StripAddress
		os_base + 0x60:  tFSDispatch,          // _FSDispatch
		os_base + 0x62:  tFreeMem,             // _PurgeSpace
		os_base + 0x63:  tClrD0A0,             // _MaxApplZone
		os_base + 0x64:  tClrD0,               // _MoveHHi
		os_base + 0x69:  tHGetState,           // _HGetState
		os_base + 0x6a:  tHSetState,           // _HSetState
		os_base + 0x90:  tSysEnvirons,         // _SysEnvirons
		os_base + 0xad:  tGestalt,             // _Gestalt
		tb_base + 0x00d: tCountResources,      // _Count1Resources
		tb_base + 0x00e: tGetIndResource,      // _Get1IndResource
		tb_base + 0x00f: tGetIndType,          // _Get1IndType
		tb_base + 0x01a: tHOpenResFile,        // _HOpenResFile
		tb_base + 0x01c: tCountTypes,          // _Count1Types
		tb_base + 0x01f: tGetResource,         // _Get1Resource
		tb_base + 0x020: tGetNamedResource,    // _Get1NamedResource
		tb_base + 0x023: tAliasDispatch,       // _AliasDispatch
		tb_base + 0x034: tPop2,                // _SetFScaleDisable
		tb_base + 0x050: tNop,                 // _InitCursor
		tb_base + 0x051: tPop4,                // _SetCursor
		tb_base + 0x052: tNop,                 // _HideCursor
		tb_base + 0x053: tNop,                 // _ShowCursor
		tb_base + 0x055: tPop8,                // _ShieldCursor
		tb_base + 0x056: tNop,                 // _ObscureCursor
		tb_base + 0x058: tBitAnd,              // _BitAnd
		tb_base + 0x059: tBitXor,              // _BitXor
		tb_base + 0x05a: tBitNot,              // _BitNot
		tb_base + 0x05b: tBitOr,               // _BitOr
		tb_base + 0x05c: tBitShift,            // _BitShift
		tb_base + 0x05d: tBitTst,              // _BitTst
		tb_base + 0x05e: tBitSet,              // _BitSet
		tb_base + 0x05f: tBitClr,              // _BitClr
		tb_base + 0x060: tWaitNextEvent,       // _WaitNextEvent
		tb_base + 0x06a: tHiWord,              // _HiWord
		tb_base + 0x06b: tLoWord,              // _LoWord
		tb_base + 0x06e: tInitGraf,            // _InitGraf
		tb_base + 0x06f: tPop4,                // _OpenPort
		tb_base + 0x073: tSetPort,             // _SetPort
		tb_base + 0x074: tGetPort,             // _GetPort
		tb_base + 0x09f: tUnimplemented,       // _Unimplemented
		tb_base + 0x0a7: tSetRect,             // _SetRect
		tb_base + 0x0a8: tOffsetRect,          // _OffsetRect
		tb_base + 0x0d8: tRetZero,             // _NewRgn
		tb_base + 0x0fe: tNop,                 // _InitFonts
		tb_base + 0x112: tNop,                 // _InitWindows
		tb_base + 0x124: tRetZero,             // _FrontWindow
		tb_base + 0x130: tNop,                 // _InitMenus
		tb_base + 0x137: tNop,                 // _DrawMenuBar
		tb_base + 0x138: tPop2,                // _HiliteMenu
		tb_base + 0x139: tPop6,                // _EnableItem
		tb_base + 0x13a: tPop6,                // _DisableItem
		tb_base + 0x13c: tPop4,                // _SetMenuBar
		tb_base + 0x13e: tMenuKey,             // _MenuKey
		tb_base + 0x149: tPop2RetZero,         // _GetMenuHandle
		tb_base + 0x14d: tPop8,                // _AppendResMenu
		tb_base + 0x170: tGetNextEvent,        // _GetNextEvent
		tb_base + 0x175: tRetZero,             // _TickCount
		tb_base + 0x179: tPop2,                // _CouldDialog
		tb_base + 0x17b: tNop,                 // _InitDialogs
		tb_base + 0x17c: tPop10RetZero,        // _GetNewDialog
		tb_base + 0x194: tCurResFile,          // _CurResFile
		tb_base + 0x197: tOpenResFile,         // _OpenResFile
		tb_base + 0x198: tUseResFile,          // _UseResFile
		tb_base + 0x199: tPop2,                // _UpdateResFile
		tb_base + 0x19b: tPop2,                // _SetResLoad
		tb_base + 0x19c: tCountResources,      // _CountResources
		tb_base + 0x19d: tGetIndResource,      // _GetIndResource
		tb_base + 0x19e: tCountTypes,          // _CountTypes
		tb_base + 0x19f: tGetIndType,          // _GetIndType
		tb_base + 0x1a0: tGetResource,         // _GetResource
		tb_base + 0x1a1: tGetNamedResource,    // _GetNamedResource
		tb_base + 0x1a3: tPop4,                // _ReleaseResource
		tb_base + 0x1a4: tHomeResFile,         // _HomeResFile
		tb_base + 0x1a5: tSizeRsrc,            // _SizeRsrc
		tb_base + 0x1a8: tGetResInfo,          // _GetResInfo
		tb_base + 0x1af: tResError,            // _ResError
		tb_base + 0x1b4: tNop,                 // _SystemTask
		tb_base + 0x1b8: tGetPattern,          // _GetPattern
		tb_base + 0x1b9: tGetCursor,           // _GetCursor
		tb_base + 0x1ba: tGetString,           // _GetString
		tb_base + 0x1bb: tGetIcon,             // _GetIcon
		tb_base + 0x1bc: tGetPicture,          // _GetPicture
		tb_base + 0x1bd: tPop10RetZero,        // _GetNewWindow
		tb_base + 0x1c0: tPop2RetZero,         // _GetNewMBar
		tb_base + 0x1c4: tOpenRFPerm,          // _OpenRFPerm
		tb_base + 0x1c8: tNop,                 // _SysBeep
		tb_base + 0x1c9: tSysError,            // _SysError
		tb_base + 0x1cc: tNop,                 // _TEInit
		tb_base + 0x1e1: tHandToHand,          // _HandToHand
		tb_base + 0x1e2: tPtrToXHand,          // _PtrToXHand
		tb_base + 0x1e3: tPtrToHand,           // _PtrToHand
		tb_base + 0x1e4: tHandAndHand,         // _HandAndHand
		tb_base + 0x1e5: tPop2,                // _InitPack
		tb_base + 0x1e6: tNop,                 // _InitAllPacks
		tb_base + 0x1f0: tLoadSeg,             // _LoadSeg
		tb_base + 0x1f1: tPop4,                // _UnloadSeg
		tb_base + 0x1f4: tExitToShell,         // _ExitToShell
		tb_base + 0x1fa: tRetZero,             // _UnlodeScrap
		tb_base + 0x1fb: tRetZero,             // _LodeScrap
		tb_base + 0x252: tHighLevelFSDispatch, // _HighLevelFSDispatch
		tb_base + 0x3ff: tDebugStr,            // _DebugStr
	}

	var err error // damn this variable shadowing behaviour!
	systemFolder, err = ioutil.TempDir("", "NuMPW")
	if err != nil {
		panic("Failed to create temp directory")
	}
	defer os.RemoveAll(systemFolder)

	// Create our fake-FS shim layer
	subEmbed, _ := fs.Sub(embedMPW, "ToolServer 1.1.1")
	gFS.Add(subEmbed, filepath.Join(systemFolder, "MPW"))

	// Command line opts
	cmdLines := []string{""} // leave space for the "chdir"
	toolServer := filepath.Join(systemFolder, "MPW", "ToolServer")

	flag.IntVar(&gDebug, "d", 0, "debug level (>=5 prints every instruction)")
	flag.StringVar(&toolServer, "ts", toolServer, "ToolServer (default to built-in version)")
	flag.Func("c", "command (/unix/path -> mac:path)", func(s string) error { cmdLines = append(cmdLines, cmdPathCvt(s)); return nil })
	flag.Func("C", "command (no path conversion)", func(s string) error { cmdLines = append(cmdLines, s); return nil })

	flag.Parse()

	// First command line opt as a "chdir"
	cwd, err := os.Getwd()
	if err != nil {
		panic(err)
	}
	cmdLines[0] = "directory " + quote(pathCvt(cwd))

	// Make a script to run
	script := unicodeToMacOrPanic(strings.Join(cmdLines, "\r"))

	dnums = []string{
		filepath.Join(systemFolder, "MPW"),
		"",
		"/",
	}

	mem = make([]byte, kHeap)

	// Poison low memory
	for i := uint32(0x40); i < kStackBase; i += 2 {
		writew(i, 0x68f1)
	}

	// Populate trap table
	for i, impl := range my_traps {
		tableAddr := kOSTable + 4*uint32(i)
		if i >= 0x100 {
			tableAddr = kToolTable + 4*uint32(i-0x100)
		}
		if impl == nil {
			writel(tableAddr, executable_ftrap(0xf89f))
		} else {
			trap := uint16(i)
			if i >= 0x100 {
				trap = trap - 0x100 + 0x800
			}
			writel(tableAddr, executable_ftrap(0xf000|trap))
		}
	}

	// Get some PACKs
	pax, _ := embedPACKs.ReadDir("PACKs")
	packAddr := uint32(kPACKs)
	for _, de := range pax {
		tbTrapNum, _ := strconv.ParseUint(strings.Split(de.Name(), ".")[0], 16, 10)
		pack, _ := embedPACKs.ReadFile("PACKs/" + de.Name())
		copy(mem[packAddr:], pack)
		writel(kToolTable+4*uint32(tbTrapNum), packAddr)
		packAddr += uint32(len(pack))
	}

	// Starting point for stack
	writel(spptr, kStackBase)
	writel(0x908, kStackBase) // CurStackBase

	// A5 world
	writel(a5ptr, kA5World)

	// Single fake heap zone, enough to pass validation
	writel(0x118, kFakeHeapHeader)     // TheZone
	writel(0x2a6, kFakeHeapHeader)     // SysZone
	writel(0x2aa, kFakeHeapHeader)     // ApplZone
	writel(kFakeHeapHeader, 0xffffffe) // bkLim
	writel(0x130, 0)                   // ApplLimit

	// 1 Drive Queue Element
	writew(0x308, 0)      // DrvQHdr.QFlags
	writel(0x308+2, kDQE) // DrvQHdr.QHead
	writel(0x308+6, kDQE) // DrvQHdr.QTail
	for i := -4; i < 16; i += 2 {
		writew(kDQE+uint32(i), 0)
	}

	// 1 Volume Control Block is needed for the "GetVRefNum" glue routine
	for i := 0; i < 178; i += 2 {
		writew(kVCB+uint32(i), 0)
	}
	writew(kVCB+78, 2)                 // vcbVRefNum
	writePstring(kVCB+44, onlyvolname) // vcbVName

	// File Control Block table
	writel(0x34e, kFCBTable)    // FCBSPtr
	writew(0x3f6, 94)           // FSFCBLen
	writew(kFCBTable, 2+94*348) // size of FCB table

	// Misc globals
	writew(0x210, get_macos_dnum(systemFolder)) // BootDrive
	writel(0x2f4, 0)                            // CaretTime = 0 ticks
	writel(0x316, 0)                            // we don't implement the 'MPGM' interface
	writel(0x31a, 0x00ffffff)                   // Lo3Bytes
	writel(0x33c, 0)                            // IAZNotify = nothing to do when swapping worlds
	writePstring(0x910, "ToolServer")
	writel(0x9d6, 0)          // WindowList empty
	writel(0xa02, 0x00010001) // OneOne
	writel(0xa06, 0xffffffff) // MinusOne
	writel(0xa1c, kMenuList)  // MenuList empty
	writel(0xa50, 0)          // TopMapHndl

	// Create some useful folders
	for _, n := range []string{"Preferences", "Desktop Folder", "Trash", "Temporary Items"} {
		os.Mkdir(filepath.Join(systemFolder, n), 0o777)
	}

	// Disable the status window in preferences
	os.MkdirAll(filepath.Join(systemFolder, "Preferences", "MPW"), 0o777)
	os.WriteFile(filepath.Join(systemFolder, "Preferences", "MPW", "ToolServer Prefs"), make([]byte, 9), 0o777)

	// Open a script as if from Finder
	writel(d0ptr, 128)
	call_m68k(executable_atrap(0xa122)) // _NewHandle
	appParms := readl(a0ptr)
	writel(0xaec, appParms)
	appParms = readl(appParms) // handle to pointer
	writew(appParms+2, 1)      // one file
	writew(appParms+4, get_macos_dnum(systemFolder))
	writel(appParms+6, 0x54455854) // TEXT
	writePstring(appParms+12, "Script")

	os.WriteFile(filepath.Join(systemFolder, "Script"), []byte(script), 0o777)
	os.WriteFile(filepath.Join(systemFolder, "Script.idump"), []byte("TEXTMPS "), 0o777)
	os.Create(filepath.Join(systemFolder, "Script.out"))
	os.Create(filepath.Join(systemFolder, "Script.err"))

	push(32, 0)
	fileNamePtr := readl(spptr)
	pushw(0) // refnum return
	pushw(2) // vol id
	pushl(uint32(get_macos_dnum(filepath.Dir(toolServer))))
	writePstring(fileNamePtr, unicodeToMacOrPanic(filepath.Base(toolServer)))
	pushl(fileNamePtr)                  // pointer to the file string
	pushw(0)                            // permission
	call_m68k(executable_atrap(0xac1a)) // _HOpenResFile ,autoPop

	writew(0xa58, readw(0xa5a)) // SysMap = CurMap

	pushl(0)                            // handle return
	pushl(0x434f4445)                   // CODE
	pushw(0)                            // ID 0
	call_m68k(executable_atrap(0xada0)) // _GetResource ,autoPop
	code0 := pop(4)
	code0 = readl(code0) // handle to pointer

	jtsize := readl(code0 + 8)
	jtoffset := readl(code0 + 12)

	copy(mem[kA5World+jtoffset:][:jtsize], mem[code0+16:][:jtsize])

	call_m68k(kA5World + jtoffset + 2)

	closeAll()

	stdout, err := os.ReadFile(filepath.Join(systemFolder, "Script.out"))
	os.Stdout.WriteString(strings.ReplaceAll(macToUnicode(macstring(stdout)), "\r", "\n"))

	stderr, _ := os.ReadFile(filepath.Join(systemFolder, "Script.err"))
	os.Stderr.WriteString(strings.ReplaceAll(macToUnicode(macstring(stderr)), "\r", "\n"))
}
