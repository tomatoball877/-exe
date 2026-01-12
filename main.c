#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#include <wininet.h>
#include "resource.h"
#pragma comment(lib, "wininet.lib")
#include <wchar.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
#include <shobjidl.h>

HBITMAP hMapBmp = NULL;

HWND hWnd;
HWND hList;

LRESULT CALLBACK WndProc(
	HWND,
	UINT,
	WPARAM,
	LPARAM
);
ATOM InitApp(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);

void LoadQuakeInfo();
int getValue(const char* json, const char* key, char* out, int outSize);
int btn();

HWND hButton;
TCHAR szClassName[] = TEXT("tomatoApp");

char g_expr[256] = "";
int g_expr_len = 0;

wchar_t g_quakeInfo[1024] = L"地震情報を取得できませんでした";
int g_px = 0;
int g_py = 0;

int WINAPI WinMain(
	HINSTANCE hCurInst,
	HINSTANCE hPrevInst,
	LPSTR lpsCmdLine,
	int nCmdShow
) {

	MSG msg;
	BOOL bRet;

	if (!InitApp(hCurInst)) return FALSE;
	if (!InitInstance(hCurInst, nCmdShow)) return FALSE;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
	return (int)msg.wParam;
}


ATOM InitApp(HINSTANCE hInst) {
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(IDI_APPLICATION), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	wc.hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = (LPCSTR)szClassName;
	wc.hIconSm = (HICON)LoadImage(NULL, MAKEINTRESOURCE(IDI_APPLICATION), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

	return (RegisterClassEx(&wc));
}

BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	hWnd = CreateWindow(
		szClassName,
		TEXT("tomatoApp"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInst,
		NULL
	);
	if (!hWnd) return FALSE;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	RECT rc;
	GetClientRect(hWnd, &rc);
	int height = rc.bottom - rc.top;
	

	hList = CreateWindow(
		TEXT("LISTBOX"),
		NULL,
		WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_BORDER,
		0, 0, 200, height,
		hWnd,
		(HMENU)1,
		hInst,
		NULL
	);

	hButton =  CreateWindow(
		TEXT("BUTTON"),
		TEXT("地震情報更新"),
		WS_VISIBLE | WS_CHILD | BS_FLAT,
		220,100,100,30,
		hWnd,
		(HMENU)2,
		hInst,
		NULL
	);

	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)TEXT("ホーム"));
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)TEXT("予定なし"));
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)TEXT("地震情報"));
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)TEXT("予定なし"));
	SendMessage(hList, LB_SETCURSEL, 0, 0);

	hMapBmp = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_MAP),
		IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

	SetTimer(hWnd, 1, 30000, NULL);
	LPARAM lp = MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top);
	btn(lp);

	return TRUE;
}



LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT msg,
	WPARAM wp,
	LPARAM lp
) {
	int id;
	RECT rect;
	int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
	switch (msg)
	{
	case WM_CHAR:
		if (index == 1) {
			char c = (char)wp;

			if (c == '\b') {
				if (g_expr_len > 0) g_expr_len--;
				g_expr[g_expr_len] = '\0';
			}
			else {
				g_expr[g_expr_len++] = c;
				g_expr[g_expr_len] = '\0';
			}

			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wp) == 1 && HIWORD(wp) == LBN_SELCHANGE) {
			InvalidateRect(hWnd, NULL, TRUE); // 再描画
		}
		if (LOWORD(wp) == 2)
		{
			LoadQuakeInfo();
			PlaySound(TEXT("quake.wav"), NULL, SND_ASYNC);
		}
		if (index == 2)
		{
			ShowWindow(hButton, SW_SHOW);
		}
		else {
			ShowWindow(hButton, SW_HIDE);
		}
		if (LOWORD(wp) == 1 && HIWORD(wp) == LBN_SELCHANGE) {
			InvalidateRect(hWnd, NULL, TRUE);

			if (index == 1) {
				SetFocus(hWnd);
			}
		}

		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	case WM_SIZE:
		btn(lp);
		MoveWindow(hList, 0, 0, 200, HIWORD(lp), TRUE);
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		id = MessageBox(
			hWnd,
			TEXT("終了しますか?"),
			TEXT("確認"),
			MB_YESNO | MB_ICONQUESTION
		);
		if (id == IDYES) DestroyWindow(hWnd);
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		if (index == 0) {
		GetClientRect(hWnd, &rect);
		rect.left = 210;
		DrawText(
			hdc,
			TEXT("ここの文字変えたら何かが変わる気がする"),
			-1,
			&rect,
			DT_LEFT | DT_TOP | DT_SINGLELINE
		);
		}
		else if (index == 1)
		{
			GetClientRect(hWnd, &rect);
			rect.left = 210;

			wchar_t wbuf[256];
			MultiByteToWideChar(CP_UTF8, 0, g_expr, -1, wbuf, 256);
			DrawTextW(
				hdc,
				wbuf,
				-1,
				&rect,
				DT_LEFT | DT_TOP | DT_SINGLELINE
			);
		}
		else if (index == 2)
		{
			GetClientRect(hWnd, &rect);
			rect.left = 210;

			if (hMapBmp) {
				HDC hMemDC = CreateCompatibleDC(hdc);
				SelectObject(hMemDC, hMapBmp);

				BitBlt(hdc, 220, 20, 800, 800, hMemDC, 0, 0, SRCCOPY);
				DeleteDC(hMemDC);
			}

			HBRUSH red = CreateSolidBrush(RGB(255, 0, 0));
			HBRUSH old = SelectObject(hdc, red);

			Ellipse(hdc, g_px - 5, g_py - 5, g_px + 5, g_py + 5);

			SelectObject(hdc, old);
			DeleteObject(red);

			DrawTextW(
				hdc,
				g_quakeInfo,
				-1,
				&rect,
				DT_LEFT | DT_TOP | DT_WORDBREAK
			);
		}
		else if (index == 3)
		{
			int y = 0;
			int x = 0;
			GetClientRect(hWnd, &rect);
			rect.left = 210;
			for (int y = 0; y < 20; y++)
			{
				for (int i = 0; i < 60; i++)
				{
					TextOut(
						hdc,
						rect.left + x + 20 * (i * y / 2),
						y + 20 * i,
						TEXT("技術くれしばくぞ"),
						lstrlen(TEXT("技術くれしばくぞ"))
					);
				}
			}
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		if (wp == 1) {
			LoadQuakeInfo();
			InvalidateRect(hWnd, NULL, TRUE); // 再描画
			PlaySound(TEXT("quake.wav"), NULL, SND_ASYNC);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			OutputDebugStringA("PlayWavDirectSound called\n");
		}
		break;
	case WM_CREATE:
	{
		CREATESTRUCT* pcs = (CREATESTRUCT*)lp;
		HINSTANCE hInst = pcs->hInstance;

		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP_ICON));
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}
	break;
	default:
		return (DefWindowProc(hWnd, msg, wp, lp));
	}
	return 0;
}

