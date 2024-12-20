#include <stdint.h>
#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.c>

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.c>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.c>

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.c>

#include <stdio.h>
#include <math.h>

#include <base/base_core.c>
#include <os/os_core.c>
#include <base/base_arena.c>
#include <base/base_math.c>
#include <base/base_string.c>
#include <base/base_file.c>
#include <os/os_gfx.c>

#undef function
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <vk_mem_alloc.h>
#define function static
#include <os/os_vulkan.c>

#if defined(OS_WIN32)
#undef function
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define function static
#include <os/os_win32.c>
#elif defined(OS_LINUX)
#undef function
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#define function static
#include <os/os_unix.c>
#elif defined(OS_APPLE)
#undef function
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#define function static
#include <os/os_unix.c>
#else
#error platform not supported
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <os/os_glfw.c>

#include <render/render_core.c>
#include <render/render_vulkan.c>

#define HAVE_SSE2
#include <blake2/blake2.h>
#include <blake2/blake2b.c>

#include <texture/texture_cache.c>
#include <winter/winter_camera.c>
#include <winter/winter_tilemap.c>

int main(int argc, char *argv[])
{
	os_init();
	r_init();
	tex_init();
	
	Arena *perm = arenaAllocSized(MB(32), GB(1));
	OS_Handle win = os_openWindow("winter steam game", 50, 50, 960, 540);
	
	typedef enum Art Art;
	enum Art
	{
		Art_ell,
		Art_marhall,
		Art_ankha,
		Art_maruko,
		Art_COUNT,
	};
	
	Str8 paths[] = 
	{
		str8_lit("../res/scratch/ell.png"),
		str8_lit("../res/scratch/marhall.png"),
		str8_lit("../res/scratch/maruko.png"),
		str8_lit("../res/scratch/ankha.png")
	};
	
	u128 hashes[Art_COUNT] = {0};
	u128 keys[Art_COUNT] = {0};

	Arena *frame = arenaAllocSized(MB(32), GB(1));
	{
		ArenaTemp temp = arenaTempBegin(frame);
		
		Str8 app_dir = os_getAppDir(temp.arena);
		
		r_vulkan_init(win, frame);
				
		for(s32 i = 0; i < Art_COUNT; i++)
		{
			Str8 bmp_path = str8_join(temp.arena, app_dir, paths[i]);
			Bitmap bmp = bitmap(temp.arena, bmp_path);
			
			u64 bmp_size = bmp.w * bmp.h * bmp.n;
			u64 total_size = sizeof(Bitmap) + bmp_size;
			
			Arena *arena = arenaAllocSized(total_size, total_size);
			Bitmap *data = pushArray(arena, Bitmap, 1);
			data->data = pushArray(arena, u8, bmp_size);
			data->w = bmp.w;
			data->h = bmp.h;
			data->n = bmp.n;
			memcpy(data->data, bmp.data, bmp_size);
			
			Str8 str = str8(data, total_size);
			
			keys[i].u64[0] = i; 
			hashes[i] = tex_hashFromKeyData(keys[i], str, arena);
		}
		
		arenaTempEnd(&temp);
	}
	
	u64 start = os_getPerfCounter();
	u64 freq = os_getPerfFreq();
	
	f64 time_elapsed = 0;
	f64 delta = 0;
	
	b32 run = 1;
	
 WIN_Camera camera = {
		.pos.x = 0,
		.pos.y = 0,
		.speed = 500,
		.zoom = 600,
		.aspect = 960.f / 540,
	};

	for(;run;)
	{
		f64 time_since_last = time_elapsed;
		ArenaTemp temp = arenaTempBegin(frame);
		
		V2S size = os_getWindowSize(win);
		camera.aspect = size.x / (size.y * 1.f);
		
		OS_EventList list = os_pollEvents(temp.arena);
		

		R_BatchList ui_batches = {0};

		static f32 counter = 0;
		counter += delta;

		win_cam_update(&camera, &list, delta);
		M4F proj = win_cam_getProj(&camera);
		M4F view = win_cam_getView(&camera);

		r_begin(frame);
			render_tilemap(&ui_batches, camera.pos);
		r_end();

		r_vulkan_beginRendering();
		
		r_vulkan_render(temp.arena, win, &ui_batches, 0, 0, proj, view, v3f(0, 0, 0));
		r_vulkan_endRendering(win);
		
		//os_eventListPrint(&list);
		
		if(os_event(&list, OS_Key_ESC, OS_EventKind_Pressed) || os_event(&list, OS_Key_NULL, OS_EventKind_CloseRequested))
		{
			run = 0;
		}
		
		arenaTempEnd(&temp);
		
		u64 end = os_getPerfCounter();
		time_elapsed = (end - start) / (freq * 1.f);
		delta = time_elapsed - time_since_last;

		//tex_clock_tick();
		//tex_evict();
	}
	printf("Quit");
}