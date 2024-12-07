#include "base_context.c"

int main(int argc, char *argv[])
{
	os_init();
	
	//hs_init();
	tex_init();
	
	Arena *perm = arenaAlloc();
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
	
	Arena *frame = arenaAllocSized(MB(32), GB(1));
	{
		ArenaTemp temp = arenaTempBegin(frame);
		
		Str8 app_dir = os_getAppDir(temp.arena);
		
		r_vulkan_init(win, frame);
		
		//r_vulkan_state->model = r_vulkan_model(str8_lit("../res/sponza/Sponza.gltf"), frame);
		
		//r_vulkan_state->cubes[0] = r_vulkan_model(str8_lit("../res/cube/cube.gltf"), frame);
		
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
	
	for(;run;)
	{
		f64 time_since_last = time_elapsed;
		ArenaTemp temp = arenaTempBegin(frame);
		
		OS_EventList list = os_pollEvents(temp.arena);
		
		R_Batch batch = {0};
		batch.cap = MB(1);
		batch.base = pushArray(temp.arena, u8, batch.cap);
		
		static f32 counter = 0;
		counter += delta;
		
		TEX_Scope *scope = tex_scopeOpen();
		for(s32 i = 0; i < 1; i++)
		{
			M4F model = m4f_translate(v3f(7.17, 0.66, i - (4)/ 2 -0.21));
			
			model = m4f_mul(m4f_scale(v3f(0.5, 0.5, 0.5)), model);
			model = m4f_mul(model, m4f_rotate(v3f(0, 1, 0), counter));
			
			u32 sprite_index = ((s32)counter) % Art_COUNT;
			
			R_Handle handle = tex_handleFromHash(scope, hashes[sprite_index]);
			R_VULKAN_Image *image = handle.u64[0];
			
			r_pushRect3(&batch, model, image->index);
		}
		tex_scopeClose(scope);
		
		//r_vulkan_beginRendering();
		r_vulkan_render(win, &list, &batch, delta, temp.arena);
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
		
		tex_clock_tick();
		tex_evict();
	}
	printf("quit\n");
}