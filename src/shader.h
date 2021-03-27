#ifndef SHADER_H
#define SHADER_H

#include "platform.h"
#include "tools.h"
#include <stdlib.h>
#include <stdio.h>

#if !defined(SHADER_INCLUDE)
#define SHADER_INCLUDE 0
#endif



typedef struct Shader
{
    GLuint ID;   
    const char* vertex_str;
    const char* fragment_str;
}Shader;



internal void use_shader(Shader* shader)
{
    glUseProgram(shader->ID);
}

internal GLuint shader_load_from_strings(char *vertex_str,  char *fragment_str, char *geometry_str)
{ 
    GLuint ID;
    // 2. compile shaders
    GLuint vertex, fragment, geometry;
    u32 success;
       
    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_str, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, error_log);
    }
      
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_str, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, error_log);
    }

    if (geometry_str)
    {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &geometry_str, NULL);
        glCompileShader(geometry);
        // print compile errors if any
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(geometry, 512, NULL, error_log);
        }
    }
      
    
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometry_str)
        glAttachShader(ID, geometry);
    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        //glGetProgramInfoLog(ID, 512, NULL, infoLog);
    }
      
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry)
        glDeleteShader(geometry);
    return ID;
}

internal void shader_load_full(Shader* s, char *vertex_path, char *fragment_path, char *geometry_path)
{
    s->vertex_str = vertex_path;
    s->fragment_str = fragment_path;
    char* vs = read_whole_file(vertex_path);
    char* fs = read_whole_file(fragment_path); 
    if (geometry_path)
    {
        char* gs = read_whole_file(geometry_path); 
        s->ID = shader_load_from_strings(vs,fs, gs);
        FREE(gs);
    }
    else
    {
        s->ID = shader_load_from_strings(vs,fs, NULL);
    }
    FREE(vs);
    FREE(fs);

}


internal void shader_load (Shader* s, char * vertex_path, char * fragment_path)
{
    shader_load_full(s,vertex_path, fragment_path, (char *)NULL);
}
internal void shader_load_compute(Shader* s, char * compute_path)
{
    //this should change but i am bored to death rn
    s->vertex_str = compute_path;
    char *cs = read_whole_file(compute_path);
 
    u32 compute;
    u32 success;
    compute= glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cs, NULL);
    glCompileShader(compute);
    // print compile errors if any
    glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(compute, 512, NULL, error_log);
    }
 
    // shader Program
    GLuint ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(compute, 512, NULL, error_log);
    }
    s->ID = ID;
    FREE(cs);
}


internal void shader_attach_geometry_shader(Shader *shader, char *geometry_path)
{
    u32 success;

    char *gs = read_whole_file(geometry_path);
    u32 geometry;
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &gs, NULL);
    glCompileShader(geometry);
    // print compile errors if any
    glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(geometry, 512, NULL, error_log);
    }
    char *vertex_path = shader->vertex_str;
    char *fragment_path = shader->fragment_str;

    char* vertex_str = read_whole_file(vertex_path);
    char* fragment_str = read_whole_file(fragment_path); 

    GLuint ID;
    GLuint vertex, fragment;
       
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_str, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success) { glGetShaderInfoLog(vertex, 512, NULL, error_log); }
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_str, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success) { glGetShaderInfoLog(fragment, 512, NULL, error_log); }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glAttachShader(ID, geometry);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success) { glGetProgramInfoLog(ID, 512, NULL, error_log); }
      
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteShader(geometry);
    shader->ID = ID;
}
internal void shader_reload_from_files( GLuint* program, char* vertex_shader_filename, char* fragment_shader_filename ) {
  assert( program && vertex_shader_filename && fragment_shader_filename );
  Shader new_shader;
  shader_load(&new_shader,vertex_shader_filename, fragment_shader_filename );
  if ( new_shader.ID ) {
    glDeleteProgram( *program );
    *program = new_shader.ID;
  }
}
internal void 
shader_set_bool(Shader* shader, char *name, b8 value)
{         
    glUniform1i(glGetUniformLocation(shader->ID, name), (int)value); 
}
internal void 
shader_set_int(Shader* shader, char *name, i32 value)
{ 
    glUniform1i(glGetUniformLocation(shader->ID, name), value); 
}
internal void 
shader_set_float(Shader * shader, char *name, f32 value)
{ 
    glUniform1f(glGetUniformLocation(shader->ID, name), value); 
} 
internal void
shader_set_vec3(Shader * shader, char *name, vec3 value)
{ 
    glUniform3f(glGetUniformLocation(shader->ID, name), value.x,value.y,value.z); 
} 

internal void
shader_set_vec4(Shader * shader, char *name, vec4 value)
{ 
    glUniform3f(glGetUniformLocation(shader->ID, name), value.x,value.y,value.z, value.w); 
} 
internal void 
shader_set_mat4fv(Shader * shader, char *name, f32* value)
{ 
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name),1,GL_FALSE, value);  //NOTE(ilias): check
}
#endif
