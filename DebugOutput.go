package main

import (
	"fmt"
	"os"
	"strings"
)

const allBugFlags = "6=68k|t=stacktrace|f=FileMgr"

var bugFlags = os.Getenv("NUMPW_DEBUG")

var (
	gDebugAny        = bugFlags != ""
	gDebugEveryInst  = strings.ContainsRune(bugFlags, '6')
	gDebugStackTrace = strings.ContainsRune(bugFlags, 't')
	gDebugFileMgr    = strings.ContainsRune(bugFlags, 'f')
)

// called before and after every trap when debugging is on
func logTrap(inst uint16, isPre bool) {
	// For OS traps, use the more informative number in d1
	if inst&0x800 == 0 {
		inst = readw(d1ptr + 2)
	}

	switch {
	case inst&0x8ff <= 0x18 || inst&0x8ff == 0x44 || inst&0x8ff == 0x60:
		// File Manager
		if gDebugFileMgr {
			logFileMgrTrap(inst, isPre)
		}
	}
}

// prettyprinting

func logf(format string, a ...interface{}) {
	str := fmt.Sprintf(format, a...)
	str = strings.TrimRight(str, "\n")

	if len(str) == 0 {
		return
	}

	for _, line := range strings.Split(str, "\n") {
		fmt.Fprintf(os.Stderr, "#### %s\n", line)
	}
}

func logln(a ...interface{}) {
	logf("%s", fmt.Sprintln(a...))
}

// File Manager dumping

var gLastFSDispatchSelector uint16 // because d0 gets clobbered

