#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

read_only OS_Key key_table[] = 
{
	[VK_F1] = OS_Key_F1, 
	[VK_F2] = OS_Key_F2, 
	[VK_F3] = OS_Key_F3, 
	[VK_F4] = OS_Key_F4, 
	[VK_F5] = OS_Key_F5, 
	[VK_F6] = OS_Key_F6, 
	[VK_F7] = OS_Key_F7,
	[VK_F8] = OS_Key_F8, 
	[VK_F9] = OS_Key_F9, 
	[VK_F10] = OS_Key_F10,
	[VK_F11] = OS_Key_F11, 
	[VK_F12] = OS_Key_F12,
	
	['A'] = OS_Key_A,
	['B'] = OS_Key_B,
	['C'] = OS_Key_C,
	['D'] = OS_Key_D,
	['E'] = OS_Key_E,
	['F'] = OS_Key_F,
	['G'] = OS_Key_G,
	['H'] = OS_Key_H,
	['I'] = OS_Key_I,
	['J'] = OS_Key_J,
	['K'] = OS_Key_K,
	['L'] = OS_Key_L,
	['M'] = OS_Key_M,
	['N'] = OS_Key_N,
	['O'] = OS_Key_O,
	['P'] = OS_Key_P,
	['Q'] = OS_Key_Q,
	['R'] = OS_Key_R,
	['S'] = OS_Key_S,
	['T'] = OS_Key_T,
	['U'] = OS_Key_U,
	['V'] = OS_Key_V,
	['W'] = OS_Key_W,
	['X'] = OS_Key_X,
	['Y'] = OS_Key_Y,
	['Z'] = OS_Key_Z,
	
	['0'] = OS_Key_0,
	['1'] = OS_Key_1,
	['2'] = OS_Key_2,
	['3'] = OS_Key_3,
	['4'] = OS_Key_4,
	['5'] = OS_Key_5,
	['6'] = OS_Key_6,
	['7'] = OS_Key_7,
	['8'] = OS_Key_8,
	['9'] = OS_Key_9,
	
	[VK_LCONTROL] = OS_Key_LCTRL,
	[VK_RCONTROL] = OS_Key_RCTRL,
	[VK_LSHIFT] = OS_Key_LSHIFT,
	[VK_RSHIFT] = OS_Key_RSHIFT,
	[VK_LMENU] = OS_Key_LALT,
	[VK_RMENU] = OS_Key_RALT,
	
	[VK_LEFT] = OS_Key_LEFT,
	[VK_RIGHT] = OS_Key_RIGHT,
	[VK_UP] = OS_Key_UP,
	[VK_DOWN] = OS_Key_DOWN,
	
	[VK_TAB] = OS_Key_TAB,
	[VK_CAPITAL] = OS_Key_CAPS,
	[VK_RETURN] = OS_Key_ENTER,
	[VK_ESCAPE] = OS_Key_ESC,
	[VK_SPACE] = OS_Key_SPACE,
};

function void *os_reserve(u64 size)
{
	void *out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	if (out != NULL)
	{
		total_res += size;
	}
	return out;
}

function b32 os_commit(void *ptr, u64 size)
{
	if (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) == NULL)
	{
		printf("VirtualAlloc commit failed: %lu\r\n", GetLastError());
		return 0;
	}
	total_cmt += size;
	
	return 1;
}

function void os_decommit(void *ptr, u64 size)
{
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

function void os_free(void *ptr, u64 size)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}

function u64 os_getPageSize()
{
	SYSTEM_INFO sysinfo = {0};
	GetSystemInfo(&sysinfo);
	return sysinfo.dwPageSize;
}

function void os_sleep(s32 ms)
{
	Sleep(ms);
}

function Str8 os_getAppDir(Arena *arena)
{
	char buffer[256];
	DWORD len = GetModuleFileName(0, buffer, 256);
	
	char *c = &buffer[len];
	while(*(--c) != '\\')
	{
		*c = 0;
		--len;
	}
	
	u8 *str = pushArray(arena, u8, len);
	memcpy(str, buffer, len);
	
	Str8 out = str8(str, len);
	
	return out;
}

function u64 os_getPerfCounter()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (u64)counter.QuadPart;
}

function u64 os_getPerfFreq()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return (u64)frequency.QuadPart;
}

function OS_Handle os_loadLibrary(char *name)
{
	OS_Handle out = {0};
	HMODULE dll = LoadLibraryA(name);
	out.u64[0] = (u64)dll;
	return out;
}

function void *os_loadFunction(OS_Handle handle, char *name)
{
	HMODULE dll = (HMODULE)handle.u64[0];
	void *out = (void*)GetProcAddress(dll, name);
	return out;
}

typedef struct OS_Window OS_Window;
struct OS_Window
{
	HWND hwnd;
 v2s size;
};

typedef struct OS_State OS_State;
struct OS_State
{
	Arena *arena;
	OS_Window win[OS_MAX_WIN];
	
