#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform float dt;
uniform int dh;
uniform int direction;


struct Particle
{
    vec4 position;
    vec4 speed;
    vec4 iposition;
    vec4 ispeed;
};


layout(std430, binding = 0) buffer particles {
    Particle data[];
};


float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}


void main()
{
    vec3 pos = data[gl_VertexID].position.xyz;
    vec3 spd = data[gl_VertexID].speed.xyz;

  //  float dt = 0.01;

    if (direction == 1) {
        pos = pos + spd * dt + vec3(0, 1, 0) * dt * dt/2.0f;
        spd = spd + vec3(0, 1, 0) * dt;
    }
    if (direction == 2) {
        pos = pos + spd * dt + vec3(-1, 0, 0) * dt * dt/2.0f;
        spd = spd + vec3(-1, 0, 0) * dt;
    }
    if (direction == 3) {
        pos = pos + spd * dt + vec3(1, 0, 0) * dt * dt/2.0f;
        spd = spd + vec3(1, 0, 0) * dt;
    }
    if (direction == 4) {
        pos = pos + spd * dt + vec3(0, 0, -1) * dt * dt/2.0f;
        spd = spd + vec3(0, 0, -1) * dt;
    }
    if (direction == 5) {
        pos = pos + spd * dt + vec3(0, 0, 1) * dt * dt/2.0f;
        spd = spd + vec3(0, 0, 1) * dt;
    }

    if(pos.y > (dh - rand(pos.xy) * dh))
    {
        pos = data[gl_VertexID].iposition.xyz;
        spd = data[gl_VertexID].ispeed.xyz;
    }

    data[gl_VertexID].position.xyz =  pos;
    data[gl_VertexID].speed.xyz =  spd;

    gl_Position = Model * vec4(pos, 1);
}
