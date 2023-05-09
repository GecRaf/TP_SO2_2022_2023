#include "operator.h"

int _tmain(int argc, TCHAR* argv[]) {
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif // UNICODE

	ControlData cd;
	cd.eventHandle = CreateEvent(NULL, FALSE, FALSE, TEXT("BoardEvent"));
	cd.hSemRead = CreateSemaphore(NULL, 0, 1, TEXT("SemRead"));
	cd.hSemWrite = CreateSemaphore(NULL, 1, 1, TEXT("SemWrite"));
	cd.hMutex = CreateSemaphore(NULL, 1, 1, TEXT("SemMutex"));

	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		TEXT("SharedMemory")
	);
	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[Operator.c/_tmain] Error opening shared memory\n"));
		return 1;
	}
	cd.g = (Game*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(Game)
	);
	if (cd.g == NULL || cd.g == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[Operator.c/_tmain] Error creating shared memory pointer\n"));
		return 1;
	}

	// Create function to display the board

	// Create threads. One for receive commands, another to send commands and another to update the board

	//Wait for the threads to finish with WaitForSingleObject

	UnmapViewOfFile(cd.g);
	CloseHandle(hMapFile);
	CloseHandle(cd.eventHandle);
	CloseHandle(cd.hSemRead);
	CloseHandle(cd.hSemWrite);
	CloseHandle(cd.hMutex);
	return 0;
}