void LoadQuakeInfo() {
	char time[256];
	char place[256];
	char mag[256];
	char maxi[256];
	char cod[64];          // ★ 震源地の緯度経度
	char buffer[4096];
	DWORD bytesRead;
	DWORD total = 0;
	char* arrayStart;
	char* first;

	HINTERNET hInternet = InternetOpen(TEXT("tomatoApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) return;

	HINTERNET hFile = InternetOpenUrl(
		hInternet,
		TEXT("https://www.jma.go.jp/bosai/quake/data/list.json"),
		NULL, 0,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID,
		0
	);
	if (!hFile) {
		InternetCloseHandle(hInternet);
		return;
	}

	while (InternetReadFile(hFile, buffer + total, sizeof(buffer) - total - 1, &bytesRead) && bytesRead > 0) {
		total += bytesRead;
	}
	buffer[total] = '\0';

	arrayStart = strchr(buffer, '[');
	if (!arrayStart) {
		wcscpy_s(g_quakeInfo, 1024, L"配列が見つかりません");
		goto END;
	}

	first = arrayStart;
	while (*first && *first != '{') first++;
	if (!*first) {
		wcscpy_s(g_quakeInfo, 1024, L"地震データが見つかりません");
		goto END;
	}

	// ★ ここで初めて first を使って値を取る
	if (!(getValue(first, "\"rdt\"", time, sizeof(time)) &&
		getValue(first, "\"anm\"", place, sizeof(place)) &&
		getValue(first, "\"mag\"", mag, sizeof(mag)) &&
		getValue(first, "\"maxi\"", maxi, sizeof(maxi)) &&
		getValue(first, "\"cod\"", cod, sizeof(cod)))) {
		wcscpy_s(g_quakeInfo, 1024, L"地震情報の解析に失敗しました");
		goto END;
	}

	// ★ cod → 緯度経度 (+30.6+130.9 みたいな形式)
	double lat = atof(cod + 1);                 // 先頭の '+' を飛ばす
	char* pLon = strchr(cod + 1, '+');
	double lon = 0.0;
	if (pLon) {
		lon = atof(pLon);                       // 2つ目の '+' から経度
	}

	// ★ 地図の範囲とピクセル座標への変換
	double minLat = 24.0;
	double maxLat = 46.0;
	double minLon = 122.0;
	double maxLon = 154.0;

	int mapX = 220;
	int mapY = 20;
	int mapW = 800;
	int mapH = 800;

	g_px = mapX + (int)((lon - minLon) / (maxLon - minLon) * mapW);
	g_py = mapY + (int)((maxLat - lat) / (maxLat - minLat) * mapH);

	g_px -= 80;
	g_py += 25;

	// ★ 震源地名を Unicode に
	wchar_t placeJP[256];
	decodeUnicodeW(place, placeJP, 256);

	swprintf_s(
		g_quakeInfo,
		1024,
		L"発表時刻: %hs\n震源地: %ls\nマグニチュード: %hs\n最大震度: %hs",
		time, placeJP, mag, maxi
	);

END:
	InternetCloseHandle(hFile);
	InternetCloseHandle(hInternet);
}


int getValue(const char* json, const char* key, char* out, int outSize) {
	const char* p;
	int i;

	p = strstr(json, key);
	if (!p) return 0;

	p = strchr(p, ':');
	if (!p) return 0;
	p++;

	while (*p == ' ' || *p == '\t') p++;

	i = 0;

	if (*p == '\"') {
		p++;
		while (*p && *p != '\"' && i < outSize - 1) {
			out[i++] = *p++;
		}
	}
	else {
		while (*p && *p != ',' && *p != '}' && i < outSize - 1) {
			out[i++] = *p++;
		}
	}

	out[i] = '\0';
	return 1;
}

// \uXXXX を UTF-8 に変換する
int decodeUnicodeW(const char* src, wchar_t* dest, int destSize)
{
	int di = 0;

	while (*src && di < destSize - 1) {
		if (src[0] == '\\' && src[1] == 'u') {
			char hex[5];
			hex[0] = src[2];
			hex[1] = src[3];
			hex[2] = src[4];
			hex[3] = src[5];
			hex[4] = '\0';

			int code = strtol(hex, NULL, 16);
			dest[di++] = (wchar_t)code;

			src += 6;
		}
		else {
			dest[di++] = (unsigned char)*src++;
		}
	}

	dest[di] = L'\0';
	return di;
}

int btn(LPARAM lp) {
	int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
	int w = LOWORD(lp);
	int h = HIWORD(lp);

	MoveWindow(hButton, w - 110, 10, 100, 30, TRUE);
	if (index == 2) {
		ShowWindow(hButton, SW_SHOW);
	}
	else {
		ShowWindow(hButton, SW_HIDE);
	}
	return 0;
}
