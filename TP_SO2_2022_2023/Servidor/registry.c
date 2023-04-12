#include <windows.h>
#include <winreg.h>
#include <tchar.h>
#include <fcntl.h>
#include <synchapi.h>
#include "server.h"

#define TAM 200


BOOL verifyRegistry() {

    HKEY hKey;
    DWORD par_valor = MAX_ROAD_LANES;
    TCHAR par_nome_carspeed[TAM] = TEXT("CARSPEED");
    DWORD par_valor_carspeed = 5;
    DWORD resultado;

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif


    //Tentar abrir a chave
    if (RegOpenKeyEx(HKEY_CURRENT_USER, KEY_PATH, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) {

        //Erro ao abrir a chave
        _tprintf(TEXT("[Registry.c/serverRegistry] Registry key doesn't exist\n"));
        return FALSE;
    }
    else {
        _tprintf(TEXT("[Registry.c/serverRegistry] Registry key opened\n"));

        //ler valor da chave
        _tprintf(_T("[Registry.c/serverRegistry] Obtaining registry key values...\n"));
        DWORD valor = 0;
        DWORD length = sizeof(DWORD);

        if (RegQueryValueEx(hKey, KEY_ROAD_LANES, NULL, NULL, (LPBYTE)(&valor), &length) != ERROR_SUCCESS) {
            _tprintf(_T("[Registry.c/serverRegistry] Couldn't obtain key value\n"));
            createRegistry(MAX_ROAD_LANES, KEY_ROAD_LANES);
        }
        else {
            _tprintf(_T("[Registry.c/serverRegistry] Obtained key value on registry: %ld\n"), valor);

            if (valor < 0 || valor > 8) {
                _tprintf(_T("[Registry.c/serverRegistry] Invalid value obtained from registry. The value must be between 0 and 8. Default value of 8 will be applied.\n")); //Review this. This shouldn't even happen. 
                                                                                                                                                                            //Every time a value is written to the registry, it should be checked if it's valid.
                valor = 8;
            }

            if (RegQueryValueEx(hKey, KEY_INIT_SPEED, NULL, NULL, (LPBYTE)(&valor), &length) != ERROR_SUCCESS) {

                _tprintf(_T("[Registry.c/serverRegistry] Não foi possível obter o valor a partir da chave\n"));

                createRegistry(MAX_CAR_SPEED, KEY_INIT_SPEED);

            }

        }

        RegCloseKey(hKey);
        return TRUE;
    }
}


BOOL createRegistry(DWORD* roadLanes, DWORD* carspeed) {
    HKEY hKey;
    DWORD result;

    //tentar criar a chave
    if (RegCreateKeyEx(HKEY_CURRENT_USER, KEY_PATH, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &result) != ERROR_SUCCESS) {
        //Não foi possível criar chave, mostrar o erro
        _ftprintf(stderr, TEXT("[Registry.c/serverRegistry] Error creating key!\n"));
        return FALSE;

    }
    else {
        if (result == REG_CREATED_NEW_KEY) {
            _tprintf(TEXT("[Registry.c/serverRegistry] Registry key created\n"));

            //criar registo par chave-valor road lanes
            if (RegSetValue(hKey, KEY_ROAD_LANES, 0, &roadLanes, sizeof(roadLanes)) == ERROR_SUCCESS) {
                _tprintf(_T("[Registry.c/serverRegistry] Registry key created: key %s | value %i\n"), KEY_ROAD_LANES, roadLanes);
            }
            else {
                _ftprintf(stderr, TEXT("[Registry.c/serverRegistry] Could not set registry value!\n"));
                return FALSE;
            }

            //criar registo par chave-valor car speed
            if (RegSetValue(hKey, KEY_INIT_SPEED, 0, REG_DWORD, &carspeed, sizeof(carspeed)) == ERROR_SUCCESS) {
                _tprintf(_T("[Registry.c/serverRegistry] Registry key created: key %s | value %i\n"), KEY_INIT_SPEED, carspeed);
            }
            else {
                _ftprintf(stderr, TEXT("[Registry.c/serverRegistry] Could not set registry value!\n"));
                return FALSE;
            }
        }
        else if (result == REG_OPENED_EXISTING_KEY){
            _ftprintf(stderr, TEXT("[Registry.c/serverRegistry] Registry key already exists!\n"));

            //criar registo par chave-valor road lanes
            if (RegSetValueEx(hKey, KEY_ROAD_LANES, 0, REG_DWORD, &roadLanes, sizeof(roadLanes)) == ERROR_SUCCESS) {
                _tprintf(_T("[Registry.c/serverRegistry] Registry key created: key %s | value %i\n"), KEY_ROAD_LANES, roadLanes);
            }
            else {
                _ftprintf(stderr, TEXT("[Registry.c/serverRegistry] Could not set registry value!\n"));
                return FALSE;
            }

            //criar registo par chave-valor car speed
            if (RegSetValueEx(hKey, KEY_INIT_SPEED, 0, REG_DWORD, &carspeed, sizeof(carspeed)) == ERROR_SUCCESS) {
                _tprintf(_T("[Registry.c/serverRegistry] Registry key created: key %s | value %i\n"), KEY_INIT_SPEED, carspeed);
            }
            else {
                _ftprintf(stderr, TEXT("[Registry.c/serverRegistry] Could not set registry value!\n"));
                return FALSE;
            }
        }
        else {
			_ftprintf(stderr, TEXT("[Registry.c/serverRegistry] Error creating key!\n"));
			return FALSE;
		}
        return TRUE;
    }

}
