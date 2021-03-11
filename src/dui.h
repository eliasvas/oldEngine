#ifndef DEV_UI_H
#define DEV_UI_H
#include "tools.h" //for vectors mainly

//this is heavily influenced by microui
#define dui_stack(T, n) struct {f32 idx; T items[n];} 
#define GEN_ID (__LINE__)
typedef struct dui_Rect
{
    f32 x,y,w,h;
}dui_Rect;

typedef u32 DUIID;

typedef struct dui_State
{
    DUIID hot;
    DUIID active;
    DUIID last_widget;
    DUIID hot_to_be; //set next frame!

    vec2 mouse_pos;
    vec2 mouse_delta;
    b32 mouse_down;
}dui_State;

typedef struct dui_Layout
{
    color4 fg;
    color4 bg, bg_dark, bg_lite;

    f32 cx, cy;
}dui_Layout;

//------these functions are application dependent-------
void dui_draw_rect(f32 x, f32 y, f32 w, f32 h, color4 color);
void dui_draw_char(f32 x, f32 y, char ch);
//------------------------------------------------------

b32 do_button(DUIID id, dui_Rect rect);
b32 do_switch(DUIID id, dui_Rect rect, b32 *value);
b32 do_slider(DUIID id, f32 x, f32 y, f32 max, i32 *value);
void dui_default(void);

void dui_frame_begin(void);
void dui_frame_end(void);

extern dui_Layout layout;
extern dui_State ui;
#endif
