#version 430 core

layout(triangles) in;
layout(line_strip, max_vertices = 48) out; // change this (3*(verticesPerStrand+1)) if you change verticesPerStrand in main file
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D hairDataTexture;
uniform float verticesPerStrand;
uniform float noOfVertices;
uniform float dataVariablesPerMasterHair;

in vec2 teTexCoord[3];
in vec3 teNormal[3];
in vec3 teVertexIDs[3];
in vec3 teTessCoords[3];

out vec2 gTexCoord;
out vec3 gPosition;
out vec3 gTangent;


vec3 getPositionFromTexture(float vertexIndex, float hairIndex) {
    float offsetWidth = (1.0f / (verticesPerStrand * dataVariablesPerMasterHair) ) * 0.5f;
    float offsetHeight = (1.0f / noOfVertices) * 0.5f;
    vec2 hairStrandPos = vec2((hairIndex / verticesPerStrand) + offsetWidth, (vertexIndex / noOfVertices) + offsetHeight);
    return vec3(texture(hairDataTexture, hairStrandPos));
}

vec3 getInterpolatedPosition(int index, int hairIndex){
    vec3 masterHairPos0 = getPositionFromTexture(teVertexIDs[index].x, hairIndex);
    vec3 masterHairPos1 = getPositionFromTexture(teVertexIDs[index].y, hairIndex);
    vec3 masterHairPos2 = getPositionFromTexture(teVertexIDs[index].z, hairIndex);

    vec3 hairPos = teTessCoords[index].x * masterHairPos0 +
                      teTessCoords[index].y * masterHairPos1 +
                      teTessCoords[index].z * masterHairPos2;

    return hairPos;
}


void main()
{
    for(int index = 0; index < gl_in.length(); index++){
        vec3 lastPos = (gl_in[index].gl_Position).xyz;
        vec3 firstHairSegmentPos = getInterpolatedPosition(index, 0);

        // Create first hair vertex
        gl_Position = projection * view * model * gl_in[index].gl_Position;
        gTexCoord = teTexCoord[index];
        gPosition = lastPos;
        gTangent = normalize(firstHairSegmentPos - lastPos);
        EmitVertex();

        // Create hair vertices
        for(int hairIndex = 1; hairIndex < verticesPerStrand; hairIndex++){
            vec3 hairPos = getInterpolatedPosition(index, hairIndex);
            gl_Position = projection * view * vec4(hairPos, 1.0);
            gTexCoord = teTexCoord[index];
            gPosition = hairPos;
            gTangent = normalize(hairPos - lastPos);
            EmitVertex();

            lastPos = hairPos;
        }

        EndPrimitive();
    }
}