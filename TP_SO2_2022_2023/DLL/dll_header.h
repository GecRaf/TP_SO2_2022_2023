#pragma once
#include <windows.h>

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif // DLL_EXPORTS

DLL_IMP_API HANDLE create_shared_memory(TCHAR* name, DWORD size);
DLL_IMP_API HANDLE open_shared_memory(TCHAR* name);
DLL_IMP_API void* map_shared_memory(HANDLE hMapFile, DWORD size);
DLL_IMP_API void unmap_shared_memory(void* ptr);
DLL_IMP_API void close_shared_memory(HANDLE hMapFile);