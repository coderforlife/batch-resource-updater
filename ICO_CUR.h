// Functions for updating icons and cursors, and their groups in resources

// NOTE: Even though the functions are called 'ICO' they work for 'CUR' files as well

#pragma once

#include "PEFile.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Extract icons/cursors from a PE-resource
bool extractICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID *data, size_t *size, Rsrc *r);
bool extractICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID *data, size_t *size, Rsrc *r);

// Delete icons/cursors from a PE-resource
bool deleteICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, Rsrc *r);
bool deleteICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, Rsrc *r);

// Add icons/cursors to a PE-resource
bool addICOIndividual(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID data, Rsrc *r, DWORD overwrite = OVERWRITE_ALWAYS);
bool addICOGroup(LPCWSTR type, LPCWSTR name, WORD lang, LPVOID data, Rsrc *r, DWORD overwrite = OVERWRITE_ALWAYS);
