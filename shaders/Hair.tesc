#version 430 core

layout (vertices = 3) out;

uniform vec3 cameraPosition;

in vec2 vTexCoord[];
in vec3 vNormal[];
in float vVertexID[];

out vec2 tcTexCoord[];
out vec3 tcNormal[];
out float tcVertexID[];

void main(void)
{

    vec3 ac = vec3(gl_in[1].gl_Position - gl_in[0].gl_Position);
    vec3 bc = vec3(gl_in[2].gl_Position - gl_in[0].gl_Position);
    vec3 triangleNormal = cross(ac, bc);

    float area = 0.5f * length(triangleNormal);
    float numberOfTesselations = area*350.f; // 350.f gave good result for the test objects used


    gl_TessLevelInner[0] = numberOfTesselations;
    gl_TessLevelOuter[0] = numberOfTesselations;
    gl_TessLevelOuter[1] = numberOfTesselations;
    gl_TessLevelOuter[2] = numberOfTesselations;

    //Pass through variables
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
    tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
    tcVertexID[gl_InvocationID] = vVertexID[gl_InvocationID];
}
