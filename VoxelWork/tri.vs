#version 330 core
layout (location = 0) in vec3 aPos;
out float zDepth;
uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * transform * vec4(aPos, 1.0);
    zDepth=((aPos.z)+1.0)*0.5;
}