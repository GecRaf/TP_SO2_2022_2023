#include "server.h"

void ascii_printer(int instance) {
	_tprintf(TEXT("\t   ____                          \n"));
	_tprintf(TEXT("\t  / __/______  ___ ____ ____ ____\n"));
	_tprintf(TEXT("\t / _// __/ _ \/ _ `/ _ `/ -_) __/\n"));
	_tprintf(TEXT("\t/_/ /_/  \___/\_, /\_, /\__/_/   \n"));
	_tprintf(TEXT("\t             /___//___/          \n"));
	if (instance == 1) { // Print Server
		_tprintf(TEXT("\t[*] SERVER INITIALIZED [*]\n"));
		_tprintf(TEXT("\n"));
	}
	else if (instance == 2) { // Print Monitor
		_tprintf(TEXT("\t[*] MONITOR INITIALIZED [*]\n"));
		_tprintf(TEXT("\n"));
	}
	else if (instance == 3) { // Print Client
		_tprintf(TEXT("\t[*] CLIENT INITIALIZED [*]\n"));
		_tprintf(TEXT("\n"));
	}
}

void clear_screen() {
	system("cls");
}

char server_manager(int instance, TCHAR command_received) {
	TCHAR command[50];

	while (1) {
		if (command_received == NULL) {
			_tprintf(TEXT("\t[*] Enter a command: "));
			_fgetts(command, 50, stdin);
			command[_tcslen(command) - 1] = '\0';
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
			ascii_printer(instance);
		}
	}
	return 0;
}


int _tmain(int argc, TCHAR* argv[]) {
	HANDLE hSemaphoreServer;
	
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif // UNICODE

	// Server instance controller

	hSemaphoreServer = CreateSemaphore(
		NULL,
		1,
		MAX_SERVER,
		SEMAPHORE_SERVER
	);

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		_ftprintf(stderr,TEXT("\n\t[!] Error! Another instance of 'server' already running\n"));
		return -1;
	}
	
	ascii_printer(1);
	server_manager(1, NULL);
	
	return 0;
}