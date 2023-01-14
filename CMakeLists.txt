cmake_minimum_required(VERSION 3.6)
project(HairSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


## Set name of folders that will be used
set(PROJECT_BUILD_DIR ${PROJECT_SOURCE_DIR}/build)
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(PROJECT_SOURCES_DIR ${PROJECT_SOURCE_DIR}/src)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DGL_GLEXT_PROTOTYPES")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lGLEW -lGL -lX11 -lpthread -lXrandr -lXi")


### OPENGL
find_package(OpenGL REQUIRED)
set(ALL_LIBRARIES ${ALL_LIBRARIES} ${OPENGL_LIBRARIES})
set(ALL_LIBRARIES ${ALL_LIBRARIES} ${OPENGL_glu_LIBRARY})

### GLEW
find_package(GLEW REQUIRED)
set(GLEW_INCLUDE_DIR ${PROJECT_INCLUDE_DIR}/GL)
set(LIB_INCLUDE_DIRS ${LIB_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR})
set(ALL_LIBRARIES ${ALL_LIBRARIES} ${GLEW_LIBRARIES})

### GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${PROJECT_LIB_DIR}/glfw-3.2.1/)
set(ALL_LIBRARIES ${ALL_LIBRARIES} glfw)

### GLM
set(LIB_INCLUDE_DIRS ${LIB_INCLUDE_DIRS} ${PROJECT_LIB_DIR}/glm)


# Lump all LIB- and project includes into one variable
set(ALL_INCLUDES ${LIB_INCLUDE_DIRS} ${PROJECT_INCLUDE_DIR} ${CMAKE_MODULE_PATH})

# Set the include directories so that #include statements work
include_directories(${ALL_INCLUDES})
message( "Include all dirs: ${ALL_INCLUDES}")

# Get all source files by traversing the source directory recursively
file(GLOB_RECURSE PROJECT_CPP_FILES ${PROJECT_SOURCES_DIR}/*.cpp)

# Adds executable files
set(SOURCE_FILES main.cpp ${PROJECT_CPP_FILES} include/shader_c.h include/shader_t.h  include/Camera.h include/Sphere.h src/Sphere.cpp include/LoadTGA.h src/LoadTGA.c)
add_executable(HairSimulation ${SOURCE_FILES})

# Links libraries
target_link_libraries(HairSimulation ${ALL_LIBRARIES})
message("Include  all libraries: ${ALL_LIBRARIES}")