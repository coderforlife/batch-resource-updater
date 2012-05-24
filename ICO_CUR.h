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

// Functions for updating icons and cursors, and their groups in resources

// NOTE: Even though the functions are called 'ICO' they work for 'CUR' files as well

#pragma once

#include "PE\PEFile.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Extract icons/cursors from a PE-resource
bool extractICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID *data, size_t *size, PE::Rsrc *r);
bool extractICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID *data, size_t *size, PE::Rsrc *r);

// Delete icons/cursors from a PE-resource
bool deleteICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, PE::Rsrc *r);
bool deleteICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, PE::Rsrc *r);

// Add icons/cursors to a PE-resource
bool addICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID data, PE::Rsrc *r, PE::Overwrite overwrite = PE::ALWAYS);
bool addICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID data, PE::Rsrc *r, PE::Overwrite overwrite = PE::ALWAYS);
