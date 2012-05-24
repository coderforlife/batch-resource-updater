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

#include "ICO_CUR.h"

#include <vector>

#pragma region General Defines and Objects
////////////////////////////////////////////////////////////////////////////////
///// General Defines and Objects
////////////////////////////////////////////////////////////////////////////////
#define ICO_ID	1
#define CUR_ID	2

#include <pshpack2.h> //these require WORD alignment, not DWORD
struct ICO_CUR_HEADER {
	WORD wReserved;  // Always 0
	WORD wResID;     // ICO_ID or CUR_ID
	WORD wNumImages; // Number of image/directory entries
};
struct ICO_CUR_RT_ENTRY {
	union {
		struct {
			BYTE  bWidth;
			BYTE  bHeight;
			BYTE  bColorCount;
			BYTE  bReserved;
		} ICO;
		struct {
			WORD  wWidth;
			WORD  wHeight; // Divide by 2 to get the actual height.
		} CUR;
	};
	WORD  wPlanes;
	WORD  wBitCount;
	DWORD dwSize;
	WORD  wID;
};
#include <poppack.h>

struct CUR_HOTSPOT {
	WORD  wHotspotX;
	WORD  wHotspotY;
};
struct ICO_CUR_ENTRY {
	BYTE  bWidth;
	BYTE  bHeight;
	BYTE  bColorCount;
	BYTE  bReserved;
	union {
		struct {
			WORD  wPlanes;
			WORD  wBitCount;
		} ICO;
		CUR_HOTSPOT CUR;
	};
	DWORD dwSize;
	DWORD dwOffset;
};
#pragma endregion

#pragma region General Functions
////////////////////////////////////////////////////////////////////////////////
///// General Functions
////////////////////////////////////////////////////////////////////////////////
// Finds the icon/cursor group that a particular icon/cursor belongs to.
// IN		type must be RT_GROUP_CURSOR or RT_GROUP_ICON
// IN		name/lang are for the icon or group
// OUT		grpName/grpIndx are set to the group found
// OUT		return value is the data for the found group, and dataSize is set to it's size
// IN_OUT	restart is the index of the group returned. If provided, the search will begin at that index.
// All of the OUT and IN_OUT parameters are OPTIONAL (you can provide NULL if you don't care about them). 
LPVOID findICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, PE::Rsrc *r, LPCWSTR *grpName, WORD *grpIndx, DWORD *dataSize, DWORD *restart = NULL) {
	std::vector<LPCWSTR> names = r->getNames(type);
	for (DWORD i = (restart ? *restart : 0); i < names.size(); i++) {
		size_t size = 0;
		LPVOID gdata = r->get(type, names[i], lang, &size);
		if (!gdata)
			continue;
		ICO_CUR_HEADER *header = (ICO_CUR_HEADER*)gdata;
		ICO_CUR_RT_ENTRY *entries = (ICO_CUR_RT_ENTRY*)((LPBYTE)gdata+sizeof(ICO_CUR_HEADER));
		for (WORD j = 0; j < header->wNumImages; j++) {
			if (MAKEINTRESOURCE(entries[j].wID) == name) {
				if (grpName)	*grpName = names[i];
				if (grpIndx)	*grpIndx = j;
				if (dataSize)	*dataSize = (DWORD)size;
				if (restart)	*restart = i;
				return gdata;
			}
		}
		free(gdata);
	}
	SetLastError(ERROR_MOD_NOT_FOUND);
	return NULL;
}
// Searches icon/cursor groups for ones that contain the icon/cursor given by the name/lang and returns the count.
// Type must be RT_GROUP_ICON or RT_GROUP_CURSOR.
DWORD countICOGroups(LPCWSTR type, LPCWSTR name, WORD lang, PE::Rsrc *r) {
	DWORD count = 0;
	std::vector<LPCWSTR> names = r->getNames(type);
	for (DWORD i = 0; i < names.size(); i++) {
		size_t size = 0;
		LPVOID gdata = r->get(type, names[i], lang, &size);
		if (!gdata)
			continue;
		ICO_CUR_HEADER *header = (ICO_CUR_HEADER*)gdata;
		ICO_CUR_RT_ENTRY *entries = (ICO_CUR_RT_ENTRY*)((LPBYTE)gdata+sizeof(ICO_CUR_HEADER));
		for (WORD j = 0; j < header->wNumImages; j++) {
			if (MAKEINTRESOURCE(entries[j].wID) == name) {
				count++;
				break;
			}
		}
		free(gdata);
	}
	return count;
}
// Find the id that should be used in a particular type (it is the next available integer)
WORD findNextAvailable(LPCWSTR type, PE::Rsrc *r) {
	std::vector<LPCWSTR> names = r->getNames(type);
	WORD i;
	for (i = 0; i < names.size(); i++) // skip all named ones
		if (IS_INTRESOURCE(names[i])) break;
	if (i == names.size() || names[i] != MAKEINTRESOURCE(1)) // see if there are no numbers or does not start with 1
		return 1;
	for (; i < names.size()-1; i++) // find any gaps in numbering
		if (PE::ResID2Int(names[i])+1 != PE::ResID2Int(names[i+1]))
			return PE::ResID2Int(names[i])+1;
	return PE::ResID2Int(names[i])+1; // return one above the highest value
}
#pragma endregion

