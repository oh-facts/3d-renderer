// TODO(mizu): organize
#define OS_MAX_WIN 10

typedef enum OS_Key OS_Key;
enum OS_Key
{
	OS_Key_NULL,
	
	OS_Key_F1, OS_Key_F2, OS_Key_F3, OS_Key_F4, OS_Key_F5, 
	OS_Key_F6, OS_Key_F7, OS_Key_F8, OS_Key_F9, OS_Key_F10,
	OS_Key_F11, OS_Key_F12,
	
	OS_Key_A, OS_Key_B, OS_Key_C, OS_Key_D, OS_Key_E,
	OS_Key_F, OS_Key_G, OS_Key_H, OS_Key_I, OS_Key_J,
	OS_Key_K, OS_Key_L, OS_Key_M, OS_Key_N, OS_Key_O,
	OS_Key_P, OS_Key_Q, OS_Key_R, OS_Key_S, OS_Key_T,
	OS_Key_U, OS_Key_V, OS_Key_W, OS_Key_X, OS_Key_Y,
	OS_Key_Z,
	
	OS_Key_0, OS_Key_1, OS_Key_2, OS_Key_3, OS_Key_4, 
	OS_Key_5, OS_Key_6, OS_Key_7, OS_Key_8, OS_Key_9,
	
	OS_Key_LCTRL, OS_Key_RCTRL, 
	OS_Key_LSHIFT, OS_Key_RSHIFT,
	OS_Key_LALT, OS_Key_RALT,
	
	OS_Key_LMB, OS_Key_RMB, OS_Key_MMB,
	
	OS_Key_LEFT, OS_Key_RIGHT, OS_Key_UP, OS_Key_DOWN,
	
	OS_Key_TAB,
	OS_Key_CAPS,
	OS_Key_ENTER,
	OS_Key_SPACE,
	
	OS_Key_ESC,
	
	OS_Key_COUNT,
};

typedef enum OS_EventKind OS_EventKind;
enum OS_EventKind
{
	OS_EventKind_NULL,
	OS_EventKind_Pressed,
	OS_EventKind_Released,
	OS_EventKind_MouseMove,
	OS_EventKind_CloseRequested,
	OS_EventKind_COUNT,
};

typedef struct OS_Event OS_Event;
struct OS_Event
{
	OS_Event *next;
	OS_Event *prev;
	OS_Key key;
	OS_EventKind kind;
	OS_Handle win;
	V2F mpos;
};

typedef struct OS_EventList OS_EventList;
struct OS_EventList
{
	OS_Event *first;
	OS_Event *last;
	u64 count;
};

typedef enum OS_CursorMode OS_CursorMode;
enum OS_CursorMode
{
    OS_CursorMode_Normal,
    OS_CursorMode_Disabled
};

// OS / video hooks ===================
function void os_innit();
function OS_EventList os_pollEvents(Arena *arena);
function OS_Handle os_openWindow(char * title, f32 x, f32 y, f32 w, f32 h);
function V2S os_getWindowSize(OS_Handle win);
function V2S os_getWindowSize(OS_Handle win);

function void os_setCursorMode(OS_CursorMode mode);
function V2F os_getCursorPos();
// =====================================

function OS_Event *os_pushEvent(Arena *arena, OS_EventList *list)
{
	list->count++;
	
	OS_Event *out = pushArray(arena, OS_Event, 1);
	
	if(!list->last)
	{
		list->last = list->first = out;
	}
	else
	{
		out->prev = list->last;
		list->last = list->last->next = out;
	}
	
	return out;
}

function OS_Event *os_eatEvent(OS_EventList *list, OS_Event *event)
{
	list->count--;
	
	if(event->prev)
	{
		event->prev->next = event->next;
	}
	else
	{
		list->first = event->next;
	}
	
	if(event->next)
	{
		event->next->prev = event->prev;
	}
	else
	{
		list->last = event->prev;
	}
	
	return event;
}

function OS_Event *os_event(OS_EventList *list, OS_Key key, OS_EventKind kind)
{
	OS_Event *out = 0;
	for(OS_Event *cur = list->first; cur; cur = cur->next)
	{
		if((cur->key == key) && (cur->kind == kind))
		{
            out = os_eatEvent(list, cur);
			break;
		}
	}
	
    return out;
}

