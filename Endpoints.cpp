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

// Implements the endpoints (sources or destinations) for resource data

#include "Endpoints.h"

#include "general.h"
#include "pe\PEFile.h"
#include "ICO_CUR.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Xml;

#pragma region Image Conversion Functions
////////////////////////////////////////////////////////////
// WARNING: ALL UNTESTED - WRITTEN UP FOR FUTURE USE
////////////////////////////////////////////////////////////
//using namespace System::Drawing;
//using namespace System::Drawing::Imaging;
//using namespace System::Runtime::InteropServices;
// Need to reference System.Drawing.dll as well
/*static void *IMG2IMG(LPVOID data, size_t size, size_t *out_size, ImageFormat format) { // free(), but not original
	UnmanagedMemoryStream ^in = gcnew UnmanagedMemoryStream((unsigned char*)data, (signed __int64)size);
	Bitmap ^b = gcnew Bitmap(in);
	in->Close();

	MemoryStream ^out = gcnew MemoryStream();
	b->Save(out, format);
	*out_size = size = (size_t)out->Length;
	LPVOID out_data = malloc(size);
	if (out_data == NULL) return NULL;
	Marshal::Copy(out->GetBuffer(), 0, IntPtr(data), size);
	out->Close();
	
	free(data);

	return out_data;
}
static void *IMG2BMP(LPVOID data, size_t size, size_t *out_size) { return IMG2IMG(data, size, out_size, ImageFormat::Bmp); } // free(), but not original
static void *BMP2IMG(LPVOID data, size_t size, size_t *out_size, ImageFormat format) { return IMG2IMG(data, size, out_size, format); } // free(), but not original

static void *BMP2DIB(LPVOID data, size_t size, size_t *out_size) { // free(), but not original
	*out_size = size-sizeof(BITMAPFILEHEADER);
	return ((BYTE*)data)+sizeof(BITMAPFILEHEADER);
}
static void *DIB2BMP(LPVOID data, size_t size, size_t *out_size) { // free(), but not original
	*out_size = size+sizeof(BITMAPFILEHEADER);
	LPVOID out_data = malloc(size+sizeof(BITMAPFILEHEADER));
	if (out_data == NULL) return NULL;

	memcpy(out_data+sizeof(BITMAPFILEHEADER), data, size);

	// need to add bitmap header
	// type="BM", size now includes header, and the bitmap starts at offset 54 (14 + second header size which is usually 40 (this should be checked))
	static const BITMAPFILEHEADER bmp = {0x4D42, size+sizeof(BITMAPFILEHEADER), 0, 0, 54};
	memcpy(out_data, &bmp, sizeof(BITMAPFILEHEADER));

	free(data);

	return out_data;
}

static void *IMG2DIB(LPVOID data, size_t size, size_t *out_size) { // free(), but not original
	size_t bmp_size;
	return BMP2DIB(IMG2BMP(data, size, &bmp_size), bmp_size, out_size);
}
static void *DIB2IMG(LPVOID data, size_t size, size_t *out_size) { // free(), but not original
	size_t bmp_size;
	return BMP2IMG(DIB2BMP(data, size, &bmp_size), bmp_size, out_size);
}*/
#pragma endregion

#pragma region Dummy (Fallback) Endpoint
bool DummyEndpoint::IsSpec(String ^) { return true; }
void DummyEndpoint::Add(String ^, void *, size_t, PE::Overwrite) { throw gcnew InvalidOperationException(L"Could not understand name."); }
void *DummyEndpoint::Get(String ^, size_t *) { throw gcnew InvalidOperationException(L"Could not understand name."); }
void DummyEndpoint::Remove(String ^) { throw gcnew InvalidOperationException(L"Could not understand name."); }
//void DummyEndpoint::Clear(String ^) { throw gcnew InvalidOperationException(L"Could not understand name."); }
void DummyEndpoint::Commit() { }
#pragma endregion