#pragma region Extract Functions
////////////////////////////////////////////////////////////////////////////////
///// Extract Functions
////////////////////////////////////////////////////////////////////////////////
bool extractICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID *data, size_t *size, PE::Rsrc *r) {
	type = (type==RT_GROUP_CURSOR||type==RT_CURSOR)?RT_GROUP_CURSOR:RT_GROUP_ICON;

	WORD grpIndx;
	LPVOID gdata = findICOGroup(type, name, lang, r, NULL, &grpIndx, NULL);
	if (!gdata)
		return false;

	ICO_CUR_RT_ENTRY *rtEntry = ((ICO_CUR_RT_ENTRY*)((LPBYTE)gdata+sizeof(ICO_CUR_HEADER)))+grpIndx;
	ICO_CUR_ENTRY entry = *(ICO_CUR_ENTRY*)(rtEntry);
	BYTE *bytes = (BYTE*)*data;

	// some extra processing is required for cursors
	if (type == RT_GROUP_CURSOR) {
		entry.bHeight = (BYTE)(rtEntry->CUR.wHeight / 2);
		entry.bWidth  = (BYTE)(rtEntry->CUR.wWidth);
		entry.bColorCount = 0;
		entry.bReserved = 0;
		entry.CUR = *(CUR_HOTSPOT*)bytes;
		bytes += sizeof(CUR_HOTSPOT);
		entry.dwSize -= sizeof(CUR_HOTSPOT); //!! is this really right?
	}

	entry.dwOffset = sizeof(ICO_CUR_HEADER)+sizeof(ICO_CUR_ENTRY);

	free(gdata);

	ICO_CUR_HEADER header = {0, (type==RT_GROUP_CURSOR)?CUR_ID:ICO_ID, 1};

	*size = sizeof(ICO_CUR_HEADER)+sizeof(ICO_CUR_ENTRY)+entry.dwSize;
	BYTE *d = (BYTE*)malloc(*size);
	memcpy(d, &header, sizeof(ICO_CUR_HEADER));
	memcpy(d+sizeof(ICO_CUR_HEADER), &entry, sizeof(ICO_CUR_ENTRY));
	memcpy(d+sizeof(ICO_CUR_HEADER)+sizeof(ICO_CUR_ENTRY), bytes, entry.dwSize);
	free(*data);
	*data = d;
	return true;
}

bool extractICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID *data, size_t *size, PE::Rsrc *r) {
	UNREFERENCED_PARAMETER(name); // unreferenced parameter

	type = (type==RT_GROUP_CURSOR||type==RT_CURSOR)?RT_CURSOR:RT_ICON;

	BYTE *bytes = (BYTE*)*data;

	// get the ICO/CUR header
	ICO_CUR_HEADER *header = (ICO_CUR_HEADER*)bytes;

	// get all the RT entries
	ICO_CUR_RT_ENTRY *entries = (ICO_CUR_RT_ENTRY*)(bytes+sizeof(ICO_CUR_HEADER));

	// calculate where the entries and img data start
	DWORD entry_offset = sizeof(ICO_CUR_HEADER);
	DWORD offset = entry_offset+sizeof(ICO_CUR_ENTRY)*header->wNumImages;

	// write the header stuff
	BYTE *d = (BYTE*)malloc(offset);
	memcpy(d, header, sizeof(ICO_CUR_HEADER));

	for (DWORD i = 0; i < header->wNumImages; i++) {

		// get the current entry and image data
		ICO_CUR_ENTRY entry = *(ICO_CUR_ENTRY*)(entries+i);
		size_t temp_size;
		LPVOID img_orig = r->get(type, MAKEINTRESOURCE(entries[i].wID), lang, &temp_size), img = img_orig;
		entry.dwSize = (DWORD)temp_size;

		// some extra processing is required for cursors
		if (type == RT_CURSOR) {
			entry.bHeight = (BYTE)(entries[i].CUR.wHeight / 2);
			entry.bWidth  = (BYTE)(entries[i].CUR.wWidth);
			entry.bColorCount = 0;
			entry.bReserved = 0;
			entry.CUR = *(CUR_HOTSPOT*)img;
			img = ((LPBYTE)img)+sizeof(CUR_HOTSPOT);
			entry.dwSize -= sizeof(CUR_HOTSPOT);
		}
		entry.dwOffset = offset;

		// write the data
		d = (BYTE*)realloc(d, offset+entry.dwSize);
		memcpy(d+entry_offset, &entry, sizeof(ICO_CUR_ENTRY));
		memcpy(d+offset, img, entry.dwSize);

		// update the offsets
		entry_offset += sizeof(ICO_CUR_ENTRY);
		offset += entry.dwSize; //entries[i].dwSize;

		// cleanup
		free(img_orig);
	}

	free(*data);
	*data = d;
	*size = offset;

	return true;
}
#pragma endregion

