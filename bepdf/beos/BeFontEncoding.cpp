/*  
	Copyright (C) 2000-2002 Michael Pfeiffer
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <SupportDefs.h>
#include <be/support/UTF8.h>
#include <stdio.h>
#include <string.h>
#include "BeFontEncoding.h"

#define NEED_SYMBOL_ENCODING        0
#define NEED_STANDARD_ENCODING      1
#define NEED_ZAPF_DINGBATS_ENCODING 0

#if NEED_SYMBOL_ENCODING
static BeCharacterEncoding _beSymbolEncoding[] = {
{"Alpha",			-1,	0x391},
{"Beta",			-1,	0x392},
{"Chi",				-1,	0x3a7},
{"Delta",			-1,	0x2206},
{"Epsilon",			-1,	0x395},
{"Eta",				-1,	0x397},
{"Euro",			-1,	0x2208}, // element of
{"Gamma",			-1,	0x393},
{"Ifraktur",		-1,	0x2111},
{"Iota",			-1,	0x399},
{"Kappa",			-1,	0x39a},
{"Lambda",			-1,	0x39b},
{"Mu", 				-1,	0x39c},
{"Nu",				-1,	0x39d},
{"Omega", 			-1,	0x3a9},
{"Omicron", 		-1,	0x39f},
{"Phi", 			-1,	0x3c6},
{"Pi", 				-1,	0x3a0},
{"Psi", 			-1,	0x3a8},
{"Rfraktur", 		-1,	0x211c},
{"Rho", 			-1,	0x3a1},
{"Sigma", 			-1,	0x3a3},
{"Tau", 			-1,	0x3a4},
{"Theta", 			-1,	0x398},
{"Ulsilon1",		-1,	0x3d2},
{"Upsilon", 		-1,	0x3a5},
{"Xi", 				-1,	0x39e},
{"Zeta", 			-1,	0x396},
{"aleph", 			-1,	0x2135},
{"alpha", 			-1,	0x3b1},
{"ampersand", 		-1,	0x26},
{"angle", 			-1,	0x2220},
{"angleleft", 		-1,	0x2329},
{"angleright", 		-1,	0x232a},
{"apple", 			-1,	0xf000}, //
{"approxequal", 	-1,	0x2248},
{"arrowboth", 		-1,	0x2194},
{"arrowdblboth", 	-1,	0x21d4},
{"arrowdbldown", 	-1,	0x21d3},
{"arrowdblleft", 	-1,	0x21d0},
{"arrowdblright", 	-1,	0x21d2},
{"arrowdblup", 		-1,	0x21d1},
{"arrowdown", 		-1,	0x2193},
{"arrowhorizex", 	-1,	0xf0be},
{"arrowleft", 		-1,	0x2190},
{"arrowright", 		-1,	0x2192},
{"arrowup", 		-1,	0x2191},
{"arrowvertex", 	-1,	0xf0bd},
{"asteriskmath", 	-1,	0x2217},
{"bar", 			-1,	0x7c},
{"beta", 			-1,	0x3b2},
{"braceex", 		-1,	0xf0ef},
{"braceleft", 		-1,	0x7b},
{"braceleftbt", 	-1,	0xf0ee},
{"braceleftmid",	-1,	0xf0ed},
{"bracelefttp", 	-1,	0xf0ec},
{"braceright", 		-1,	0x7d},
{"bracerightbt", 	-1,	0xf0fe},
{"bracerightmid", 	-1,	0xf0fd},
{"bracerighttp", 	-1,	0xf0fc},
{"bracketleft", 	-1,	0x5b},
{"bracketleftbt", 	-1,	0xf0eb},
{"bracketleftex", 	-1,	0xf0ea},
{"bracketlefttp", 	-1,	0xf0e9},
{"bracketright", 	-1,	0x5d},
{"bracketrightbt", 	-1,	0xf0fb},
{"bracketrightex", 	-1,	0xf0fa},
{"bracketrighttp", 	-1,	0xf0f9},
{"bullet", 			-1,	0x2022},
{"carriagereturn",	-1,	0x21b5},
{"chi", 			-1,	0x3c7},
{"circlemultiply", 	-1,	0x2297},
{"circleplus", 		-1,	0x2295},
{"club", 			-1,	0x2663},
{"colon", 			-1,	0x3a},
{"comma", 			-1,	0x2c},
{"congruent", 		-1,	0x2245},
{"copyrightsans", 	-1,	0xf0e2},
{"copyrightserif", 	-1,	0xa9},
{"degree",			-1,	0xb0},
{"delta",			-1,	0x3b4},
{"diamond",			-1,	0x2666},
{"divide",			-1,	0xf7},
{"dotmath",			-1,	0x22c5},
{"eight",			-1,	0x38},
{"element",			-1,	0x2208},
{"ellipsis",		-1,	0x2026},
{"emptyset",		-1,	0x2205},
{"epsilon",			-1,	0x3b5},
{"equal",			-1,	0x3d},
{"equivalence",		-1,	0x2261},
{"eta",				-1,	0x3b7},
{"exclam",			-1,	0x21},
{"existential",		-1,	0x2203},
{"five",			-1,	0x35},
{"florin",			-1,	0x192},
{"four",			-1,	0x34},
{"fraction",		-1,	0x2215},
{"gamma",			-1,	0x3b3},
{"gradient",		-1,	0x2207},
{"greater",			-1,	0x3e},
{"greaterequal",	-1,	0x2265},
{"heart",			-1,	0x2665},
{"infinity",		-1,	0x221e},
{"integral",		-1,	0x222b},
{"integralbt",		-1,	0x2321},
{"integralex",		-1,	0xf0f4}, // ?
{"integraltp",		-1,	0x2320},
{"intersection",	-1,	0x2229},
{"iota",			-1,	0x3b9},
{"kappa",			-1,	0x3ba},
{"lambda",			-1,	0x3bb},
{"less",			-1,	0x3c},
{"lessequal",		-1,	0x2264},
{"logicaland",		-1,	0x2227},
{"logicalnot",		-1,	0xac},
{"logicalor",		-1,	0x2228},
{"lozenge",			-1,	0x25ca},
{"minus",			-1,	0x2212},
{"minute",			-1,	0x2032},
{"mu",				-1,	0xb5},
{"multiply",		-1,	0xd7},
{"nine",			-1,	0x39},
{"notelement",		-1,	0x2209},
{"notequal",		-1,	0x2260},
{"notsubset",		-1,	0x2284},
{"nu",				-1,	0x3bd},
{"numbersign",		-1,	0x23},
{"omega",			-1,	0x3c9},
{"omega1",			-1,	0x3d6},
{"omicron",			-1,	0x3bf},
{"one",				-1,	0x31},
{"parenleft",		-1,	0x28},
{"parenleftbt",		-1,	0xf0e8},
{"parenleftex",		-1,	0xf0e7},
{"parenlefttp",		-1,	0xf0e6},
{"parenright",		-1,	0x29},
{"parenrightbt",	-1,	0xf0f8},
{"parenrightex",	-1,	0xf0f7},
{"parenrighttp",	-1,	0xf0f6},
{"partialdiff",		-1,	0x2202},
{"percent",			-1,	0x25},
{"period",			-1,	0x2e},
{"perpendicular",	-1,	0x22a5},
{"phi",				-1,	0x3c6},
{"phil",			-1,	0x3d5},
{"pi",				-1,	0x3c0},
{"plus",			-1,	0x2b},
{"plusminus",		-1,	0xb1},
//{"prime",			-1,	0x3c6}, // cmsy char name for phi
{"product",			-1,	0x220f},
{"productsuperset",	-1,	0x2283},
{"propersubset",	-1,	0x2282},
{"proportional",	-1,	0x221d},
{"psi",				-1,	0x3c8},
{"question",		-1,	0x3f},
{"radical",			-1,	0x221a},
{"radicalex",		-1,	0x203e},
{"reflexsubset",	-1,	0x2286},
{"reflexsuperset",	-1,	0x2287},
{"registersans",	-1,	0xf0e2},
{"registerserif",	-1,	0xae},
{"rho",				-1,	0x3c1},
{"second",			-1,	0x2033},
{"semicolon",		-1,	0x3b},
{"seven",			-1,	0x37},
{"sigma",			-1,	0x3c3},
{"sigmal",			-1,	0x3c2},
{"similar",			-1,	0x223c},
{"six",				-1,	0x36},
{"slash",			-1,	0x2f},
{"space",			-1,	0x20},
{"spade",			-1,	0x2660},
{"suchthat",		-1,	0x220b},
{"summation",		-1,	0x2211},
{"tau",				-1,	0x3c4},
{"therefore",		-1,	0x2234},
{"theta",			-1,	0x3b8},
{"theta1",			-1,	0x3d1},
{"three",			-1,	0x33},
{"trademarksans",	-1,	0xf0e4},
{"trademarkserif",	-1,	0x2122},
{"two",				-1,	0x32},
{"underscore",		-1,	0x5f},
{"union",			-1,	0x222a},
{"universal",		-1,	0x2200},
{"upsilon",			-1,	0x3c5},
{"weierstrass",		-1,	0x2218},
{"xi",				-1,	0x3be},
{"zero",			-1,	0x30},
{"zeta",			-1,	0x3b6},
{NULL}
};

BeFontEncoding gSymbolEncoding(_beSymbolEncoding);

#endif

#if NEED_STANDARD_ENCODING
// code is from pdf doc encoding
static BeCharacterEncoding _beStandardEncoding[] = {
{"A",				65, 65},
{"AE",				198,	198},
{"Aacute",			193,	193},
{"Acircumflex",		194,	194},
{"Adieresis",		196,	196},
{"Agrave",			192,	192},
{"Aring",			197,	197},
{"Atilde",			195,	195},
{"B",				66,	66},
{"C",				67,	67},
{"Ccedilla",		199,	199},
{"D",				68,	68},
{"E",				69,	69},
{"Eacute",			201,	201},
{"Ecircumflex",		202,	202},
{"Edieresis",		203,	203},
{"Egrave",			200,	200},
{"Eth",				208,	208},
{"Euro",			160,	0x2208},//
{"F",				70,	70},
{"G",				71,	71},
{"H",				72,	72},
{"I",				73,	73},
{"Iacute",			205,	205},
{"Icircumflex",		206,	206},
{"Idieresis",		207,	207},
{"Igrave",			204,	204},
{"J",				74,	74},
{"K",				75,	75},
{"L",				76,	76},
{"Lslash",			149,	0x141},//
{"M",				77,	77},
{"N",				78,	78},
{"Ntilde",			209,	209},
{"O",				79,	79},
{"OE",				150,	0x152},
{"Oacute",			211,	211},
{"Ocircumflex",		212,	212},
{"Odieresis",		214,	214},
{"Ograve",			210,	210},
{"Oslash",			216,	216},
{"Otilde",			213,	213},
{"P",				80,	80},
{"Q",				81,	81},
{"R",				82,	82},
{"S",				83,	83},
{"Scaron",			151,	0x160},
{"T",				84,	84},
{"Thorn",			222,	222},
{"U",				85,	85},
{"Uacute",			218,	218},
{"Ucircumflex",		219,	219},
{"Udieresis",		220,	220},
{"Ugrave",			217,	217},
{"V",				86,	86},
{"W",				87,	87},
{"X",				88,	88},
{"Y",				89,	89},
{"Yacute",			221,	221},
{"Ydieresis",		152,	0x178},
{"Z",				90,	90},
{"Zcaron",			153,	0x17d},
{"a",				97,	97},
{"aacute",			225,	225},
{"acircumflex",		226,	226},
{"acute",			180,	180},
{"adieresis",		228,	228},
{"ae",				230,	230},
{"agrave",			224,	224},
{"ampersand",		38,	38},
{"aring",			229,	229},
{"asciicircum",		94,	94},
{"asciitilde",		126,	126},
{"asterisk",		42,	42},
{"at",				64,	64},
{"atilde",			227,	227},
{"b",				98,	98},
{"backslash",		92,	92},
{"bar",				124,	124},
{"braceleft",		123,	123},
{"braceright",		125,	125},
{"bracketleft",		91,	91},
{"bracketright",	93,	93},
{"breve",			24,	0x2d8},
{"brokenbar",		166,	166},
{"bullet",			128,	0x2022},
{"c",				99,	99},
{"caron",			25,	0x2c7},
{"ccedilla",		231,	231},
{"cedilla",			184,	184},
{"cent",			162,	162},
{"circumflex",		26,	0x2c6},
{"colon",			58,	58},
{"comma",			44,	44},
{"copyright",		169,	169},
{"currency",		164,	164},
{"d",				100,	100},
{"dagger",			129,	0x2020},
{"daggerdbl",		130,	0x2021},
{"degree",			176,	176},
{"dieresis",		168,	168},
{"divide",			247,	247},
{"dollar",			36,	36},
{"dotaccent",		27,	0x2d9},
{"dotlessi",		154,	0x131},
{"e",				101,	101},
{"eacute",			233,	233},
{"ecircumflex",		234,	234},
{"edieresis",		235,	235},
{"egrave",			232,	232},
{"eight",			56,	56},
{"ellipsis",		131,	0x2026},
{"emdash",			132,	0x2014},
{"endash",			133,	0x2013},
{"equal",			61,	61},
{"eth",				240,	240},
{"exclam",			33,	33},
{"exclamdown",		161,	161},
{"f",				102,	102},
{"fi",				147, -1},//
{"five",			53,	53},
{"fl",				148, -1},//
{"florin",			134,	0x192},
{"four",			52,	52},
{"fraction",		135,	0x2215},
{"g",				103,	103},
{"germandbls",		223,	223},
{"grave",			96,	96},
{"greater",			62,	62},
{"guillemotleft",	171,	171},
{"guillemotright",	187,	187},
{"guilsinglleft",	136,	0x2039},
{"guilsinglright",	137,	0x203a},
{"h",				104,	104},
{"hungarumlaut",	28,	0x2dd},
{"hyphen",			45,	173},
{"i",				105,	105},
{"iacute",			237,	237},
{"icircumflex",		238,	238},
{"idieresis",		239,	239},
{"igrave",			236,	236},
{"j",				106,	106},
{"k",				107,	107},
{"l",				108,	108},
{"less",			60,	60},
{"logicalnot",		172,	172},
{"lslash",			155,	0x142},
{"m",				109,	109},
{"macron",			175,	175},
{"minus",			138,	45},
{"mu",				181,	181},
{"multiply",		215,	215},
{"n",				110,	110},
{"nine",			57,	57},
{"ntilde",			241,	241},
{"numbersign",		35,	35},
{"o",				111,	111},
{"oacute",			243,	243},
{"ocircumflex",		244,	244},
{"odieresis",		246,	246},
{"oe",				156,	0x153},
{"ogonek",			29,	0x2db},
{"ograve",			242,	242},
{"one",				49,	49},
{"onehalf",			189,	189},
{"onequarter",		188,	188},
{"onesuperior",		185,	185},
{"ordfeminine",		170,	170},
{"ordmasculine",	186,	186},
{"oslash",			248,	248},
{"otilde",			245,	245},
{"p",				112,	112},
{"paragraph",		182,	182},
{"parenleft",		40,	40},
{"parenright",		41,	41},
{"percent",			37,	37},
{"period",			46,	46},
{"periodcentered",	183,	183},
{"perthousand",		139,	0x2030},
{"plus",			43,	43},
{"plusminus",		177,	177},
{"q",				113,	113},
{"question",		63,	63},
{"questiondown",	191,	191},
{"quotedbl",		34,	34},
{"quotedblbase",	140,	0x201e},
{"quotedblleft",	141,	0x201c},
{"quotedblright",	142,	0x201d},
{"quoteleft",		143,	0x2018},
{"quoteright",		144,	0x2019},
{"quotesinglbase",	145,	0x201a},
{"quotesingle",		39,	0x2019},
{"r",				114,	114},
{"registered",		174,	174},
{"ring",			30,	0x2da},
{"s",				115,	115},
{"scaron",			157,	0x161},
{"section",			167,	167},
{"semicolon",		59,	59},
{"seven",			55,	55},
{"six",				54,	54},
{"slash",			47,	47},
{"space",			32,	32},
{"sterling",		163,	163},
{"t",				116,	116},
{"thorn",			254,	254},
{"three",			51,	51},
{"threequarters",	190,	190},
{"threesuperior",	179,	179},
{"tilde",			31,	0x2dc},
{"trademark",		146,	0x2122},
{"two",				50,	50},
{"twosuperior",		178,	178},
{"u",				117,	117},
{"uacute",			250,	250},
{"ucircumflex",		251,	251},
{"udieresis",		252,	252},
{"ugrave",			249,	249},
{"underscore",		95,	95},
{"v",				118,	118},
{"w",				119,	119},
{"x",				120,	120},
{"y",				121,	121},
{"yacute",			253,	253},
{"ydieresis",		255,	255},
{"yen",				165,	165},
{"z",				122,	122},
{"zcaron",			158,	0x17e},
{"zero",			48,	48},
{NULL}
//
//"no-break-space",	,	160
};

BeFontEncoding gStandardEncoding(_beStandardEncoding);

#endif

#if NEED_ZAPF_DINGBATS_ENCODING
static BeCharacterEncoding _beZapfdingbatsEncoding[] = {
{"a1", -1,	0x21}, // UPPER BLADE SCISSORS
{"a10", -1,	0x41}, // STAR OF DAVID
{"a100", -1,	0x7E}, // HEAVY DOUBLE COMMA QUOTATION MARK ORNAMENT
{"a101", -1,	0xA1}, // CURVED STEM PARAGRAPH SIGN ORNAMENT
{"a102", -1,	0xA2}, // HEAVY EXCLAMATION MARK ORNAMENT
{"a103", -1,	0xA3}, // HEAVY HEART EXCLAMATION MARK ORNAMENT
{"a104", -1,	0xA4}, // HEAVY BLACK HEART
{"a105", -1,	0x30}, // UPPER RIGHT PENCIL
{"a106", -1,	0xA5}, // ROTATED HEAVY BLACK HEART BULLET
{"a107", -1,	0xA6}, // FLORAL HEART
{"a108", -1,	0xA7}, // ROTATED FLORAL HEART BULLET
{"a109", -1,	0xAB}, // BLACK SPADE SUIT
{"a11", -1,	0x2A}, // BLACK RIGHT POINTING INDEX
{"a110", -1,	0xAA}, // BLACK HEART SUIT
{"a111", -1,	0xA9}, // BLACK DIAMOND SUIT
{"a112", -1,	0xA8}, // BLACK CLUB SUIT
{"a117", -1,	0x29}, // ENVELOPE
{"a118", -1,	0x28}, // AIRPLANE
{"a119", -1,	0x27}, // TAPE DRIVE
{"a12", -1,	0x2B}, // WHITE RIGHT POINTING INDEX
{"a120", -1,	0xAC}, // CIRCLED DIGIT ONE
{"a121", -1,	0xAD}, // CIRCLED DIGIT TWO
{"a122", -1,	0xAE}, // CIRCLED DIGIT THREE
{"a123", -1,	0xAF}, // CIRCLED DIGIT FOUR
{"a124", -1,	0xB0}, // CIRCLED DIGIT FIVE
{"a125", -1,	0xB1}, // CIRCLED DIGIT SIX
{"a126", -1,	0xB2}, // CIRCLED DIGIT SEVEN
{"a127", -1,	0xB3}, // CIRCLED DIGIT EIGHT
{"a128", -1,	0xB4}, // CIRCLED DIGIT NINE
{"a129", -1,	0xB5}, // CIRCLED NUMBER TEN
{"a13", -1,	0x2C}, // VICTORY HAND
{"a130", -1,	0xB6}, // DINGBAT NEGATIVE CIRCLED DIGIT ONE
{"a131", -1,	0xB7}, // DINGBAT NEGATIVE CIRCLED DIGIT TWO
{"a132", -1,	0xB8}, // DINGBAT NEGATIVE CIRCLED DIGIT THREE
{"a133", -1,	0xB9}, // DINGBAT NEGATIVE CIRCLED DIGIT FOUR
{"a134", -1,	0xBA}, // DINGBAT NEGATIVE CIRCLED DIGIT FIVE
{"a135", -1,	0xBB}, // DINGBAT NEGATIVE CIRCLED DIGIT SIX
{"a136", -1,	0xBC}, // DINGBAT NEGATIVE CIRCLED DIGIT SEVEN
{"a137", -1,	0xBD}, // DINGBAT NEGATIVE CIRCLED DIGIT EIGHT
{"a138", -1,	0xBE}, // DINGBAT NEGATIVE CIRCLED DIGIT NINE
{"a139", -1,	0xBF}, // DINGBAT NEGATIVE CIRCLED NUMBER TEN
{"a14", -1,	0x2D}, // WRITING HAND
{"a140", -1,	0xC0}, // DINGBAT CIRCLED SANS-SERIF DIGIT ONE
{"a141", -1,	0xC1}, // DINGBAT CIRCLED SANS-SERIF DIGIT TWO
{"a142", -1,	0xC2}, // DINGBAT CIRCLED SANS-SERIF DIGIT THREE
{"a143", -1,	0xC3}, // DINGBAT CIRCLED SANS-SERIF DIGIT FOUR
{"a144", -1,	0xC4}, // DINGBAT CIRCLED SANS-SERIF DIGIT FIVE
{"a145", -1,	0xC5}, // DINGBAT CIRCLED SANS-SERIF DIGIT SIX
{"a146", -1,	0xC6}, // DINGBAT CIRCLED SANS-SERIF DIGIT SEVEN
{"a147", -1,	0xC7}, // DINGBAT CIRCLED SANS-SERIF DIGIT EIGHT
{"a148", -1,	0xC8}, // DINGBAT CIRCLED SANS-SERIF DIGIT NINE
{"a149", -1,	0xC9}, // DINGBAT CIRCLED SANS-SERIF NUMBER TEN
{"a15", -1,	0x2E}, // LOWER RIGHT PENCIL
{"a150", -1,	0xCA}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT ONE
{"a151", -1,	0xCB}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT TWO
{"a152", -1,	0xCC}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT THREE
{"a153", -1,	0xCD}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FOUR
{"a154", -1,	0xCE}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FIVE
{"a155", -1,	0xCF}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SIX
{"a156", -1,	0xD0}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SEVEN
{"a157", -1,	0xD1}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT EIGHT
{"a158", -1,	0xD2}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT NINE
{"a159", -1,	0xD3}, // DINGBAT NEGATIVE CIRCLED SANS-SERIF NUMBER TEN
{"a16", -1,	0x2F}, // PENCIL
{"a160", -1,	0xD4}, // HEAVY WIDE-HEADED RIGHTWARDS ARROW
{"a161", -1,	0xD5}, // RIGHTWARDS ARROW
{"a162", -1,	0xE3}, // THREE-D BOTTOM-LIGHTED RIGHTWARDS ARROWHEAD
{"a163", -1,	0xD6}, // LEFT RIGHT ARROW
{"a164", -1,	0xD7}, // UP DOWN ARROW
{"a165", -1,	0xD9}, // HEAVY RIGHTWARDS ARROW
{"a166", -1,	0xDB}, // DRAFTING POINT RIGHTWARDS ARROW
{"a167", -1,	0xDC}, // HEAVY ROUND-TIPPED RIGHTWARDS ARROW
{"a168", -1,	0xDD}, // TRIANGLE-HEADED RIGHTWARDS ARROW
{"a169", -1,	0xDE}, // HEAVY TRIANGLE-HEADED RIGHTWARDS ARROW
{"a17", -1,	0x31}, // WHITE NIB
{"a170", -1,	0xDF}, // DASHED TRIANGLE-HEADED RIGHTWARDS ARROW
{"a171", -1,	0xE0}, // HEAVY DASHED TRIANGLE-HEADED RIGHTWARDS ARROW
{"a172", -1,	0xE1}, // BLACK RIGHTWARDS ARROW
{"a173", -1,	0xE2}, // THREE-D TOP-LIGHTED RIGHTWARDS ARROWHEAD
{"a174", -1,	0xE4}, // BLACK RIGHTWARDS ARROWHEAD
{"a175", -1,	0xE5}, // HEAVY BLACK CURVED DOWNWARDS AND RIGHTWARDS ARROW
{"a176", -1,	0xE6}, // HEAVY BLACK CURVED UPWARDS AND RIGHTWARDS ARROW
{"a177", -1,	0xE7}, // SQUAT BLACK RIGHTWARDS ARROW
{"a178", -1,	0xE8}, // HEAVY CONCAVE-POINTED BLACK RIGHTWARDS ARROW
{"a179", -1,	0xE9}, // RIGHT-SHADED WHITE RIGHTWARDS ARROW
{"a18", -1,	0x32}, // BLACK NIB
{"a180", -1,	0xEB}, // BACK-TILTED SHADOWED WHITE RIGHTWARDS ARROW
{"a181", -1,	0xED}, // HEAVY LOWER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
{"a182", -1,	0xEF}, // NOTCHED LOWER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
{"a183", -1,	0xF2}, // CIRCLED HEAVY WHITE RIGHTWARDS ARROW
{"a184", -1,	0xF3}, // WHITE-FEATHERED RIGHTWARDS ARROW
{"a185", -1,	0xF5}, // BLACK-FEATHERED RIGHTWARDS ARROW
{"a186", -1,	0xF8}, // HEAVY BLACK-FEATHERED RIGHTWARDS ARROW
{"a187", -1,	0xFA}, // TEARDROP-BARBED RIGHTWARDS ARROW
{"a188", -1,	0xFB}, // HEAVY TEARDROP-SHANKED RIGHTWARDS ARROW
{"a189", -1,	0xFC}, // WEDGE-TAILED RIGHTWARDS ARROW
{"a19", -1,	0x33}, // CHECK MARK
{"a190", -1,	0xFD}, // HEAVY WEDGE-TAILED RIGHTWARDS ARROW
{"a191", -1,	0xFE}, // OPEN-OUTLINED RIGHTWARDS ARROW
{"a192", -1,	0xDA}, // HEAVY NORTH EAST ARROW
{"a193", -1,	0xEA}, // LEFT-SHADED WHITE RIGHTWARDS ARROW
{"a194", -1,	0xF6}, // BLACK-FEATHERED NORTH EAST ARROW
{"a195", -1,	0xF9}, // HEAVY BLACK-FEATHERED NORTH EAST ARROW
{"a196", -1,	0xD8}, // HEAVY SOUTH EAST ARROW
{"a197", -1,	0xF4}, // BLACK-FEATHERED SOUTH EAST ARROW
{"a198", -1,	0xF7}, // HEAVY BLACK-FEATHERED SOUTH EAST ARROW
{"a199", -1,	0xEC}, // FRONT-TILTED SHADOWED WHITE RIGHTWARDS ARROW
{"a2", -1,	0x22}, // BLACK SCISSORS
{"a20", -1,	0x34}, // HEAVY CHECK MARK
{"a200", -1,	0xEE}, // HEAVY UPPER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
{"a201", -1,	0xF1}, // NOTCHED UPPER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
{"a202", -1,	0x23}, // LOWER BLADE SCISSORS
{"a203", -1,	0x70}, // UPPER RIGHT DROP-SHADOWED WHITE SQUARE
{"a204", -1,	0x72}, // UPPER RIGHT SHADOWED WHITE SQUARE
{"a205", -1,	0x86}, // HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT
{"a206", -1,	0x88}, // HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT
{"a21", -1,	0x35}, // MULTIPLICATION X
{"a22", -1,	0x36}, // HEAVY MULTIPLICATION X
{"a23", -1,	0x37}, // BALLOT X
{"a24", -1,	0x38}, // HEAVY BALLOT X
{"a25", -1,	0x39}, // OUTLINED GREEK CROSS
{"a26", -1,	0x3A}, // HEAVY GREEK CROSS
{"a27", -1,	0x3B}, // OPEN CENTRE CROSS
{"a28", -1,	0x3C}, // HEAVY OPEN CENTRE CROSS
{"a29", -1,	0x42}, // FOUR TEARDROP-SPOKED ASTERISK
{"a3", -1,	0x24}, // WHITE SCISSORS
{"a30", -1,	0x43}, // FOUR BALLOON-SPOKED ASTERISK
{"a31", -1,	0x44}, // HEAVY FOUR BALLOON-SPOKED ASTERISK
{"a32", -1,	0x45}, // FOUR CLUB-SPOKED ASTERISK
{"a33", -1,	0x46}, // BLACK FOUR POINTED STAR
{"a34", -1,	0x47}, // WHITE FOUR POINTED STAR
{"a35", -1,	0x48}, // BLACK STAR
{"a36", -1,	0x49}, // STRESS OUTLINED WHITE STAR
{"a37", -1,	0x4A}, // CIRCLED WHITE STAR
{"a38", -1,	0x4B}, // OPEN CENTRE BLACK STAR
{"a39", -1,	0x4C}, // BLACK CENTRE WHITE STAR
{"a4", -1,	0x25}, // BLACK TELEPHONE
{"a40", -1,	0x4D}, // OUTLINED BLACK STAR
{"a41", -1,	0x4E}, // HEAVY OUTLINED BLACK STAR
{"a42", -1,	0x4F}, // PINWHEEL STAR
{"a43", -1,	0x50}, // SHADOWED WHITE STAR
{"a44", -1,	0x51}, // HEAVY ASTERISK
{"a45", -1,	0x52}, // OPEN CENTRE ASTERISK
{"a46", -1,	0x53}, // EIGHT SPOKED ASTERISK
{"a47", -1,	0x54}, // EIGHT POINTED BLACK STAR
{"a48", -1,	0x55}, // EIGHT POINTED PINWHEEL STAR
{"a49", -1,	0x56}, // SIX POINTED BLACK STAR
{"a5", -1,	0x26}, // TELEPHONE LOCATION SIGN
{"a50", -1,	0x57}, // EIGHT POINTED RECTILINEAR BLACK STAR
{"a51", -1,	0x58}, // HEAVY EIGHT POINTED RECTILINEAR BLACK STAR
{"a52", -1,	0x59}, // TWELVE POINTED BLACK STAR
{"a53", -1,	0x5A}, // SIXTEEN POINTED ASTERISK
{"a54", -1,	0x5B}, // TEARDROP-SPOKED ASTERISK
{"a55", -1,	0x5C}, // OPEN CENTRE TEARDROP-SPOKED ASTERISK
{"a56", -1,	0x5D}, // HEAVY TEARDROP-SPOKED ASTERISK
{"a57", -1,	0x5E}, // SIX PETALLED BLACK AND WHITE FLORETTE
{"a58", -1,	0x5F}, // BLACK FLORETTE
{"a59", -1,	0x60}, // WHITE FLORETTE
{"a6", -1,	0x3D}, // LATIN CROSS
{"a60", -1,	0x61}, // EIGHT PETALLED OUTLINED BLACK FLORETTE
{"a61", -1,	0x62}, // CIRCLED OPEN CENTRE EIGHT POINTED STAR
{"a62", -1,	0x63}, // HEAVY TEARDROP-SPOKED PINWHEEL ASTERISK
{"a63", -1,	0x64}, // SNOWFLAKE
{"a64", -1,	0x65}, // TIGHT TRIFOLIATE SNOWFLAKE
{"a65", -1,	0x66}, // HEAVY CHEVRON SNOWFLAKE
{"a66", -1,	0x67}, // SPARKLE
{"a67", -1,	0x68}, // HEAVY SPARKLE
{"a68", -1,	0x69}, // BALLOON-SPOKED ASTERISK
{"a69", -1,	0x6A}, // EIGHT TEARDROP-SPOKED PROPELLER ASTERISK
{"a7", -1,	0x3E}, // SHADOWED WHITE LATIN CROSS
{"a70", -1,	0x6B}, // HEAVY EIGHT TEARDROP-SPOKED PROPELLER ASTERISK
{"a71", -1,	0x6C}, // BLACK CIRCLE
{"a72", -1,	0x6D}, // SHADOWED WHITE CIRCLE
{"a73", -1,	0x6E}, // BLACK SQUARE
{"a74", -1,	0x6F}, // LOWER RIGHT DROP-SHADOWED WHITE SQUARE
{"a75", -1,	0x71}, // LOWER RIGHT SHADOWED WHITE SQUARE
{"a76", -1,	0x73}, // BLACK UP-POINTING TRIANGLE
{"a77", -1,	0x74}, // BLACK DOWN-POINTING TRIANGLE
{"a78", -1,	0x75}, // BLACK DIAMOND
{"a79", -1,	0x76}, // BLACK DIAMOND MINUS WHITE X
{"a8", -1,	0x3F}, // OUTLINED LATIN CROSS
{"a81", -1,	0x77}, // RIGHT HALF BLACK CIRCLE
{"a82", -1,	0x78}, // LIGHT VERTICAL BAR
{"a83", -1,	0x79}, // MEDIUM VERTICAL BAR
{"a84", -1,	0x7A}, // HEAVY VERTICAL BAR
{"a85", -1,	0x87}, // HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT
{"a86", -1,	0x89}, // HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT
{"a87", -1,	0x8A}, // LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT
{"a88", -1,	0x8B}, // LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT
{"a89", -1,	0x80}, // MEDIUM LEFT PARENTHESIS ORNAMENT
{"a9", -1,	0x40}, // MALTESE CROSS
{"a90", -1,	0x81}, // MEDIUM RIGHT PARENTHESIS ORNAMENT
{"a91", -1,	0x84}, // MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT
{"a92", -1,	0x85}, // MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT
{"a93", -1,	0x82}, // MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT
{"a94", -1,	0x83}, // MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT
{"a95", -1,	0x8C}, // MEDIUM LEFT CURLY BRACKET ORNAMENT
{"a96", -1,	0x8D}, // MEDIUM RIGHT CURLY BRACKET ORNAMENT
{"a97", -1,	0x7B}, // HEAVY SINGLE TURNED COMMA QUOTATION MARK ORNAMENT
{"a98", -1,	0x7C}, // HEAVY SINGLE COMMA QUOTATION MARK ORNAMENT
{"a99", -1,	0x7D}, // HEAVY DOUBLE TURNED COMMA QUOTATION MARK ORNAMENT
{"space", -1,	0x20}, // SPACE
{NULL}
};

BeFontEncoding gZapfdingbatsEncoding(_beZapfdingbatsEncoding);

#endif 

static const char *emptyString = "";

uint16 BeFontEncoding::toUnicode(const char *string) {
	int32 srcLength = strlen(string), length = 2, state = 0;
	uint16 ch;
	convert_from_utf8(B_UNICODE_CONVERSION,
			string,
			&srcLength,
			(char*)&ch,
			&length,
			&state);
	if (length == 0)
		return 0;
	else
		return ch;
}

int32 BeFontEncoding::toUtf8(uint16 unicode, char *string) {
	int32 srcLength = 2, length = 4, state = 0;
	char s[2];

	s[0] = unicode / 256;
	s[1] = unicode % 256;

	convert_to_utf8(B_UNICODE_CONVERSION, 
		s, 
		&srcLength,
		string, 
		&length, 
		&state);
		
	string[length] = 0;
	return length;
}

BeFontEncoding::BeFontEncoding(BeCharacterEncoding *encoding) {
	this->encoding = encoding;
	BeCharacterEncoding *enc = encoding;
	size = 0;
	for (int16 i = 0; i <= 255; i++) {
		map[i][0] = ' '; map[i][1] = '\0';
	}
	while (enc->name != NULL) {
		if (enc->unicode == -1) {
			strcpy(enc->utf8, enc->name);
		} else {
			toUtf8(enc->unicode, enc->utf8);
		}
		if ((enc->code >= 0) && (enc->code <= 255)) {
			strcpy(map[enc->code], enc->utf8);
		}
		enc ++;	size ++;
	}	
}

int32 BeFontEncoding::getIndex(const char *name) {
int16 min = 0, max = size - 1, k, current;
	while (min <= max) {
		current = (min + max) / 2;
		k = strcmp(name, encoding[current].name);
		if (k < 0) {
			max = current - 1;
		} else if (k > 0) {
			min = current + 1;
		} else {
			return current;
		}
	}
	return -1;
}

const char * BeFontEncoding::getUtf8(const char *name) {
int32 i = getIndex(name);
	if (i != -1) {
		return encoding[i].utf8;
	} else {
		return emptyString;
	}
}

const char *BeFontEncoding::getUtf8At(int32 i) {
	if ((i >= 0) && (i < size)) {
		return encoding[i].utf8;
	} else {
		return emptyString;
	}
};