#pragma region File Endpoint
bool Files::IsSpec(String ^spec) {
	return spec->IndexOf(L'|') == -1 && spec->IndexOfAny(Path::GetInvalidPathChars()) == -1;
}
void Files::Add(String ^spec, void *data, size_t size, PE::Overwrite overwrite) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid file name"); }
	spec = Path::GetFullPath(spec);
	if (Directory::Exists(spec) || !shouldSave(File::Exists(spec), overwrite)) {
		throw gcnew UnauthorizedAccessException(L"Could not overwrite file");
	}
	array<unsigned char> ^bytes = gcnew array<unsigned char>((int)size);
	Runtime::InteropServices::Marshal::Copy(IntPtr(data), bytes, 0, (int)size);
	Directory::CreateDirectory(Path::GetDirectoryName(spec));
	File::WriteAllBytes(spec, bytes);
}
void *Files::Get(String ^spec, size_t *size) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid file name"); }
	array<unsigned char> ^bytes = File::ReadAllBytes(spec);
	*size = bytes->Length;
	void *data = malloc(bytes->Length);
	Runtime::InteropServices::Marshal::Copy(bytes, 0, IntPtr(data), bytes->Length);
	return data;
}
void Files::Remove(String ^spec) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid file name"); }
	File::Delete(spec);
}
//void Files::Clear(String ^spec) { throw gcnew NotSupportedException(L"Files do not support being cleared."); }
void Files::Commit() { }
#pragma endregion

bool extractBitmap(LPVOID *data, size_t *size) {
	BYTE *d = (BYTE*)realloc(*data, *size+sizeof(BITMAPFILEHEADER));
	if (!d) { return false; }
	memmove(d+sizeof(BITMAPFILEHEADER), *data = d, *size);
	*size += sizeof(BITMAPFILEHEADER);
	// need to add bitmap header
	// type="BM", size now includes header, and the bitmap starts at offset 54 (14 + second header size which is usually 40 (this should be checked))
	BITMAPFILEHEADER bmp = {0x4D42, (int)(*size+sizeof(BITMAPFILEHEADER)), 0, 0, 54};
	memcpy(d, &bmp, sizeof(BITMAPFILEHEADER));
	return true;
}

#pragma region PE File Endpoint
PEFiles::PEFiles() {
	peFiles = gcnew Dictionary<String^,IntPtr>();
	edited = gcnew Dictionary<String^,bool>();
}
void *PEFiles::GetPEFile(String ^filename) {
	if (peFiles->ContainsKey(filename)) {
		return (PE::File*)peFiles[filename].ToPointer();
	}
	PE::File *peFile = new PE::File(as_native(filename));
	if (!peFile->isLoaded()) {
		ReportLastError(L"Opening PE File: ");
		delete peFile;
		return NULL;
	}
	peFiles[filename] = IntPtr(peFile);
	edited[filename] = false;
	return peFile;
}
bool PEFiles::IsSpec(String ^spec) {
	UInt16 x;
	array<String^> ^parts = spec->Split(L'|');
	if (parts->Length != 4 || !UInt16::TryParse(parts[3], x))
		return false;
	parts[0] = parts[0]->ToLower();
	for (int i = 0; i < pe_exts->Length; i++)
		if (parts[0]->EndsWith(pe_exts[i]))
			return true;
	return false;
}
void PEFiles::Add(String ^spec, void *data, size_t size, PE::Overwrite overwrite) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid PE file specification"); }
	array<String^> ^parts = spec->Split(L'|');
	PE::File *peFile = (PE::File*)GetPEFile(parts[0]);
	if (!peFile) { return; }
	LPCWSTR type = getBuiltInResourceType(parts[1]);
	LPCWSTR name = convertToId(parts[2]);
	WORD lang = UInt16::Parse(parts[3]);

	PE::Rsrc *res = peFile->getResources();
	bool b;
	if (type == RT_BITMAP) { // need to remove 14 byte bitmap header
		b = res->add(type, name, lang, ((BYTE*)data)+14, size-14, overwrite);
	} else 	if (type == RT_ICON || type == RT_CURSOR) {
		b = addICOIndividual(type, name, lang, data, res, overwrite);
	} else if (type == RT_GROUP_ICON || type == RT_GROUP_CURSOR) {
		b = addICOGroup(type, name, lang, data, res, overwrite);
	} else {
		b = res->add(type, name, lang, data, size, overwrite);
	}

	if (!b)
		throw gcnew UnauthorizedAccessException(L"Could not overwrite entry");
	edited[parts[0]] = true;
}
void *PEFiles::Get(String ^spec, size_t *size) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid PE file specification"); }
	array<String^> ^parts = spec->Split(L'|');
	PE::File *peFile = (PE::File*)GetPEFile(parts[0]);
	if (!peFile) { return NULL; }
	LPCWSTR type = getBuiltInResourceType(parts[1]);
	LPCWSTR name = convertToId(parts[2]);
	WORD lang = UInt16::Parse(parts[3]);

	bool b = false;
	PE::Rsrc *res = peFile->getResources();
	void *data = res->get(type, name, lang, size);
	if (data) {
		if (type == RT_BITMAP) {
			b = extractBitmap(&data, size);
		} else if (type == RT_ICON || type == RT_CURSOR) {
			b = extractICOIndividual(type, name, lang, &data, size, res);
		} else if (type == RT_GROUP_ICON || type == RT_GROUP_CURSOR) {
			b = extractICOGroup(type, name, lang, &data, size, res);
		} else {
			b = true;
		}
	}

	if (!b)
		throw gcnew Exception(L"Failed to get entry");

	return data;
}
void PEFiles::Remove(String ^spec) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid PE file specification"); }
	array<String^> ^parts = spec->Split(L'|');
	PE::File *peFile = (PE::File*)GetPEFile(parts[0]);
	if (!peFile) { return; }
	LPCWSTR type = getBuiltInResourceType(parts[1]);
	LPCWSTR name = convertToId(parts[2]);
	WORD lang = UInt16::Parse(parts[3]);

	bool b = false;
	PE::Rsrc *res = peFile->getResources();
	if (type == RT_ICON || type == RT_CURSOR)
		b = deleteICOIndividual(type, name, lang, res);
	else if (type == RT_GROUP_ICON || type == RT_GROUP_CURSOR)
		b = deleteICOGroup(type, name, lang, res);
	else
		b = res->remove(type, name, lang);

	if (!b)
		throw gcnew Exception(L"Failed to remove entry");
	edited[parts[0]] = true;
}
//void PEFiles::Clean(String ^spec) {}
void PEFiles::Commit() {
	for each (KeyValuePair<String^, IntPtr> ^kv in peFiles) {
		String ^filename = kv->Key;
		PE::File *peFile = (PE::File*)kv->Value.ToPointer();
		if (edited[filename]) {
			if (!peFile->save())
				ReportLastError(L"Closing PE File: ");
			else
				Console::WriteLine(filename + L" Saved");
		}
		delete peFile;
	}
	peFiles->Clear();
	edited->Clear();
}
#pragma endregion

