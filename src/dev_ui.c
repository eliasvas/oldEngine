#include "dev_ui.h"
#include "platform.h"

#if _WIN32
#include "renderer.h"
extern Renderer rend;
void dui_draw_rect(f32 x, f32 y, f32 w, f32 h, color4 color)
{
    renderer_push_filled_rect(&rend, v3(x/global_platform.window_width,y/global_platform.window_height,0), v2(w/global_platform.window_width,h/global_platform.window_height), color);
}
void dui_draw_char(f32 x, f32 y, f32 w, f32 h, char ch)
{
    renderer_push_char(&rend, v3(x/global_platform.window_width,y/global_platform.window_height,0), v2(w/global_platform.window_width,h/global_platform.window_height), ch);
}
#endif
void dui_begin(void)
{
}