read_only char *key_names[] = 
{
	[OS_Key_F1] = "OS_Key_F1", 
	[OS_Key_F2] = "OS_Key_F2", 
	[OS_Key_F3] = "OS_Key_F3", 
	[OS_Key_F4] = "OS_Key_F4", 
	[OS_Key_F5] = "OS_Key_F5", 
	[OS_Key_F6] = "OS_Key_F6", 
	[OS_Key_F7] = "OS_Key_F7",
	[OS_Key_F8] = "OS_Key_F8", 
	[OS_Key_F9] = "OS_Key_F9", 
	[OS_Key_F10] = "OS_Key_F10",
	[OS_Key_F11] = "OS_Key_F11", 
	[OS_Key_F12] = "OS_Key_F12",
	
	[OS_Key_A] = "OS_Key_A",
	[OS_Key_B] = "OS_Key_B",
	[OS_Key_C] = "OS_Key_C",
	[OS_Key_D] = "OS_Key_D",
	[OS_Key_E] = "OS_Key_E",
	[OS_Key_F] = "OS_Key_F",
	[OS_Key_G] = "OS_Key_G",
	[OS_Key_H] = "OS_Key_H",
	[OS_Key_I] = "OS_Key_I",
	[OS_Key_J] = "OS_Key_J",
	[OS_Key_K] = "OS_Key_K",
	[OS_Key_L] = "OS_Key_L",
	[OS_Key_M] = "OS_Key_M",
	[OS_Key_N] = "OS_Key_N",
	[OS_Key_O] = "OS_Key_O",
	[OS_Key_P] = "OS_Key_P",
	[OS_Key_Q] = "OS_Key_Q",
	[OS_Key_R] = "OS_Key_R",
	[OS_Key_S] = "OS_Key_S",
	[OS_Key_T] = "OS_Key_T",
	[OS_Key_U] = "OS_Key_U",
	[OS_Key_V] = "OS_Key_V",
	[OS_Key_W] = "OS_Key_W",
	[OS_Key_X] = "OS_Key_X",
	[OS_Key_Y] = "OS_Key_Y",
	[OS_Key_Z] = "OS_Key_Z",
	
	[OS_Key_0] = "OS_Key_0",
	[OS_Key_1] = "OS_Key_1",
	[OS_Key_2] = "OS_Key_2",
	[OS_Key_3] = "OS_Key_3",
	[OS_Key_4] = "OS_Key_4",
	[OS_Key_5] = "OS_Key_5",
	[OS_Key_6] = "OS_Key_6",
	[OS_Key_7] = "OS_Key_7",
	[OS_Key_8] = "OS_Key_8",
	[OS_Key_9] = "OS_Key_9",
	
	[OS_Key_LCTRL] = "OS_Key_LCTRL",
	[OS_Key_RCTRL] = "OS_Key_RCTRL",
	[OS_Key_LSHIFT] = "OS_Key_LSHIFT",
	[OS_Key_RSHIFT] = "OS_Key_RSHIFT",
	[OS_Key_LALT] = "OS_Key_LALT",
	[OS_Key_RALT] = "OS_Key_RALT",
	
	[OS_Key_LMB] = "lmb",
    [OS_Key_RMB] = "rmb",
	[OS_Key_MMB] = "mmb",
	
	[OS_Key_LEFT] = "OS_Key_LEFT",
	[OS_Key_RIGHT] = "OS_Key_RIGHT",
	[OS_Key_UP] = "OS_Key_UP",
	[OS_Key_DOWN] = "OS_Key_DOWN",
	
	[OS_Key_TAB] = "OS_Key_TAB",
	[OS_Key_CAPS] = "OS_Key_CAPS",
	[OS_Key_ENTER] = "OS_Key_ENTER",
	[OS_Key_ESC] = "OS_Key_ESC",
	[OS_Key_SPACE] = "OS_Key_SPACE",
};

read_only char *event_names[] = 
{
	[OS_EventKind_Pressed] = "pressed",
	[OS_EventKind_Released] = "released",
	[OS_EventKind_CloseRequested] = "close requested",
    [OS_EventKind_MouseMove] = "mouse moved",
};

function void os_eventListPrint(OS_EventList *list)
{
    for(OS_Event *event = list->first; event; event = event->next)
    {
        printf("%s ", event_names[event->kind]);
        
        switch(event->kind)
        {
            case OS_EventKind_Pressed:
            case OS_EventKind_Released:
            {
                printf("[%s]", key_names[event->key]);
            }break;
            
            case OS_EventKind_MouseMove:
            {
                printf("[%.f %.f]", event->mpos.x, event->mpos.y);
            }break;
            
            case OS_EventKind_CloseRequested:
            case OS_EventKind_NULL:
            default:
            {
                
            }break;
        }
        
        printf("\r\n");
    }
}