#pragma region RES file Endpoint
RESFiles::RESFiles() {
	resFiles = gcnew Dictionary<String^,IntPtr>();
	edited = gcnew Dictionary<String^,bool>();
}
void *RESFiles::GetRESFile(String ^filename) {
	if (resFiles->ContainsKey(filename)) {
		return (PE::Rsrc*)resFiles[filename].ToPointer();
	}
	PE::Rsrc *resFile;
	if (File::Exists(filename)) {
		array<unsigned char> ^bytes = File::ReadAllBytes(filename);
		resFile = PE::Rsrc::createFromRESFile(NATIVE(bytes));
	} else {
		resFile = PE::Rsrc::createEmpty();
	}
	if (resFile == NULL) {
		ReportLastError(L"Opening RES File: ");
		return NULL;
	}
	resFiles[filename] = IntPtr(resFile);
	edited[filename] = false;
	return resFile;
}
bool RESFiles::IsSpec(String ^spec) {
	UInt16 x;
	array<String^> ^parts = spec->Split(L'|');
	return parts->Length == 4 && UInt16::TryParse(parts[3], x) && parts[0]->ToLower()->EndsWith(".res");
}
void RESFiles::Add(String ^spec, void *data, size_t size, PE::Overwrite overwrite) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid PE file specification"); }
	array<String^> ^parts = spec->Split(L'|');
	PE::Rsrc *res = (PE::Rsrc*)GetRESFile(parts[0]);
	if (!res) { return; }
	LPCWSTR type = getBuiltInResourceType(parts[1]);
	LPCWSTR name = convertToId(parts[2]);
	WORD lang = UInt16::Parse(parts[3]);

	bool b;
	if (type == RT_BITMAP) { // need to remove 14 byte bitmap header
		b = res->add(type, name, lang, ((BYTE*)data)+14, size-14, overwrite);
	} else 	if (type == RT_ICON || type == RT_CURSOR) {
		b = addICOIndividual(type, name, lang, data, res, overwrite);
	} else if (type == RT_GROUP_ICON || type == RT_GROUP_CURSOR) {
		b = addICOGroup(type, name, lang, data, res, overwrite);
	} else {
		b = res->add(type, name, lang, data, size, overwrite);
	}

	if (!b)
		throw gcnew UnauthorizedAccessException(L"Could not overwrite entry");
	edited[parts[0]] = true;
}
void *RESFiles::Get(String ^spec, size_t *size) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid PE file specification"); }
	array<String^> ^parts = spec->Split(L'|');
	PE::Rsrc *res = (PE::Rsrc*)GetRESFile(parts[0]);
	if (!res) { return NULL; }
	LPCWSTR type = getBuiltInResourceType(parts[1]);
	LPCWSTR name = convertToId(parts[2]);
	WORD lang = UInt16::Parse(parts[3]);

	bool b = false;
	void *data = res->get(type, name, lang, size);
	if (data) {
		if (type == RT_BITMAP) {
			b = extractBitmap(&data, size);
		} else if (type == RT_ICON || type == RT_CURSOR) {
			b = extractICOIndividual(type, name, lang, &data, size, res);
		} else if (type == RT_GROUP_ICON || type == RT_GROUP_CURSOR) {
			b = extractICOGroup(type, name, lang, &data, size, res);
		} else {
			b = true;
		}
	}

	if (!b)
		throw gcnew Exception(L"Failed to get entry");

	return data;
}
void RESFiles::Remove(String ^spec) {
	if (!IsSpec(spec)) { throw gcnew InvalidOperationException(L"Invalid PE file specification"); }
	array<String^> ^parts = spec->Split(L'|');
	PE::Rsrc *res = (PE::Rsrc*)GetRESFile(parts[0]);
	if (!res) { return; }
	LPCWSTR type = getBuiltInResourceType(parts[1]);
	LPCWSTR name = convertToId(parts[2]);
	WORD lang = UInt16::Parse(parts[3]);

	bool b = false;
	if (type == RT_ICON || type == RT_CURSOR)
		b = deleteICOIndividual(type, name, lang, res);
	else if (type == RT_GROUP_ICON || type == RT_GROUP_CURSOR)
		b = deleteICOGroup(type, name, lang, res);
	else
		b = res->remove(type, name, lang);

	if (!b)
		throw gcnew Exception(L"Failed to remove entry");
	edited[parts[0]] = true;
}
//void RESFiles::Clean(String ^spec) {}
void RESFiles::Commit() {
	for each (KeyValuePair<String^, IntPtr> ^kv in resFiles) {
		String ^filename = kv->Key;
		PE::Rsrc *resFile = (PE::Rsrc*)kv->Value.ToPointer();
		if (edited[filename]) {
			size_t size;
			LPVOID data = resFile->compileRES(&size);
			try {
				File::WriteAllBytes(filename, to_managed((LPBYTE)data, size));
				Console::WriteLine(filename + L" Saved");
			} finally {
				free(data);
			}
		}
		delete resFile;
	}
	resFiles->Clear();
	edited->Clear();
}
#pragma endregion

