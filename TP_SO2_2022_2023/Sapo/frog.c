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
        szProgName, // Nome da janela (programa) definido acima
        TEXT("Frog"), // Texto que figura na barra do título
        WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, // Estilo da janela (WS_OVERLAPPED= normal)
        CW_USEDEFAULT, // Posição x pixels (default=à direita da última)
        CW_USEDEFAULT, // Posição y pixels (default=abaixo da última)
        1000, // Largura da janela (em pixels)
        800, // Altura da janela (em pixels)
        (HWND)HWND_DESKTOP, // handle da janela pai (se se criar uma a partir de
        // outra) ou HWND_DESKTOP se a janela for a primeira, 
        // criada a partir do "desktop"
        (HMENU)NULL, // handle do menu da janela (se tiver menu)
        (HINSTANCE)hInst, // handle da instância do programa actual ("hInst" é 
        // passado num dos parâmetros de WinMain()
        0); // Não há parâmetros adicionais para a janela


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

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, int nCmdShow) {
    HDC hdc;
    RECT rect;
    static HBITMAP hBmp;
    static HBITMAP hBmpBoard;
    static BITMAP bmp = { 0 };
    static BITMAP bmpBoard = { 0 };
    static HDC bmpDC = NULL;
    static HDC bmpBoardDC = NULL;

    PAINTSTRUCT ps;

    switch (messg) {

    case WM_CREATE:
        hBmp = (HBITMAP)LoadImage(NULL, TEXT("../../Bitmaps/frog1.bmp"), IMAGE_BITMAP, 35, 35, LR_LOADFROMFILE);
        hBmpBoard = (HBITMAP)LoadImage(NULL, TEXT("../../Bitmaps/areaJogo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        
        GetObject(hBmp, sizeof(bmp), &bmp);
        GetObject(hBmpBoard,sizeof(bmpBoard),&bmpBoard);

        hdc = GetDC(hWnd);
        bmpDC = CreateCompatibleDC(hdc);
        bmpBoardDC = CreateCompatibleDC(hdc);

        SelectObject(bmpDC, hBmp);
        SelectObject(bmpBoardDC, hBmpBoard);

        ReleaseDC(hWnd, hdc);

        GetClientRect(hWnd, &rect);

        // Defina o novo tamanho desejado para o bitmap hBmpBoard
        int novoLargura = 50; // Nova largura em pixels
        int novoAltura = 50; // Nova altura em pixels

        // Criar um novo bitmap com o novo tamanho
        HBITMAP hNovoBmpBoard = CreateCompatibleBitmap(hdc, novoLargura, novoAltura);

        // Selecionar o novo bitmap no contexto de dispositivo
        HBITMAP hOldBmpBoard = (HBITMAP)SelectObject(bmpBoardDC, hNovoBmpBoard);

        // Redimensionar o bitmap usando StretchBlt
        StretchBlt(bmpBoardDC, 0, 0, novoLargura, novoAltura, hdc, 0, 0, bmpBoard.bmWidth, bmpBoard.bmHeight, SRCCOPY);

        // Restaurar o bitmap original no contexto de dispositivo
        SelectObject(bmpBoardDC, hOldBmpBoard);

        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);

        // Obter as dimensões da janela
        RECT rect;
        GetClientRect(hWnd, &rect);
        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;

        // Obter as dimensões do bitmap

        int bitmapWidth = bmpBoard.bmWidth;
        int bitmapHeight = bmpBoard.bmHeight;



        // Calcular as coordenadas para centralizar o bitmap
        int x = (windowWidth - bitmapWidth) / 2;
        int y = (windowHeight - bitmapHeight) / 2;


        BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
        BitBlt(hdc, x, y, bitmapWidth, bitmapHeight, bmpBoardDC, 0, 0, SRCCOPY);


        EndPaint(hWnd, &ps);

        break;
    case WM_CLOSE:
        if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
            DestroyWindow(hWnd);
        }
        break;

    case WM_DESTROY:// Destruir a janela e terminar o programa 
        // "PostQuitMessage(Exit Status)"
        DeleteDC(bmpDC);

        DeleteObject(hBmp);

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