#pragma region Delete Functions
////////////////////////////////////////////////////////////////////////////////
///// Delete Functions
////////////////////////////////////////////////////////////////////////////////
bool deleteICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, PE::Rsrc *r) {
	type = (type==RT_GROUP_CURSOR||type==RT_CURSOR)?RT_GROUP_CURSOR:RT_GROUP_ICON;

	for (;;) {
		LPCWSTR grpName;
		WORD grpIndx;
		DWORD size;
		LPVOID gdata = findICOGroup(type, name, lang, r, &grpName, &grpIndx, &size);
		if (!gdata)
			break;

		ICO_CUR_HEADER *header = (ICO_CUR_HEADER*)gdata;
		ICO_CUR_RT_ENTRY *entries = (ICO_CUR_RT_ENTRY*)((LPBYTE)gdata+sizeof(ICO_CUR_HEADER));
		header->wNumImages -= 1;
		if (header->wNumImages == 0) {
			r->remove(type, grpName, lang);
		} else {
			for (WORD i = grpIndx; i < header->wNumImages; i++)
				entries[i] = entries[i+1];
			r->add(type, grpName, lang, gdata, size, PE::ONLY);
		}
		free(gdata);
	}

	return r->remove((type == RT_GROUP_ICON ? RT_ICON : RT_CURSOR), name, lang);
}
bool deleteICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, PE::Rsrc *r) {
	type = (type==RT_GROUP_CURSOR||type==RT_CURSOR)?RT_CURSOR:RT_ICON;

	LPWSTR type2 = type == RT_ICON ? RT_GROUP_ICON : RT_GROUP_CURSOR;
	size_t size = 0;
	LPVOID data = r->get(type2, name, lang, &size);
	if (data == NULL)
		return false;
	ICO_CUR_HEADER *header = (ICO_CUR_HEADER*)data;
	ICO_CUR_RT_ENTRY *entries = (ICO_CUR_RT_ENTRY*)((LPBYTE)data+sizeof(ICO_CUR_HEADER));
	WORD i;
	for (i = 0; i < header->wNumImages; i++) {
		if (countICOGroups(type2, MAKEINTRESOURCE(entries[i].wID), lang, r) == 1) {
			r->remove(type, MAKEINTRESOURCE(entries[i].wID), lang);
		}
	}

	bool retval = (i == header->wNumImages) && r->remove(type2, name, lang);
	free(data);
	return retval;
}
#pragma endregion

