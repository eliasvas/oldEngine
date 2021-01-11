#ifndef ANIMATION_H
#define ANIMATION_H
#include "tools.h"
#include "platform.h"
#include "shader.h"
#include "texture.h"
#include "objloader.h"//for Vertex definition

 Shader anim_shader;

//NOTE(ilias): lets implement frozen transitions!

typedef struct Joint
{
    u32 index;
    String name;
    String sid;
    u32 num_of_children;
    u32 parent_id;
    mat4 animated_transform; //joint transform
    mat4 local_bind_transform;
    mat4 inv_bind_transform;
}Joint;
typedef struct AnimatedVertex
{
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
    ivec3 joint_ids;
    vec3 weights;
}AnimatedVertex;

typedef struct MeshData{
    vec3 *positions; 
    vec3 *normals; 
    vec2 *tex_coords; 
    Vertex *verts; //just for rendering
    i32 vertex_count;
    i32 *joint_ids; 
    f32 *weights; 
    u32 size;

    mat4 *transforms;
    i32 transforms_count;
    mat4 bind_shape_matrix;

    Joint root;
    AnimatedVertex *vertices;

    Joint *joints;
    u32 joint_count;

} MeshData;


//represents the position and rotation of a joint in an animation frame (wrt parent)
typedef struct JointTransform
{
    vec3 position;
    Quaternion rotation;
    mat4 transform; //this is not mandatory
}JointTransform;

typedef struct JointKeyFrame
{
    f32 timestamp;
    u32 joint_index;
    JointTransform transform;
}JointKeyFrame;

typedef struct JointAnimation
{
    JointKeyFrame *keyframes;
    u32 keyframe_count;
    f32 length;
}JointAnimation;

typedef struct Animation
{
    JointAnimation *joint_animations;
    u32 joint_anims_count;
    f32 length;
    f32 playback_rate;
}Animation;



typedef struct AnimatedModel
{
    //skin
    GLuint vao;
    mat4 * transforms;
    Texture * diff_tex;
    //Texture * spec_tex;
    
    //skeleton
    Joint root;
    u32 joint_count;
    Joint *joints;
    mat4 bind_shape_matrix;
    u32 vertices_count;
    
}AnimatedModel;


typedef struct Animator
{
    AnimatedModel model;
    Animation* anim;
    f32 animation_time;

    JointKeyFrame *prev_pose;
    f32 blend_percentage; //in [0,1], tells us how much of prev_pose we should blend
    f32 blend_time; //time the (linear) blending should take place =1?
}Animator;

//is this correct? sure hope so..
typedef Animator AnimatorComponent;

 Joint 
joint_sid(u32 index, String name,String sid, mat4 local_bind_transform);

 Joint 
joint(u32 index, String name, mat4 local_bind_transform);

 JointTransform 
joint_transform(vec3 position, Quaternion rotation);

 mat4
get_joint_transform_matrix(JointTransform j);

 JointTransform
interpolate_joint_transforms(JointTransform l, JointTransform r, f32 time);

 void 
increase_animation_time(Animator* animator);

 mat4
concat_local_transforms(Joint *joints, mat4 *local_transforms, u32 index);

 void
calc_animated_transform(Animator *animator, Joint *joints, mat4 *local_transforms, u32 index);

 void 
set_joint_transform_uniforms(Shader *s, Joint *j);


 JointKeyFrame* 
get_previous_and_next_keyframes(Animator *animator, i32 joint_animation_index);


 f32 calc_progress(Animator *animator, JointKeyFrame prev, JointKeyFrame next);



JointKeyFrame interpolate_poses(JointKeyFrame prev, JointKeyFrame next, f32 x);


 JointKeyFrame calc_current_animation_pose(Animator *animator, u32 joint_animation_index);

 void
update_animator(Animator *animator);

 GLuint 
animated_model_create_vao(MeshData *data);

 void 
render_animated_model(AnimatedModel *model, Shader* s, mat4 proj, mat4 view);

AnimatedModel
animated_model_init(Texture *diff, Joint root,MeshData* data);


#endif

