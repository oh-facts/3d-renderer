#include "base_context.c"

int main(int argc, char *argv[])
{
	os_innit();
	
	OS_Handle win = os_openWindow("Ladybird", 50, 50, 960, 540);
	
	Arena *frame = arenaAlloc();
	r_vulkanInnit(win);
	
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
		
		//r_vulkan_beginRendering();
		r_vulkanRender(win, &list, delta);
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
		
		//printf("%f ms\n %f fps\n\n", delta, 1/delta);
#if 0
		// poor man's vsync--------------------------------
		f64 time_left = (1 / 60.f) - delta;
		if (time_left > 0) 
		{
			os_sleep(time_left * 1000);
		}
		// -------------------------------------------------
#endif
	}
	printf("quit\n");
}