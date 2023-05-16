// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

__declspec(dllexport) HANDLE create_shared_memory(TCHAR* name, DWORD size) {
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		size,
		name
	);
	if (hMapFile == NULL) {
		_tprintf(TEXT("[DLL_main.c/create_shared_memory] Error creating shared memory.\n"));
		return NULL;
	}
	else {
		_tprintf(TEXT("[DLL_main.c/create_shared_memory] Shared memory created successfully.\n"));
		return hMapFile;
	}
}

__declspec(dllexport) HANDLE open_shared_memory(TCHAR* name) {
	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		name
	);
	if (hMapFile == NULL) {
		_tprintf(TEXT("[DLL_main.c/open_shared_memory] Error opening shared memory.\n"));
		return NULL;
	}
	else {
		_tprintf(TEXT("[DLL_main.c/open_shared_memory] Shared memory opened successfully.\n"));
		return hMapFile;
	}
}

__declspec(dllexport) void* map_shared_memory(HANDLE hMapFile, DWORD size) {
	void* ptr = MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		size
	);
	if (ptr == NULL) {
		_tprintf(TEXT("[DLL_main.c/map_shared_memory] Error mapping shared memory.\n"));
		return NULL;
	}
	else {
		_tprintf(TEXT("[DLL_main.c/map_shared_memory] Shared memory mapped successfully.\n"));
		return ptr;
	}
}

__declspec(dllexport) void unmap_shared_memory(void* ptr) {
	if (UnmapViewOfFile(ptr) == 0) {
		_tprintf(TEXT("[DLL_main.c/unmap_shared_memory] Error unmapping shared memory.\n"));
	}
	else {
		_tprintf(TEXT("[DLL_main.c/unmap_shared_memory] Shared memory unmapped successfully.\n"));
	}
}

__declspec(dllexport) void close_shared_memory(HANDLE hMapFile) {
	if (CloseHandle(hMapFile) == 0) {
		_tprintf(TEXT("[DLL_main.c/close_shared_memory] Error closing shared memory.\n"));
	}
	else {
		_tprintf(TEXT("[DLL_main.c/close_shared_memory] Shared memory closed successfully.\n"));
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		_tprintf(TEXT("[DLL_main.c/DllMain] DLL attached.\n"));
		break;
    case DLL_THREAD_ATTACH:
		_tprintf(TEXT("[DLL_main.c/DllMain] DLL detached.\n"));
		break;
    case DLL_THREAD_DETACH:
		_tprintf(TEXT("[DLL_main.c/DllMain] Thread attached.\n"));
		break;
    case DLL_PROCESS_DETACH:
		_tprintf(TEXT("[DLL_main.c/DllMain] Thread detached.\n"));
		break;
    }
    return TRUE;
}

