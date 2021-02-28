#include "dui.h"
#include "platform.h"
dui_Layout layout;
dui_State ui;

#if _WIN32
#include "renderer.h"
extern Renderer rend;
void dui_draw_rect(f32 x, f32 y, f32 w, f32 h, vec4 color)
{
    renderer_push_filled_rect(&rend, v3((f32)x/global_platform.window_width,(f32)y/global_platform.window_height,0), 
            v2((f32)w/global_platform.window_width,(f32)h/global_platform.window_height), color);
    //sprintf(error_log, "%f %f %f %f",(f32)x/global_platform.window_width,(f32)y/global_platform.window_height,(f32)w/global_platform.window_width,(f32)h/global_platform.window_height);
}
void dui_draw_char(f32 x, f32 y, f32 w, f32 h, char ch)
{
    renderer_push_char(&rend, v3(x/(f32)global_platform.window_width,y/(f32)global_platform.window_height,0), v2(w/(f32)global_platform.window_width,h/(f32)global_platform.window_height), ch);
}
#endif

b32 dui_rect_hit(dui_Rect rect)
{
    if (rect.x > ui.mouse_pos.x || rect.x + rect.w < ui.mouse_pos.x || rect.y > ui.mouse_pos.y || rect.y + rect.h  < ui.mouse_pos.y)
        return 0; 
    //sprintf(error_log, "collision!!!");
    return 1;
}
void dui_frame_begin(void)
{
    ui.hot = 0;
    ui.mouse_pos = v2(global_platform.mouse_x, global_platform.window_height - global_platform.mouse_y);
    ui.mouse_down = global_platform.left_mouse_down;
}

void dui_frame_end(void)
{

    if (ui.mouse_down == 0)
        ui.active = 0;
    else 
        if (ui.active == 0)
            ui.active = -1;
}
b32 do_button(DUIID id, dui_Rect rect)
{

    //renderer_push_filled_rect(&rend, v3(0,0,0), v2(0.2,0.2), v4(1,1,1,1)); 
    if (dui_rect_hit(rect))
    {
        ui.hot = id;
        if (ui.active == 0 && ui.mouse_down)
            ui.active = id;
    }
    //renderer_push_filled_rect(&rend,v3(0.10,0.10,0), v2(0.3,0.3), v4(1.f,1.f,0,1.f));
    if (ui.hot == id)
    {
        if (ui.active ==id)
            dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.bg_lite);
        else 
            dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.fg);
    }
    else 
        dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.bg);

    //user just clicked!
    if (ui.mouse_down == 0 && ui.hot == id && ui.active == id)return 1;
    //#not
    return 0;
}

void dui_default(void)
{
    layout.fg = v4(0.8,0.2,0.2,0.9f);
    layout.bg = v4(0.2,0.2,0.2,0.9f);
    layout.bg_dark = v4(0.1,0.1,0.1,0.9f);
    layout.bg_lite = v4(0.3,0.3,0.3,0.9f);

    layout.cx = 0;
    layout.cy = 0;
}
