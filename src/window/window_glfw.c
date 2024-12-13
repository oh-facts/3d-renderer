// TODO(mizu): os window from glfw window instead of using first window from os state. will be required during multi window set up

function WIN_Key win_keyFromSym(s32 sym)
{
	WIN_Key out = WIN_Key_NULL;

	if(GLFW_KEY_SPACE >= 0 && sym < GLFW_KEY_LAST)
	{
		read_only WIN_Key key_table[GLFW_KEY_LAST] =
		{
			[GLFW_KEY_F1] = WIN_Key_F1,
			[GLFW_KEY_F2] = WIN_Key_F2,
			[GLFW_KEY_F3] = WIN_Key_F3,
			[GLFW_KEY_F4] = WIN_Key_F4,
			[GLFW_KEY_F5] = WIN_Key_F5,
			[GLFW_KEY_F6] = WIN_Key_F6,
			[GLFW_KEY_F7] = WIN_Key_F7,
			[GLFW_KEY_F8] = WIN_Key_F8,
			[GLFW_KEY_F9] = WIN_Key_F9,
			[GLFW_KEY_F10] = WIN_Key_F10,
			[GLFW_KEY_F11] = WIN_Key_F11,
			[GLFW_KEY_F12] = WIN_Key_F12,

			[GLFW_KEY_A] = WIN_Key_A,
			[GLFW_KEY_B] = WIN_Key_B,
			[GLFW_KEY_C] = WIN_Key_C,
			[GLFW_KEY_D] = WIN_Key_D,
			[GLFW_KEY_E] = WIN_Key_E,
			[GLFW_KEY_F] = WIN_Key_F,
			[GLFW_KEY_G] = WIN_Key_G,
			[GLFW_KEY_H] = WIN_Key_H,
			[GLFW_KEY_I] = WIN_Key_I,
			[GLFW_KEY_J] = WIN_Key_J,
			[GLFW_KEY_K] = WIN_Key_K,
			[GLFW_KEY_L] = WIN_Key_L,
			[GLFW_KEY_M] = WIN_Key_M,
			[GLFW_KEY_N] = WIN_Key_N,
			[GLFW_KEY_O] = WIN_Key_O,
			[GLFW_KEY_P] = WIN_Key_P,
			[GLFW_KEY_Q] = WIN_Key_Q,
			[GLFW_KEY_R] = WIN_Key_R,
			[GLFW_KEY_S] = WIN_Key_S,
			[GLFW_KEY_T] = WIN_Key_T,
			[GLFW_KEY_U] = WIN_Key_U,
			[GLFW_KEY_V] = WIN_Key_V,
			[GLFW_KEY_W] = WIN_Key_W,
			[GLFW_KEY_X] = WIN_Key_X,
			[GLFW_KEY_Y] = WIN_Key_Y,
			[GLFW_KEY_Z] = WIN_Key_Z,

			[GLFW_KEY_0] = WIN_Key_0,
			[GLFW_KEY_1] = WIN_Key_1,
			[GLFW_KEY_2] = WIN_Key_2,
			[GLFW_KEY_3] = WIN_Key_3,
			[GLFW_KEY_4] = WIN_Key_4,
			[GLFW_KEY_5] = WIN_Key_5,
			[GLFW_KEY_6] = WIN_Key_6,
			[GLFW_KEY_7] = WIN_Key_7,
			[GLFW_KEY_8] = WIN_Key_8,
			[GLFW_KEY_9] = WIN_Key_9,

			[GLFW_KEY_LEFT_CONTROL] = WIN_Key_LCTRL,
			[GLFW_KEY_RIGHT_CONTROL] = WIN_Key_RCTRL,
			[GLFW_KEY_LEFT_SHIFT] = WIN_Key_LSHIFT,
			[GLFW_KEY_RIGHT_SHIFT] = WIN_Key_RSHIFT,
			[GLFW_KEY_LEFT_ALT] = WIN_Key_LALT,
			[GLFW_KEY_RIGHT_ALT] = WIN_Key_RALT,

			[GLFW_KEY_LEFT] = WIN_Key_LEFT,
			[GLFW_KEY_RIGHT] = WIN_Key_RIGHT,
			[GLFW_KEY_UP] = WIN_Key_UP,
			[GLFW_KEY_DOWN] = WIN_Key_DOWN,

			[GLFW_KEY_TAB] = WIN_Key_TAB,
			[GLFW_KEY_CAPS_LOCK] = WIN_Key_CAPS,
			[GLFW_KEY_ENTER] = WIN_Key_ENTER,
			[GLFW_KEY_ESCAPE] = WIN_Key_ESC,
			[GLFW_KEY_SPACE] = WIN_Key_SPACE,
		};

		out = key_table[sym];
	}

	return out;
}

