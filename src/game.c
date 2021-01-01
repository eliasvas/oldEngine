#include "platform.h"
#include "tools.h"
#include "camera.h"

#include "quad.h" 
global mat4 view, proj;

global Quad q;
global Camera cam;

internal void 
init(void)
{
    quad_init(&q, "../assets/dirt.tga");
    camera_init(&cam);
}



internal void 
update(void)
{
  glViewport(0,0,global_platform.window_width, global_platform.window_height);
  camera_update(&cam);
  proj = perspective_proj(45.f,global_platform.window_width / (f32)global_platform.window_height, 0.1f,100.f); 
  view = get_view_mat(&cam);
}

internal void 
render(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.7,0.8,1,1);

    quad_render(&q);
}
