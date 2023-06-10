#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "frog.h"


int _tmain(int argc, LPTSTR argv[]) {
    TCHAR buf[256];
    HANDLE hPipe;
    int i = 0;
    BOOL ret;
    DWORD n;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    _tprintf(TEXT("Wait for pipe '%s' (WaitNamedPipe)\n"),
        NOME_PIPE);
    if (!WaitNamedPipe(NOME_PIPE, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(TEXT("[ERROR] Connect to pipe '%s'! (WaitNamedPipe)\n"), NOME_PIPE);
        exit(-1);
    }
    _tprintf(TEXT("Binding to the writer pipe... (CreateFile)\n"));
    hPipe = CreateFile(NOME_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hPipe == NULL) {
        _tprintf(TEXT("[ERRO]R Connect to pipe '%s'! (CreateFile)\n"), NOME_PIPE);
        exit(-1);
    }
    _tprintf(TEXT("CONNECTED...\n"));

    while (1) {
        ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
        buf[n / sizeof(TCHAR)] = '\0';
        if (!ret || !n) {
            _tprintf(TEXT(" %d %d... (ReadFile)\n"), ret, n);
            break;
        }
       
        CharUpperBuff(buf, _tcslen(buf));


        if (!WriteFile(hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
            _tprintf(TEXT("[ERROR] Write on pipe! (WriteFile)\n"));
            exit(-1);
        }
        
    }
    CloseHandle(hPipe);
    Sleep(200);
    return 0;
}