function WIN_Key win_keyFromMouseButton(u32 butt)
{
	WIN_Key out = WIN_Key_NULL;

	if(butt > 0 && butt < (GLFW_MOUSE_BUTTON_3 + 1))
	{
		read_only WIN_Key mouse_button_table[GLFW_MOUSE_BUTTON_3 + 1] =
		{
			[GLFW_MOUSE_BUTTON_LEFT] = WIN_Key_LMB,
			[GLFW_MOUSE_BUTTON_MIDDLE] = WIN_Key_MMB,
			[GLFW_MOUSE_BUTTON_RIGHT] = WIN_Key_RMB,
		};

		out = mouse_button_table[butt];
	}

	return out;
}

typedef struct WIN_Window WIN_Window;
struct WIN_Window
{
	GLFWwindow* v;
	V2S size;
};

typedef struct WIN_State WIN_State;
struct WIN_State
{
	Arena *arena;
	WIN_Window win[WIN_MAX_WIN];
	s32 num;
};

global WIN_State *win_state;
global WIN_EventList event_list;
global Arena *event_arena;

function void win_init()
{
	Arena *arena = arenaAlloc();
	win_gfx_state = pushArray(arena, WIN_State, 1);
	win_gfx_state->arena = arena;
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

function WIN_Window *win_windowFromHandle(WIN_Handle handle)
{
	return (WIN_Window*)handle.u64[0];
}

function WIN_EventList win_pollEvents(Arena *arena)
{
	event_arena = arena;
	event_list = (WIN_EventList){0};

	glfwPollEvents();

	if(glfwWindowShouldClose(win_state->win[0].v))
	{
		WIN_Event *event = win_pushEvent(event_arena, &event_list);
		event->key = WIN_Key_NULL;
		event->kind = WIN_EventKind_CloseRequested;
	}

	return event_list;
}

function void win_glfw_keyCallback(GLFWwindow* window, int _key, int scancode, int action, int mods)
{
	s32 key = win_keyFromSym(_key);

	if(action == GLFW_PRESS)
	{
		WIN_Event *win_event = win_pushEvent(event_arena, &event_list);

		win_event->key = key;
		win_event->kind = WIN_EventKind_Pressed;
	}
	else if(action == GLFW_RELEASE)
	{
		WIN_Event *win_event = win_pushEvent(event_arena, &event_list);

		win_event->key = key;
		win_event->kind = WIN_EventKind_Released;
	}
}

function void win_glfw_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	s32 key = win_keyFromMouseButton(button);

	if(action == GLFW_PRESS)
	{
		WIN_Event *win_event = win_pushEvent(event_arena, &event_list);

		win_event->key = key;
		win_event->kind = WIN_EventKind_Pressed;
	}
	else if(action == GLFW_RELEASE)
	{
		WIN_Event *win_event = win_pushEvent(event_arena, &event_list);

		win_event->key = key;
		win_event->kind = WIN_EventKind_Released;
	}
}

function void win_glfw_mousePositonCallback(GLFWwindow* window, double xpos, double ypos)
{
	WIN_Event *win_event = win_pushEvent(event_arena, &event_list);
	win_event->kind = WIN_EventKind_MouseMove;
	win_event->mpos.x = xpos;
	win_event->mpos.y = ypos;
}

function void win_glfw_windowSizeCallback(GLFWwindow* window, int width, int height)
{
	win_state->win[0].size.x = width;
	win_state->win[0].size.y = height;
}

function WIN_Handle win_openWindow(char * title, f32 x, f32 y, f32 w, f32 h)
{
	WIN_Window *win = win_state->win + win_state->num++;

	win->v = glfwCreateWindow(w, h, title, 0, 0);
	glfwSetWindowPos(win->v, x, y);

	glfwGetWindowSize(win->v, &win->size.x, &win->size.y);

	glfwSetCursorPosCallback(win->v, win_glfw_mousePositonCallback);
	glfwSetKeyCallback(win->v, win_glfw_keyCallback);
	glfwSetMouseButtonCallback(win->v, win_glfw_mouseButtonCallback);
	glfwSetWindowSizeCallback(win->v, win_glfw_windowSizeCallback);

	WIN_Handle out = {0};
	out.u64[0] = win;
	return out;
}

function V2S win_getWindowSize(WIN_Handle handle)
{
	return win_state->win[0].size;
}

function void win_setCursorMode(WIN_CursorMode mode)
{
	if(mode == WIN_CursorMode_Disabled)
	{
		glfwSetInputMode(win_state->win[0].v, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if(mode == WIN_CursorMode_Normal)
	{
		glfwSetInputMode(win_state->win[0].v, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

function V2F win_getCursorPos()
{
	double x, y;
	glfwGetCursorPos(win_state->win[0].v, &x, &y);

	V2F out = {x, y};
	return out;
}