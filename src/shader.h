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

internal GLuint shader_load_from_strings (const char * vertex_str, const char * fragment_str)
{ 
    GLuint ID;
    // 2. compile shaders
    unsigned int vertex, fragment;
    int success;
       
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
      
    
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
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
    return ID;
}
internal void shader_load (Shader* s, const char * vertex_path, const char * fragment_path)
{
    s->vertex_str = vertex_path;
    s->fragment_str = fragment_path;
    const char* vs = read_whole_file(vertex_path);
    const char* fs = read_whole_file(fragment_path); 
    s->ID = shader_load_from_strings(vs,fs);

}

internal void shader_reload_from_files( GLuint* program, const char* vertex_shader_filename, const char* fragment_shader_filename ) {
  assert( program && vertex_shader_filename && fragment_shader_filename );
  Shader new_shader;
  shader_load(&new_shader,vertex_shader_filename, fragment_shader_filename );
  if ( new_shader.ID ) {
    glDeleteProgram( *program );
    *program = new_shader.ID;
  }
}
internal void 
shader_set_bool(Shader* shader, const char *name, b8 value)
{         
    glUniform1i(glGetUniformLocation(shader->ID, name), (int)value); 
}
internal void 
shader_set_int(Shader* shader, const char *name, i32 value)
{ 
    glUniform1i(glGetUniformLocation(shader->ID, name), value); 
}
internal void 
shader_set_float(Shader * shader, const char *name, f32 value)
{ 
    glUniform1f(glGetUniformLocation(shader->ID, name), value); 
} 
internal void
shader_set_vec3(Shader * shader, const char *name, vec3 value)
{ 
    glUniform3f(glGetUniformLocation(shader->ID, name), value.x,value.y,value.z); 
} 

internal void
shader_set_vec4(Shader * shader, const char *name, vec4 value)
{ 
    glUniform3f(glGetUniformLocation(shader->ID, name), value.x,value.y,value.z, value.w); 
} 
internal void 
shader_set_mat4fv(Shader * shader, const char *name, f32* value)
{ 
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name),1,GL_FALSE, value);  //NOTE(ilias): check
}
#endif
