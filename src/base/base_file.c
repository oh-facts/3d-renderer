// change to OS stuff
// and add app directory to OS_State

typedef struct FileData FileData;
struct FileData
{
	u8 *bytes;
	u64 size;
};

#if defined(OS_WIN32)
#define fileOpenImpl(file, filepath, mode) fopen_s(file, filepath, mode)
#elif defined (OS_LINUX) || defined (OS_APPLE)
#define fileOpenImpl(file, filepath, mode) *file = fopen(filepath, mode)
#endif

function FileData readFile(Arena *arena, Arena *scratch, Str8 filepath)
{
	FileData out = {0};
	FILE *file;
	
	fileOpenImpl(&file, filepath.c, "rb");
	
	fseek(file, 0, SEEK_END);
	
	out.size = ftell(file);
	
	fseek(file, 0, SEEK_SET);
	
	out.bytes = pushArray(arena, u8, out.size);
	fread(out.bytes, sizeof(u8), out.size, file);
	
	fclose(file);
	
	return out;
}

function Str8 dirFromFile(Arena *arena)
{
	Str8 out = str8_cpy()
	char *c = &buffer[len];
	while(*(--c) != '/')
	{
		*c = 0;
		--len;
	}
	
	u8 *str = pushArray(arena, u8, len);
	memcpy(str, buffer, len);
	
	Str8 out = str8(str, len);
	
	return out;
}

function Str8 fileNameFromPath(Arena *arena, Str8 path)
{
	char *cur = (char*)&path.c[path.len - 1];
	u32 count = 0;
	
	//NOTE(mizu): pig
	while(*cur != '/' && *cur != '\\' && *cur != '\0')
	{
		cur--;
		count++;
	}
	
	Str8 file_name_cstr = {0};
	file_name_cstr.c = pushArray(arena, u8, count + 1);
	file_name_cstr.len = count + 1;
	memcpy(file_name_cstr.c, cur + 1, count);
	file_name_cstr.c[count] = '\0';
	
	return file_name_cstr;
}

typedef struct Bitmap Bitmap;
struct Bitmap
{
	void *data;
	s32 w;
	s32 h;
	s32 n;
};

function Bitmap bitmap(Arena *arena, Str8 path)
{
	Bitmap out = {0};
	
	stbi_set_flip_vertically_on_load(1);
	
	s32 w = 0;
	s32 h = 0;
	s32 n = 0;
	
	u8 *bytes = stbi_load(path.c, &w, &h, &n, STBI_rgb_alpha);
	
	out.w = w;
	out.h = h;
	out.n = 4;
	out.data = pushArray(arena, u8, out.w * out.h * out.n);
	memcpy(out.data, bytes, out.w * out.h * out.n);
	
	stbi_image_free(bytes);
	
	return out;
}