	s32 num;
};

global OS_State *os_state;
global OS_EventList event_list;
global Arena *event_arena;

function void os_innit()
{
	Arena *arena = arenaAlloc();
	os_state = pushArray(arena, OS_State, 1);
	os_state->arena = arena;
	
	SetProcessDPIAware();
}

function OS_EventList os_pollEvents(Arena *arena)
{
	event_arena = arena;
	event_list = (OS_EventList){0};
	
	MSG msg;
	while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	
	return event_list;
}

function OS_Window *os_windowFromHWND(HWND hwnd)
{
	OS_Window *out = 0;
	for(s32 i = 0; i < os_state->num; i++)
	{
		OS_Window *cur = os_state->win + i;
		if(hwnd == cur->hwnd)
		{
			out = cur;
			break;
		}
	}
	return out;
}

function OS_Window *os_windowFromHandle(OS_Handle handle)
{
	return (OS_Window*)handle.u64[0];
}

function LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OS_Window *win = os_windowFromHWND(hwnd);
	
	switch (msg)
	{
		case WM_DESTROY:
		{
			printf("re");
			OS_Event *event = os_pushEvent(event_arena, &event_list);
			event->key = OS_Key_NULL;
			event->kind = OS_EventKind_CloseRequested;
		}break;
		
		// bonne article
		// https://blog.molecular-matters.com/2011/09/05/properly-handling-keyboard-input/
		case WM_INPUT:
		{
			char buffer[sizeof(RAWINPUT)] = {0};
			UINT size = sizeof(RAWINPUT);
			
			GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));
			
			RAWINPUT *raw = (RAWINPUT *)(buffer);
			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				const RAWKEYBOARD *rawKB = &raw->data.keyboard;
				
				UINT virtualKey = rawKB->VKey;
				UINT scanCode = rawKB->MakeCode;
				UINT flags = rawKB->Flags;
				
				if (virtualKey == 255)
				{
					// discard "fake keys" which are part of an escaped sequence
					return 0;
				}
				else if (virtualKey == VK_SHIFT)
				{
					// correct left-hand / right-hand SHIFT
					virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
				}
				else if (virtualKey == VK_NUMLOCK)
				{
					// correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit
					scanCode = (MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC) | 0x100);
				}
				
				b32 isE0 = ((flags & RI_KEY_E0) != 0);
				b32 isE1 = ((flags & RI_KEY_E1) != 0);
				
				if (isE1)
				{
					// for escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
					// however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
					if (virtualKey == VK_PAUSE)
						scanCode = 0x45;
					else
						scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
				}
				
				switch (virtualKey)
				{
					// right-hand CONTROL and ALT have their e0 bit set
					case VK_CONTROL:
					if (isE0)
						virtualKey = VK_RCONTROL;
					else
						virtualKey = VK_LCONTROL;
					break;
					case VK_MENU:
					if (isE0)
						virtualKey = VK_RMENU;
					else
						virtualKey = VK_LMENU;
					break;
					
					// NUMPAD ENTER has its e0 bit set
					/*
					* I don't care for numpad enter
					case VK_RETURN:
					if (isE0)
						virtualKey = VK_NUMPAD_ENTER (FUCK MICROSOFT!);
					break;
					*/
					// the standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have their e0 bit set, but the
					// corresponding keys on the NUMPAD will not.
					case VK_INSERT:
					if (!isE0)
						virtualKey = VK_NUMPAD0;
					break;
					/*
					case VK_DELETE:
					if (!isE0)
									virtualKey = VK_NUMPADP_DECIMAL// (FUCK MICROSOFT!);
					break;
					*/
					case VK_HOME:
					if (!isE0)
						virtualKey = VK_NUMPAD7;
					break;
					
					case VK_END:
					if (!isE0)
						virtualKey = VK_NUMPAD1;
					break;
					
					case VK_PRIOR:
					if (!isE0)
						virtualKey = VK_NUMPAD9;
					break;
					
					case VK_NEXT:
					if (!isE0)
						virtualKey = VK_NUMPAD3;
					break;
					
					// the standard arrow keys will always have their e0 bit set, but the
					// corresponding keys on the NUMPAD will not.
					case VK_LEFT:
					if (!isE0)
						virtualKey = VK_NUMPAD4;
					break;
					
					case VK_RIGHT:
					if (!isE0)
						virtualKey = VK_NUMPAD6;
					break;
					
					case VK_UP:
					if (!isE0)
						virtualKey = VK_NUMPAD8;
					break;
					
					case VK_DOWN:
					if (!isE0)
						virtualKey = VK_NUMPAD2;
					break;
					
					// NUMPAD 5 doesn't have its e0 bit set
					case VK_CLEAR:
					if (!isE0)
						virtualKey = VK_NUMPAD5;
					break;
				}
				
				b32 wasUp = ((flags & RI_KEY_BREAK) != 0);
				OS_Key key = key_table[virtualKey];
				
				if(key)
				{
					OS_Event *event = os_pushEvent(event_arena, &event_list);
					
					event->key = key;
					event->kind = wasUp ? OS_EventKind_Released : OS_EventKind_Pressed;
				}
			}
			else if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_DOWN)
				{
					OS_Event *event = os_pushEvent(event_arena, &event_list);
					
					event->key = OS_Key_LMB;
					event->kind = OS_EventKind_Pressed;
				}
				else if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_UP)
				{
					OS_Event *event = os_pushEvent(event_arena, &event_list);
					
					event->key = OS_Key_LMB;
					event->kind = OS_EventKind_Released;
				}
				
				if (raw->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_DOWN)
				{
					OS_Event *event = os_pushEvent(event_arena, &event_list);
					
					event->key = OS_Key_RMB;
					event->kind = OS_EventKind_Pressed;
				}
				else if (raw->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_UP)
				{
					OS_Event *event = os_pushEvent(event_arena, &event_list);
					
					event->key = OS_Key_RMB;
					event->kind = OS_EventKind_Released;
				}
				
				if (raw->data.mouse.usButtonFlags == RI_MOUSE_MIDDLE_BUTTON_DOWN)
				{
					OS_Event *event = os_pushEvent(event_arena, &event_list);
					
					event->key = OS_Key_MMB;
					event->kind = OS_EventKind_Pressed;
				}
				else if (raw->data.mouse.usButtonFlags == RI_MOUSE_MIDDLE_BUTTON_UP)
				{
					OS_Event *event = os_pushEvent(event_arena, &event_list);
					
					event->key = OS_Key_MMB;
					event->kind = OS_EventKind_Released;
				}
				
				// printf("%ld\n", raw->data.mouse.lLastX);
			}
		}break;
		
		case WM_MOUSEMOVE:
		{
			s32 xpos = GET_X_LPARAM(lParam);
			s32 ypos = GET_Y_LPARAM(lParam);
			
			OS_Event *event = os_pushEvent(event_arena, &event_list);
			event->mpos = (v2f){(f32)xpos, (f32)ypos};
			event->kind = OS_EventKind_MouseMove;
			
			/*
				POINT point = { (LONG)(win->win_data.size_x / 2.f),(LONG)(win->win_data.size_y / 2.f) };

				ClientToScreen((HWND)win->win_handle, &point);
				SetCursorPos(point.x, point.y);
			*/
			// ykm_print_v2(yk_input_mouse_mv(&win->mouse_pos));
		}
		break;
		
		// to stop that annoying alt window focus. People use it for accessibilty or something.
		// But I am the devil
		case WM_SYSKEYDOWN:
		if (wParam == VK_MENU) 
		{
			return 0;
		}
		else if (wParam == VK_F10) 
		{
			return 0;
		}
		break;
		
		case WM_SYSKEYUP:
		if (wParam == VK_MENU) 
		{
			return 0;
		}
		else if (wParam == VK_F10) 
		{
			return 0;
		}
		break;
		
	}
	
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

