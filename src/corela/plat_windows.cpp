#ifdef _WIN32

#define _WIN32_WINNT 0x0501
#include <Windows.h>
#include <Commdlg.h>
#include <MMSystem.h>
#include "corela_t.h"
#include "glutils.h"
#include <stdio.h>
#include <string>
#include <vector>

DWORD dwWinStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

static FRAME* g_ActiveFrame = 0;
static int g_frameCount = 0;
static int g_touches = 0;
static float g_touchX[64];
static float g_touchY[64];

// reverse engineered NEC multitouch data
#pragma pack(1)
struct NEC_HID_INFO_TOUCH {
	unsigned char touchdown;
	unsigned char index;
	unsigned short x;
	unsigned short y;
	unsigned short k;
	unsigned short l;
};
#pragma pack(1)
struct NEC_HID_INFO {
	unsigned long size;
	unsigned long count;
	unsigned char unknown1;
	NEC_HID_INFO_TOUCH touch[5];
};

std::string RequestFileName() {
	OPENFILENAMEA ofn;
	char fileName[MAX_PATH] = "";
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn) ) {
		return std::string(&fileName[0]);
	}
	return "";
}

void _FrameResize(FRAME* frame) {
	HWND hwnd = (HWND)frame->hwnd;
	RECT rc;
	GetClientRect(hwnd, &rc);
	glViewport(0, 0, rc.right-rc.left, rc.bottom-rc.top);
	frame->width = (float)(rc.right-rc.left);
	frame->height = (float)(rc.bottom-rc.top);
}

