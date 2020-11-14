// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdmisc.h"

#include "nel/misc/events.h"
#include "nel/misc/string_conversion.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {


// ***************************************************************************
// The conversion table
static const CStringConversion<TKey>::CPair stringTable [] =
{
	// BUFFER
	{ "Key0", Key0 },
	{ "Key1", Key1 },
	{ "Key2", Key2 },
	{ "Key3", Key3 },
	{ "Key4", Key4 },
	{ "Key5", Key5 },
	{ "Key6", Key6 },
	{ "Key7", Key7 },
	{ "Key8", Key8 },
	{ "Key9", Key9 },
	{ "KeyA", KeyA },
	{ "KeyB", KeyB },
	{ "KeyC", KeyC },
	{ "KeyD", KeyD },
	{ "KeyE", KeyE },
	{ "KeyF", KeyF },
	{ "KeyG", KeyG },
	{ "KeyH", KeyH },
	{ "KeyI", KeyI },
	{ "KeyJ", KeyJ },
	{ "KeyK", KeyK },
	{ "KeyL", KeyL },
	{ "KeyM", KeyM },
	{ "KeyN", KeyN },
	{ "KeyO", KeyO },
	{ "KeyP", KeyP },
	{ "KeyQ", KeyQ },
	{ "KeyR", KeyR },
	{ "KeyS", KeyS },
	{ "KeyT", KeyT },
	{ "KeyU", KeyU },
	{ "KeyV", KeyV },
	{ "KeyW", KeyW },
	{ "KeyX", KeyX },
	{ "KeyY", KeyY },
	{ "KeyZ", KeyZ },
	{ "KeyLBUTTON", KeyLBUTTON },
	{ "KeyRBUTTON", KeyRBUTTON },
	{ "KeyCANCEL", KeyCANCEL },
	{ "KeyMBUTTON", KeyMBUTTON },
	{ "KeyBACK", KeyBACK },
	{ "KeyTAB", KeyTAB },
	{ "KeyCLEAR", KeyCLEAR },
	{ "KeyRETURN", KeyRETURN },
	{ "KeySHIFT", KeySHIFT },
	{ "KeyCONTROL", KeyCONTROL },
	{ "KeyMENU", KeyMENU },
	{ "KeyPAUSE", KeyPAUSE },
	{ "KeyCAPITAL", KeyCAPITAL },
	{ "KeyKANA", KeyKANA },
	{ "KeyHANGEUL", KeyHANGEUL },
	{ "KeyHANGUL", KeyHANGUL },
	{ "KeyJUNJA", KeyJUNJA },
	{ "KeyFINAL", KeyFINAL },
	{ "KeyHANJA", KeyHANJA },
	{ "KeyKANJI", KeyKANJI },
	{ "KeyESCAPE", KeyESCAPE },
	{ "KeyCONVERT", KeyCONVERT },
	{ "KeyNONCONVERT", KeyNONCONVERT },
	{ "KeyACCEPT", KeyACCEPT },
	{ "KeyMODECHANGE", KeyMODECHANGE },
	{ "KeySPACE", KeySPACE },
	{ "KeyPRIOR", KeyPRIOR },
	{ "KeyNEXT", KeyNEXT },
	{ "KeyEND", KeyEND },
	{ "KeyHOME", KeyHOME },
	{ "KeyLEFT", KeyLEFT },
	{ "KeyUP", KeyUP },
	{ "KeyRIGHT", KeyRIGHT },
	{ "KeyDOWN", KeyDOWN },
	{ "KeySELECT", KeySELECT },
	{ "KeyPRINT", KeyPRINT },
	{ "KeyEXECUTE", KeyEXECUTE },
	{ "KeySNAPSHOT", KeySNAPSHOT },
	{ "KeyINSERT", KeyINSERT },
	{ "KeyDELETE", KeyDELETE },
	{ "KeyHELP", KeyHELP },
	{ "KeyLWIN", KeyLWIN },
	{ "KeyRWIN", KeyRWIN },
	{ "KeyAPPS", KeyAPPS },
	{ "KeyNUMPAD0", KeyNUMPAD0 },
	{ "KeyNUMPAD1", KeyNUMPAD1 },
	{ "KeyNUMPAD2", KeyNUMPAD2 },
	{ "KeyNUMPAD3", KeyNUMPAD3 },
	{ "KeyNUMPAD4", KeyNUMPAD4 },
	{ "KeyNUMPAD5", KeyNUMPAD5 },
	{ "KeyNUMPAD6", KeyNUMPAD6 },
	{ "KeyNUMPAD7", KeyNUMPAD7 },
	{ "KeyNUMPAD8", KeyNUMPAD8 },
	{ "KeyNUMPAD9", KeyNUMPAD9 },
	{ "KeyMULTIPLY", KeyMULTIPLY },
	{ "KeyADD", KeyADD },
	{ "KeySEPARATOR", KeySEPARATOR },
	{ "KeySUBTRACT", KeySUBTRACT },
	{ "KeyDECIMAL", KeyDECIMAL },
	{ "KeyDIVIDE", KeyDIVIDE },
	{ "KeyF1", KeyF1 },
	{ "KeyF2", KeyF2 },
	{ "KeyF3", KeyF3 },
	{ "KeyF4", KeyF4 },
	{ "KeyF5", KeyF5 },
	{ "KeyF6", KeyF6 },
	{ "KeyF7", KeyF7 },
	{ "KeyF8", KeyF8 },
	{ "KeyF9", KeyF9 },
	{ "KeyF10", KeyF10 },
	{ "KeyF11", KeyF11 },
	{ "KeyF12", KeyF12 },
	{ "KeyF13", KeyF13 },
	{ "KeyF14", KeyF14 },
	{ "KeyF15", KeyF15 },
	{ "KeyF16", KeyF16 },
	{ "KeyF17", KeyF17 },
	{ "KeyF18", KeyF18 },
	{ "KeyF19", KeyF19 },
	{ "KeyF20", KeyF20 },
	{ "KeyF21", KeyF21 },
	{ "KeyF22", KeyF22 },
	{ "KeyF23", KeyF23 },
	{ "KeyF24", KeyF24 },
	{ "KeyNUMLOCK", KeyNUMLOCK },
	{ "KeySCROLL", KeySCROLL },
	{ "KeyLSHIFT", KeyLSHIFT },
	{ "KeyRSHIFT", KeyRSHIFT },
	{ "KeyLCONTROL", KeyLCONTROL },
	{ "KeyRCONTROL", KeyRCONTROL },
	{ "KeyLMENU", KeyLMENU },
	{ "KeyRMENU", KeyRMENU },
	{ "KeyMUTE", KeyMUTE },
	{ "KeyPLAYPAUSE", KeyPLAYPAUSE },
	{ "KeyVOLUMEDOWN", KeyVOLUMEDOWN },
	{ "KeyVOLUMEUP", KeyVOLUMEUP },
	{ "KeyCALC", KeyCALC },
	{ "KeySEMICOLON", KeySEMICOLON },
	{ "KeyEQUALS", KeyEQUALS },
	{ "KeyCOMMA", KeyCOMMA },
	{ "KeyDASH", KeyDASH },
	{ "KeyPERIOD", KeyPERIOD },
	{ "KeySLASH", KeySLASH },
	{ "KeyTILDE", KeyTILDE },
	{ "KeyLBRACKET", KeyLBRACKET },
	{ "KeyBACKSLASH", KeyBACKSLASH },
	{ "KeyRBRACKET", KeyRBRACKET },
	{ "KeyAPOSTROPHE", KeyAPOSTROPHE },
	{ "KeyPARAGRAPH", KeyPARAGRAPH },
	{ "KeyOEM_102", KeyOEM_102 },
	{ "KeyPROCESSKEY", KeyPROCESSKEY },
	{ "KeyATTN", KeyATTN },
	{ "KeyCRSEL", KeyCRSEL },
	{ "KeyEXSEL", KeyEXSEL },
	{ "KeyEREOF", KeyEREOF },
	{ "KeyPLAY", KeyPLAY },
	{ "KeyZOOM", KeyZOOM },
	{ "KeyNONAME", KeyNONAME },
	{ "KeyPA1", KeyPA1 },
	{ "KeyOEM_CLEAR", KeyOEM_CLEAR }
};


static	CStringConversion<TKey> KeyConversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  KeyCount);


// ***************************************************************************
TKey				CEventKey::getKeyFromString(const std::string &str)
{
	return KeyConversion.fromString(str);
}

// ***************************************************************************
const std::string	&CEventKey::getStringFromKey(TKey k)
{
	return KeyConversion.toString(k);
}



} // NLMISC
