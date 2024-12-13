typedef struct WIN_Handle WIN_Handle;
struct WIN_Handle
{
	u64 u64[1];
};

typedef enum WIN_Key WIN_Key;
enum WIN_Key
{
	WIN_Key_NULL,
	
	WIN_Key_F1, WIN_Key_F2, WIN_Key_F3, WIN_Key_F4, WIN_Key_F5, 
	WIN_Key_F6, WIN_Key_F7, WIN_Key_F8, WIN_Key_F9, WIN_Key_F10,
	WIN_Key_F11, WIN_Key_F12,
	
	WIN_Key_A, WIN_Key_B, WIN_Key_C, WIN_Key_D, WIN_Key_E,
	WIN_Key_F, WIN_Key_G, WIN_Key_H, WIN_Key_I, WIN_Key_J,
	WIN_Key_K, WIN_Key_L, WIN_Key_M, WIN_Key_N, WIN_Key_O,
	WIN_Key_P, WIN_Key_Q, WIN_Key_R, WIN_Key_S, WIN_Key_T,
	WIN_Key_U, WIN_Key_V, WIN_Key_W, WIN_Key_X, WIN_Key_Y,
	WIN_Key_Z,
	
	WIN_Key_0, WIN_Key_1, WIN_Key_2, WIN_Key_3, WIN_Key_4, 
	WIN_Key_5, WIN_Key_6, WIN_Key_7, WIN_Key_8, WIN_Key_9,
	
	WIN_Key_LCTRL, WIN_Key_RCTRL, 
	WIN_Key_LSHIFT, WIN_Key_RSHIFT,
	WIN_Key_LALT, WIN_Key_RALT,
	
	WIN_Key_LMB, WIN_Key_RMB, WIN_Key_MMB,
	
	WIN_Key_LEFT, WIN_Key_RIGHT, WIN_Key_UP, WIN_Key_DOWN,
	
	WIN_Key_TAB,
	WIN_Key_CAPS,
	WIN_Key_ENTER,
	WIN_Key_SPACE,
	
	WIN_Key_ESC,
	
	WIN_Key_COUNT,
};

typedef enum WIN_EventKind WIN_EventKind;
enum WIN_EventKind
{
	WIN_EventKind_NULL,
	WIN_EventKind_Pressed,
	WIN_EventKind_Released,
	WIN_EventKind_MouseMove,
	WIN_EventKind_CloseRequested,
	WIN_EventKind_COUNT,
};

typedef struct WIN_Event WIN_Event;
struct WIN_Event
{
	WIN_Event *next;
	WIN_Event *prev;
	WIN_Key key;
	WIN_EventKind kind;
	WIN_Handle win;
	V2F mpos;
};

typedef struct WIN_EventList WIN_EventList;
struct WIN_EventList
{
	WIN_Event *first;
	WIN_Event *last;
	u64 count;
};

typedef enum WIN_CursorMode WIN_CursorMode;
enum WIN_CursorMode
{
	WIN_CursorMode_Normal,
	WIN_CursorMode_Disabled
};

