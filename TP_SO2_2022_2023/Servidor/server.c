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

void printBoard(Game* game) {
	for (int i = 0; i < game->number_of_lanes; i++) {
		_tprintf(TEXT("\n"));
		for (int j = 0; j < MAX_BOARD_COL; j++) {
			_tprintf(TEXT("%c"),game->board[i][j]);
		}
	}
	_tprintf(TEXT("\n"));
}

void boardInitializer(ControlData* cd) {
	for (int i = 0; i < cd->g->number_of_lanes; i++) {
		for (int j = 0; j < MAX_BOARD_COL; j++) {
			if (i == 0 || i == cd->g->number_of_lanes - 1) {
				cd->g->board[i][j] = TEXT('-');
			}
			else {
				cd->g->board[i][j] = TEXT('.');
			}
		}
	}
}

void clear_screen() {
	system("cls");
}

// TODO: Rafael (Console function) 
/*if (command_received == NULL) {
			_tprintf(TEXT("\t[*] Enter a command: "));
			_fgetts(command, 50, stdin);
			//command[_tcslen(command) - 1] = '\0';
			_tprintf(TEXT("\n"));
		}
		else
		{
			_tcscpy_s(command, 50, command_received);
		}

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
		}*/

DWORD WINAPI server_manager(LPVOID lparam) {
	ControlData* cd = (ControlData*)lparam;
	TCHAR command[50][50];
	BufferItem buffer_item;
	clear_screen();
	ascii_printer();
	printBoard(cd->g);

	while (!cd->threadStop) {
		_tprintf(TEXT("\t[*] 3...\n"));
		Sleep(1000);
		_tprintf(TEXT("\t[*] 2...\n"));
		Sleep(1000);
		_tprintf(TEXT("\t[*] 1...\n"));
		Sleep(1000);
		_tprintf(TEXT("\t[*] GO!\n"));
		SetEvent(cd->eventHandle);
		_tprintf(TEXT("\t[*] Signal sent!\n"));
		WaitForSingleObject(cd->hSemRead, INFINITE);
		WaitForSingleObject(cd->hMutex, INFINITE);
		CopyMemory(&buffer_item, &cd->g->buffer[cd->g->out], sizeof(BufferItem));
		cd->g->out++;
		if(cd->g->out == MAX_BUFFER_SIZE) cd->g->out = 0;

		_tprintf(TEXT("\t[*] Command received: %s\n"), buffer_item.command);

		//TODO: Rafaela (Command check)
		// stop
		// obstacle
		// invert

		ReleaseMutex(cd->hMutex);
		ReleaseSemaphore(cd->hSemRead, 1, NULL);
	}
	return 0;
}

void init_server(int argc, TCHAR* argv[]) {
	// Initialize structs
	ControlData cd;
	Game* game = (Game*)malloc(sizeof(Game));
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
	}
	

	// Shared memory 
	
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		sizeof(Game),     // maximum object size (low-order DWORD)
		KEY_SHARED_MEMORY);   // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("[Server.c/init_server] Error creating shared memory\n"));
		return 1;
	}

	_tprintf(TEXT("[Server.c/init_server] Shared memory created successfully\n"));

	// Create pointer for Shared Memory
	cd.g = (Game*)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		sizeof(Game));
	
	if (cd.g == NULL || cd.g == INVALID_HANDLE_VALUE){
			_tprintf(TEXT("[Server.c/init_server] Error creating shared memory pointer\n"));
			return 1;
	}
	_tprintf(TEXT("[Server.c/init_server] Shared memory pointer created successfully\n"));

	// Initialize shared memory
	cd.g->number_of_lanes = nr_of_lanes;
	cd.g->initial_speed = init_speed;
	cd.g->in = 0;
	cd.g->out = 0;
	cd.threadStop = 0;
	cd.hSemRead = CreateSemaphore(NULL, 0, MAX_BUFFER_SIZE, TEXT("SemRead"));
	cd.hSemWrite = CreateSemaphore(NULL, MAX_BUFFER_SIZE, MAX_BUFFER_SIZE, TEXT("SemWrite"));
	cd.hMutex = CreateMutex(NULL, FALSE, TEXT("SemMutex"));
	cd.eventHandle = CreateEvent(NULL, TRUE, FALSE, TEXT("BoardEvent"));

	boardInitializer(&cd);

	// Create a thread to manage the server
	// Lots of threads going on, one for acceptiing clients, others for managing the clients and messages
	Sleep(3000);
	HANDLE hThread = CreateThread(
		NULL,
		0,
		server_manager,
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