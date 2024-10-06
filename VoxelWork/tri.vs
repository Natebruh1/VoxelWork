#version 440 core


layout (location = 0) in vec3 aPos;
layout (location = 1) in int axis;
layout (location = 2) in ivec2 aTexCoord;

out float zDepth;
out vec3 voxCoords;
out vec2 texCoords;
out vec2 oDims;
flat out int ax;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * transform * vec4(aPos, 1.0);
    zDepth=((aPos.z)+1.0)*(1.0/8.0);
    
    //texCoords=vec2(clamp(aTexCoord,0,1));
    texCoords=vec2(aTexCoord);
    //texCoords=clamp(texCoords,0.0,1.0);
    //oDims=vec2(dims);
    //;

    //if (dims.x>0)
    //{
        //normCoords.x=dims.x/dims.x;
    //}
    //if (dims.y>0)
    //{
        //normCoords.y=dims.y/dims.y;
    //}
    //texCoords=vec2(normCoords);
    
    voxCoords=vec3(0.0,0.0,0.0);

    switch (axis)
    {
    case 0:
        voxCoords=vec3(aPos.x,aPos.y,aPos.z);
        break;
    case 1:
        voxCoords=vec3(aPos.x-1.0,aPos.y,aPos.z);
        break;
    case 2:
        voxCoords=vec3(aPos.x,aPos.y,aPos.z);
        break;
    case 3:
        voxCoords=vec3(aPos.x,aPos.y,aPos.z-1.0);
        break;
    case 4:
        voxCoords=vec3(aPos.x,aPos.y,aPos.z);
        break;
    case 5:
        voxCoords=vec3(aPos.x,aPos.y-1.0,aPos.z);
        break;
    }
    ax=axis;
    voxCoords=vec3(0.0,0.0,0.0);
}