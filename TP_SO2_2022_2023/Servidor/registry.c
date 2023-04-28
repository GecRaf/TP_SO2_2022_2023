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
    TCHAR key_name_roadLanes[TAM] = TEXT("ROADLANES");

    DWORD key_value_carspeed = MAX_CAR_SPEED;
    TCHAR key_name_carsSpeed[TAM] = TEXT("CARSPEED");
    

    DWORD resultado; //resultado do RegCreateKey

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif


    //criar a chave
    if (RegCreateKeyEx(
        HKEY_CURRENT_USER,
        TEXT("Software\\TP_SO2_2122\\"),
        0,
        NULL,
        0,
        KEY_ALL_ACCESS,
        NULL,
        &chave,
        &resultado
    ) != ERROR_SUCCESS) {
        _tprintf(TEXT("Error creating/opening key (%d)\n"), GetLastError());
        return -1;
    }

    // As chaves foram criadas
    if (resultado == REG_CREATED_NEW_KEY) {

        //Criar um par "nome-valor" para as road Lanes
        if (RegSetValueEx(
            chave,
            key_name_roadLanes,
            0,
            REG_DWORD,
            (LPBYTE)&key_value_roadLanes,
            sizeof(key_value_roadLanes)
            ) != ERROR_SUCCESS) {
            _tprintf(stderr,TEXT("[Error]Error creating key(% s)\n"), key_name_roadLanes);
            return -1;
        }

        //Criar um par "nome-valor" para as car speed
        if (RegSetValueEx(
            chave,
            key_name_carsSpeed,
            0,
            REG_DWORD,
            (LPBYTE)&key_value_carspeed,
            sizeof(key_value_carspeed)
        ) != ERROR_SUCCESS) {
            _tprintf(stderr,TEXT("[Error]Error creating key(% s)\n"), key_name_carsSpeed);
            return -1;
        }

        _tprintf(TEXT("key created successfully!\n\n%s: %i\n%s: %i\n"), key_name_roadLanes, key_value_roadLanes, key_name_carsSpeed, key_value_carspeed);

        return 0;
    }

    _tprintf(TEXT("Key already exists and the program has not changed anything!\n"));

    RegCloseKey(chave);

    return 0;

}