func logFileMgrTrap(num uint16, isPre bool) {
	pb := readl(a0ptr)

	trapName := func(s string) {
		if isPre {
			logf("%04X=%s (\n", num, s)
		}
	}

	if !isPre {
		logln(") returns (")
		dumpPBField("ioResult")
	}

	switch num & 0xff {
	case 0x00:
		trapName("_Open")
		if isPre {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
			dumpPBField("ioPermssn")
		} else {
			dumpPBField("ioRefNum")
		}

	case 0x01:
		trapName("_Close")
		if isPre {
			dumpPBField("ioRefNum")
		}

	case 0x02:
		trapName("_Read")
		if isPre {
			dumpPBField("ioRefNum")
			dumpPBField("ioBuffer")
			dumpPBField("ioReqCount")
			dumpPBField("ioPosMode")
			dumpPBField("ioPosOffset")
		} else {
			dumpPBField("ioActCount")
			dumpPBField("ioPosOffset")
		}

	case 0x03:
		trapName("_Write")
		if isPre {
			dumpPBField("ioRefNum")
			dumpPBField("ioBuffer")
			dumpPBField("ioReqCount")
			dumpPBField("ioPosMode")
			dumpPBField("ioPosOffset")
		} else {
			dumpPBField("ioActCount")
			dumpPBField("ioPosOffset")
		}

	case 0x04:
		trapName("_Control")
		if isPre {
			dumpPBField("ioVRefNum")
			dumpPBField("ioRefNum")
			dumpPBField("csCode")
			dumpPBField("csParam")
		}

	case 0x05:
		trapName("_Status")
		if isPre {
			dumpPBField("ioVRefNum")
			dumpPBField("ioRefNum")
			dumpPBField("csCode")
		} else {
			dumpPBField("csParam")
		}

	case 0x06:
		trapName("_KillIO")
		if isPre {
			dumpPBField("ioRefNum")
		}

	case 0x07:
		trapName("_GetVInfo")
		if isPre {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
			dumpPBField("ioVolIndex")
		} else {
			dumpPBField("ioVCrDate")
			dumpPBField("ioVLsBkUp")
			dumpPBField("ioVAtrb")
			dumpPBField("ioVNmFls")
			dumpPBField("ioVDirSt")
			dumpPBField("ioVBlLn")
			dumpPBField("ioVNmAlBlks")
			dumpPBField("ioVAlBlkSiz")
			dumpPBField("ioVClpSiz")
			dumpPBField("ioAlBlSt")
			dumpPBField("ioVNxtFNum")
			dumpPBField("ioVFrBlk")
			if num&0x200 != 0 {
				dumpPBField("ioVSigWord")
				dumpPBField("ioVDrvInfo")
				dumpPBField("ioVDRefNum")
				dumpPBField("ioVFSID")
				dumpPBField("ioVBkUp")
				dumpPBField("ioVSeqNum")
				dumpPBField("ioVWrCnt")
				dumpPBField("ioVFilCnt")
				dumpPBField("ioVDirCnt")
				dumpPBField("ioVFndrInfo")
			}
		}

	case 0x08:
		trapName("_Create")
		if isPre {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
		}

	case 0x09:
		trapName("_Delete")
		if isPre {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
		}

	case 0x0a:
		trapName("_OpenRF")
		if isPre {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
			dumpPBField("ioPermssn")
		} else {
			dumpPBField("ioRefNum")
		}

	case 0x0b:
		trapName("_Rename")
		if isPre {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
			dumpPBField("ioPermssn")
		} else {
			dumpPBField("ioRefNum")
		}

	case 0x0c:
		trapName("_GetFInfo")
		if isPre {
			if int16(readw(pb+28)) <= 0 {
				dumpPBField("ioNamePtr")
			} else {
				logln("    (no ioNamePtr because of ioFDirIndex)")
			}
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
			dumpPBField("ioFDirIndex")
		} else {
			dumpPBField("ioFlAttrib")
			dumpPBField("ioFlFndrInfo")
			dumpPBField("ioFlNum")
			dumpPBField("ioFlStBlk")
			dumpPBField("ioFlLgLen")
			dumpPBField("ioFlPyLen")
			dumpPBField("ioFlRStBlk")
			dumpPBField("ioFlRLgLen")
			dumpPBField("ioFlRPyLen")
			dumpPBField("ioFlCrDat")
			dumpPBField("ioFlMdDat")
		}

	case 0x0d:
		trapName("_SetFInfo")
		if isPre {
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
			dumpPBField("ioFlFndrInfo")
			dumpPBField("ioFlCrDat")
			dumpPBField("ioFlMdDat")
		}

	case 0x10:
		trapName("_Allocate")
		if isPre {
			dumpPBField("ioRefNum")
			dumpPBField("ioReqCount")
		} else {
			dumpPBField("ioActCount")
		}

	case 0x11:
		trapName("_GetEOF")
		if isPre {
			dumpPBField("ioRefNum")
		} else {
			dumpPBField("ioMisc")
		}

	case 0x12:
		trapName("_SetEOF")
		if isPre {
			dumpPBField("ioRefNum")
			dumpPBField("ioMisc")
		}

	case 0x13:
		trapName("_FlushVol")
		if isPre {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
		}

	case 0x14:
		trapName("_GetVol")
		if isPre {
		} else {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
			if num&0x200 != 0 {
				dumpPBField("ioWDProcID")
				dumpPBField("ioWDVRefNum")
				dumpPBField("ioWDDirID")
			}
		}

	case 0x15:
		trapName("_SetVol")
		if isPre {
		} else {
			dumpPBField("ioNamePtr")
			dumpPBField("ioVRefNum")
			dumpPBField("ioDirID?")
		}

	case 0x18:
		trapName("_GetFPos")
		if isPre {
			dumpPBField("ioRefNum")
		} else {
			dumpPBField("ioReqCount")
			dumpPBField("ioActCount")
			dumpPBField("ioPosMode")
			dumpPBField("ioPosOffset")
		}

	case 0x44:
		trapName("_SetFPos")
		if isPre {
			dumpPBField("ioRefNum")
			dumpPBField("ioPosMode")
			dumpPBField("ioPosOffset")
		} else {
			dumpPBField("ioPosOffset")
		}

	case 0x60:
		if isPre {
			gLastFSDispatchSelector = readw(d0ptr + 2)
		}

		switch gLastFSDispatchSelector {
		case 1:
			trapName("_FSDispatch _OpenWD")
			if isPre {
				dumpPBField("ioNamePtr")
				dumpPBField("ioVRefNum")
				dumpPBField("ioWDDirID")
				dumpPBField("ioWDProcID")
			} else {
				dumpPBField("ioVRefNum")
			}

		case 2:
			trapName("_FSDispatch _CloseWD")
			if isPre {
				dumpPBField("ioVRefNum")
			}

		case 7:
			trapName("_FSDispatch _GetWDInfo")
			if isPre {
				dumpPBField("ioVRefNum")
				dumpPBField("ioWDIndex")
				dumpPBField("ioWDProcID")
				dumpPBField("ioWDVRefNum")
			} else {
				dumpPBField("ioNamePtr")
				dumpPBField("ioVRefNum")
				dumpPBField("ioWDProcID")
				dumpPBField("ioWDVRefNum")
				dumpPBField("ioWDDirID")
			}

		case 8:
			trapName("_FSDispatch _GetFCBInfo")
			if isPre {
				dumpPBField("ioVRefNum")
				dumpPBField("ioRefNum")
				dumpPBField("ioFCBIndx")
			} else {
				dumpPBField("ioNamePtr")
				dumpPBField("ioVRefNum")
				dumpPBField("ioRefNum")

				dumpPBField("ioFCBFlNm")
				dumpPBField("ioFCBFlags")
				dumpPBField("ioFCBStBlk")
				dumpPBField("ioFCBEOF")
				dumpPBField("ioFCBPLen")
				dumpPBField("ioFCBCrPs")
				dumpPBField("ioFCBVRefNum")
				dumpPBField("ioFCBClpSiz")
				dumpPBField("ioFCBParID")
			}

		case 9:
			trapName("_FSDispatch _GetCatInfo")
			if isPre {
				if readw(pb+28) == 0 {
					dumpPBField("ioNamePtr")
				} else {
					logln("    (no ioNamePtr because of ioFDirIndex)")
				}
				dumpPBField("ioVRefNum")
				dumpPBField("ioDirID?")
				dumpPBField("ioFDirIndex")
			} else {
				isDir := readb(pb+30)&0x10 != 0
				dumpPBField("ioNamePtr")
				dumpPBField("ioDirID")
				dumpPBField("ioFRefNum")
				dumpPBField("ioFlAttrib")
				if isDir {
					dumpPBField("ioDrUsrWds")
					dumpPBField("ioDrNmFls")
					dumpPBField("ioDrCrDat")
					dumpPBField("ioDrMdDat")
					dumpPBField("ioDrBkDat")
					dumpPBField("ioDrFndrInfo")
					dumpPBField("ioDrParID")
				} else {
					dumpPBField("ioACUser")
					dumpPBField("ioFlFndrInfo")
					dumpPBField("ioFlStBlk")
					dumpPBField("ioFlLgLen")
					dumpPBField("ioFlPyLen")
					dumpPBField("ioFlRStBlk")
					dumpPBField("ioFlRLgLen")
					dumpPBField("ioFlRPyLen")
					dumpPBField("ioFlCrDat")
					dumpPBField("ioFlMdDat")
					dumpPBField("ioFlBkDat")
					dumpPBField("ioFlXFndrInfo")
					dumpPBField("ioFlParID")
					dumpPBField("ioFlClpSiz")
				}
			}

		case 26:
			trapName("_FSDispatch _OpenDF")
			if isPre {
				dumpPBField("ioNamePtr")
				dumpPBField("ioVRefNum")
				dumpPBField("ioDirID?")
				dumpPBField("ioPermssn")
			} else {
				dumpPBField("ioRefNum")
			}

		case 27:
			trapName("_FSDispatch _MakeFSSpec")
			if isPre {
				dumpPBField("ioNamePtr")
				dumpPBField("ioVRefNum")
				dumpPBField("ioDirID")
				dumpPBField("ioPermssn")
				dumpPBField("ioMisc")
			}

		default:
			trapName(fmt.Sprintf("_FSDispatch %#x", gLastFSDispatchSelector))
			if isPre {
			}
		}

	default:
		trapName("???")
		if isPre {
		}
	}

	if !isPre {
		logln(")")
	}
}