function WIN_Event *win_pushEvent(Arena *arena, WIN_EventList *list)
{
	list->count++;
	
	WIN_Event *out = pushArray(arena, WIN_Event, 1);
	
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

function WIN_Event *win_eatEvent(WIN_EventList *list, WIN_Event *event)
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

function WIN_Event *win_event(WIN_EventList *list, WIN_Key key, WIN_EventKind kind)
{
	WIN_Event *out = 0;
	for(WIN_Event *cur = list->first; cur; cur = cur->next)
	{
		if((cur->key == key) && (cur->kind == kind))
		{
			out = win_eatEvent(list, cur);
			break;
		}
	}
	
	return out;
}

read_only char *key_names[] = 
{
	[WIN_Key_F1] = "WIN_Key_F1", 
	[WIN_Key_F2] = "WIN_Key_F2", 
	[WIN_Key_F3] = "WIN_Key_F3", 
	[WIN_Key_F4] = "WIN_Key_F4", 
	[WIN_Key_F5] = "WIN_Key_F5", 
	[WIN_Key_F6] = "WIN_Key_F6", 
	[WIN_Key_F7] = "WIN_Key_F7",
	[WIN_Key_F8] = "WIN_Key_F8", 
	[WIN_Key_F9] = "WIN_Key_F9", 
	[WIN_Key_F10] = "WIN_Key_F10",
	[WIN_Key_F11] = "WIN_Key_F11", 
	[WIN_Key_F12] = "WIN_Key_F12",
	
	[WIN_Key_A] = "WIN_Key_A",
	[WIN_Key_B] = "WIN_Key_B",
	[WIN_Key_C] = "WIN_Key_C",
	[WIN_Key_D] = "WIN_Key_D",
	[WIN_Key_E] = "WIN_Key_E",
	[WIN_Key_F] = "WIN_Key_F",
	[WIN_Key_G] = "WIN_Key_G",
	[WIN_Key_H] = "WIN_Key_H",
	[WIN_Key_I] = "WIN_Key_I",
	[WIN_Key_J] = "WIN_Key_J",
	[WIN_Key_K] = "WIN_Key_K",
	[WIN_Key_L] = "WIN_Key_L",
	[WIN_Key_M] = "WIN_Key_M",
	[WIN_Key_N] = "WIN_Key_N",
	[WIN_Key_O] = "WIN_Key_O",
	[WIN_Key_P] = "WIN_Key_P",
	[WIN_Key_Q] = "WIN_Key_Q",
	[WIN_Key_R] = "WIN_Key_R",
	[WIN_Key_S] = "WIN_Key_S",
	[WIN_Key_T] = "WIN_Key_T",
	[WIN_Key_U] = "WIN_Key_U",
	[WIN_Key_V] = "WIN_Key_V",
	[WIN_Key_W] = "WIN_Key_W",
	[WIN_Key_X] = "WIN_Key_X",
	[WIN_Key_Y] = "WIN_Key_Y",
	[WIN_Key_Z] = "WIN_Key_Z",
	
	[WIN_Key_0] = "WIN_Key_0",
	[WIN_Key_1] = "WIN_Key_1",
	[WIN_Key_2] = "WIN_Key_2",
	[WIN_Key_3] = "WIN_Key_3",
	[WIN_Key_4] = "WIN_Key_4",
	[WIN_Key_5] = "WIN_Key_5",
	[WIN_Key_6] = "WIN_Key_6",
	[WIN_Key_7] = "WIN_Key_7",
	[WIN_Key_8] = "WIN_Key_8",
	[WIN_Key_9] = "WIN_Key_9",
	
	[WIN_Key_LCTRL] = "WIN_Key_LCTRL",
	[WIN_Key_RCTRL] = "WIN_Key_RCTRL",
	[WIN_Key_LSHIFT] = "WIN_Key_LSHIFT",
	[WIN_Key_RSHIFT] = "WIN_Key_RSHIFT",
	[WIN_Key_LALT] = "WIN_Key_LALT",
	[WIN_Key_RALT] = "WIN_Key_RALT",
	
	[WIN_Key_LMB] = "lmb",
	[WIN_Key_RMB] = "rmb",
	[WIN_Key_MMB] = "mmb",
	
	[WIN_Key_LEFT] = "WIN_Key_LEFT",
	[WIN_Key_RIGHT] = "WIN_Key_RIGHT",
	[WIN_Key_UP] = "WIN_Key_UP",
	[WIN_Key_DOWN] = "WIN_Key_DOWN",
	
	[WIN_Key_TAB] = "WIN_Key_TAB",
	[WIN_Key_CAPS] = "WIN_Key_CAPS",
	[WIN_Key_ENTER] = "WIN_Key_ENTER",
	[WIN_Key_ESC] = "WIN_Key_ESC",
	[WIN_Key_SPACE] = "WIN_Key_SPACE",
};

read_only char *event_names[] = 
{
	[WIN_EventKind_Pressed] = "pressed",
	[WIN_EventKind_Released] = "released",
	[WIN_EventKind_CloseRequested] = "close requested",
	[WIN_EventKind_MouseMove] = "mouse moved",
};

function void WIN_EventListPrint(WIN_EventList *list)
{
	for(WIN_Event *event = list->first; event; event = event->next)
	{
		printf("%s ", event_names[event->kind]);
		
		switch(event->kind)
		{
			case WIN_EventKind_Pressed:
			case WIN_EventKind_Released:
			{
				printf("[%s]", key_names[event->key]);
			}break;
			
			case WIN_EventKind_MouseMove:
			{
				printf("[%.f %.f]", event->mpos.x, event->mpos.y);
			}break;
			
			case WIN_EventKind_CloseRequested:
			case WIN_EventKind_NULL:
			default:
			{
				
			}break;
		}
		
		printf("\r\n");
	}
}

// window hooks ===================
function void win_init();
function WIN_EventList win_pollEvents(Arena *arena);
function WIN_Handle win_open(char * title, f32 x, f32 y, f32 w, f32 h);
function V2S win_getSize(WIN_Handle win);

function void win_setCursorMode(WIN_CursorMode mode);
function V2F win_getCursorPos();
// =====================================