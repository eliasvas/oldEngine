#include "tools.h"
#include "animation.h"
 Joint 
joint_sid(u32 index, String name,String sid, mat4 local_bind_transform)
{
    Joint j;
    j.index = index;
    j.name = name;
    
    j.sid = sid;
    j.local_bind_transform = local_bind_transform;
    //this could just be a size in j.children 
    j.num_of_children = 0;
    //j.children = NULL;
    //j.children = (Joint*)malloc(sizeof(Joint) * 10);
    j.inv_bind_transform = m4d(1.f);
    j.animated_transform = m4d(1.f);
    
    return j;
}

 Joint 
joint(u32 index, String name, mat4 local_bind_transform)
{
    Joint j;
    
    j.index = index;
    j.name = name;
    j.local_bind_transform = local_bind_transform;
    
    return j;
}

 JointTransform 
joint_transform(vec3 position, Quaternion rotation)
{
    JointTransform res;
    res.position = position;
    res.rotation = rotation;
    return res;
}

 mat4
get_joint_transform_matrix(JointTransform j)
{
    mat4 res;
    res = mat4_mul(mat4_translate(j.position), quat_to_mat4(j.rotation));
    return res;
}

JointTransform
interpolate_joint_transforms(JointTransform l, JointTransform r, f32 time)
{
    JointTransform res = {0};
    vec3 pos = vec3_lerp(l.position,r.position, time);
    Quaternion q = nlerp(l.rotation, r.rotation, time); //maybe we need slerp??
    res.position = pos;
    res.rotation = q;
    return res;
}
void 
increase_animation_time(AnimationController* ac)
{
    assert(ac);
    ac->animation_time += global_platform.dt * ac->current_animation->playback_rate; //this should be the Δt from global platform but its bugged rn
    //ac->animation_time += global_platform.dt * ac->current_animation->playback_rate / 20.f; //this should be the Δt from global platform but its bugged rn
    if (ac->animation_time > ac->current_animation->length)
        ac->animation_time -= ac->current_animation->length;
    //NOTE(ilias): this is in case playback rate is negative
    if (ac->animation_time < 0.f)
      ac->animation_time += ac->current_animation->length;
    //TODO(ilias): check da math
    ac->fade_blend_percentage -= (1.f/(ac->fade_blend_time)) *global_platform.dt;
    if (ac->fade_blend_percentage < 0.f)ac->fade_blend_percentage = 0.f;

}

//mat4 animated_joint_transform = concat_local_transforms(joints, local_transforms, index); 
 mat4
concat_local_transforms(Joint *joints, mat4 *local_transforms, u32 index)
{
    //root has parent id == its index
    if (index == joints[index].parent_id)
        return local_transforms[index];
    return mat4_mul(concat_local_transforms(joints, local_transforms, joints[index].parent_id), local_transforms[index]); 
}


 void
calc_animated_transform(AnimationController *ac, Joint *joints, mat4 *local_transforms, u32 index)
{
    //here we get the animated joint transform meaning the world pos of the joint in the animation
    mat4 animated_joint_transform = concat_local_transforms(joints, local_transforms, index); 
    joints[index].animated_transform = mat4_mul(animated_joint_transform, joints[index].inv_bind_transform);
}

internal char joint_transforms[21] = "joint_transforms[00]";
internal char joint_transforms_one[20] = "joint_transforms[0]";
 void 
set_joint_transform_uniforms(Shader* s, Joint *j)
{
    if (j->index >= 10){
        joint_transforms[17] = '0' + (j->index/10);
        joint_transforms[18] = '0' + (j->index -(((int)(j->index/10)) * 10));
        shader_set_mat4fv(s, joint_transforms, (f32*)j->animated_transform.elements);
    }else
    {
        joint_transforms_one[17] = '0' + (j->index);
        shader_set_mat4fv(s, joint_transforms_one, (f32*)j->animated_transform.elements);
    }
}




 JointKeyFrame* 
