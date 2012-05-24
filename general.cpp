// BatchResourceUpdater: program for automated reading, writing, and removing resources from pe-files
// Copyright (C) 2012  Jeffrey Bush  jeff@coderforlife.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "general.h"

using namespace System;

bool shouldSave(bool exists, PE::Overwrite overwrite) {
	return (overwrite == PE::ALWAYS) || (exists && overwrite == PE::ONLY) || (!exists && overwrite == PE::NEVER);
}

String ^getDisplayFileSize(DWORD size) {
	if (size < 1024) {
		return size+L"B";
	} else if (size < 1024*1024) {
		return String::Format(L"{0:F3}KB", size/1024.0);
	} else {
		return String::Format(L"{0:F3}MB", size/1048576.0);
	}
}

String ^LastErrorString() {
	DWORD err = GetLastError();
	String ^str;
	LPTSTR s;
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (LPTSTR)&s, 0, NULL)) {
		LPTSTR p = _tcsrchr(s, _T('\r'));
		if (p != NULL) { *p = _T('\0'); }
		str = (gcnew String(s))+String::Format(L" [0x{0:X} ({0:D})]", err, LOWORD(err));
		LocalFree(s);
	} else {
		str = String::Format(L"Unknown error: 0x{0:X} ({0:D})", err, LOWORD(err));
	}
	return str;
}

void ReportLastError(String ^s) {
	ReportLastError(s, false);
}
void ReportLastError(String ^s, bool warning) {
	Console::Error->WriteLine((warning ? L"* Warning: " : L"! Error: ")+s+LastErrorString());
}

LPCWSTR convertToId(String ^s)
{
	String ^S = s->Trim();
	UInt16 x;
	return (UInt16::TryParse((S[0] == L'#') ? S->Substring(1) : S, x)) ? MAKEINTRESOURCE(x) : as_native(s);
}

LPCWSTR getBuiltInResourceType(String ^s) {
	String ^S = s->Trim();
	UInt16 x;
	if (UInt16::TryParse((S[0] == L'#') ? S->Substring(1) : S, x)) {
		return MAKEINTRESOURCE(x);
	}
	S = s->Replace(L" ", L"");
	if (S == L"CURSOR")			return RT_CURSOR;
	if (S == L"BITMAP")			return RT_BITMAP;
	if (S == L"ICON")			return RT_ICON;
	if (S == L"MENU")			return RT_MENU;
	if (S == L"DIALOG")			return RT_DIALOG;
	if (S == L"STRING" || S == L"STRINGTABLE")				return RT_STRING;
	if (S == L"FONTDIR")		return RT_FONTDIR;
	if (S == L"FONT")			return RT_FONT;
	if (S == L"ACCELERATOR" || S == L"ACCELERATORTABLE")	return RT_ACCELERATOR;
	if (S == L"RCDATA")			return RT_RCDATA;
	if (S == L"MESSAGETABLE")	return RT_MESSAGETABLE;
	if (S == L"ICONGROUP" || S == L"GROUPICON")				return RT_GROUP_ICON;
	if (S == L"CURSORGROUP" || S == L"GROUPCURSOR")			return RT_GROUP_CURSOR;
	if (S == L"VERSION" || S == L"VERSIONINFO")				return RT_VERSION;
	if (S == L"DLGINCLUDE")		return RT_DLGINCLUDE;
	if (S == L"VXD")			return RT_VXD;
	if (S == L"PLUGPLAY" || S == L"PLUGANDPLAY" || S == L"PLUG&PLAY")		return RT_PLUGPLAY;
	if (S == L"ANICURSOR" || S == L"ANIMATEDCURSOR")		return RT_ANICURSOR;
	if (S == L"ANIICON" || S == L"ANIMATEDICON")			return RT_ANIICON;
	if (S == L"HTML")			return RT_HTML;
	if (S == L"MANIFEST")		return RT_MANIFEST;

	// Other built-in ones?
	// PNG, WAVE, IMAGE (JPEG), GIF, MUI

	return as_native(s);
}
