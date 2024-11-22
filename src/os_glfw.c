// TODO(mizu): os window from glfw window instead of using first window from os state. will be required during multi window set up

function OS_Key os_keyFromSym(s32 sym)
{
	OS_Key out = OS_Key_NULL;
	
	if(GLFW_KEY_SPACE >= 0 && sym < GLFW_KEY_LAST)
	{
		read_only OS_Key key_table[GLFW_KEY_LAST] = 
		{
			[GLFW_KEY_F1] = OS_Key_F1, 
			[GLFW_KEY_F2] = OS_Key_F2, 
			[GLFW_KEY_F3] = OS_Key_F3, 
			[GLFW_KEY_F4] = OS_Key_F4, 
			[GLFW_KEY_F5] = OS_Key_F5, 
			[GLFW_KEY_F6] = OS_Key_F6, 
			[GLFW_KEY_F7] = OS_Key_F7,
			[GLFW_KEY_F8] = OS_Key_F8, 
			[GLFW_KEY_F9] = OS_Key_F9, 
			[GLFW_KEY_F10] = OS_Key_F10,
			[GLFW_KEY_F11] = OS_Key_F11, 
			[GLFW_KEY_F12] = OS_Key_F12,
			
			[GLFW_KEY_A] = OS_Key_A,
			[GLFW_KEY_B] = OS_Key_B,
			[GLFW_KEY_C] = OS_Key_C,
			[GLFW_KEY_D] = OS_Key_D,
			[GLFW_KEY_E] = OS_Key_E,
			[GLFW_KEY_F] = OS_Key_F,
			[GLFW_KEY_G] = OS_Key_G,
			[GLFW_KEY_H] = OS_Key_H,
			[GLFW_KEY_I] = OS_Key_I,
			[GLFW_KEY_J] = OS_Key_J,
			[GLFW_KEY_K] = OS_Key_K,
			[GLFW_KEY_L] = OS_Key_L,
			[GLFW_KEY_M] = OS_Key_M,
			[GLFW_KEY_N] = OS_Key_N,
			[GLFW_KEY_O] = OS_Key_O,
			[GLFW_KEY_P] = OS_Key_P,
			[GLFW_KEY_Q] = OS_Key_Q,
			[GLFW_KEY_R] = OS_Key_R,
			[GLFW_KEY_S] = OS_Key_S,
			[GLFW_KEY_T] = OS_Key_T,
			[GLFW_KEY_U] = OS_Key_U,
			[GLFW_KEY_V] = OS_Key_V,
			[GLFW_KEY_W] = OS_Key_W,
			[GLFW_KEY_X] = OS_Key_X,
			[GLFW_KEY_Y] = OS_Key_Y,
			[GLFW_KEY_Z] = OS_Key_Z,
			
			[GLFW_KEY_0] = OS_Key_0,
			[GLFW_KEY_1] = OS_Key_1,
			[GLFW_KEY_2] = OS_Key_2,
			[GLFW_KEY_3] = OS_Key_3,
			[GLFW_KEY_4] = OS_Key_4,
			[GLFW_KEY_5] = OS_Key_5,
			[GLFW_KEY_6] = OS_Key_6,
			[GLFW_KEY_7] = OS_Key_7,
			[GLFW_KEY_8] = OS_Key_8,
			[GLFW_KEY_9] = OS_Key_9,
			
			[GLFW_KEY_LEFT_CONTROL] = OS_Key_LCTRL,
			[GLFW_KEY_RIGHT_CONTROL] = OS_Key_RCTRL,
			[GLFW_KEY_LEFT_SHIFT] = OS_Key_LSHIFT,
			[GLFW_KEY_RIGHT_SHIFT] = OS_Key_RSHIFT,
			[GLFW_KEY_LEFT_ALT] = OS_Key_LALT,
			[GLFW_KEY_RIGHT_ALT] = OS_Key_RALT,
			
			[GLFW_KEY_LEFT] = OS_Key_LEFT,
			[GLFW_KEY_RIGHT] = OS_Key_RIGHT,
			[GLFW_KEY_UP] = OS_Key_UP,
			[GLFW_KEY_DOWN] = OS_Key_DOWN,
			
			[GLFW_KEY_TAB] = OS_Key_TAB,
			[GLFW_KEY_CAPS_LOCK] = OS_Key_CAPS,
			[GLFW_KEY_ENTER] = OS_Key_ENTER,
			[GLFW_KEY_ESCAPE] = OS_Key_ESC,
			[GLFW_KEY_SPACE] = OS_Key_SPACE,
		};
		
		out = key_table[sym];
	}
	
	return out;
}