func dumpPBField(f string) {
	logField := func(n ...interface{}) {
		n = append([]interface{}{"   "}, n...)
		for i := range n {
			switch n[i].(type) {
			case uint32:
				n[i] = fmt.Sprintf("%#08x", n[i])
			case uint16:
				n[i] = fmt.Sprintf("%#04x", n[i])
			case uint8:
				n[i] = fmt.Sprintf("%#02x", n[i])
			}
		}
		logln(n...)
	}

	pb := readl(a0ptr)
	switch f {
	case "ioTrap":
		logField(f, readw(pb+6))
	case "ioResult":
		logField(f, readw(pb+16), int16(readw(pb+16)))
	case "ioVRefNum":
		vrefnum := readw(pb + 22)
		logField(f, vrefnum, dnums[vrefnum])
	case "ioRefNum", "ioFRefNum":
		refnum := readw(pb + 24)
		logField(f, refnum, readPstring(fcbFromRefnum(refnum)+62))
	case "ioWDIndex", "csCode":
		logField(f, readw(pb+26))
	case "ioPermssn":
		logField(f, readb(pb+27))
	case "ioMisc", "ioWDProcID":
		logField(f, readl(pb+28))
	case "ioVolIndex", "ioFDirIndex", "ioFCBIndx":
		logField(f, readw(pb+28))
	case "csParam":
		logField(f, fmt.Sprintf("%X", mem[pb+28:][:22]))
	case "ioFlAttrib":
		logField(f, readb(pb+30))
	case "ioVCrDate":
		logField(f, readl(pb+30))
	case "ioACUser":
		logField(f, readb(pb+31))
	case "ioBuffer", "ioFCBFlNm":
		logField(f, readl(pb+32))
	case "ioWDVRefNum":
		logField(f, readw(pb+32))
	case "ioFlFndrInfo", "ioDrUsrWds":
		logField(f, fmt.Sprintf("%X", mem[pb+32:][:16]))
	case "ioNamePtr":
		nameptr := readl(pb + 18)
		logField(f, nameptr, macToUnicode(readPstring(nameptr)))
	case "ioVLsBkUp":
		logField(f, readl(pb+34))
	case "ioReqCount":
		logField(f, readl(pb+36))
	case "ioFCBFlags":
		logField(f, readw(pb+36))
	case "ioVAtrb", "ioFCBStBlk":
		logField(f, readw(pb+38))
	case "ioActCount", "ioFCBEOF":
		logField(f, readl(pb+40))
	case "ioVNmFls":
		logField(f, readw(pb+40))
	case "ioVDirSt":
		logField(f, readw(pb+42))
	case "ioPosMode", "ioVBlLn":
		logField(f, readw(pb+44))
	case "ioFCBPLen":
		logField(f, readl(pb+44))
	case "ioPosOffset":
		logField(f, readl(pb+46))
	case "ioVNmAlBlks":
		logField(f, readw(pb+46))
	case "ioDirID?":
		if readw(d1ptr+2)&0x200 != 0 {
			dumpPBField("ioDirID")
		}
	case "ioDirID", "ioWDDirID":
		dirid := readl(pb + 48)
		logField(f, dirid, dnums[dirid])
	case "ioFlNum", "ioVAlBlkSiz", "ioFCBCrPs":
		logField(f, readl(pb+48))
	case "ioFlStBlk", "ioFCBVRefNum":
		logField(f, readw(pb+52))
	case "ioVClpSiz", "ioDrNmFls":
		logField(f, readl(pb+52))
	case "ioFlLgLen", "ioFCBClpSiz":
		logField(f, readl(pb+54))
	case "ioAlBlSt":
		logField(f, readl(pb+56))
	case "ioVNxtFNum", "ioFlPyLen":
		logField(f, readl(pb+58))
	case "ioFCBParID":
		dirid := readl(pb + 58)
		logField(f, dirid, dnums[dirid])
	case "ioVFrBlk", "ioFlRStBlk":
		logField(f, readw(pb+62))
	case "ioFlRLgLen":
		logField(f, readl(pb+64))
	case "ioVSigWord":
		logField(f, readw(pb+64))
	case "ioVDrvInfo":
		logField(f, readw(pb+66))
	case "ioFlRPyLen":
		logField(f, readl(pb+68))
	case "ioVDRefNum":
		logField(f, readw(pb+68))
	case "ioVFSID":
		logField(f, readw(pb+70))
	case "ioVBkUp", "ioFlCrDat", "ioDrCrDat":
		logField(f, readl(pb+72))
	case "ioFlMdDat", "ioDrMdDat":
		logField(f, readl(pb+76))
	case "ioVSeqNum":
		logField(f, readw(pb+76))
	case "ioVWrCnt":
		logField(f, readl(pb+78))
	case "ioFlBkDat", "ioDrBkDat":
		logField(f, readl(pb+80))
	case "ioVFilCnt":
		logField(f, readl(pb+82))
	case "ioFlXFndrInfo", "ioDrFndrInfo":
		logField(f, fmt.Sprintf("%X", mem[pb+84:][:16]))
	case "ioVDirCnt":
		logField(f, readl(pb+86))
	case "ioVFndrInfo":
		logField(f, fmt.Sprintf("%X", mem[pb+90:][:32]))
	case "ioFlParID", "ioDrParID":
		dirid := readl(pb + 100)
		logField(f, dirid, dnums[dirid])
	case "ioFlClpSiz":
		logField(f, readl(pb+104))
	default:
		panic("uncoded paramblk field " + f)
	}
}