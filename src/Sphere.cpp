#include "Sphere.h"


// Destructor: clean up allocated data
Sphere::~Sphere() {
    if(glIsVertexArray(vao)) {
        glDeleteVertexArrays(1, &vao);
    }
    vao = 0;

    if(glIsBuffer(vertexbuffer)) {
        glDeleteBuffers(1, &vertexbuffer);
    }
    vertexbuffer = 0;

    if(glIsBuffer(indexbuffer)) {
        glDeleteBuffers(1, &indexbuffer);
    }
    indexbuffer = 0;

    if(vertexarray) {
        delete[] vertexarray;
        vertexarray = NULL;
    }
    if(indexarray) 	{
        delete[] indexarray;
        indexarray = NULL;
    }
    nverts = 0;
    ntris = 0;
};



//Constructor: create a sphere (approximated by polygon segments)
// Author: Stefan Gustavson (stegu@itn.liu.se) 2014.
// This code is in the public domain.
Sphere::Sphere(float radius, int segments) {

    int i, j, base, i0;
    float x, y, z, R;
    double theta, phi;
    int vsegs, hsegs;
    int stride = 8;

    vsegs = segments;
    if (vsegs < 2) vsegs = 2;
    hsegs = vsegs * 2;
    nverts = 1 + (vsegs-1) * (hsegs+1) + 1; // top + middle + bottom
    ntris = hsegs + (vsegs-2) * hsegs * 2 + hsegs; // top + middle + bottom
    vertexarray = new float[nverts * 8];
    indexarray = new GLuint[ntris * 3];

    // The vertex array: 3D xyz, 3D normal, 2D st (8 floats per vertex)
    // First vertex: top pole (+z is "up" in object local coords)
    vertexarray[0] = 0.0f;
    vertexarray[1] = 0.0f;
    vertexarray[2] = radius;
    vertexarray[3] = 0.0f;
    vertexarray[4] = 0.0f;
    vertexarray[5] = 1.0f;
    vertexarray[6] = 0.5f;
    vertexarray[7] = 1.0f;
    // Last vertex: bottom pole
    base = (nverts-1)*stride;
    vertexarray[base] = 0.0f;
    vertexarray[base+1] = 0.0f;
    vertexarray[base+2] = -radius;
    vertexarray[base+3] = 0.0f;
    vertexarray[base+4] = 0.0f;
    vertexarray[base+5] = -1.0f;
    vertexarray[base+6] = 0.5f;
    vertexarray[base+7] = 0.0f;
    // All other vertices:
    // vsegs-1 latitude rings of hsegs+1 vertices each
    // (duplicates at texture seam s=0 / s=1)
#ifndef M_PI
#define M_PI 3.1415926536
#endif // M_PI
    for(j=0; j<vsegs-1; j++) { // vsegs-1 latitude rings of vertices
        theta = (double)(j+1)/vsegs*M_PI;
        z = cos(theta);
        R = sin(theta);
        for (i=0; i<=hsegs; i++) { // hsegs+1 vertices in each ring (duplicate for texcoords)
            phi = (double)i/hsegs*2.0*M_PI;
            x = R*cos(phi);
            y = R*sin(phi);
            base = (1+j*(hsegs+1)+i)*stride;
            vertexarray[base] = radius*x;
            vertexarray[base+1] = radius*y;
            vertexarray[base+2] = radius*z;
            vertexarray[base+3] = x;
            vertexarray[base+4] = y;
            vertexarray[base+5] = z;
            vertexarray[base+6] = (float)i/hsegs;
            vertexarray[base+7] = 1.0f-(float)(j+1)/vsegs;
        }
    }

    // The index array: triplets of integers, one for each triangle
    // Top cap
    for(i=0; i<hsegs; i++) {
        indexarray[3*i]=0;
        indexarray[3*i+1]=1+i;
        indexarray[3*i+2]=2+i;
    }
    // Middle part (possibly empty if vsegs=2)
    for(j=0; j<vsegs-2; j++) {
        for(i=0; i<hsegs; i++) {
            base = 3*(hsegs + 2*(j*hsegs + i));
            i0 = 1 + j*(hsegs+1) + i;
            indexarray[base] = i0;
            indexarray[base+1] = i0+hsegs+1;
            indexarray[base+2] = i0+1;
            indexarray[base+3] = i0+1;
            indexarray[base+4] = i0+hsegs+1;
            indexarray[base+5] = i0+hsegs+2;
        }
    }
    // Bottom cap
    base = 3*(hsegs + 2*(vsegs-2)*hsegs);
    for(i=0; i<hsegs; i++) {
        indexarray[base+3*i] = nverts-1;
        indexarray[base+3*i+1] = nverts-2-i;
        indexarray[base+3*i+2] = nverts-3-i;
    }

    // Generate one vertex array object (VAO) and bind it
    glGenVertexArrays(1, &(vao));
    glBindVertexArray(vao);

    // Generate two buffer IDs
    glGenBuffers(1, &vertexbuffer);
    glGenBuffers(1, &indexbuffer);

    // Activate the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Present our vertex coordinates to OpenGL
    glBufferData(GL_ARRAY_BUFFER,
                 8*nverts * sizeof(GLfloat), vertexarray, GL_STATIC_DRAW);
    // Specify how many attribute arrays we have in our VAO
    glEnableVertexAttribArray(0); // Vertex coordinates
    glEnableVertexAttribArray(1); // Normals
    glEnableVertexAttribArray(2); // Texture coordinates
    // Specify how OpenGL should interpret the vertex buffer data:
    // Attributes 0, 1, 2 (must match the lines above and the layout in the shader)
    // Number of dimensions (3 means vec3 in the shader, 2 means vec2)
    // Type GL_FLOAT
    // Not normalized (GL_FALSE)
    // Stride 8 (interleaved array with 8 floats per vertex)
    // Array buffer offset 0, 3, 6 (offset into first vertex)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          8*sizeof(GLfloat), (void*)0); // xyz coordinates
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          8*sizeof(GLfloat), (void*)(3*sizeof(GLfloat))); // normals
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          8*sizeof(GLfloat), (void*)(6*sizeof(GLfloat))); // texcoords

    // Activate the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
    // Present our vertex indices to OpenGL
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 3*ntris*sizeof(GLuint), indexarray, GL_STATIC_DRAW);

    // Deactivate (unbind) the VAO and the buffers again.
    // Do NOT unbind the buffers while the VAO is still bound.
    // The index buffer is an essential part of the VAO state.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

};

//Used to render the geometry
//mode : Specifies what kind of primitives to render
void Sphere::draw(GLenum mode){
    glBindVertexArray(vao);
    glDrawElements(mode, 3 * ntris, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
};


