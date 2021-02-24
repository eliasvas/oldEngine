#ifndef DEV_UI_H
#define DEV_UI_H
#include "tools.h" //for vectors mainly

typedef struct dui_Rect
{
    f32 x,y,w,h;
}dui_Rect;

typedef u32 dui_Id;

typedef struct dui_Layout
{
    dui_Rect screen_rect;
}dui_Layout;

//------these functions are application dependent-------
void dui_draw_rect(f32 x, f32 y, f32 w, f32 h, color4 color);
void dui_draw_char(f32 x, f32 y, f32 w, f32 h, char ch);
//------------------------------------------------------
void dui_begin(void);
void dui_end(void);
#endif
