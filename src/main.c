#include "base_context.c"

int main(int argc, char *argv[])
{
	os_init();
	tcxt_init();
	hs_init();
	
	Arena *perm = arenaAlloc();
	OS_Handle win = os_openWindow("Ladybird", 50, 50, 960, 540);
	
	Arena *frame = arenaAllocSized(MB(32), GB(1));
	{
		ArenaTemp temp = arenaTempBegin(frame);
		
		r_vulkan_init(win, frame);
		
		r_vulkan_uploadVertexIndexData(frame);
		r_vulkan_updateDescriptorSets(frame);
		
		arenaTempEnd(&temp);
	}
	
	typedef struct
	{
		u32 age;
		char name[5];
	}test;
	
	typedef enum boba
	{
		boba_hi,
		boba_bye
	}boba;
	
	typedef union id
	{
		boba boba;
		u128 key;
	}id;
	
	id id1 = {.boba = boba_hi};
	id id2 = {.boba = boba_bye};
	
	{
		test *hi = malloc(sizeof(test));
		hi->age = 10;
		hi->name[0] = '1';
		hi->name[1] = '2';
		hi->name[2] = '3';
		hi->name[3] = '4';
		hi->name[4] = 0;
		
		hs_submit(id1.key, str8(hi, sizeof(test)));
	}
	
	{
		test *bye = malloc(sizeof(test));
		bye->age = 10;
		bye->name[0] = '3';
		bye->name[1] = '1';
		bye->name[2] = '1';
		bye->name[3] = 'a';
		bye->name[4] = 0;
		
		hs_submit(id2.key, str8(bye, sizeof(test)));
	}
	
	{
		u128 hash = hs_hashFromKey(id1.key);
		u128 hash2 = hs_hashFromKey(id2.key);
		
		HS_Scope *scope = hs_scopeOpen();
		
		test *hi = hs_dataFromHash(scope, hash).c;
		test *bye = hs_dataFromHash(scope, hash2).c;
		
		hs_scopeClose(scope);
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
		
		//r_vulkan_beginRendering();
		r_vulkanRender(win, &list, delta, temp.arena);
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
	printf("quit\n");
}