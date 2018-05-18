
# depending only on LOCAL_BASE allows CMAKE_MODULE_PATH use
# of this FindGLM.cmake without any special environment, other
# than LOCAL_BASE

#set(GLM_PREFIX "${CMAKE_INSTALL_PREFIX}/externals/glm")

# this is needed by odcs- external so gave to 
# used more general prefixing that works both within
# and without of the Opticks CMake

#set(GLM_LIBRARIES "")
#set(GLM_INCLUDE_DIRS "${GLM_PREFIX}/glm")
#set(GLM_DEFINITIONS "")


#unset(GLM_INCLUDE_DIR CACHE)
find_path(
    GLM_INCLUDE_DIR
    NAMES "glm/glm.hpp"
    PATHS "${CMAKE_INSTALL_PREFIX}/externals/glm/glm"
)




if(GLM_INCLUDE_DIR)
set(GLM_FOUND "YES")
else()
set(GLM_FOUND "NO")
endif()



if(GLM_DEBUG)
message(STATUS "CMAKE_INSTALL_PREFIX : ${CMAKE_INSTALL_PREFIX} ")
message(STATUS "GLM_INCLUDE_DIR      : ${GLM_INCLUDE_DIR} ")
message(STATUS "GLM_FOUND            : ${GLM_FOUND}")
endif()

if(GLM_FOUND AND NOT TARGET Opticks::GLM)
    add_library(Opticks::GLM INTERFACE IMPORTED)
    set_target_properties(Opticks::GLM PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIR}"
    )
endif()