#pragma region Add Functions
////////////////////////////////////////////////////////////////////////////////
///// Add Functions
////////////////////////////////////////////////////////////////////////////////
bool addICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID data, PE::Rsrc *r, PE::Overwrite overwrite) {
	type = (type==RT_GROUP_CURSOR||type==RT_CURSOR)?RT_GROUP_CURSOR:RT_GROUP_ICON;

	if (!IS_INTRESOURCE(name)) {
		SetLastError(ERROR_INVALID_ICON_HANDLE);
		return false;
	}
	
	bool exists = r->exists(type, name, lang);
	if (exists) {
		if (overwrite == PE::NEVER)
			return false;
	} else if (overwrite == PE::ONLY)
		return false;
	DWORD nGroups = exists ? countICOGroups(type, name, lang, r) : 0;

	ICO_CUR_HEADER *header = (ICO_CUR_HEADER*)data;
	if (header->wNumImages < 1)
		return false;
	// if header->wNumImages > 1, we just use the first one
	ICO_CUR_ENTRY *entry = (ICO_CUR_ENTRY*)((LPBYTE)data+sizeof(ICO_CUR_HEADER));

	ICO_CUR_RT_ENTRY rt = *(ICO_CUR_RT_ENTRY*)(entry);
	rt.wID = PE::ResID2Int(name);

	// some extra processing is required for cursors
	if (type == RT_GROUP_CURSOR) {
		rt.CUR.wHeight = entry->bHeight * 2;
		rt.CUR.wWidth  = entry->bWidth;
		rt.dwSize += sizeof(CUR_HOTSPOT);
		LPBYTE temp = new BYTE[rt.dwSize];
		memcpy(temp, &entry->CUR, sizeof(CUR_HOTSPOT));
		memcpy(temp+sizeof(CUR_HOTSPOT), (LPBYTE)data+entry->dwOffset, entry->dwSize);
		r->add(RT_CURSOR, name, lang, temp, rt.dwSize, overwrite);
		delete[] temp;
	} else {
		r->add(RT_ICON, name, lang, (LPBYTE)data+entry->dwOffset, entry->dwSize, overwrite);
	}

	bool b = false;

	if (nGroups > 0) {
		// update existing groups
		DWORD restart = 0;
		for (DWORD i = 0; i < nGroups; i++) {
			LPCWSTR grpName;
			WORD grpIndx;
			DWORD dataSize;
			LPBYTE gdata = (LPBYTE)findICOGroup(type, name, lang, r, &grpName, &grpIndx, &dataSize, &restart);
			if (gdata) {
				memcpy(gdata+sizeof(ICO_CUR_HEADER)+grpIndx*sizeof(ICO_CUR_RT_ENTRY), &rt, sizeof(ICO_CUR_RT_ENTRY));
				b = r->add(type, grpName, lang, gdata, dataSize, PE::ONLY);
				free(gdata);
			}
		}
	} else {
		// make a new group
		DWORD gdataSize = sizeof(ICO_CUR_HEADER)+sizeof(ICO_CUR_RT_ENTRY);
		LPBYTE gdata = new BYTE[gdataSize];
		memcpy(gdata, header, sizeof(ICO_CUR_HEADER));
		memcpy(gdata+sizeof(ICO_CUR_HEADER), &rt, sizeof(ICO_CUR_RT_ENTRY));
		b = r->add(type, MAKEINTRESOURCE(findNextAvailable(type, r)), lang, gdata, gdataSize, PE::NEVER);
		delete[] gdata;
	}

	return b;
}
bool addICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID data, PE::Rsrc *r, PE::Overwrite overwrite) {
	type = (type==RT_GROUP_CURSOR||type==RT_CURSOR)?RT_CURSOR:RT_ICON;
	LPWSTR type2 = type==RT_ICON ? RT_GROUP_ICON : RT_GROUP_CURSOR;

	if (r->exists(type2, name, lang)) {
		if (overwrite == PE::NEVER || !deleteICOGroup(type2, name, lang, r))
			return false;
	} else if (overwrite == PE::ONLY)
		return false;

	ICO_CUR_HEADER *header = (ICO_CUR_HEADER*)data;
	if (header->wNumImages < 1)
		return false;
	ICO_CUR_ENTRY *entries = (ICO_CUR_ENTRY*)((LPBYTE)data+sizeof(ICO_CUR_HEADER));

	DWORD gdataSize = sizeof(ICO_CUR_HEADER)+header->wNumImages*sizeof(ICO_CUR_RT_ENTRY);
	LPBYTE gdata = new BYTE[gdataSize];
	memcpy(gdata, header, sizeof(ICO_CUR_HEADER));
	
	DWORD offset = sizeof(ICO_CUR_HEADER);
	for (WORD i = 0; i < header->wNumImages; i++) {
		ICO_CUR_ENTRY entry = entries[i];
		ICO_CUR_RT_ENTRY rt = *(ICO_CUR_RT_ENTRY*)(entries+i);
		rt.wID = findNextAvailable(type, r);

		// some extra processing is required for cursors
		if (type == RT_CURSOR) {
			rt.CUR.wHeight = entry.bHeight * 2;
			rt.CUR.wWidth  = entry.bWidth;
			rt.dwSize += sizeof(CUR_HOTSPOT);
			LPBYTE temp = new BYTE[rt.dwSize];
			memcpy(temp, &entry.CUR, sizeof(CUR_HOTSPOT));
			memcpy(temp+sizeof(CUR_HOTSPOT), (LPBYTE)data+entry.dwOffset, entry.dwSize);
			r->add(RT_CURSOR, MAKEINTRESOURCE(rt.wID), lang, temp, rt.dwSize, PE::NEVER);
			delete[] temp;
		} else {
			r->add(RT_ICON, MAKEINTRESOURCE(rt.wID), lang, (LPBYTE)data+entry.dwOffset, entry.dwSize, PE::NEVER);
		}

		memcpy(gdata+offset, &rt, sizeof(ICO_CUR_RT_ENTRY));
		offset += sizeof(ICO_CUR_RT_ENTRY);
	}

	bool b = r->add(type2, name, lang, gdata, gdataSize, overwrite);
	delete[] gdata;
	return b;
}
#pragma endregion
