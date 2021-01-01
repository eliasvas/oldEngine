#include "platform.h"
#include "tools.h"

#include "quad.h" 
global Quad q;

internal void 
init(void)
{
    quad_init(&q, "../assets/dirt.tga");
}



internal void 
update(void)
{
}

internal void 
render(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.7,0.8,1,1);

    quad_render(&q);
}
