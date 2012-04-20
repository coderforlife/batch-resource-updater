#pragma once

// Interop
template<typename T> __forceinline T *as_native(array<T> ^a) { pin_ptr<T> p = &a[0]; return p; }
#define NATIVE(a) as_native(a), a->Length
template<typename T>
static array<T> ^to_managed(T *a, size_t l) { array<T> ^m = gcnew array<T>((int)l); System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)a, m, 0, (int)l); return m; }

#include <vcclr.h>
__forceinline const wchar_t *as_native(System::String ^s) { pin_ptr<const wchar_t> p = PtrToStringChars(s); return p; }
__forceinline System::String ^as_managed(wchar_t *s) { return System::Runtime::InteropServices::Marshal::PtrToStringUni((System::IntPtr)s); }


#define OVERWRITE_ALWAYS	0	//always adds the resource, even if it already exists
#define OVERWRITE_NEVER		1	//only adds a resource is it does not already exist
#define OVERWRITE_ONLY		2	//only adds a resource if it will overwrite another resource

// Looks if a file should be saved, given if it exists and the desire to overwrite
bool shouldSave(bool exists, int overwrite);

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