#pragma region Endpoints Endpoint
// This endpoint is actually a collection of endpoints, and looks for the one to use based on IsSpec()
Endpoints::Endpoints() {
	endpoints = gcnew array<Endpoint^>{ gcnew Files(), gcnew PEFiles(), gcnew RESFiles(), gcnew DummyEndpoint() };
}
Endpoint ^Endpoints::GetEndpoint(System::String ^spec) {
	for (int i = 0; i < endpoints->Length; i++) {
		if (endpoints[i]->IsSpec(spec))
			return endpoints[i];
	}
	return nullptr;
}
bool Endpoints::IsSpec(System::String ^spec) { return GetEndpoint(spec) != nullptr; }
void Endpoints::Add(System::String ^spec, void *data, size_t size, PE::Overwrite overwrite) { GetEndpoint(spec)->Add(spec, data, size, overwrite); }
void *Endpoints::Get(System::String ^spec, size_t *size) { return GetEndpoint(spec)->Get(spec, size); }
void Endpoints::Remove(System::String ^spec) { GetEndpoint(spec)->Remove(spec); }
//void Endpoints::Clean(System::String ^spec) { GetEndpoint(spec)->Clean(spec); }
void Endpoints::Commit() { for each (Endpoint ^e in endpoints) { e->Commit(); } }
#pragma endregion