function OS_Handle os_openWindow(char *title, f32 x, f32 y, f32 w, f32 h)
{
	OS_Window *win = &os_state->win[os_state->num++];
	WNDCLASSA wc = {0};
	
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = "main";
	
	RegisterClass(&wc);
	
	win->hwnd = CreateWindowA(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, x, y, w, h, 0, 0, wc.hInstance, 0);
	win->size = (v2s){w, h};
 
	RAWINPUTDEVICE devices[2] = {0};
	
	devices[0].usUsagePage = 0x01;
	devices[0].usUsage = 0x06;
	devices[0].dwFlags = 0;
	devices[0].hwndTarget = win->hwnd;
	
	devices[1].usUsagePage = 0x01;
	devices[1].usUsage = 0x02;
	devices[1].dwFlags = 0;
	devices[1].hwndTarget = win->hwnd;
	
	RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE));
	
	ShowWindow(win->hwnd, SW_SHOWNORMAL);
	UpdateWindow(win->hwnd);
	
	OS_Handle out = {0};
	out.u64[0] = (u64)win;
	return out;
}

function v2s os_getWindowSize(OS_Handle handle)
{
	return os_state->win[0].size;
}

// win32/Vulkan layer ======================================================

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

function OS_Handle os_vulkan_loadLibrary()
{
	return os_loadLibrary("vulkan-1.dll");
}

function void os_vulkan_loadSurfaceFunction(OS_Handle vkdll)
{
	vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)os_loadFunction(vkdll, "vkCreateWin32SurfaceKHR");
}

function char *os_vulkan_surfaceExtentionName()
{
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

function VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface)
{
	VkWin32SurfaceCreateInfoKHR win32_surf_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = 0,
		.flags = 0,
		.hinstance = GetModuleHandle(0),
		.hwnd = os_windowFromHandle(handle)->hwnd
	};
	
	VkResult res = vkCreateWin32SurfaceKHR(instance, &win32_surf_info, 0, surface);
	return res;
}