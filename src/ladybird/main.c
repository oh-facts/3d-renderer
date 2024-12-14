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
#include <ladybird/camera.c>
#include <ladybird/assets.c>

int main(int argc, char *argv[])
{
	os_init();
	
	tex_init();
	
	Arena *perm = arenaAllocSized(MB(32), GB(1));
	OS_Handle win = os_openWindow("Ladybird", 50, 50, 960, 540);
	
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

//	R_VULKAN_Model model;
//	R_VULKAN_Model cubes[3];
	
	GLTF_Scene scene = {0};

	Arena *frame = arenaAllocSized(MB(32), GB(1));
	{
		ArenaTemp temp = arenaTempBegin(frame);
		
		Str8 app_dir = os_getAppDir(temp.arena);
		
		r_vulkan_init(win, frame);
		
		Str8 scene_path = str8_join(temp.arena, app_dir, str8_lit("../res/sponza/Sponza.gltf"));
		pushArray(temp.arena, u8, 1);
		printf("%.*s\n", str8_varg(scene_path));
		scene = gltf_loadMesh(perm, temp.arena, scene_path);

		gltf_upload(perm, &scene);
		
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
	
 Camera camera = {
		.pos.x = 7.17,
		.pos.y = 0.66,
		.pos.z = -0.21,
		.yaw = 180,
		.pitch = 0,
		.speed = 5
	};	

	
		//r_vulkan_state->cubes[0] = r_vulkan_model(str8_lit("../res/cube/cube.gltf"), frame);


	for(;run;)
 {
		f64 time_since_last = time_elapsed;
		ArenaTemp temp = arenaTempBegin(frame);
		
		OS_EventList list = os_pollEvents(temp.arena);
  cam_update(&camera, &list, delta);
		
		R_Batch rect3_batch = {0};
		rect3_batch.cap = MB(1);
		rect3_batch.base = pushArray(temp.arena, u8, rect3_batch.cap);
		
		R_Batch rect2_batch = {0};
		rect2_batch.cap = MB(1);
		rect2_batch.base = pushArray(temp.arena, u8, rect2_batch.cap);

		R_Batch mesh_batch = {0};
		mesh_batch.cap = MB(128);
		mesh_batch.base = pushArray(temp.arena, u8, mesh_batch.cap);
		
		static f32 counter = 0;
		counter += delta;

#if 1
		TEX_Scope *scope = tex_scopeOpen();
		for(s32 i = 0; i < 1; i++)
		{
			M4F model = m4f_translate(v3f(7.17, 0.66, i - (4)/ 2 -0.21));
			
			model = m4f_mul(m4f_scale(v3f(0.5, 0.5, 0.5)), model);
			model = m4f_mul(model, m4f_rotate(v3f(0, 1, 0), counter));
			
			u32 sprite_index = ((s32)counter) % Art_COUNT;
			
			R_Handle handle = tex_handleFromHash(scope, hashes[sprite_index]);
			R_VULKAN_Image *image = handle.u64[0];
			
			r_pushRect3(&rect3_batch, model, image->index);
		}
		
		{
			R_Handle handle = tex_handleFromHash(scope, hashes[0]);
			R_VULKAN_Image *image = handle.u64[0];
			
			R_Rect2 *test_ui = r_pushRect2(&rect2_batch, rectF32(0, 0, 128, 128), v4f(1, 1, 1, 1));
			test_ui->border_color = v4f(1, 0, 0, 1);
			test_ui->radius = 4;
			test_ui->border_thickness = 4;
			test_ui->tex_id = image->index;
		}
		
		tex_scopeClose(scope);
#endif
		
		gltf_draw(&mesh_batch, &scene);

		//r_vulkan_beginRendering();

  M4F view = cam_getView(&camera);
  
		r_vulkan_render(temp.arena, win, view, camera.pos, &rect3_batch, &rect2_batch, &mesh_batch);
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
	}
}