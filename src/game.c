#include "platform.h"
#include "tools.h"
#include "objloader.h"
#include "camera.h"
#include "model.h"
#include "fbo.h"

#include "quad.h" 
global mat4 view, proj;

global Camera cam;
global Model debug_cube;

internal void 
init(void)
{
    camera_init(&cam);
    model_init_cube(&debug_cube);
}



internal void 
update(void)
{
  camera_update(&cam);
  proj = perspective_proj(45.f,global_platform.window_width / (f32)global_platform.window_height, 0.1f,100.f); 
  view = get_view_mat(&cam);
}

internal void 
render(void)
{
    glViewport(0,0,global_platform.window_width, global_platform.window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.7,0.8,1,1);

    for (int i = 0; i < 20; i+=2)
    {
      for (int j = 0; j < 20;j+=2)
      {
        debug_cube.position = v3(i + 0.001,sin(i * global_platform.current_time),j);
        model_render(&debug_cube, &proj, &view);
      }
    }

    //model_render(&debug_cube, &proj, &view);
}
