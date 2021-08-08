package main

import (
	"bytes"
)

// Different because believed to be macRoman formatted!
type macstring string

var macToUnicodeTable = [...]rune{
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
	0x00c4, 0x00c5, 0x00c7, 0x00c9, 0x00d1, 0x00d6, 0x00dc, 0x00e1,
	0x00e0, 0x00e2, 0x00e4, 0x00e3, 0x00e5, 0x00e7, 0x00e9, 0x00e8,
	0x00ea, 0x00eb, 0x00ed, 0x00ec, 0x00ee, 0x00ef, 0x00f1, 0x00f3,
	0x00f2, 0x00f4, 0x00f6, 0x00f5, 0x00fa, 0x00f9, 0x00fb, 0x00fc,
	0x2020, 0x00b0, 0x00a2, 0x00a3, 0x00a7, 0x2022, 0x00b6, 0x00df,
	0x00ae, 0x00a9, 0x2122, 0x00b4, 0x00a8, 0x2260, 0x00c6, 0x00d8,
	0x221e, 0x00b1, 0x2264, 0x2265, 0x00a5, 0x00b5, 0x2202, 0x2211,
	0x220f, 0x03c0, 0x222b, 0x00aa, 0x00ba, 0x03a9, 0x00e6, 0x00f8,
	0x00bf, 0x00a1, 0x00ac, 0x221a, 0x0192, 0x2248, 0x2206, 0x00ab,
	0x00bb, 0x2026, 0x00a0, 0x00c0, 0x00c3, 0x00d5, 0x0152, 0x0153,
	0x2013, 0x2014, 0x201c, 0x201d, 0x2018, 0x2019, 0x00f7, 0x25ca,
	0x00ff, 0x0178, 0x2044, 0x20ac, 0x2039, 0x203a, 0xfb01, 0xfb02,
	0x2021, 0x00b7, 0x201a, 0x201e, 0x2030, 0x00c2, 0x00ca, 0x00c1,
	0x00cb, 0x00c8, 0x00cd, 0x00ce, 0x00cf, 0x00cc, 0x00d3, 0x00d4,
	0xf8ff, 0x00d2, 0x00da, 0x00db, 0x00d9, 0x0131, 0x02c6, 0x02dc,
	0x00af, 0x02d8, 0x02d9, 0x02da, 0x00b8, 0x02dd, 0x02db, 0x02c7,
}

var macSortTab = [...]byte{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x22, 0x23, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
	0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
	0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
	0x47, 0x48, 0x57, 0x59, 0x5d, 0x5f, 0x66, 0x68,
	0x6a, 0x6c, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7e,
	0x8c, 0x8e, 0x90, 0x92, 0x95, 0x97, 0x9e, 0xa0,
	0xa2, 0xa4, 0xa7, 0xa9, 0xaa, 0xab, 0xac, 0xad,
	0xae, 0x4e, 0x58, 0x5b, 0x5e, 0x61, 0x67, 0x69,
	0x6b, 0x6d, 0x73, 0x75, 0x77, 0x79, 0x7c, 0x83,
	0x8d, 0x8f, 0x91, 0x93, 0x96, 0x99, 0x9f, 0xa1,
	0xa3, 0xa5, 0xa8, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
	0x4a, 0x4c, 0x5a, 0x60, 0x7b, 0x7f, 0x98, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x5c, 0x62, 0x63,
	0x64, 0x65, 0x6e, 0x6f, 0x70, 0x71, 0x7d, 0x84,
	0x85, 0x86, 0x87, 0x88, 0x9a, 0x9b, 0x9c, 0x9d,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0x94,
	0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0x4d, 0x81,
	0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
	0xc9, 0xca, 0xcb, 0x55, 0x8a, 0xcc, 0x56, 0x89,
	0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0x26,
	0x27, 0xd4, 0x20, 0x49, 0x4b, 0x80, 0x82, 0x8b,
	0xd5, 0xd6, 0x24, 0x25, 0x2d, 0x2e, 0xd7, 0xd8,
	0xa6, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
}

var macUpperTab = [...]byte{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x61, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0xcb, 0x89, 0x80, 0xcc, 0x81, 0x82, 0x83, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x84, 0x97,
	0x98, 0x99, 0x85, 0xcd, 0x9c, 0x9d, 0x9e, 0x86,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xae, 0xaf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xce,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
}