get_previous_and_next_keyframes(AnimationController *ac, i32 joint_animation_index)
{
    JointKeyFrame frames[2];
    JointKeyFrame* all_frames = ac->current_animation->joint_animations[joint_animation_index].keyframes;
    JointKeyFrame prev = all_frames[0];
    JointKeyFrame next = all_frames[0];
    f32 animation_time = ac->animation_time;
    for (i32 i = 1; i < ac->current_animation->joint_animations[joint_animation_index].keyframe_count; ++i)
    {
        next = all_frames[i];
        if (next.timestamp >= ac->animation_time)
            break;
        prev = all_frames[i];
    }
    frames[0] = prev;
    frames[1] = next;
    return (frames);
}

 f32 calc_progress(AnimationController *ac, JointKeyFrame prev, JointKeyFrame next)
{
    f32 total_time = next.timestamp - prev.timestamp;
    f32 current_time = ac->animation_time - prev.timestamp;
    //if (current_time/total_time < 0.4)return 0;
    return current_time / total_time;
}

JointKeyFrame interpolate_poses(JointKeyFrame prev, JointKeyFrame next, f32 x)
{
    JointKeyFrame res;
    //if (x > 1 || x < 0)sprintf(error_log, "x: ", x);
    
    vec3 angle1 = quat_to_angle(prev.transform.rotation);
    vec3 angle2 = quat_to_angle(next.transform.rotation);
    res.transform.position = vec3_lerp(prev.transform.position, next.transform.position, x);
    prev.transform.rotation = quat_normalize(prev.transform.rotation); 
    next.transform.rotation = quat_normalize(next.transform.rotation); 
    res.transform.rotation = nlerp(prev.transform.rotation, next.transform.rotation, x);
    //THIS IS HUUUUUUUUUUUUGE
    //https://stackoverflow.com/questions/42428136/quaternion-is-flipping-sign-for-very-similar-rotations
    if (quat_mul(prev.transform.rotation, quat_conj(next.transform.rotation)).w < 0)
        res.transform.rotation = nlerp(prev.transform.rotation, next.transform.rotation, -x);
    res.joint_index = prev.joint_index;
    
    return res;
    
}


 JointKeyFrame calc_current_animation_pose(AnimationController *ac, u32 joint_animation_index)
{
    JointKeyFrame* frames = get_previous_and_next_keyframes(ac, joint_animation_index);
    f32 x = calc_progress(ac, frames[0],frames[1]);
    //if (joint_animation_index == 28 && global_platform.current_time >=1.f)snprintf(error_log, sizeof(error_log), "%f", x);
    return interpolate_poses(frames[0],frames[1],x);
}


void
animation_controller_update_works(AnimationController *ac)
{
    //this first step gives default animations where they can be given
    if (ac->current_animation == NULL && ac->anims_count > 0)ac->current_animation = &ac->anims[0];
    if (ac->current_animation == NULL)return;
    increase_animation_time(ac);
      //this is the array holding the animated local bind transforms for each joint,
    //if there is no animation in a certain joint its simply m4d(1.f)
    mat4 *local_animated_transforms= (mat4*)arena_alloc(&global_platform.frame_storage, sizeof(mat4) * ac->model.joint_count);
    for (i32 i = 0; i < ac->model.joint_count; ++i)
    {
        local_animated_transforms[i] = m4d(1.f);//mul_mat4(translate_mat4((vec3){0,0,0}), quat_om_angle((vec3){0,1,0}, 0));
    }

    //setting every prev joint pose to m4d(1.f)
    for (u32 i = 0; i < ac->model.joint_count; ++i)
    {
        JointKeyFrame current_pose = calc_current_animation_pose(ac, i); 
        if (ac->fade_blend_percentage < 0.001f)
        {
          ac->prev_pose[current_pose.joint_index] = (JointKeyFrame){0};
          //this might be the error
          ac->prev_pose[current_pose.joint_index].transform.rotation = (Quaternion){0};//quat_from_angle(v3(0,1,0),0);
        }
    }
    //we put the INTERPOLATED local(wrt parent) animated transforms in the array
    for (u32 i = 0; i < ac->current_animation->joint_anims_count; ++i)
    {
        JointKeyFrame current_pose = calc_current_animation_pose(ac, i); 
        //JointKeyFrame current_pose = ac->anim->joint_animations[i].keyframes[((int)(global_platform.current_time * 24) % ac->anim->joint_animations[i].keyframe_count)];
        mat4 local_animated_transform = mat4_mul(mat4_translate(current_pose.transform.position), quat_to_mat4(current_pose.transform.rotation));
        local_animated_transforms[current_pose.joint_index] = local_animated_transform;
        if (ac->fade_blend_percentage > 0.001f)
        {
          current_pose = interpolate_poses(current_pose,ac->prev_pose[current_pose.joint_index], ac->fade_blend_percentage);
          mat4 local_animated_transform = mat4_mul(mat4_translate(current_pose.transform.position), quat_to_mat4(current_pose.transform.rotation));
          local_animated_transforms[current_pose.joint_index] = local_animated_transform;
          ac->prev_pose[current_pose.joint_index] = current_pose;
        }else
          ac->prev_pose[current_pose.joint_index] = current_pose;
    }

    //now we recursively apply the pose to get the animated bind(wrt world) transform
    for (u32 i = 0; i < ac->model.joint_count;++i)
        calc_animated_transform(ac, ac->model.joints, local_animated_transforms, ac->model.joints[i].index);

    //for (u32 i = 0; i < ac->model.joint_count; ++i)
        //ac->model.joints[i].animated_transform = mat4_mul(ac->model.joints[i].animated_transform, ac->model.bind_shape_matrix);

}




 void
