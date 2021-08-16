#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#include "renderer.h"

#define MAX_PARTICLES 200 
typedef struct Particle
{
    vec3 pos, speed;
    vec4 color;
    f32 size, angle, weight;
    f32 life;
}Particle;


typedef struct ParticleEmitter
{
    vec3 emitter_pos;
    Particle particles[MAX_PARTICLES];
    u32 next_index;
}ParticleEmitter;

internal Particle particle_default(void)
{
    Particle p;
    p.pos = v3(random01(), random01(), random01());
    //p.color = v4(random01(), random01(), random01(), 1.f);
    p.color = v4(random01()+0.5, random01()+0.5, random01()+0.5, 1.f);
    p.speed = v3(random01()*2.f - 1, random01() * 2.f - 1, random01() * 2.f - 1);
    p.speed = vec3_mulf(p.speed, 2.f);
    p.weight = 0.1f;
    p.life = random01()*3;
    return p;
}
internal void particle_emitter_init(ParticleEmitter *emitter, vec3 pos)
{
    emitter->emitter_pos = pos;
    emitter->next_index = 0;
}

internal void particle_emitter_add_particle(ParticleEmitter *emitter, Particle p)
{
    if (emitter->next_index >= MAX_PARTICLES)return;
    emitter->particles[emitter->next_index++] = p;
}

internal void particle_emitter_delete_particle(ParticleEmitter *emitter, u32 index)
{
    if (emitter->next_index <= 0)return;
    emitter->particles[index] = emitter->particles[--emitter->next_index];
}

internal void particle_emitter_simulate(ParticleEmitter *emitter)
{
    u32 particles_to_add = random01() * 10.f;
    f32 dt = global_platform.dt;
    while (particles_to_add--)
            particle_emitter_add_particle(emitter, particle_default());

    for (u32 i = 0; i < emitter->next_index; ++i)
    {
        //spawn new particles
        //delete i, if neccesary
        if (emitter->particles[i].life <= 0.f)
        {
            particle_emitter_delete_particle(emitter, i);
        }
        emitter->particles[i].life -= dt;
        //simulate particle i
        emitter->particles[i].pos = vec3_add(emitter->particles[i].pos, vec3_mulf(emitter->particles[i].speed, dt));
        //emitter->particles[i].pos = vec3_sub(emitter->particles[i].pos, vec3_mulf(v3(0,emitter->particles[i].weight * 9.8,0), dt));
        emitter->particles[i].speed = vec3_divf(emitter->particles[i].speed, 0.997);
    }
}


internal void particle_emitter_render(ParticleEmitter *emitter, Renderer *rend)
{

    for (u32 i = 0; i < emitter->next_index; ++i)
    {
        //renderer_push_billboard(Renderer *rend, vec3 center, vec4 color)
        renderer_push_billboard(rend, vec3_add(emitter->particles[i].pos, emitter->emitter_pos), emitter->particles[i].color);
    }
}
#endif
