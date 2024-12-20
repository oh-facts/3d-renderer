#define COLOR_RED (V4F){1, 0, 0, 1}
#define COLOR_GREEN (V4F){0, 1, 0, 1}
#define COLOR_BLUE (V4F){0, 0, 1, 1}
#define COLOR_CYAN (V4F){0, 1, 1, 1}

function V4F color_inverse(V4F color)
{
	V4F out = {
		abs(1 - color.x),
		abs(1 - color.y),
		abs(1 - color.z),
		color.w
	};

	return out;
}

function void render_tilemap(R_BatchList *batch_list)
{
	TEX_Scope *scope = tex_scopeOpen();
	{
		for(s32 y = 0; y < 2; y+=1)
		{
			for(s32 x = 0; x < 2; x+=1)
			{								
				V4F colors[] = 
				{
					COLOR_RED,
					COLOR_GREEN,
					COLOR_BLUE,
				};

				V4F color = colors[(y + x) % 3];
				
				RectF32 dst = {
					.min.x = x * 64,
					.min.y = y * 64,
					.max.x = dst.min.x + 64,
					.max.y = dst.min.y + 64,
				};

				R_Rect2 *rect = r_pushRect2(batch_list, dst, color);
				
				//rect->border_color = color_inverse(color);
				//rect->radius = 4;
				//rect->border_thickness = 4;
			}
		}
	}
	tex_scopeClose(scope);
}