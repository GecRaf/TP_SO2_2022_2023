#include "server.h"

//▬ᴥ

void ascii_printer() {
	_tprintf(TEXT("\t   ____                          \n"));
	_tprintf(TEXT("\t  / __/______  ___ ____ ____ ____\n"));
	_tprintf(TEXT("\t / _// __/ _ \\/ _ `/ _ `/ -_) __/\n"));
	_tprintf(TEXT("\t/_/ /_/  \\___/\\_, /\\_, /\\__/_/   \n"));
	_tprintf(TEXT("\t             /___//___/          \n"));
	_tprintf(TEXT("\t[*] SERVER INITIALIZED [*]\n"));
	_tprintf(TEXT("\n"));
	
}

void boardInitializer() {
	for (int i = 0; i < MAX_BOARD_ROW; i++) {
		_tprintf(TEXT("\n"));
		for (int j = 0; j < MAX_BOARD_COL; j++) {
			if (i == 0 || i == MAX_BOARD_ROW - 1) {
				_tprintf(TEXT("-"));
			}else{
			_tprintf(TEXT(".")); 
			}
		}
	}
	_tprintf(TEXT("\n"));
}

void clear_screen() {
	system("cls");
}

DWORD WINAPI server_manager(LPVOID lparam) {
	ControlData *cd = (ControlData*)lparam;
	clear_screen();
	TCHAR command[50][50];
	ascii_printer();
	//boardInitializer();
	TCHAR command_received[100];

	while (1) {
		WaitForSingleObject(cd->hSemRead, INFINITE);
		WaitForSingleObject(cd->hMutex, INFINITE);
		CopyMemory(&command_received, &cd->g->buffer[cd->g->out], sizeof(cd->g->buffer));
		cd->g->out++;
		if(cd->g->out == MAX_BUFFER_SIZE) cd-> g->out = 0;

		if (command_received == NULL) {
			_tprintf(TEXT("\t[*] Enter a command: "));
			_fgetts(command, 50, stdin);
			//command[_tcslen(command) - 1] = '\0';
			_tprintf(TEXT("\n"));
		}
		else
		{
			_tcscpy_s(command, 50, command_received);
		}
		//SetEvent(cd->eventHandle);

		if (_tcscmp(command, TEXT("exit")) == 0) {
			_tprintf(TEXT("\t[!] Server shutting down...\n"));
			_tprintf(TEXT("\t[~] Exiting...\n"));
			Sleep(1000);
			exit(0);
		}
		else {
			_tprintf(TEXT("\tUnknown command.\n"));
			Sleep(1000);
			clear_screen();
			ascii_printer();
		}

		ReleaseMutex(cd->hMutex);
		ReleaseSemaphore(cd->hSemRead, 1, NULL);
	}
	return 0;
}

void init_server(int argc, TCHAR* argv[]) {
	// Initialize structs
	ControlData cd;
	Game* game = (Game*)malloc(sizeof(Game));
	cd.g = game;
	int nr_of_lanes = 0;
	int init_speed = 0;
	
	// Server instance controller

	HANDLE hSemaphoreServer = CreateSemaphore(
		NULL,
		1,
		MAX_SERVER,
		SEMAPHORE_SERVER
	);

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		_ftprintf(stderr, TEXT("\n\t[!] Error! Another instance of 'server' already running\n"));
		return -1;
	}

	ascii_printer();
	
	// Verify if there is a registry file already
	BOOL result = verifyRegistry();
	
	if (result == -1)
	{
		_tprintf(TEXT("[Server.c/init_server] No values stored in Registry!\n"));
		// Ask for number of lanes and initial speed to the user
		int nr_of_lanes = 0;
		int init_speed = 0;

		if (argc == 3) {
			nr_of_lanes = _ttoi(argv[1]);
			init_speed = _ttoi(argv[2]);
			// Print values
			_tprintf(TEXT("[Server.c/init_server] Number of lanes: %d\n"), nr_of_lanes);
			_tprintf(TEXT("[Server.c/init_server] Initial speed: %d\n"), init_speed);
			BOOL result = createRegistry(nr_of_lanes, init_speed);
			if (result == -1) {
				_tprintf(TEXT("\t[Server.c/init_server] Error creating registry file!\n"));
				return -1;
			}
		}
		else
		{
			_tprintf(TEXT("[Server.c/init_server] Invalid number of arguments!\n"));
			return -1;
		}
	}
	else
	{
		_tprintf(TEXT("[Server.c/init_server] Values stored in Registry!\n"));
		// Read values from registry
		BOOL result = readRegistry(KEY_ROAD_LANES);
		if (result == -1) {
			_tprintf(TEXT("\t[Server.c/init_server] Error reading registry file!\n"));
			return -1;
		}
		else {
			nr_of_lanes = result;
		}

		result = readRegistry(KEY_INIT_SPEED);
		if (result == -1) {
			_tprintf(TEXT("\t[Server.c/init_server] Error reading registry file!\n"));
			return -1;
		}
		else {
			init_speed = result;
		}
		// Print values
		_tprintf(TEXT("\t[Server.c/init_server] Number of lanes: %d\n"), nr_of_lanes);
		_tprintf(TEXT("\t[Server.c/init_server] Initial speed: %d\n"), init_speed);
		cd.g->number_of_lanes = nr_of_lanes;
		cd.g->initial_speed = init_speed;
	}
	

	// Shared memory 
	
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		sizeof(ControlData),     // maximum object size (low-order DWORD)
		TEXT("SharedMemory"));   // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("[Server.c/init_server] Error creating shared memory\n"));
		return 1;
	}

	_tprintf(TEXT("[Server.c/init_server] Shared memory created successfully\n"));

	// Create pointer for Shared Memory
	cd.shared_memmory_ptr = (ControlData*)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		sizeof(ControlData));
	
	if (cd.shared_memmory_ptr == NULL || cd.shared_memmory_ptr == INVALID_HANDLE_VALUE){
			_tprintf(TEXT("[Server.c/init_server] Error creating shared memory pointer\n"));
			return 1;
	}
	_tprintf(TEXT("[Server.c/init_server] Shared memory pointer created successfully\n"));

	// Initialize shared memory
	cd.g = game;
	cd.g->in = 0;
	cd.g->out = 0;
	cd.threadStop = 0;
	cd.hMutex = CreateSemaphore(NULL, 1, 1, TEXT("SemMutex"));
	cd.eventHandle = CreateEvent(NULL, TRUE, FALSE, TEXT("BoardEvent"));

	// Create a thread to manage the server
	// Lots of threads going on, one for acceptiing clients, others for managing the clients and messages

	HANDLE hThread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)server_manager,
		&cd,
		0,
		NULL
	);

	if (hThread == NULL)
	{
		_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
		return;
	}

	_tprintf(TEXT("[Server.c/init_server] Server manager thread created successfully\n"));

	WaitForSingleObject(hThread, INFINITE);
	// Somewhere overhere, we goota initialize the game board

	//server_manager(NULL);

	// Wait for the threads to finish with WaitForSingleObject
	UnmapViewOfFile(cd.shared_memmory_ptr);
	CloseHandle(hMapFile);
	CloseHandle(hSemaphoreServer);
	CloseHandle(cd.eventHandle);
	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif // UNICODE
	
	init_server(argc, argv);
	
	return 0;
}