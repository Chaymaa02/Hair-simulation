#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h> 
#include <cstdio>  
#include <cmath>   
#include <cstring> 

// Some <cmath> headers define M_PI, some don't. Make sure we have it.
#ifndef M_PI
#define M_PI (3.14159265359)
#endif // M_PI

class Sphere {

public:

    // Destructor: clean up allocated data
    ~Sphere();

    //Constructor: create a sphere (approximated by polygon segments)
    Sphere(float radius, int segments);

    GLfloat* getVertexArray() const{
        return vertexarray;
    }

    int getNoOfVertices() const{
        return nverts;
    }

    //Used to render the geometry
    //mode : Specifies what kind of primitives to render
    void draw(GLenum mode);

private:

    GLuint vao;          // Vertex array object, the main handle for geometry
    int nverts; // Number of vertices in the vertex array
    int ntris;  // Number of triangles in the index array (may be zero)
    GLuint vertexbuffer; // Buffer ID to bind to GL_ARRAY_BUFFER
    GLuint indexbuffer;  // Buffer ID to bind to GL_ELEMENT_ARRAY_BUFFER
    GLfloat *vertexarray; // Vertex array on interleaved format: x y z nx ny nz s t
    GLuint *indexarray;   // Element index array
};