var macStripTab = [...]byte{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x41, 0x41, 0x43, 0x45, 0x4e, 0x4f, 0x55, 0x61,
	0x61, 0x61, 0x61, 0x61, 0x61, 0x63, 0x65, 0x65,
	0x65, 0x65, 0x69, 0x69, 0x69, 0x69, 0x6e, 0x6f,
	0x6f, 0x6f, 0x6f, 0x6f, 0x75, 0x75, 0x75, 0x75,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0x4f,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0xba, 0x61, 0x6f, 0xbd, 0xbe, 0x6f,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0xca, 0x41, 0x41, 0x4f, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0x79, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
}

func macToUnicode(mac macstring) string {
	var buf bytes.Buffer
	for i := 0; i < len(mac); i++ {
		buf.WriteRune(macToUnicodeTable[mac[i]])
	}
	return buf.String()
}

func unicodeToMacOrPanic(str string) macstring {
	macstr, ok := unicodeToMac(str)
	if !ok {
		panic("Not convertible to Mac Roman: " + str)
	}
	return macstr
}

// Slightly overcooked code that turnes (de)composed UTF-8 and gives Mac Roman
func unicodeToMac(str string) (retval macstring, ok bool) {
	var macstr []byte

	defer func() {
		if recover() != nil {
			ok = false
		}
	}()

	for _, codepoint := range str {
		var macbyte byte
		switch {
		case codepoint < 0x80:
			macbyte = byte(codepoint)
		case codepoint == 0xc4: // LATIN CAPITAL LETTER A WITH DIAERESIS
			macbyte = 0x80
		case codepoint == 0xc5: // LATIN CAPITAL LETTER A WITH RING ABOVE
			macbyte = 0x81
		case codepoint == 0xc7: // LATIN CAPITAL LETTER C WITH CEDILLA
			macbyte = 0x82
		case codepoint == 0xc9: // LATIN CAPITAL LETTER E WITH ACUTE
			macbyte = 0x83
		case codepoint == 0xd1: // LATIN CAPITAL LETTER N WITH TILDE
			macbyte = 0x84
		case codepoint == 0xd6: // LATIN CAPITAL LETTER O WITH DIAERESIS
			macbyte = 0x85
		case codepoint == 0xdc: // LATIN CAPITAL LETTER U WITH DIAERESIS
			macbyte = 0x86
		case codepoint == 0xe1: // LATIN SMALL LETTER A WITH ACUTE
			macbyte = 0x87
		case codepoint == 0xe0: // LATIN SMALL LETTER A WITH GRAVE
			macbyte = 0x88
		case codepoint == 0xe2: // LATIN SMALL LETTER A WITH CIRCUMFLEX
			macbyte = 0x89
		case codepoint == 0xe4: // LATIN SMALL LETTER A WITH DIAERESIS
			macbyte = 0x8a
		case codepoint == 0xe3: // LATIN SMALL LETTER A WITH TILDE
			macbyte = 0x8b
		case codepoint == 0xe5: // LATIN SMALL LETTER A WITH RING ABOVE
			macbyte = 0x8c
		case codepoint == 0xe7: // LATIN SMALL LETTER C WITH CEDILLA
			macbyte = 0x8d
		case codepoint == 0xe9: // LATIN SMALL LETTER E WITH ACUTE
			macbyte = 0x8e
		case codepoint == 0xe8: // LATIN SMALL LETTER E WITH GRAVE
			macbyte = 0x8f
		case codepoint == 0xea: // LATIN SMALL LETTER E WITH CIRCUMFLEX
			macbyte = 0x90
		case codepoint == 0xeb: // LATIN SMALL LETTER E WITH DIAERESIS
			macbyte = 0x91
		case codepoint == 0xed: // LATIN SMALL LETTER I WITH ACUTE
			macbyte = 0x92
		case codepoint == 0xec: // LATIN SMALL LETTER I WITH GRAVE
			macbyte = 0x93
		case codepoint == 0xee: // LATIN SMALL LETTER I WITH CIRCUMFLEX
			macbyte = 0x94
		case codepoint == 0xef: // LATIN SMALL LETTER I WITH DIAERESIS
			macbyte = 0x95
		case codepoint == 0xf1: // LATIN SMALL LETTER N WITH TILDE
			macbyte = 0x96
		case codepoint == 0xf3: // LATIN SMALL LETTER O WITH ACUTE
			macbyte = 0x97
		case codepoint == 0xf2: // LATIN SMALL LETTER O WITH GRAVE
			macbyte = 0x98
		case codepoint == 0xf4: // LATIN SMALL LETTER O WITH CIRCUMFLEX
			macbyte = 0x99
		case codepoint == 0xf6: // LATIN SMALL LETTER O WITH DIAERESIS
			macbyte = 0x9a
		case codepoint == 0xf5: // LATIN SMALL LETTER O WITH TILDE
			macbyte = 0x9b
		case codepoint == 0xfa: // LATIN SMALL LETTER U WITH ACUTE
			macbyte = 0x9c
		case codepoint == 0xf9: // LATIN SMALL LETTER U WITH GRAVE
			macbyte = 0x9d
		case codepoint == 0xfb: // LATIN SMALL LETTER U WITH CIRCUMFLEX
			macbyte = 0x9e
		case codepoint == 0xfc: // LATIN SMALL LETTER U WITH DIAERESIS
			macbyte = 0x9f
		case codepoint == 0x2020: // DAGGER
			macbyte = 0xa0
		case codepoint == 0xb0: // DEGREE SIGN
			macbyte = 0xa1
		case codepoint == 0xa2: // CENT SIGN
			macbyte = 0xa2
		case codepoint == 0xa3: // POUND SIGN
			macbyte = 0xa3
		case codepoint == 0xa7: // SECTION SIGN
			macbyte = 0xa4
		case codepoint == 0x2022: // BULLET
			macbyte = 0xa5
		case codepoint == 0xb6: // PILCROW SIGN
			macbyte = 0xa6
		case codepoint == 0xdf: // LATIN SMALL LETTER SHARP S
			macbyte = 0xa7
		case codepoint == 0xae: // REGISTERED SIGN
			macbyte = 0xa8
		case codepoint == 0xa9: // COPYRIGHT SIGN
			macbyte = 0xa9
		case codepoint == 0x2122: // TRADE MARK SIGN
			macbyte = 0xaa
		case codepoint == 0xb4: // ACUTE ACCENT
			macbyte = 0xab
		case codepoint == 0xa8: // DIAERESIS
			macbyte = 0xac
		case codepoint == 0x2260: // NOT EQUAL TO
			macbyte = 0xad
		case codepoint == 0xc6: // LATIN CAPITAL LETTER AE
			macbyte = 0xae
		case codepoint == 0xd8: // LATIN CAPITAL LETTER O WITH STROKE
			macbyte = 0xaf
		case codepoint == 0x221e: // INFINITY
			macbyte = 0xb0
		case codepoint == 0xb1: // PLUS-MINUS SIGN
			macbyte = 0xb1
		case codepoint == 0x2264: // LESS-THAN OR EQUAL TO
			macbyte = 0xb2
		case codepoint == 0x2265: // GREATER-THAN OR EQUAL TO
			macbyte = 0xb3
		case codepoint == 0xa5: // YEN SIGN
			macbyte = 0xb4
		case codepoint == 0xb5: // MICRO SIGN
			macbyte = 0xb5
		case codepoint == 0x2202: // PARTIAL DIFFERENTIAL
			macbyte = 0xb6
		case codepoint == 0x2211: // N-ARY SUMMATION
			macbyte = 0xb7
		case codepoint == 0x220f: // N-ARY PRODUCT
			macbyte = 0xb8
		case codepoint == 0x3c0: // GREEK SMALL LETTER PI
			macbyte = 0xb9
		case codepoint == 0x222b: // INTEGRAL
			macbyte = 0xba
		case codepoint == 0xaa: // FEMININE ORDINAL INDICATOR
			macbyte = 0xbb
		case codepoint == 0xba: // MASCULINE ORDINAL INDICATOR
			macbyte = 0xbc
		case codepoint == 0x3a9: // GREEK CAPITAL LETTER OMEGA
			macbyte = 0xbd
		case codepoint == 0xe6: // LATIN SMALL LETTER AE
			macbyte = 0xbe
		case codepoint == 0xf8: // LATIN SMALL LETTER O WITH STROKE
			macbyte = 0xbf
		case codepoint == 0xbf: // INVERTED QUESTION MARK
			macbyte = 0xc0
		case codepoint == 0xa1: // INVERTED EXCLAMATION MARK
			macbyte = 0xc1
		case codepoint == 0xac: // NOT SIGN
			macbyte = 0xc2
		case codepoint == 0x221a: // SQUARE ROOT
			macbyte = 0xc3
		case codepoint == 0x192: // LATIN SMALL LETTER F WITH HOOK
			macbyte = 0xc4
		case codepoint == 0x2248: // ALMOST EQUAL TO
			macbyte = 0xc5
		case codepoint == 0x2206: // INCREMENT
			macbyte = 0xc6
		case codepoint == 0xab: // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
			macbyte = 0xc7
		case codepoint == 0xbb: // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
			macbyte = 0xc8
		case codepoint == 0x2026: // HORIZONTAL ELLIPSIS
			macbyte = 0xc9
		case codepoint == 0xa0: // NO-BREAK SPACE
			macbyte = 0xca
		case codepoint == 0xc0: // LATIN CAPITAL LETTER A WITH GRAVE
			macbyte = 0xcb
		case codepoint == 0xc3: // LATIN CAPITAL LETTER A WITH TILDE
			macbyte = 0xcc
		case codepoint == 0xd5: // LATIN CAPITAL LETTER O WITH TILDE
			macbyte = 0xcd
		case codepoint == 0x152: // LATIN CAPITAL LIGATURE OE
			macbyte = 0xce
		case codepoint == 0x153: // LATIN SMALL LIGATURE OE
			macbyte = 0xcf
		case codepoint == 0x2013: // EN DASH
			macbyte = 0xd0
		case codepoint == 0x2014: // EM DASH
			macbyte = 0xd1
		case codepoint == 0x201c: // LEFT DOUBLE QUOTATION MARK
			macbyte = 0xd2
		case codepoint == 0x201d: // RIGHT DOUBLE QUOTATION MARK
			macbyte = 0xd3
		case codepoint == 0x2018: // LEFT SINGLE QUOTATION MARK
			macbyte = 0xd4
		case codepoint == 0x2019: // RIGHT SINGLE QUOTATION MARK
			macbyte = 0xd5
		case codepoint == 0xf7: // DIVISION SIGN
			macbyte = 0xd6
		case codepoint == 0x25ca: // LOZENGE
			macbyte = 0xd7
		case codepoint == 0xff: // LATIN SMALL LETTER Y WITH DIAERESIS
			macbyte = 0xd8
		case codepoint == 0x178: // LATIN CAPITAL LETTER Y WITH DIAERESIS
			macbyte = 0xd9
		case codepoint == 0x2044: // FRACTION SLASH
			macbyte = 0xda
		case codepoint == 0x20ac: // EURO SIGN
			macbyte = 0xdb
		case codepoint == 0x2039: // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
			macbyte = 0xdc
		case codepoint == 0x203a: // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
			macbyte = 0xdd
		case codepoint == 0xfb01: // LATIN SMALL LIGATURE FI
			macbyte = 0xde
		case codepoint == 0xfb02: // LATIN SMALL LIGATURE FL
			macbyte = 0xdf
		case codepoint == 0x2021: // DOUBLE DAGGER
			macbyte = 0xe0
		case codepoint == 0xb7: // MIDDLE DOT
			macbyte = 0xe1
		case codepoint == 0x201a: // SINGLE LOW-9 QUOTATION MARK
			macbyte = 0xe2
		case codepoint == 0x201e: // DOUBLE LOW-9 QUOTATION MARK
			macbyte = 0xe3
		case codepoint == 0x2030: // PER MILLE SIGN
			macbyte = 0xe4
		case codepoint == 0xc2: // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
			macbyte = 0xe5
		case codepoint == 0xca: // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
			macbyte = 0xe6
		case codepoint == 0xc1: // LATIN CAPITAL LETTER A WITH ACUTE
			macbyte = 0xe7
		case codepoint == 0xcb: // LATIN CAPITAL LETTER E WITH DIAERESIS
			macbyte = 0xe8
		case codepoint == 0xc8: // LATIN CAPITAL LETTER E WITH GRAVE
			macbyte = 0xe9
		case codepoint == 0xcd: // LATIN CAPITAL LETTER I WITH ACUTE
			macbyte = 0xea
		case codepoint == 0xce: // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
			macbyte = 0xeb
		case codepoint == 0xcf: // LATIN CAPITAL LETTER I WITH DIAERESIS
			macbyte = 0xec
		case codepoint == 0xcc: // LATIN CAPITAL LETTER I WITH GRAVE
			macbyte = 0xed
		case codepoint == 0xd3: // LATIN CAPITAL LETTER O WITH ACUTE
			macbyte = 0xee
		case codepoint == 0xd4: // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
			macbyte = 0xef
		case codepoint == 0xf8ff: // apple logo
			macbyte = 0xf0
		case codepoint == 0xd2: // LATIN CAPITAL LETTER O WITH GRAVE
			macbyte = 0xf1
		case codepoint == 0xda: // LATIN CAPITAL LETTER U WITH ACUTE
			macbyte = 0xf2
		case codepoint == 0xdb: // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
			macbyte = 0xf3
		case codepoint == 0xd9: // LATIN CAPITAL LETTER U WITH GRAVE
			macbyte = 0xf4
		case codepoint == 0x131: // LATIN SMALL LETTER DOTLESS I
			macbyte = 0xf5
		case codepoint == 0x2c6: // MODIFIER LETTER CIRCUMFLEX ACCENT
			macbyte = 0xf6
		case codepoint == 0x2dc: // SMALL TILDE
			macbyte = 0xf7
		case codepoint == 0xaf: // MACRON
			macbyte = 0xf8
		case codepoint == 0x2d8: // BREVE
			macbyte = 0xf9
		case codepoint == 0x2d9: // DOT ABOVE
			macbyte = 0xfa
		case codepoint == 0x2da: // RING ABOVE
			macbyte = 0xfb
		case codepoint == 0xb8: // CEDILLA
			macbyte = 0xfc
		case codepoint == 0x2dd: // DOUBLE ACUTE ACCENT
			macbyte = 0xfd
		case codepoint == 0x2db: // OGONEK
			macbyte = 0xfe
		case codepoint == 0x2c7: // CARON
			macbyte = 0xff
		case codepoint == 0x300: // COMBINING GRAVE ACCENT
			switch macstr[len(macstr)-1] {
			case 'A':
				macstr[len(macstr)-1] = 0xcb
			case 'E':
				macstr[len(macstr)-1] = 0xe9
			case 'I':
				macstr[len(macstr)-1] = 0xed
			case 'O':
				macstr[len(macstr)-1] = 0xf1
			case 'U':
				macstr[len(macstr)-1] = 0xf4
			case 'a':
				macstr[len(macstr)-1] = 0x88
			case 'e':
				macstr[len(macstr)-1] = 0x8f
			case 'i':
				macstr[len(macstr)-1] = 0x93
			case 'o':
				macstr[len(macstr)-1] = 0x98
			case 'u':
				macstr[len(macstr)-1] = 0x9d
			default:
				panic("unicode")
			}
			continue
		case codepoint == 0x301: // COMBINING ACUTE ACCENT
			switch macstr[len(macstr)-1] {
			case 'A':
				macstr[len(macstr)-1] = 0xe7
			case 'E':
				macstr[len(macstr)-1] = 0x83
			case 'I':
				macstr[len(macstr)-1] = 0xea
			case 'O':
				macstr[len(macstr)-1] = 0xee
			case 'U':
				macstr[len(macstr)-1] = 0xf2
			case 'a':
				macstr[len(macstr)-1] = 0x87
			case 'e':
				macstr[len(macstr)-1] = 0x8e
			case 'i':
				macstr[len(macstr)-1] = 0x92
			case 'o':
				macstr[len(macstr)-1] = 0x97
			case 'u':
				macstr[len(macstr)-1] = 0x9c
			default:
				panic("unicode")
			}
			continue
		case codepoint == 0x302: // COMBINING CIRCUMFLEX ACCENT
			switch macstr[len(macstr)-1] {
			case 'A':
				macstr[len(macstr)-1] = 0xe5
			case 'E':
				macstr[len(macstr)-1] = 0xe6
			case 'I':
				macstr[len(macstr)-1] = 0xeb
			case 'O':
				macstr[len(macstr)-1] = 0xef
			case 'U':
				macstr[len(macstr)-1] = 0xf3
			case 'a':
				macstr[len(macstr)-1] = 0x89
			case 'e':
				macstr[len(macstr)-1] = 0x90
			case 'i':
				macstr[len(macstr)-1] = 0x94
			case 'o':
				macstr[len(macstr)-1] = 0x99
			case 'u':
				macstr[len(macstr)-1] = 0x9e
			default:
				panic("unicode")
			}
			continue
		case codepoint == 0x303: // COMBINING TILDE
			switch macstr[len(macstr)-1] {
			case 'A':
				macstr[len(macstr)-1] = 0xcc
			case 'N':
				macstr[len(macstr)-1] = 0x84
			case 'O':
				macstr[len(macstr)-1] = 0xcd
			case 'a':
				macstr[len(macstr)-1] = 0x8b
			case 'n':
				macstr[len(macstr)-1] = 0x96
			case 'o':
				macstr[len(macstr)-1] = 0x9b
			default:
				panic("unicode")
			}
			continue
		case codepoint == 0x308: // COMBINING DIAERESIS
			switch macstr[len(macstr)-1] {
			case 'A':
				macstr[len(macstr)-1] = 0x80
			case 'E':
				macstr[len(macstr)-1] = 0xe8
			case 'I':
				macstr[len(macstr)-1] = 0xec
			case 'O':
				macstr[len(macstr)-1] = 0x85
			case 'U':
				macstr[len(macstr)-1] = 0x86
			case 'Y':
				macstr[len(macstr)-1] = 0xd9
			case 'a':
				macstr[len(macstr)-1] = 0x8a
			case 'e':
				macstr[len(macstr)-1] = 0x91
			case 'i':
				macstr[len(macstr)-1] = 0x95
			case 'o':
				macstr[len(macstr)-1] = 0x9a
			case 'u':
				macstr[len(macstr)-1] = 0x9f
			case 'y':
				macstr[len(macstr)-1] = 0xd8
			default:
				panic("unicode")
			}
			continue
		case codepoint == 0x30a: // COMBINING RING ABOVE
			switch macstr[len(macstr)-1] {
			case 'A':
				macstr[len(macstr)-1] = 0x81
			case 'a':
				macstr[len(macstr)-1] = 0x8c
			default:
				panic("unicode")
			}
			continue
		case codepoint == 0x327: // COMBINING CEDILLA
			switch macstr[len(macstr)-1] {
			case 'C':
				macstr[len(macstr)-1] = 0x82
			case 'c':
				macstr[len(macstr)-1] = 0x8d
			default:
				panic("unicode")
			}
			continue
		case codepoint == 0x338: // COMBINING LONG SOLIDUS OVERLAY
			switch macstr[len(macstr)-1] {
			case '=':
				macstr[len(macstr)-1] = 0xad
			default:
				panic("unicode")
			}
			continue
		default:
			panic("unicode")
		}
		macstr = append(macstr, macbyte)
	}

	return macstring(macstr), true
}

func translateByTable(in macstring, table *[256]byte) macstring {
	out := make([]byte, len(in))
	for i := 0; i < len(in); i++ {
		out[i] = table[in[i]]
	}
	return macstring(out)
}

func macStripDiacritics(in macstring) macstring {
	return translateByTable(in, &macStripTab)
}

func macUpper(in macstring) macstring {
	return translateByTable(in, &macUpperTab)
}

// works like _RelString trap
func relString(a, b macstring, caseSens, diacSens bool) uint32 {
	if !diacSens {
		a = macStripDiacritics(a)
		b = macStripDiacritics(b)
	}
	if !caseSens {
		a = macUpper(a)
		b = macUpper(b)
	}

	n := len(a)
	if len(b) < n {
		n = len(b)
	}

	for i := 0; i < n; i++ {
		an := macSortTab[a[i]]
		bn := macSortTab[b[i]]

		if an > bn {
			return 1
		} else if an < bn {
			return 0xffffffff
		}
	}

	if len(a) > len(b) {
		return 1
	} else if len(a) < len(b) {
		return 0xffffffff
	} else {
		return 0
	}
}