animation_controller_update(AnimationController *ac)
{
    //this first step gives default animations where they can be given
    if (ac->current_animation == NULL && ac->anims_count > 0)ac->current_animation = &ac->anims[0];
    if (ac->current_animation == NULL)return;
    AnimationClip *secondary_animation = &ac->anims[1];
    increase_animation_time(ac);
    mat4 *local_animated_transforms= (mat4*)arena_alloc(&global_platform.frame_storage, sizeof(mat4) * ac->model.joint_count);
    mat4 *local_animated_transforms_secondary= (mat4*)arena_alloc(&global_platform.frame_storage, sizeof(mat4) * ac->model.joint_count);
    for (i32 i = 0; i < ac->model.joint_count; ++i)
    {
        local_animated_transforms[i] = m4d(1.f);
        local_animated_transforms_secondary[i] = m4d(1.f);
    }
    ac->current_animation = &ac->anims[1];
    for (u32 i = 0; i < ac->current_animation->joint_anims_count; ++i)
    {
        JointKeyFrame current_pose = calc_current_animation_pose(ac, i); 
        mat4 local_animated_transform = mat4_mul(mat4_translate(current_pose.transform.position), quat_to_mat4(current_pose.transform.rotation));
        local_animated_transforms[current_pose.joint_index] = local_animated_transform;
    }
    ac->current_animation = &ac->anims[0];
    for (u32 i = 0; i < ac->current_animation->joint_anims_count; ++i)
    {
        JointKeyFrame current_pose = calc_current_animation_pose(ac, i); 
        mat4 local_animated_transform = mat4_mul(mat4_translate(current_pose.transform.position), quat_to_mat4(current_pose.transform.rotation));
        local_animated_transforms_secondary[current_pose.joint_index] = local_animated_transform;
    }
    ac->current_animation = &ac->anims[1];
    /*
    for (u32 i = 0; i < ac->current_animation->joint_anims_count; ++i)
    {
       JointKeyFrame current_pose = calc_current_animation_pose(ac, i); //here just for the index 
       mat4 D = mat4_mul(local_animated_transforms[current_pose.joint_index],mat4_transpose(local_animated_transforms_secondary[current_pose.joint_index]));
       local_animated_transforms[current_pose.joint_index] = mat4_add(D, local_animated_transforms_secondary[current_pose.joint_index]); 
    }
    */
    for (u32 i = 0; i < ac->current_animation->joint_anims_count; ++i)
    {
       JointKeyFrame current_pose = calc_current_animation_pose(ac, i); //here just for the index 
       mat4 D = mat4_mul(local_animated_transforms_secondary[current_pose.joint_index],mat4_inv(local_animated_transforms[current_pose.joint_index]));
       //local_animated_transforms[current_pose.joint_index] = mat4_mul(D, local_animated_transforms_secondary[current_pose.joint_index]); 
       mat4 A = mat4_mul(D, local_animated_transforms_secondary[current_pose.joint_index]); 
       mat4 M = local_animated_transforms_secondary[current_pose.joint_index];

       JointTransform t1 = {v3(A.elements[3][0],A.elements[3][1],A.elements[3][2]),mat4_to_quat(A), A};
       JointTransform t2 = {v3(M.elements[3][0],M.elements[3][1],M.elements[3][2]),mat4_to_quat(M), M};

       JointTransform final = interpolate_joint_transforms(t2, t1, 0.f);
       mat4 final_transform = mat4_mul(mat4_translate(final.position), quat_to_mat4(final.rotation));
       local_animated_transforms[current_pose.joint_index] = final_transform;
    }


    for (u32 i = 0; i < ac->model.joint_count;++i)
        calc_animated_transform(ac, ac->model.joints, local_animated_transforms, ac->model.joints[i].index);
    //for (u32 i = 0; i < ac->model.joint_count; ++i)
        //ac->model.joints[i].animated_transform = mat4_mul(ac->model.joints[i].animated_transform, ac->model.bind_shape_matrix);

}


