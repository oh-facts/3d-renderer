typedef enum FILE_TYPE FILE_TYPE;
enum FILE_TYPE
{
	FILE_TYPE_TEXT,
	FILE_TYPE_BINARY,
	FILE_TYPE_COUNT
};

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

function FileData readFile(Arena *arena, char *filepath, FILE_TYPE type)
{
	FileData out = {0};
	FILE *file;
	
	read_only char *file_type_table[FILE_TYPE_COUNT] = 
	{
		"r",
		"rb"
	};
	
	fileOpenImpl(&file, filepath, file_type_table[type]);
	
	fseek(file, 0, SEEK_END);
	
	out.size = ftell(file);
	//print("%d", len);
	
	fseek(file, 0, SEEK_SET);
	
	out.bytes = pushArray(arena, u8, out.size);
	fread(out.bytes, sizeof(u8), out.size, file);
	
	fclose(file);
	
	return out;
}

function void writeFile(const char *filepath, FILE_TYPE type, void *data, size_t size)
{
	FILE *file;
	
	read_only char *file_type_table[FILE_TYPE_COUNT] = 
	{
		"w",
		"wb"
	};
	
	fileOpenImpl(&file, filepath, file_type_table[type]);
	
	fwrite(data, size, 1, file);
	
	fclose(file);
	
}

function b32 copyFile(const char* sourcePath, char* destinationPath)
{
	b32 out = 0;
	
	FILE* sourceFile, * destinationFile;
	char buffer[4096];
	size_t bytesRead;
	
	fileOpenImpl(&sourceFile, sourcePath, "rb");
	
	if(sourceFile)
	{
		fileOpenImpl(&destinationFile, destinationPath, "wb");
		
		if(destinationFile)
		{
			while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) 
			{
				fwrite(buffer, 1, bytesRead, destinationFile);
			}
			
			fclose(sourceFile);
			fclose(destinationFile);
			
			out = 1;
		}
	}
	
	return out;
}

typedef struct Bitmap Bitmap;
struct Bitmap
{
    void *data;
    s32 w;
    s32 h;
    s32 n;
};

function Bitmap bitmap(Str8 path)
{
	Bitmap out = {0};
	
	stbi_set_flip_vertically_on_load(1);
	
	out.data = stbi_load((char*)path.c, &out.w, &out.h, &out.n, 0);
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
