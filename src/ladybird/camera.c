#define CAMERA_UP ((V3F){{0, -1, 0}})
#define CAMERA_FRONT ((V3F){{0, 0, -1}})

typedef struct Camera Camera;
struct Camera
{ 
	V3F pos;	
	V3F mv;
	f32 pitch;
	f32 yaw;
	V2F old_mpos;
	b32 enable;
	f32 speed;
};

function void cam_update(Camera *camera, OS_EventList *list, f32 delta)
{
	if(os_event(list, OS_Key_SPACE, OS_EventKind_Pressed))
	{
		if(camera->enable)
		{
			os_setCursorMode(OS_CursorMode_Normal);
		}
		else
		{
			os_setCursorMode(OS_CursorMode_Disabled);
		}
		
		camera->enable = !camera->enable;
		camera->mv = (V3F){0};
		os_event(list, OS_Key_SPACE, OS_EventKind_Released);
	}
	
	if(camera->enable)
	{
		if(os_event(list, OS_Key_W, OS_EventKind_Pressed))
		{
			camera->mv.z += -1;
		}
		else if(os_event(list, OS_Key_W, OS_EventKind_Released))
		{
			camera->mv.z -= -1;
		}
		
		if(os_event(list, OS_Key_A, OS_EventKind_Pressed))
		{
			camera->mv.x += 1;
		}
		else if(os_event(list, OS_Key_A, OS_EventKind_Released))
		{
			camera->mv.x -= 1;
		}
		
		if(os_event(list, OS_Key_D, OS_EventKind_Pressed))
		{
			camera->mv.x += -1;
		}
		else if(os_event(list, OS_Key_D, OS_EventKind_Released))
		{
			camera->mv.x -= -1;
		}
		
		if(os_event(list, OS_Key_S, OS_EventKind_Pressed))
		{
			camera->mv.z += 1;
		}
		else if(os_event(list, OS_Key_S, OS_EventKind_Released))
		{
			camera->mv.z -= 1;
		}
		
		if(os_event(list, OS_Key_LSHIFT, OS_EventKind_Pressed))
		{
			camera->mv.y += 1;
		}
		else if(os_event(list, OS_Key_LSHIFT, OS_EventKind_Released))
		{
			camera->mv.y -= 1;
		}
		
		V3F dir = {0};
		dir.x = cos(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
		dir.y = sin(degToRad(camera->pitch)) * 0.001;
		dir.z = sin(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
		dir = v3f_normalize(dir);
		
		if(camera->mv.z > 0)
		{
			V3F vel = dir;
			vel.x *= delta;
			vel.y *= delta;
			vel.z *= delta;
			
			camera->pos = v3f_sub(camera->pos, vel);
		}
		else if(camera->mv.z < 0)
		{
			V3F vel = dir;
			vel.x *= delta * camera->speed;
			vel.y *= delta * camera->speed;
			vel.z *= delta * camera->speed;
			
			camera->pos = v3f_add(camera->pos, vel);
		}
		
		V3F up = v3f(0.0f, 1.0f, 0.0f); 
		V3F cameraRight = v3f_normalize(v3f_cross(up, dir));
		V3F cameraUp = v3f_cross(dir, cameraRight);
		V3F vel = v3f_normalize(v3f_cross(dir, cameraUp));
		vel.x *= delta;
		vel.y *= delta;
		vel.z *= delta;
		
		if(camera->mv.x > 0)
		{
			camera->pos = v3f_sub(camera->pos, vel);
		}
		else if(camera->mv.x < 0)
		{
			camera->pos = v3f_add(camera->pos, vel);
		}
		
		
		OS_Event *mpos = os_event(list, OS_Key_NULL, OS_EventKind_MouseMove);
		if(mpos)
		{
			camera->yaw += mpos->mpos.x - camera->old_mpos.x;
			camera->pitch += camera->old_mpos.y - mpos->mpos.y;
			camera->old_mpos = mpos->mpos;
		}
	}
	else
	{
		camera->old_mpos = os_getCursorPos();
	}
}

function M4F cam_getView(Camera *camera)
{
	V3F dir = {0};
	dir.x = cos(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
	dir.y = sin(degToRad(camera->pitch)) * 0.001;
	dir.z = sin(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
	dir = v3f_normalize(dir);
	
	return m4f_lookAt(camera->pos, v3f_add(camera->pos, dir), CAMERA_UP);
}
