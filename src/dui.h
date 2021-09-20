#ifndef DEV_UI_H
#define DEV_UI_H
#include "tools.h" //for vectors mainly

//this is heavily influenced by microui
#define dui_stack(T, n) struct {f32 idx; T items[n];} 
#define GEN_ID (__LINE__)
#define DUI_DEF_X 128
#define DUI_DEF_Y 32

typedef struct dui_Rect
{
    f32 x,y,w,h;
}dui_Rect;

typedef u32 DUIID;

typedef struct dui_State
{
    DUIID hot; //the item below the mouse cursor
    DUIID active; //the item pressed
    DUIID last_widget; //last item processed
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
    f32 scale;
}dui_Layout;

//------these functions are application dependent-------
void dui_draw_rect(f32 x, f32 y, f32 w, f32 h, color4 color);
void dui_draw_char(f32 x, f32 y, char ch);
//------------------------------------------------------

b32 dui_button(DUIID id, dui_Rect rect);
b32 dui_switch(DUIID id, dui_Rect rect, b32 *value);
b32 dui_slider(DUIID id, f32 x, f32 y, f32 max, i32 *value);
b32 dui_slider_text(DUIID id, f32 x, f32 y, f32 max, char *string, i32 *value);
void dui_default(void);

void dui_frame_begin(void);
void dui_frame_end(void);

extern dui_Layout layout;
extern dui_State ui;
#endif
