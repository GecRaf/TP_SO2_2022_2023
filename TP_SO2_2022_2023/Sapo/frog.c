#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windowsx.h>
#include "frog.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("Frog");

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow){

    HWND hWnd;
    MSG lpMsg;
    WNDCLASSEX wcApp;

// ============================================================================
// 1. Definição das características da janela "wcApp" 
//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
// ============================================================================
    wcApp.cbSize = sizeof(WNDCLASSEX);   // Tamanho da estrutura WNDCLASSEX
    wcApp.hInstance = hInst; // Instância da janela actualmente exibida 
    wcApp.lpszClassName = szProgName;  // Nome da janela
    wcApp.lpfnWndProc = TrataEventos;  // Endereço da função de processamento da janela
    wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela
    wcApp.hIcon = LoadIcon(NULL, IDI_WARNING); // "hIcon" = handler do ícon normal
    wcApp.hIconSm = LoadIcon(NULL, IDI_SHIELD); // "hIconSm" = handler do ícon pequeno
    wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // "hCursor" = handler do cursor (rato) 
    wcApp.lpszMenuName = NULL; // Classe do menu que a janela pode ter
    wcApp.cbClsExtra = 0;
    wcApp.cbWndExtra = 0;
    wcApp.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); //cor de fundo da janela

// ============================================================================
// 2. Registar a classe "wcApp" no Windows
// ============================================================================
    if (!RegisterClassEx(&wcApp))
        return(0);

// ============================================================================
// 3. Criar a janela
// ============================================================================

    hWnd = CreateWindow(
        szProgName,
        TEXT("Frog Game"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (HWND)HWND_DESKTOP,
        (HMENU)NULL,
        (HINSTANCE)hInst,
        0
    );


// ============================================================================
// 4. Mostrar a janela
// ============================================================================
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

// ============================================================================
// 5. Loop de Mensagens
// ============================================================================

    while (GetMessage(&lpMsg, NULL, 0, 0)) {
        TranslateMessage(&lpMsg);
        DispatchMessage(&lpMsg);
    }

// ============================================================================
// 6. Fim do programa
// ============================================================================
    return((int)lpMsg.wParam);
}


typedef struct {
    TCHAR c;
    int xPos, yPos;
}PosChar;
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    RECT rect;
    PAINTSTRUCT ps;
    static PosChar posicoes[1000];
    static int totalPos = 0;
    static TCHAR curChar = '?';
    int i;

    switch (messg) {

    case WM_CLOSE:
        if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
            DestroyWindow(hWnd);
        }
        break;

    case WM_DESTROY:// Destruir a janela e terminar o programa 
        // "PostQuitMessage(Exit Status)"
        PostQuitMessage(0);
        break;
    default:
        // Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
        // não é efectuado nenhum processamento, apenas se segue o "default" do Windows
        return(DefWindowProc(hWnd, messg, wParam, lParam));
        break;  // break tecnicamente desnecessário por causa do return
    }
    return(0);
}



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
