#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION

#include "ui_global.h"

#include <nuklear/nuklear.h>
#include <nuklear/nuklear_gdi.h>
#include "draw.h"

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

bool show_debug_layout = false;
bool data_debug_layout = false;

static LRESULT CALLBACK
WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			if (wparam == VK_F11) {
				show_debug_layout = !show_debug_layout;
				InvalidateRect(wnd, NULL, FALSE); // force redraw
			}
			if (wparam == VK_F10) {
				data_debug_layout = !data_debug_layout;
				InvalidateRect(wnd, NULL, FALSE); // force redraw
				return 0;
			}
			break;
		}
		case WM_SIZE: {
			width = LOWORD(lparam);
			height = HIWORD(lparam);

			InvalidateRect(wnd, NULL, TRUE); // force redraw
			return 0;
		}
	}

	if (nk_gdi_handle_event(wnd, msg, wparam, lparam))
		return 0;

	return DefWindowProcW(wnd, msg, wparam, lparam);
}
static volatile int running = 1;
void ui_mainTerminate() {
	running = 0;
}

int ui_main()
{
	running = 1;
	struct nk_context* ctx;
	WNDCLASSW wc;
	ATOM atom;
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD exstyle = WS_EX_APPWINDOW;
	HWND wnd;
	HDC dc;
	int needs_refresh = 1;

	/* Win32 */
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandleW(0);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"osu! Tracker";
	atom = RegisterClassW(&wc);

	AdjustWindowRectEx(&rect, style, FALSE, exstyle);
	wnd = CreateWindowExW(exstyle, wc.lpszClassName, WIDEN(OSU_TRACKER_TITLE),
		style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, wc.hInstance, NULL);
	dc = GetDC(wnd);
	/* GUI */
	GdiFont* font;
	GdiFont* fontHeader;
	GdiFont* fontSmall;

	int fontSizeOffset = 2;
	fontHeader = nk_gdifont_create("Segoe UI", 24 + fontSizeOffset);
	font = nk_gdifont_create("Segoe UI", 16 + fontSizeOffset);
	fontSmall = nk_gdifont_create("Segoe UI", 16 + fontSizeOffset);

	ctx = nk_gdi_init(font, dc, WINDOW_WIDTH, WINDOW_HEIGHT);

	DWORD lastFrameTime = GetTickCount();
	const DWORD targetFrameMS = 1000 / 60; // 60 FPS

	while (running)
	{
		MSG msg;
		bool hasInput = false;

		nk_input_begin(ctx); // Begin input handling

		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				running = 0;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			hasInput = true;
		}

		nk_input_end(ctx); // End input handling

		DWORD now = GetTickCount();
		DWORD delta = now - lastFrameTime;

		if (!hasInput) {
			// Limit to 60 FPS when idle
			if (delta < targetFrameMS) {
				Sleep(targetFrameMS - delta);
				continue;
			}
		}
		lastFrameTime = now;
		// Always draw
		if (width != nk_width || height != nk_height) {
			nk_gdi_shutdown();
			ctx = nk_gdi_init(font, dc, width, height);
			nk_width = width;
			nk_height = height;
		}
		nk_style_default(ctx);
		drawContent(ctx, font, fontSmall, fontHeader, width, height, _app, _user, _entries, _entry_count, show_debug_layout, data_debug_layout);
		nk_end(ctx);
		nk_gdi_render(nk_rgb(0, 0, 0));
	}

	// Cleanup
	nk_gdifont_del(font);
	nk_gdifont_del(fontHeader);
	nk_gdifont_del(fontSmall);
	ReleaseDC(wnd, dc);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
	nk_gdi_shutdown();
	return 0;
}