GLuint 
animated_model_create_vao(MeshData* data)
{
    GLuint vao;
    GLuint ebo;
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    //glGenBuffers(1,&ebo);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); 
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(data->indices_size), &data->indices, GL__DRAW);
    
    //positions
    glEnableVertexAttribArray(0);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(AnimatedModel) * data->vertex_count, &data->vertices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*)(0));
    //normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 3));
    //tex_coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 6));
    //joints (max 3)
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 3, GL_INT,sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 8));
    //joint weights (max 3)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 8 + sizeof(int) * 3));
    
    glBindVertexArray(0);
    
    
    return vao;
}
 void 
render_animated_model(AnimatedModel* model, Shader* s, mat4 proj, mat4 view)
{
    use_shader(s);
    
    shader_set_mat4fv(s, "projection_matrix", (GLfloat*)proj.elements);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,model->diff_tex->id);
    shader_set_int(s, "diffuse_map", 1); //we should really make the texture manager global or something(per Scene?)... sigh
    //for(i32 i = 0; i < model->joint_count; ++i)
    //to start things off, everything is identity!
#if 1
    {
        mat4 identity = m4d(1.f);
        shader_set_mat4fv(s, "joint_transforms[0]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[1]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[2]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[3]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[4]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[5]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[6]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[7]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[8]", (GLfloat*)identity.elements);
        shader_set_mat4fv(s, "joint_transforms[9]", (GLfloat*)identity.elements);
        //@memleak
        char *str = "joint_transforms[xx]";

        for (i32 i = 10; i < model->joint_count; ++i)
        {
            str[17] = '0' + (i/10);
            str[18] = '0' + (i -(((int)(i/10)) * 10));
            shader_set_mat4fv(s, str, (GLfloat*)identity.elements);
        }
    }
#endif

    for (u32 i = 0; i < model->joint_count; ++i)
        set_joint_transform_uniforms(s, &model->joints[i]);
    shader_set_mat4fv(s, "view_matrix", (GLfloat*)view.elements);
    glUniform3f(glGetUniformLocation(s->ID, "light_direction"), 0.43,0.34,0.f); 

    glBindVertexArray(model->vao);
    glDrawArrays(GL_TRIANGLES,0, model->vertices_count);
    //glDrawArrays(GL_LINES,0, 20000);
    glBindVertexArray(0);
    
}

AnimatedModel
animated_model_init(Texture* diff, Joint root,MeshData* data)
{
    AnimatedModel model = {0};
    
    model.vao = animated_model_create_vao(data);
    model.diff_tex = diff;
    model.root = root;
    model.transforms = data->transforms;
    model.joint_count = data->joint_count;
    model.joints = data->joints;
    model.bind_shape_matrix = data->bind_shape_matrix;
    model.vertices_count = data->vertex_count;
    model.model = m4d(1.f);
    
    //not sure about this one -- let's figure out the parser first
    data->transforms = arena_alloc(&global_platform.permanent_storage, sizeof(mat4) * model.root.num_of_children);
    return model;
}

void
animation_controller_play_anim(AnimationController *ac, u32 i)
{
    ac->current_animation = &ac->anims[i];
    ac->fade_blend_percentage = 1.f;
    ac->fade_blend_time = 0.4f;
}

//TODO: this joint index might _NOT_ be the index of the joint, but an offset to the table? investigate
mat4 animation_controller_get_bone_transform(AnimationController *ac,u32 joint_index)
{
    if (!ac)return m4d(0.f);
    assert(ac->current_animation->joint_anims_count > joint_index);
    mat4 s = ac->model.joints[joint_index].animated_transform;
    s = mat4_mul(s, mat4_inv(ac->model.joints[joint_index].inv_bind_transform)); //why tho ?? :O
    s = mat4_mul(ac->model.model, s);
    return s;

}

mat4 animation_controller_socket(AnimationController *ac, u32 joint_index, mat4 socket_transform)
{
    mat4 bone_transform = animation_controller_get_bone_transform(ac,joint_index);
    return mat4_mul(socket_transform, bone_transform);
}

