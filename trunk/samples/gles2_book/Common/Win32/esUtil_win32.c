//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// esUtil_win32.c
//
//    This file contains the Win32 implementation of the windowing functions. 


///
// Includes
//

#include <windows.h>
#include <windowsx.h>
#include "esUtil.h"
#include <GLES2/gl2.h>

//////////////////////////////////////////////////////////////////
//
//  Private Functions
//
//

///
//  ESWindowProc()
//
//      Main window procedure
//
LRESULT WINAPI ESWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT lRet = 1;
	int lbutton = 0;
	int rbutton = 0;
	int mbutton = 0;

	ESContext *esContext = (ESContext*) (LONG_PTR) GetWindowLongPtr(hWnd, GWL_USERDATA);
	POINT point;
    GetCursorPos( &point );

	switch (uMsg) {
	case WM_CREATE:
		break;

	case WM_SIZE:
		if (esContext) {
			esContext->width = LOWORD( lParam );
			esContext->height = HIWORD( lParam );
			if (esContext->resizeFunc)
				esContext->resizeFunc(esContext, esContext->width,
						esContext->height);
			InvalidateRect(esContext->hWnd, NULL, FALSE);
		}
		// fall through to repaint

	case WM_PAINT:

		if (esContext && esContext->drawFunc)
			esContext->drawFunc(esContext);

		if (esContext)
			eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);

		if (esContext)
			ValidateRect(esContext->hWnd, NULL);
		break;

	case WM_KEYDOWN:
		if (esContext && esContext->keyFunc)
			esContext->keyFunc(esContext, ES_KEY_DOWN, wParam, 0);
		break;
	case WM_KEYUP:
		if (esContext && esContext->keyFunc)
			esContext->keyFunc(esContext, ES_KEY_UP, wParam, 0);
		break;
	case WM_CHAR:
		if (esContext && esContext->keyFunc)
			esContext->keyFunc(esContext, ES_KEY_TYPED, 0, wParam);
		break;

	case WM_LBUTTONDOWN:
		if(esContext && esContext->mouseFunc) {
			esContext->mouseFunc(esContext, ES_MOUSE_DOWN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ES_MOUSE_LBUTTON);
		}
		break;

	case WM_LBUTTONUP:
		if(esContext && esContext->mouseFunc) {
			esContext->mouseFunc(esContext, ES_MOUSE_UP, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ES_MOUSE_LBUTTON);
		}
		break;

	case WM_RBUTTONDOWN:
		if(esContext && esContext->mouseFunc) {
			esContext->mouseFunc(esContext, ES_MOUSE_DOWN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ES_MOUSE_RBUTTON);
		}
		break;

	case WM_RBUTTONUP:
		if(esContext && esContext->mouseFunc) {
			esContext->mouseFunc(esContext, ES_MOUSE_UP, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ES_MOUSE_RBUTTON);
		}
		break;

	case WM_MBUTTONDOWN:
		if(esContext && esContext->mouseFunc) {
			esContext->mouseFunc(esContext, ES_MOUSE_DOWN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ES_MOUSE_RBUTTON);
		}
		break;

	case WM_MBUTTONUP:
		if(esContext && esContext->mouseFunc) {
			esContext->mouseFunc(esContext, ES_MOUSE_UP, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ES_MOUSE_RBUTTON);
		}
		break;

	case WM_MOUSEMOVE:
		if(esContext && esContext->mouseFunc) {
			lbutton = wParam & MK_LBUTTON?ES_MOUSE_LBUTTON:0;
			rbutton = wParam & MK_RBUTTON?ES_MOUSE_RBUTTON:0;
			mbutton = wParam & MK_MBUTTON?ES_MOUSE_MBUTTON:0;

			esContext->mouseFunc(esContext, ES_MOUSE_MOVE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), lbutton | rbutton | mbutton);
		}
		break;

	case WM_MOUSEWHEEL:
		if(esContext && esContext->mouseFunc) {
			esContext->mouseFunc(esContext, ES_MOUSE_WHEEL, GET_WHEEL_DELTA_WPARAM(wParam), 0, 0);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return lRet;
}

//////////////////////////////////////////////////////////////////
//
//  Public Functions
//
//

///
//  WinCreate()
//
//      Create Win32 instance and window
//
GLboolean WinCreate(ESContext *esContext, LPCTSTR title, int flags) {
	WNDCLASS wndclass = { 0 };
	DWORD wStyle = 0;
	RECT windowRect;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC) ESWindowProc;
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = TEXT("opengles2.0");	

	if (!RegisterClass(&wndclass))
		return FALSE;

	if (flags & ES_WINDOW_FULLSCREEN) {
		wStyle = WS_VISIBLE | WS_POPUP;
	} else {
		wStyle = WS_OVERLAPPEDWINDOW;
	}

	// Adjust the window rectangle so that the client area has
	// the correct number of pixels
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = esContext->width;
	windowRect.bottom = esContext->height;

	AdjustWindowRect(&windowRect, wStyle, FALSE);

	esContext->hWnd = CreateWindow(
			TEXT("opengles2.0"),
			title,
			wStyle,
			0,
			0,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			hInstance,
			NULL);

	// Set the ESContext* to the GWL_USERDATA so that it is available to the
	// ESWindowProc
	SetWindowLongPtr(esContext->hWnd, GWL_USERDATA, (LONG) (LONG_PTR) esContext);

	if (esContext->hWnd == NULL)
		return GL_FALSE;

	ShowWindow(esContext->hWnd, TRUE);

	return GL_TRUE;
}

///
//  winLoop()
//
//      Start main windows loop
//
void WinLoop(ESContext *esContext) {
	MSG msg = { 0 };
	int done = 0;
	DWORD lastTime = GetTickCount();

	if(esContext && esContext->resizeFunc) {
		esContext->resizeFunc(esContext, esContext->width, esContext->height);
	}

	while (!done) {
		int gotMsg = (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
		DWORD curTime = GetTickCount();
		float deltaTime = (float) (curTime - lastTime) / 1000.0f;
		lastTime = curTime;

		if (gotMsg) {
			if (msg.message == WM_QUIT) {
				done = 1;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
			else
			SendMessage(esContext->hWnd, WM_PAINT, 0, 0);

		// Call update function if registered
		if (esContext->updateFunc != NULL)
			esContext->updateFunc(esContext, deltaTime);
	}
	
	if(esContext->quitFunc != NULL)
		esContext->quitFunc(esContext);
	eglTerminate(esContext->eglDisplay);
}

void ESUTIL_API esQuit (ESContext *esContext) {
	PostQuitMessage(0);
}