void FrameToggleFull(FRAME* frame) {
	if(!frame) { return; }

	HWND hwnd = (HWND)frame->hwnd;
	RECT rc;
	POINT ptClient;

	if(GetWindowLongPtr(hwnd, GWL_STYLE) & dwWinStyle) {  // change to frameless
		ptClient.x = 0;
		ptClient.y = 0;
		ClientToScreen(hwnd, &ptClient);
		GetClientRect(hwnd, &rc);
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(hwnd, HWND_NOTOPMOST, ptClient.x, ptClient.y, rc.right-rc.left+1, rc.bottom-rc.top+1, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		Sleep(20);
#ifdef _DEBUG
		SetWindowPos(hwnd, HWND_NOTOPMOST, ptClient.x, ptClient.y, rc.right-rc.left, rc.bottom-rc.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
#else
		SetWindowPos(hwnd, HWND_TOPMOST, ptClient.x, ptClient.y, rc.right-rc.left, rc.bottom-rc.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
#endif
	} else {  // change to normal window (with frame)
		GetWindowRect(hwnd, &rc);
		AdjustWindowRect(&rc, dwWinStyle, FALSE);
		SetWindowLongPtr(hwnd, GWL_STYLE, dwWinStyle);
		SetWindowPos(hwnd, HWND_NOTOPMOST, rc.left, rc.top, rc.right-rc.left+1, rc.bottom-rc.top+1, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		Sleep(20);
		SetWindowPos(hwnd, HWND_NOTOPMOST, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}
	SetForegroundWindow(hwnd);
	_FrameResize(frame);
}

void _FrameGetMousePosition(FRAME* frame, double* x, double* y) {
	// global mouse position
	POINT ptMouse;
	GetCursorPos(&ptMouse);
	ScreenToClient((HWND)frame->hwnd, &ptMouse);

	// translate mouse position to window
	RECT rcClient;
	GetClientRect((HWND)frame->hwnd, &rcClient);

	// return result
	if(x) {
		*x = (double)ptMouse.x / frame->width * frame->vwidth;
	}
	if(y) {
		*y = (double)ptMouse.y / frame->height * frame->vheight;
	}
}

static std::string ScancodeToAscii(UINT vk) {
	static HKL layout = GetKeyboardLayout(0);
	static unsigned char State[256];
	unsigned short result;

	if (GetKeyboardState(State) == FALSE) return 0;

	DWORD scancode = MapVirtualKeyEx(vk, 4, layout);
	//ToAsciiEx(vk, scancode, State, &result, 0, layout);
	
	WCHAR uc[8] = {0,0,0,0,0,0,0,0};
	int uclen;
	uclen = ToUnicodeEx(vk, scancode, State, uc, 6, 0, layout);
	
	char utf8[8] = {0,0,0,0,0,0,0,0};
	//wctomb(utf8, uc);
	WideCharToMultiByte(CP_UTF8, 0, uc, wcslen(uc), utf8, 6, 0, 0);
	
	std::string out = utf8;

	return out;
}

#define EVENT_RAISE(e,_x,_y,_px,_py,_a,_b,_t) {if(frame){fevent.id=e;fevent.sender=frame;fevent.user=frame->user;fevent.x=_x;fevent.y=_y;fevent.prevX=_px;fevent.prevY=_py;fevent.button=_a;fevent.key=_b;fevent.text=_t;frame->cb(&fevent);}}
LRESULT CALLBACK _FrameWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// get frame object from window's user data
	FRAME* frame = (FRAME*)GetWindowLongPtrA(hwnd, GWL_USERDATA);

	// get translated mouse position
	static double mx=-1.0,my=-1.0;
	double px = mx, py = my;
	if(frame) {
		_FrameGetMousePosition(frame, &mx, &my);
	}

	FRAME_EVENT fevent;
	switch(msg) {
	case WM_CREATE:
		break;

	case WM_CLOSE:
		EVENT_RAISE(EVENT_CLOSE, mx, my, px, py, 0, wParam, "");
		break;

	case WM_QUIT:
		PostQuitMessage(0);
		break;

	case WM_WINDOWPOSCHANGED:
		_FrameResize(frame);
		break;

	case WM_SIZE:
		break;

	case WM_LBUTTONDOWN:
		EVENT_RAISE(EVENT_MOUSE_BUTTON_DOWN, mx, my, px, py, 0, 0, "");
		break;

	case WM_LBUTTONUP:
		EVENT_RAISE(EVENT_MOUSE_BUTTON_UP, mx, my, px, py, 0, 0, "");
		break;

	case WM_RBUTTONDOWN:
		EVENT_RAISE(EVENT_MOUSE_BUTTON_DOWN, mx, my, px, py, 1, 0, "");
		break;

	case WM_RBUTTONUP:
		EVENT_RAISE(EVENT_MOUSE_BUTTON_UP, mx, my, px, py, 1, 0, "");
		break;

	case WM_MOUSEMOVE:
		EVENT_RAISE(EVENT_MOUSE_MOVE, mx, my, px, py, 0, 0, "");
		break;

	case WM_KEYDOWN:
		if(~lParam & 0x40000000) {
			EVENT_RAISE(EVENT_KEY_DOWN, mx, my, px, py, 0, wParam, ScancodeToAscii(wParam));
		}
		break;

	case WM_KEYUP:
		EVENT_RAISE(EVENT_KEY_UP, mx, my, px, py, 0, wParam, ScancodeToAscii(wParam));
		if(wParam == VK_F11) {
			FrameToggleFull(frame);
		}
		if(wParam == VK_ESCAPE) {
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
		if(wParam == VK_F12) {
			//FrameShowCursor(frame, !frame->showCursor);
			FrameShowCursor(frame, true);
			ShowWindow(GetConsoleWindow(), SW_SHOW);
		}
		break;

	case WM_SETCURSOR:
		if(frame) {
			if(!frame->showCursor) {
				SetCursor(NULL);
			}
		}
		break;

	case WM_INPUT: {
		// determine the size of the input data
		UINT data_size = 0;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &data_size, sizeof(RAWINPUTHEADER));
		// preallocate our buffer
		std::vector<BYTE> data;
		data.resize (data_size);
		// and then read the input data in
		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &data[0], &data_size, sizeof(RAWINPUTHEADER)) != data_size) { break; }
		// the RAWINPUT structure starts at the beginning of our data array
		RAWINPUT* raw = (RAWINPUT*)(&data [0]);
		if (raw->header.dwType == RIM_TYPEHID) {
			// get device info to check device vendor and product id
			RID_DEVICE_INFO info;
			UINT len = sizeof(RID_DEVICE_INFO);
			GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICEINFO, &info, &len);

			// NEC display multitouch:
			if(info.hid.dwVendorId == 0x1926 && info.hid.dwProductId == 0x367) {
				for (DWORD index = 0; index < raw->data.hid.dwCount; ++index) {
					NEC_HID_INFO* mt = (NEC_HID_INFO*)&raw->data.hid;
					g_touches = 0;
					if(raw->data.hid.dwSizeHid != 54) { break; }
					while(mt->touch[g_touches].touchdown) {
						g_touchX[g_touches] = float(mt->touch[g_touches].x) / 32767.0;
						g_touchY[g_touches] = float(mt->touch[g_touches].y) / 32767.0;
						g_touches++;
						if(g_touches == 5) { break; }
					}
				}
			}
		}
		break;
	}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void FrameShow(FRAME* frame, int show) {
	if(show) {
		ShowWindow((HWND)frame->hwnd, SW_SHOW);
	} else {
		ShowWindow((HWND)frame->hwnd, SW_HIDE);
	}
}

FRAME* FrameCreate(const char* title, EVENT_CALLBACK event_cb, void* user, long x, long y, long w, long h) {
	char className[16];
	sprintf(className, "xstart-%d", g_frameCount++);

	Log(LOG_DEBUG, "Registering window class ...");
	WNDCLASSEXA wc;
	memset(&wc,0,sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = _FrameWinProc;
	wc.lpszClassName = className;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hCursor       = NULL; //LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.hInstance     = GetModuleHandle(0);
	wc.hIcon         = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
	wc.hIconSm       = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, 0);
	RegisterClassEx(&wc);

	Log(LOG_DEBUG, "Creating window ...");
#ifdef _DEBUG
	HWND hwnd = CreateWindowExA(0, className, title, WS_POPUP, x, y, w, h, NULL, NULL, GetModuleHandle(0), NULL);
#else
	HWND hwnd = CreateWindowExA(WS_EX_TOPMOST, className, title, WS_POPUP, x, y, w, h, NULL, NULL, GetModuleHandle(0), NULL);
#endif

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_COPY,    // Flags
		PFD_TYPE_RGBA,            // The kind of framebuffer. RGBA or palette.
		32,                       // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,         // Colorbuffer bits
		0,0,                      // Alpha bits, alpha shift
		32,                       // Accumulation buffer depth
		0, 0, 0, 0,               // Accumulation buffer bits
		24,                       // Number of bits for the depthbuffer
		8,                        // Number of bits for the stencilbuffer
		0,                        // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	// get device context for window
	Log(LOG_DEBUG, "Creating device-context ...");
	HDC hdc = GetDC(hwnd);

	// apply pixel format
	// TODO: Change to wglChoosePixelFormatARB.
	int ipf = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, ipf, &pfd);

	// create GL rendering context and make current
	static HGLRC prev_rc = 0;
	HGLRC rc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, rc);
	if(prev_rc) {
		wglShareLists(prev_rc, rc);
	}
	if(!prev_rc) {
		prev_rc = rc;
	}

	// create frame object
	FRAME* frame = (FRAME*)malloc(sizeof(FRAME));
	frame->hwnd = hwnd;
	frame->hdc = hdc;
	frame->rc = rc;
	frame->user = user;
	frame->width = (float)w;
	frame->height = (float)h;
	frame->vwidth = (float)w;
	frame->vheight = (float)h;
	frame->cb = event_cb;
	frame->showCursor = false;
	SetWindowLongPtrA(hwnd, GWL_USERDATA, (LONG)frame);

	// setup for 2D drawing
	glUtilsTo2D(0.0, 0.0, frame->vwidth, frame->vheight, 0.0, 1.0);

	// set some defaults
	glClearColor(0.0,0.0,0.0,1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// clear buffer and swap
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SwapBuffers(hdc);

	// initialize opengl
	Log(LOG_DEBUG, "Initializing GL support ...");
	glUtilsInit();
	GLSLInit();
	Log(LOG_DEBUG, "OpenGL system created successfully.");

	//register for multi-touch HID messages
	RAWINPUTDEVICE raw_input_device [1];
	raw_input_device [0].usUsagePage = 0x0D;
	raw_input_device [0].dwFlags = RIDEV_INPUTSINK | RIDEV_PAGEONLY;
	raw_input_device [0].usUsage = 0x00;
	raw_input_device [0].hwndTarget = hwnd;
	RegisterRawInputDevices(raw_input_device, 1, sizeof(raw_input_device[0]));

#ifndef _DEBUG
	// hide console
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	return frame;
}

void FrameClose(FRAME* frame) {
	CloseWindow((HWND)frame->hwnd);
	DestroyWindow((HWND)frame->hwnd);
	free(frame);
}

bool FrameUpdate() {
	HWND hwnd = NULL;
	MSG msg;

	while(PeekMessage(&msg, hwnd, 0, 0, FALSE)) {
		GetMessage(&msg, hwnd, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void FrameSelect(FRAME* frame) {
	g_ActiveFrame = frame;
	wglMakeCurrent(frame->hdc, frame->rc);
	_FrameResize(frame);
}

void FrameFlip(FRAME* frame) {
	SwapBuffers(frame->hdc);
}

void FrameShowCursor(FRAME* frame, coBool show) {
	frame->showCursor = show;
	if(show) {
		SetClassLong((HWND)frame->hwnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
	} else {
		SetClassLong((HWND)frame->hwnd, GCL_HCURSOR, NULL);
	}
}

void FrameMaximize(FRAME* frame) {
	ShowWindow((HWND)frame->hwnd, SW_MAXIMIZE);
}

void FrameMinimize(FRAME* frame) {
	ShowWindow((HWND)frame->hwnd, SW_MINIMIZE);
}

void* DLLGetFunctionPtr(const char* dll, const char* function) {
	HINSTANCE hdll = LoadLibraryA(dll);
	if(hdll == NULL) {
		Log(LOG_ERROR, "Dynamic library '%s' not found, function '%s' unavailable!", dll, function);
		return 0;
	}
	FARPROC fnptr = GetProcAddress(hdll, function);
	if(!fnptr) {
		Log(LOG_ERROR, "Function '%s' not found in libary '%s'!", function, dll);
		return 0;
	}
	return (void*)fnptr;
}

void FrameSetScissor(FRAME* frame, int left, int top, int right, int bottom) {
	if(left >= 0) {
		glEnable(GL_SCISSOR_TEST);

		left = (int)( (float)left / frame->vwidth * frame->width );
		top = (int)( (float)top / frame->vheight * frame->height );
		right = (int)( (float)right / frame->vwidth * frame->width);
		bottom = (int)( (float)bottom / frame->vheight * frame->height );

		//glScissor(left, frame->height - bottom, right - left, bottom - top);
		glScissor(left, top, right - left, bottom - top);
	} else {
		glDisable(GL_SCISSOR_TEST);
	}
}

void FrameActivate(FRAME* frame) {
	SetForegroundWindow((HWND)frame->hwnd);
	SetActiveWindow((HWND)frame->hwnd);
	SetFocus((HWND)frame->hwnd);

	HWND hwndFore = GetForegroundWindow();
	if(hwndFore != (HWND)frame->hwnd) {
//		SetParent(hwndFore, (HWND)frame->hwnd);
//		SetForegroundWindow((HWND)frame->hwnd);
//		SetActiveWindow((HWND)frame->hwnd);
//		SetFocus((HWND)frame->hwnd);

		ShowWindow((HWND)frame->hwnd, SW_MINIMIZE);
		ShowWindow((HWND)frame->hwnd, SW_RESTORE);
	}
}

int FrameGetMultitouchCount(FRAME* frame) {
	return g_touches;
}

bool FrameGetMultitouch(FRAME* frame, int index, float* x, float* y) {
	if(index < g_touches) {
		*x = g_touchX[index];
		*y = g_touchY[index];
		return true;
	}
	return false;
}

#pragma comment(lib, "Winmm")
void SoundSimplePlay(const char* file) {
	PlaySoundA(file, NULL, SND_FILENAME | SND_ASYNC);
}

int nMonitors;
RECT rcMonitors[8];
BOOL CALLBACK EnumMonitorProc(HMONITOR hm, HDC hdc, LPRECT pRect, LPARAM dwData) {
	rcMonitors[nMonitors] = *pRect;
	nMonitors++;
	return TRUE;
}

bool GetMonitorRect(int i, CORECT* rcOut) {
	nMonitors = 0;
	EnumDisplayMonitors(NULL, NULL, EnumMonitorProc, 0);
	if(i > nMonitors) { return false; }
	if(rcOut) {
		rcOut->left = rcMonitors[i].left;
		rcOut->top = rcMonitors[i].top;
		rcOut->right = rcMonitors[i].right;
		rcOut->bottom = rcMonitors[i].bottom;
	}
	return true;
}


#endif
