typedef struct Glyph Glyph;
struct Glyph
{
	u8 *bmp;
	s32 w;
	s32 h;
	V2S bearing;
	s32 x0, y0, x1, y1;
	s32 advance;
};

typedef struct Atlas Atlas;
struct Atlas
{
	Glyph glyphs[256];
};

function Glyph *glyphFromCodepoint(Atlas *aminas, char c)
{
	Glyph *out = aminas->glyphs + (u32)c;
	return out;
}

function Glyph *glyphsFromFile(u8* path, char *codepoints, u32 num_cp, Arena* arena)
{
	u8 *file_data = read_file(arena, (char*)path, FILE_TYPE_BINARY).bytes;
    
	stbtt_fontinfo font;
	stbtt_InitFont(&font, (u8*)file_data, stbtt_GetFontOffsetForIndex((u8*)file_data,0));
    
	Glyph *out = pushArray(arena, Glyph, num_cp);
	for(u32 i = 0; i < num_cp; i++)
	{
        s32 w,h,xoff,yoff;
        f32 size = stbtt_ScaleForPixelHeight(&font, 64);
        
        u8* bmp = stbtt_GetCodepointBitmap(&font, 0, size, codepoints[i] ,&w,&h, &xoff, &yoff);
        
        stbtt_GetCodepointHMetrics(&font, codepoints[i], &out[i].advance, &out[i].bearing.x);
        out[i].w = w;
        out[i].h = h;
        
        s32 x0, y0, x1, y1;
        stbtt_GetCodepointBox(&font, codepoints[i], &x0, &y0, &x1, &y1);
        
        out[i].bearing.y = y0;
        
        out[i].x0 = x0;
        out[i].y0 = y0;
        out[i].x1 = x1;
        out[i].y1 = y1;
        
        out[i].bmp = pushArray(arena,u8,w * h * 4);
        
        u8* src_row = bmp + w*(h-1);
        u8* dest_row = out[i].bmp;
        
        for(s32 y = 0; y < h; y ++)
        {
            u32 *dest = (u32*)dest_row;
            u8 *src = src_row;
            for(s32 x = 0; x < w; x ++)
            {
                u8 alpha = *src++;
                
                *dest++ = ((alpha <<24) |
                           (0xFF <<16) |
                           (0xFF << 8) |
                           (0xFF));
            }
            dest_row += 4 * w;
            src_row -= w;
        }
        
        stbtt_FreeBitmap(bmp, 0);
	}
    
	return out;
}
