#ifndef CAMERA_H
#define CAMERA_H

#include "tools.h"

local_persist f32 MAX_DISTANCE = 5.f;

typedef struct Camera
{
    vec3 pos;
    vec3 front;
    vec3 up;
    f32 camera_speed;


    f32 yaw;
    f32 pitch;
    f32 last_x;
    f32 last_y;
    b32 first_mouse;
    b32 can_rotate;
}Camera;

internal void 
camera_init(Camera* cam)
{
    cam->pos = v3(0,3,10);
    cam->front = v3(0.0f,0.0f,-1.0f);
    cam->up = v3(0.f,1.f,0.f);
    cam->yaw = -90.0f;
    
    //cam->camera_speed = 100.f;
    cam->camera_speed = 15.f;
    cam->first_mouse = 0;
}

internal void 
camera_update(Camera* cam)
{
    //global_platform.dt = 0.01f;
    if (global_platform.key_down[KEY_A])
        cam->pos = vec3_sub(cam->pos, vec3_mulf(vec3_cross(cam->front, cam->up), cam->camera_speed * global_platform.dt));
    if (global_platform.key_down[KEY_D])
        cam->pos = vec3_add(cam->pos, vec3_mulf(vec3_cross(cam->front, cam->up), cam->camera_speed * global_platform.dt));
    if (global_platform.key_down[KEY_W])
        cam->pos = vec3_add(cam->pos, vec3_mulf(cam->front,cam->camera_speed* global_platform.dt));
    if (global_platform.key_down[KEY_S])
        cam->pos = vec3_sub(cam->pos, vec3_mulf(cam->front,cam->camera_speed* global_platform.dt));
    if (global_platform.key_down[KEY_SPACE])
        cam->pos = vec3_sub(cam->pos, vec3_mulf(cam->up,(-1.f)*cam->camera_speed* global_platform.dt));
    if (global_platform.key_down[KEY_CTRL])
        cam->pos = vec3_sub(cam->pos, vec3_mulf(cam->up,cam->camera_speed* global_platform.dt));




    if(!global_platform.left_mouse_down){cam->first_mouse = 1;return;}

    if(cam->first_mouse)
    {
        cam->last_x = global_platform.mouse_x;
        cam->last_y = global_platform.mouse_y;
        cam->first_mouse = 0;
    }

    f32 x_offset = global_platform.mouse_x - cam->last_x;
    f32 y_offset = global_platform.mouse_y - cam->last_y;
    cam->last_x = global_platform.mouse_x;
    cam->last_y = global_platform.mouse_y;

    f32 sensitivity = 0.05f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    if (!cam->can_rotate)return;
    cam->yaw   += x_offset;
    cam->pitch += y_offset;

    //for gimbal lock
    if(cam->pitch > 89.0f)
            cam->pitch = 89.0f;
    if(cam->pitch < -89.0f)
        cam->pitch = -89.0f;
    
    
    vec3 direction;
    direction.x = cos(to_radians(cam->yaw)) * cos(to_radians(cam->pitch));
    direction.y = sin(to_radians(cam->pitch));
    direction.z = sin(to_radians(cam->yaw)) * cos(to_radians(cam->pitch));
    cam->front = vec3_normalize(direction);
}

internal mat4 get_view_mat(Camera* cam)
{
    mat4 camera = look_at(cam->pos,vec3_add(cam->pos, cam->front),cam->up);
    return camera;
}
 

/* in case there is no math lib
INLINE mat4 look_at(vec3 eye, vec3 center, vec3 fake_up)
{
    mat4 res = m4();

    vec3 f = normalize_vec3(sub_vec3(center, eye));
    vec3 r = normalize_vec3(cross_vec3(f, fake_up));
    vec3 up = cross_vec3(r, f);

    res.elements[0][0] = r.x;
    res.elements[0][1] = up.x;
    res.elements[0][2] = -f.x;
    res.elements[0][3] = 0.0f;

    res.elements[1][0] = r.y;
    res.elements[1][1] = up.y;
    res.elements[1][2] = -f.y;
    res.elements[1][3] = 0.0f;

    res.elements[2][0] = r.z;
    res.elements[2][1] = up.z;
    res.elements[2][2] = -f.z;
    res.elements[2][3] = 0.0f;

    res.elements[3][0] = -dot_vec3(r, eye);
    res.elements[3][1] = -dot_vec3(up, eye);
    res.elements[3][2] = dot_vec3(f, eye);
    res.elements[3][3] = 1.0f;

    return res;
}
*/

#endif
