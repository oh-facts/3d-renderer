#define CAMERA_UP ((V3F){{0, -1, 0}})
#define CAMERA_FRONT ((V3F){{0, 0, -1}})

typedef struct WIN_Camera WIN_Camera;
struct WIN_Camera
{ 
	V3F pos;	
	V2F mv;
	b32 enable;
	f32 speed;
	f32 zoom;
	f32 aspect;
};

function void win_cam_update(WIN_Camera *camera, OS_EventList *list, f32 delta)
{
	if(os_event(list, OS_Key_W, OS_EventKind_Pressed))
	{
		camera->mv.y += -1;
	}
	else if(os_event(list, OS_Key_W, OS_EventKind_Released))
	{
		camera->mv.y -= -1;
	}
	
	if(os_event(list, OS_Key_A, OS_EventKind_Pressed))
	{
		camera->mv.x += -1;
	}
	else if(os_event(list, OS_Key_A, OS_EventKind_Released))
	{
		camera->mv.x -= -1;
	}
	
	if(os_event(list, OS_Key_D, OS_EventKind_Pressed))
	{
		camera->mv.x += 1;
	}
	else if(os_event(list, OS_Key_D, OS_EventKind_Released))
	{
		camera->mv.x -= 1;
	}
	
	if(os_event(list, OS_Key_S, OS_EventKind_Pressed))
	{
		camera->mv.y += 1;
	}
	else if(os_event(list, OS_Key_S, OS_EventKind_Released))
	{
		camera->mv.y -= 1;
	}
			
	camera->pos.x += camera->mv.x * camera->speed * delta;
	camera->pos.y += camera->mv.y * camera->speed * delta;
}

function M4F win_cam_getProj(WIN_Camera *cam)
{
	f32 z = cam->zoom;
	f32 za = z * cam->aspect;
	return m4f_orthoProj(-za, za, -z, z, 0.001, 1000).fwd;
}

function M4F win_cam_getView(WIN_Camera *cam)
{
	return m4f_lookAt(cam->pos, v3f_add(cam->pos, v3f(0, 0, 1)), v3f(0, 1, 0));
}