function OS_Key os_keyFromMouseButton(u32 butt)
{
	OS_Key out = OS_Key_NULL;
	
	if(butt > 0 && butt < (GLFW_MOUSE_BUTTON_3 + 1))
	{
		read_only OS_Key mouse_button_table[GLFW_MOUSE_BUTTON_3 + 1] = 
		{
			[GLFW_MOUSE_BUTTON_LEFT] = OS_Key_LMB,
			[GLFW_MOUSE_BUTTON_MIDDLE] = OS_Key_MMB,
			[GLFW_MOUSE_BUTTON_RIGHT] = OS_Key_RMB,
		};
		
		out = mouse_button_table[butt];
	}
	
	return out;
}

typedef struct OS_Window OS_Window;
struct OS_Window
{
	GLFWwindow* v;
	V2S size;
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
	glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

function OS_Window *os_windowFromHandle(OS_Handle handle)
{
	return (OS_Window*)handle.u64[0];
}

function OS_EventList os_pollEvents(Arena *arena)
{
    event_arena = arena;
	event_list = (OS_EventList){0};
	
    glfwPollEvents();
    
    if(glfwWindowShouldClose(os_state->win[0].v))
    {
        OS_Event *event = os_pushEvent(event_arena, &event_list);
        event->key = OS_Key_NULL;
        event->kind = OS_EventKind_CloseRequested;
    }
    
    return event_list;
}

function void os_glfw_keyCallback(GLFWwindow* window, int _key, int scancode, int action, int mods)
{
    s32 key = os_keyFromSym(_key);
    
    if(action == GLFW_PRESS)
    {
        OS_Event *os_event = os_pushEvent(event_arena, &event_list);
        
        os_event->key = key;
        os_event->kind = OS_EventKind_Pressed;
    }
    else if(action == GLFW_RELEASE)
    {
        OS_Event *os_event = os_pushEvent(event_arena, &event_list);
        
        os_event->key = key;
        os_event->kind = OS_EventKind_Released;
    }
}

function void os_glfw_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    s32 key = os_keyFromMouseButton(button);
    
    if(action == GLFW_PRESS)
    {
        OS_Event *os_event = os_pushEvent(event_arena, &event_list);
        
        os_event->key = key;
        os_event->kind = OS_EventKind_Pressed;
    }
    else if(action == GLFW_RELEASE)
    {
        OS_Event *os_event = os_pushEvent(event_arena, &event_list);
        
        os_event->key = key;
        os_event->kind = OS_EventKind_Released;
    }
}

function void os_glfw_mousePositonCallback(GLFWwindow* window, double xpos, double ypos)
{
    OS_Event *os_event = os_pushEvent(event_arena, &event_list);
    os_event->kind = OS_EventKind_MouseMove;
    os_event->mpos.x = xpos;
    os_event->mpos.y = ypos;
}

function void os_glfw_windowSizeCallback(GLFWwindow* window, int width, int height)
{
    os_state->win[0].size.x = width;
    os_state->win[0].size.y = height;
}

function OS_Handle os_openWindow(char * title, f32 x, f32 y, f32 w, f32 h)
{
	OS_Window *win = os_state->win + os_state->num++;
	
    win->v = glfwCreateWindow(w, h, title, 0, 0);
    glfwSetWindowPos(win->v, x, y);
    
    glfwGetWindowSize(win->v, &win->size.x, &win->size.y);
    
	glfwSetCursorPosCallback(win->v, os_glfw_mousePositonCallback);
    glfwSetKeyCallback(win->v, os_glfw_keyCallback);
    glfwSetMouseButtonCallback(win->v, os_glfw_mouseButtonCallback);
    glfwSetWindowSizeCallback(win->v, os_glfw_windowSizeCallback);
    
	OS_Handle out = {0};
	out.u64[0] = win;
	return out;
}

function void os_setCursorMode(OS_CursorMode mode)
{
    if(mode == OS_CursorMode_Disabled)
    {
        glfwSetInputMode(os_state->win[0].v, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if(mode == OS_CursorMode_Normal)
    {
        glfwSetInputMode(os_state->win[0].v, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    
}

function V2F os_getCursorPos()
{
    double x, y;
    glfwGetCursorPos(os_state->win[0].v, &x, &y);
    
    V2F out = {x, y};
    return out;
}

function V2S os_getWindowSize(OS_Handle handle)
{
	return os_state->win[0].size;
}

function s32 os_vulkan_getPlatformExtentions(char *extentions[])
{
    uint32_t count;
    char** extensions = glfwGetRequiredInstanceExtensions(&count);
    
    if(extentions)
    {
        for(u32 i = 0; i < count; i++)
        {
            extentions[i] = extensions[i];
        }
    }
    
    return count;
}

function VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface)
{
	VkResult res = glfwCreateWindowSurface(instance, os_windowFromHandle(handle)->v, 0, surface);
	return res;
}