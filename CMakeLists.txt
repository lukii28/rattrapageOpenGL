cmake_minimum_required(VERSION 3.0.0)
project(rattrapage_opengl VERSION 0.1.0)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(rattrapage_opengl main.cpp 
)

target_include_directories(rattrapage_opengl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${GLUT_INCLUDE_DIRS}
    )

    TARGET_LINK_LIBRARIES(${PROJECT_NAME} GLEW)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} glfw)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} GL)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} IL)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} assimp)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GLUT_LIBRARY})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} GLU)
    
    