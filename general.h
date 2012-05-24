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

#pragma once

#include "PE\PEDataTypes.h"

// Interop
template<typename T> __forceinline T *as_native(array<T> ^a) { pin_ptr<T> p = &a[0]; return p; }
#define NATIVE(a) as_native(a), a->Length
template<typename T>
static array<T> ^to_managed(T *a, size_t l) { array<T> ^m = gcnew array<T>((int)l); System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)a, m, 0, (int)l); return m; }

#include <vcclr.h>
__forceinline const wchar_t *as_native(System::String ^s) { pin_ptr<const wchar_t> p = PtrToStringChars(s); return p; }
__forceinline System::String ^as_managed(wchar_t *s) { return System::Runtime::InteropServices::Marshal::PtrToStringUni((System::IntPtr)s); }


// Looks if a file should be saved, given if it exists and the desire to overwrite
bool shouldSave(bool exists, PE::Overwrite overwrite);

// Converts a managed System::String to a wide-character string. The returned string needs to be deleted with delete[].
//LPWSTR toLPWSTR(System::String ^s);

// Gets a string that contains the size and the units
System::String ^getDisplayFileSize(DWORD size);

// Gets the last error (GetLastError()) as a string
System::String ^LastErrorString();
// Outputs the last error
void ReportLastError(System::String ^s);
// Outputs the last error, possibly only as a warning
void ReportLastError(System::String ^s, bool warning);

// Convert a string to the resource ID, which may involve converting to WORD
LPCWSTR convertToId(System::String ^s);

// Converts a string to a PE file resource variable, converting names like "BITMAP" to the proper id and numbers to number ids.
LPCWSTR getBuiltInResourceType(System::String ^s);
