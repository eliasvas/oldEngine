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
}
void dui_draw_char(f32 x, f32 y, char ch)
{
    renderer_push_char(&rend, v3((f32)x/global_platform.window_width,(f32)y/global_platform.window_height,0),v2((f32)12/global_platform.window_width,(f32)12/global_platform.window_height), ch);
}
void dui_draw_char2(f32 x, f32 y,f32 w, f32 h, char ch)
{
    renderer_push_char(&rend, v3((f32)x/global_platform.window_width,(f32)y/global_platform.window_height,0),v2((f32)w/global_platform.window_width,(f32)h/global_platform.window_height), ch);
}


#endif


u32 dui_draw_string(i32 x, i32 y, char *string)
{
    u32 width = 0;
    while(*string)
    {
        dui_draw_char(x-16, y, *string);
        width+=10;
        x+=10;
        ++string;
    }
    return width;
}
u32 dui_draw_stringn(i32 x, i32 y, char *string, u32 max_letters)
{
    u32 width = 0;
	u32 i = 0;
    while(*string && i < max_letters)
    {
        dui_draw_char(x-16, y, *string);
        width+=10;
        x+=10;
        ++string;
		++i;
    }
    return width;
}
//return the total width of a string in window pixels
u32 dui_str_len( char *string)
{
    u32 width = 0;
    while(*string)
    {
        width+=10;
        ++string;
    }
    return width;
}

b32 dui_rect_hit(dui_Rect rect)
{
    if (rect.x > ui.mouse_pos.x || rect.x + rect.w < ui.mouse_pos.x || rect.y > ui.mouse_pos.y || rect.y + rect.h  < ui.mouse_pos.y)
        return FALSE; 
    return TRUE;
}
void dui_frame_begin(void)
{
    ui.hot = FALSE;
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
b32 dui_button(DUIID id, dui_Rect rect)
{

    if (dui_rect_hit(rect))
    {
        ui.hot = id;
        if (ui.active == 0 && ui.mouse_down)
            ui.active = id;
    }
    if (ui.hot == id)
    {
        if (ui.active ==id)
            dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.fg);
        else 
            dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.bg_lite);
    }
    else 
        dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.bg);

    //user just clicked!
    if (ui.mouse_down == 0 && ui.hot == id && ui.active == id)return 1;
    //#not
    return 0;
}

b32 dui_switch(DUIID id, dui_Rect rect, b32 *value)
{
    if (dui_rect_hit(rect))
    {
        ui.hot = id;
        if (ui.active == 0 && ui.mouse_down)
            ui.active = id;
    }
    if (*value)
        dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.fg);
    else 
        dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.bg_lite);

    //user just clicked!
    if (ui.mouse_down == 0 && ui.hot == id && ui.active == id)
    {
        if (*value)*value = 0;
        else *value = 1;
        return 1;
    }

    //#not
    return 0;

}

b32 dui_text_box(DUIID id, dui_Rect rect, char *string, u32 max_letters_to_draw)
{
	if (dui_rect_hit(rect))
    {
        ui.hot = id;
        if (ui.active == 0 && ui.mouse_down)
            ui.active = id;
    }
    if (ui.hot == id)
    {
        if (ui.active ==id)
            dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.fg);
        else 
            dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.bg_lite);
    }
    else 
        dui_draw_rect(rect.x, rect.y, rect.w, rect.h, layout.bg);
	
	u32 starting_height = rect.y + rect.h - 16;
	u32 starting_width = rect.x;
	u32 partitions = dui_str_len(string) / rect.w;
	for (u32 i = 0; i <=partitions && max_letters_to_draw > 0; ++i)
	{
		dui_draw_stringn(starting_width, starting_height, string, minimum(rect.w /10, max_letters_to_draw));
		starting_height -= 16;
		string += (u32)rect.w / 10;
		max_letters_to_draw -= (u32)rect.w / 10;
	}
    //user just clicked!
    if (ui.mouse_down == 0 && ui.hot == id && ui.active == id)
	{
		ui.accum = INT_MAX;
		return 1;
	}
    //#not
    return 0;
}

b32 dui_slider(DUIID id, f32 x, f32 y, f32 max, i32 *value)
{
    i32 xpos = ((DUI_DEF_X- 8) * (*value)) / max;

    if (dui_rect_hit((dui_Rect){x,y, DUI_DEF_X,DUI_DEF_Y}))
    {
        ui.hot = id;
        if (ui.active == 0 && ui.mouse_down)
            ui.active = id;
    }
    //render slider
    dui_draw_rect(x, y, DUI_DEF_X, DUI_DEF_Y, layout.bg);

    if (ui.active ==id || ui.hot == id)
        dui_draw_rect(x + xpos, y, 8, DUI_DEF_Y, layout.fg);
    else
        dui_draw_rect(x + xpos, y, 8, DUI_DEF_Y, layout.bg_lite);
    //update slider value
    if (ui.active == id)
    {
        i32 mouse_pos = ui.mouse_pos.x - (x+8);
        if (mouse_pos < 0)mouse_pos = 0;
        if (mouse_pos > DUI_DEF_X)mouse_pos = DUI_DEF_X;
        i32 v = (mouse_pos * max) / DUI_DEF_X;
        if (v!=*value){*value = v;return 1;}
    }
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

b32 dui_slider_text(DUIID id, f32 x, f32 y, f32 max,char *string,  i32 *value)
{
    vec2 mid = v2(x + DUI_DEF_X/2, y + DUI_DEF_Y/ 2);
    mid.x -= dui_str_len(string) / 2;
    mid.y -= 16/2;
    if(string != NULL)
        dui_draw_string(mid.x,mid.y,string);


    return dui_slider(id, x, y, max, value);
}

b32 dui_button_text(DUIID id, dui_Rect rect, char *string)
{
    vec2 mid = v2(rect.x + rect.w /2, rect.y + rect.h / 2);
    mid.x -= dui_str_len(string) / 2;
    mid.y -= 16/2;

    if(string != NULL)
        dui_draw_string(mid.x, mid.y, string);

    return dui_button(id, rect);
}

b32 dui_switch_text(DUIID id, dui_Rect rect, char *string, b32 *value)
{
    vec2 mid = v2(rect.x + rect.w /2, rect.y + rect.h / 2);
    mid.x -= dui_str_len(string) / 2;
    mid.y -= 16/2;
    if(string != NULL)
        dui_draw_string(mid.x, mid.y, string);

    return dui_switch(id, rect, value);
}


/*
//@COROUTINE TEST
char to_print[8];
u32 count = 0;
dui_draw_string_timed_basic(i32 x, i32 y, char *string, Coroutine *co)
{
    if (count < 18)
    {
        f32 milli = 1.f;
        COROUTINE_START(co);
        COROUTINE_WAIT(co, milli, global_platform.dt);
        COROUTINE_CALL(co, count++);
        COROUTINE_END(co);
    }

    memcpy(to_print, string, count);
    dui_draw_string(x, y, to_print);

}
*/

