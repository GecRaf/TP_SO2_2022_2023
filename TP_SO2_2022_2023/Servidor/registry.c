#include <windows.h>
#include <winreg.h>
#include <tchar.h>
#include <fcntl.h>
#include <synchapi.h>
#include "server.h"

#define TAM 100


int verifyRegistry() {

    HKEY chave; //handle para a chave principal

    DWORD key_value_roadLanes = MAX_ROAD_LANES;
    DWORD key_value_carspeed = MAX_CAR_SPEED;

    DWORD resultado; //resultado do RegCreateKey

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    // Open registry key
    resultado = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        KEY_PATH,
        0,
        KEY_ALL_ACCESS,
        &chave
    );

    if (resultado != ERROR_SUCCESS) {
        		_tprintf(TEXT("\t[!][Registry.c/verifyRegistry] Error opening key (%d)\n"), GetLastError());
		return -1;
	}

    if (resultado == REG_OPENED_EXISTING_KEY){
		_tprintf(TEXT("[Registry.c/verifyRegistry] Key opened successfully!\n"));
        LSTATUS result;
        DWORD error;

        // Read values from registry
        DWORD roadLanes = 0;
        DWORD carSpeed = 0;
        DWORD size = sizeof(DWORD);

        result = RegQueryValueEx(chave, KEY_ROAD_LANES, NULL, NULL, (LPBYTE)&roadLanes, &size);

        if (result != ERROR_SUCCESS) {
			error = GetLastError();
			return -1;
		}
        
        result = RegQueryValueEx(chave, KEY_INIT_SPEED, NULL, NULL, (LPBYTE)&carSpeed, &size);
        if (result != ERROR_SUCCESS) {
			error = GetLastError();
			return -1;
		}
		_tprintf(TEXT("[Registry.c/verifyRegistry] Registry values read successfully!\n"));
        if (0 > roadLanes > MAX_ROAD_LANES || 0 > carSpeed > MAX_CAR_SPEED) {
            _tprintf(TEXT("\t[!][Registry.c/verifyRegistry] Invalid registry values!\n"));
            _tprintf(TEXT("\t[!][Registry.c/verifyRegistry] Default values will be applied!\n"));
			return -1;
		}


		RegCloseKey(chave);
		return 0;
	}

    RegCloseKey(chave);

    return 0;
}

int createRegistry(int roadLanes, int carSpeed) {
	HKEY chave; //handle para a chave principal
	DWORD key_value_roadLanes = roadLanes;
	DWORD key_value_carspeed = carSpeed;
	DWORD resultado; //resultado do RegCreateKey

    #ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
    #endif

    if (RegCreateKeyEx(
	    HKEY_CURRENT_USER,
        KEY_PATH,
            	0,
            	NULL,
            	0,
            	KEY_ALL_ACCESS,
            	NULL,
            	&chave,
            	&resultado
            ) != ERROR_SUCCESS) {
            	_tprintf(TEXT("\t[!][Registry.c/createRegistry] Error creating/opening key (%d)\n"), GetLastError());
	    return -1;
	}
    if (resultado == REG_CREATED_NEW_KEY) {
	    _tprintf(TEXT("\t[+][Registry.c/createRegistry] Key created successfully!\n"));
	}
	//escrever na chave
    if (RegSetValueEx(
        chave,
        KEY_ROAD_LANES,
        0,
        REG_DWORD,
        (LPBYTE)&key_value_roadLanes,
        sizeof(key_value_roadLanes)
    ) != ERROR_SUCCESS) {
	    _tprintf(TEXT("\t[!][Registry.c/createRegistry] Error writing to key (%d)\n"), GetLastError());
	    return -1;
	}
    if (RegSetValueEx(
        chave,
        KEY_INIT_SPEED,
        0,
        REG_DWORD,
        (LPBYTE)&key_value_carspeed,
        sizeof(key_value_carspeed)
    ) != ERROR_SUCCESS) {
	    _tprintf(TEXT("\t[!][Registry.c/createRegistry] Error writing to key (%d)\n"), GetLastError());
	    return -1;
	}
	_tprintf(TEXT("\t[+][Registry.c/createRegistry] Key value %s: %i\n"), KEY_ROAD_LANES, key_value_roadLanes);
	_tprintf(TEXT("\t[+][Registry.c/createRegistry] Key value %s: %i\n"), KEY_INIT_SPEED, key_value_carspeed);
	RegCloseKey(chave);
	return 0;   
}

int readRegistry(char type[]) {
    if (strcmp(type, KEY_ROAD_LANES) != 0 && strcmp(type, KEY_INIT_SPEED) != 0) {
		_tprintf(TEXT("\t[!][Registry.c/readRegistry] Invalid request!\n"));
		return -1;
	}
    else if (strcmp(type, KEY_ROAD_LANES) == 0) {
        // Open registry key
        HKEY chave; //handle para a chave principal
        DWORD resultado; //resultado do RegCreateKey
        DWORD roadLanes = 0;
        DWORD size = sizeof(DWORD);

        if (RegOpenKeyEx(
            HKEY_CURRENT_USER,
            KEY_PATH,
            0,
            KEY_ALL_ACCESS,
            &chave
        ) != ERROR_SUCCESS) {
			_tprintf(TEXT("\t[!][Registry.c/readRegistry] Error opening key (%d)\n"), GetLastError());
			return -1;
		}
        if (RegQueryValueEx(chave, KEY_ROAD_LANES, NULL, NULL, (LPBYTE)&roadLanes, &size) == ERROR_SUCCESS) {
            return roadLanes;
        }
        else {
			_tprintf(TEXT("\t[!][Registry.c/readRegistry] Error reading registry value (%d)\n"), GetLastError());
			return -1;
        }
    }
    else if (strcmp(type, KEY_INIT_SPEED) == 0) {
		// Open registry key
		HKEY chave; //handle para a chave principal
		DWORD resultado; //resultado do RegCreateKey
		DWORD carSpeed = 0;
		DWORD size = sizeof(DWORD);
        if (RegOpenKeyEx(
            HKEY_CURRENT_USER,
            KEY_PATH,
            0,
            KEY_ALL_ACCESS,
            &chave
        ) != ERROR_SUCCESS) {
            _tprintf(TEXT("\t[!][Registry.c/readRegistry] Error opening key (%d)\n"), GetLastError());
            return -1;
        }

        if (RegQueryValueEx(chave, KEY_INIT_SPEED, NULL, NULL, (LPBYTE)&carSpeed, &size) == ERROR_SUCCESS) {
			return carSpeed;
		}
        else {
			_tprintf(TEXT("\t[!][Registry.c/readRegistry] Error reading registry value (%d)\n"), GetLastError());
			return -1;
		}
    }
}