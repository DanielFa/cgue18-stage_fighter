cmake_minimum_required(VERSION 3.9)

project("Stage Fighter")

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

set(BINARY "Stage_Figther")
set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(EXTERN "${CMAKE_SOURCE_DIR}/extern")

set(CMAKE_CXX_STANDARD 11)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

#
# Comiler Stuff:
#

#if (MSVC)
#	set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
#
#	foreach (flag CMAKE_C_FLAGS
#				  CMAKE_C_FLAGS_DEBUG
#				  CMAKE_C_FLAGS_RELEASE
#				  CMAKE_C_FLAGS_MINSIZEREL
#				  CMAKE_C_FLAGS_RELWITHDEBINFO)
#
#		if (${flag} MATCHES "/MD")
#			string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
#		endif()
#		if (${flag} MATCHES "/MDd")
#			string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
#		endif()
#
#	endforeach()
#endif()

# Use higher warning level than default:
if(MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") # -Wextra -pedantic
endif()

message("CXX Flags          : ${CMAKE_CXX_FLAGS}")
message("Release Build Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message("Debug Build Flags  : ${CMAKE_CXX_FLAGS_DEBUG}")

#
# Dependencies:
#

## GLFW ###
if(UNIX)
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
else()
	if(NOT EXISTS "${EXTERN}/glfw/CMakeLists.txt")
		find_package(glfw REQUIRED)
	endif()
endif()

if(NOT GLFW_FOUND)
	message(STATUS "Using GLFW in ${EXTERN}/glfw")
	
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

	add_subdirectory("${EXTERN}/glfw")
	
	set(GLFW_INCLUDE_DIRS "${EXTERN}/glfw/include")
	set(GLFW_STATIC_LIBRARIES "glfw")
endif()

## BULLET ###
find_package(Bullet CONFIGS "BulletConfig.cmake")
if(BULLET_FOUND)
	if(UNIX)
		include_directories("/usr/include/bullet")
	endif()
else()
	message(STATUS "Using Bullet in ${EXTERN}/bullet3")

	 list(APPEND BulletCmakeArgs
		-DCMAKE_INSTALL_PREFIX=${EXTERN}/bullet3
        -DBUILD_BULLET2_DEMOS=OFF 
        -DBUILD_CLSOCKET=OFF 
        -DBUILD_CPU_DEMOS=OFF 
        -DBUILD_ENET=OFF 
        -DBUILD_OPENGL3_DEMOS=OFF 
        -DBUILD_UNIT_TESTS=OFF 
        -DINCLUDE_INSTALL_DIR=include/ 
        -DLIB_DESTINATION=lib/
        -DUSE_GRAPHICAL_BENCHMARK=OFF 
        -DBULLET2_USE_OPEN_MP_MULTITHREADING=ON 
        -DINSTALL_LIBS=ON 
		-DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON
        -DCMAKE_BUILD_TYPE=RELEASE -G ${CMAKE_GENERATOR}
      )
	  
	  ExternalProject_Add(Bullet 
		PREFIX "${EXTERN}/bullet3"
		BUILD_IN_SOURCE 1
		SOURCE_DIR "${EXTERN}/bullet3"
		CMAKE_ARGS "${BulletCmakeArgs}"
		LOG_BUILD 1
	)

    # LinearMath;Bullet3Common;BulletInverseDynamics;BulletCollision;BulletDynamics;BulletSoftBody
	set (BULLET_LIBS_RAW
            BulletSoftBody
            BulletDynamics
            BulletCollision
            BulletInverseDynamics
            Bullet3Common
            LinearMath
	)
	
#	if(WIN32)
#		set (BULLET_LIBRARIES "")
#		foreach(x ${BULLET_LIBS_RAW})
#			list (APPEND BULLET_LIBRARIES debug ${x}_Debug optimized ${x})
#		endforeach()
#	else()
		set(BULLET_LIBRARIES ${BULLET_LIBS_RAW})
#	endif()

	set(BULLET_INCLUDE_DIRS "${EXTERN}/bullet3/include")
	link_directories("${EXTERN}/bullet3/lib")
endif()

## GLAD ##
set(GLAD_INCLUDE_DIR "${EXTERN}/glad/include")
add_library("glad" "${EXTERN}/glad/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_INCLUDE_DIR}")

## GLM ##
if(UNIX)
	# fails on win32 if extern/glm is present? 
	find_package(glm REQUIRED)
	include_directories(${GLM_INCLUDE_DIRS})
	link_directories(${GLM_LIBRARY_DIRS})
	add_definitions(${GLM_DEFINITIONS})
	set(GLM_FOUND ON)
endif()

if (GLM_FOUND)
	add_definitions(${GLM_DEFINITIONS})
else()
	message(STATUS "Using glm from ${EXTERN}/glm")
	
	list(APPEND GLMCmakeArgs
		-DCMAKE_INSTALL_PREFIX=${EXTERN}/glm
		-DGLM_TEST_ENABLE_SIMD_AVX=ON
        -DINSTALL_LIBS=ON 
		-DGLM_INSTALL_ENABLE=ON
		-DINCLUDE_INSTALL_DIR=include/
		-DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -G ${CMAKE_GENERATOR}
      )
	  
	  ExternalProject_Add(GLM 
		PREFIX "${EXTERN}/glm"
		BUILD_IN_SOURCE 1
		SOURCE_DIR "${EXTERN}/glm"
		CMAKE_ARGS "${GLMCmakeArgs}"
		LOG_BUILD 1
	)
	
	set(GLM_INCLUDE_DIRS "${EXTERN}/glm/include")
	set(GLM_LIBRARY_DIRS "")
endif()

include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})

## Kaguya ##
#if(UNIX)
#	find_package(PkgConfig REQUIRED)
#    pkg_search_module(LUA REQUIRED lua5.3 lua-5.3 lua53)
#    message(STATUS "LUA_FOUND: ${LUA_FOUND}")
#    set(LUA_FOUND ON)
#
#	if(LUA_FOUND)
#		message(STATUS "Lua was found by pkg-config: '${LUA_INCLUDE_DIRS}'")
#		if(LUA_INCLUDE_DIRS STREQUAL "")
#			message(STATUS "Assume that lua.h is in /usr/include")
#			set(LUA_INCLUDE_DIRS "/usr/include")
#		endif()
#	endif()
#endif()

#if(NOT LUA_FOUND)
	message(STATUS "Using lua in ${EXTERN}/lua")
	
	file(GLOB LUA_SOURCES "${EXTERN}/lua/*.c")
	
	set(LOCAL_LUA_DIRECTORY "${EXTERN}/lua")
	set(LUA_INCLUDE_DIRS "${LOCAL_LUA_DIRECTORY}")
	
	add_library(lualib ${LUA_SOURCES})
	set(LUA_LIBRARIES lualib)
	
	include_directories(${LOCAL_LUA_DIRECTORY})
#endif()

set(KAGUYA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(KAGUYA_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
set(KAGUYA_BUILD_EXAMPLE_GLFW_BIND OFF CACHE BOOL "" FORCE)
set(KAGUYA_SINGLE_HEADER_VERSION OFF CACHE BOOL "" FORCE)

add_subdirectory("${EXTERN}/kaguya")

set(KAGUYA_INCLUDE_DIRS "${EXTERN}/kaguya/include")

## Logger ##
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory("${EXTERN}/spdlog")
include_directories("${EXTERN}/spdlog/include")

## stb single headers ##
include_directories("${EXTERN}/stb")

## tinygltf-loader ##
include_directories(${EXTERN}/tinygltf)

## freetype2 ##
if (UNIX)
	find_package(PkgConfig REQUIRED)
	pkg_search_module(FREETYPE REQUIRED freetype2)
    #find_package("freetype2")
endif()

if (NOT FREETYPE_FOUND)
    message(STATUS "Freetype2 not found, using extern/freetype2 as dependency")
    add_subdirectory("${EXTERN}/freetype2")

    set(FREETYPE_INCLUDE_DIRS ${EXTERN}/freetype2/include)
	set(FREETYPE_LIBRARIES freetype)
endif()

## Nuklear ##
file(MAKE_DIRECTORY ${EXTERN}/nuklear/include)
file(COPY ${EXTERN}/nuklear/nuklear.h DESTINATION ${EXTERN}/nuklear/include)
include_directories(${EXTERN}/nuklear/include)

## irrKlang ##
include_directories(${EXTERN}/irrKlang/include)
if (MSVC)
	link_directories(${EXTERN}/irrKlang/lib/Winx64-visualStudio/)
	set(IRRKLANG_LIB irrKlang.lib)
elseif(UNIX)
	link_directories(${EXTERN}/irrKlang/bin/linux-gcc-64/)
	set(IRRKLANG_LIB IrrKlang)

	SET(CMAKE_INSTALL_RPATH "./")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-new-dtags,-rpath,'./'")
else()
	message(ERROR "IRRKLANG_LIB is not defined for your platform!")
endif()

#SET(CMAKE_VERBOSE_MAKEFILE ON)

#
# Link & Include Stuff into the binary
#

include_directories(
		${GLFW_INCLUDE_DIRS}
		${BULLET_INCLUDE_DIRS}
		${KAGUYA_INCLUDE_DIRS}
        PRIVATE ${GLAD_INCLUDE_DIR}
        ${FREETYPE_INCLUDE_DIRS}
	)

add_executable(
		"${BINARY}"
		src/main.cpp
		src/Window.cpp src/Window.h src/helper/Callback.h src/BulletUniverse.cpp src/BulletUniverse.h
		src/object3d/Object3D.cpp src/object3d/Object3D.h src/Shader.cpp src/Shader.h src/manager/ShaderManager.cpp
		src/manager/ShaderManager.h src/Texture.cpp src/Texture.h src/manager/TextureManager.cpp src/entity/Entity.h
		src/manager/TextureManager.h src/Camera.cpp src/Camera.h src/controller/CameraController.cpp src/controller/CameraController.h
		src/manager/ModelManager.cpp src/manager/ModelManager.h src/object3d/Model3DObject.cpp src/object3d/Model3DObject.h
		src/bullet/GLDebugDrawer.cpp src/bullet/GLDebugDrawer.h src/level/Level.cpp src/level/Level.h src/level/LuaClassWrapper.h
		src/manager/FontManager.cpp src/manager/FontManager.h src/widget/Label.cpp src/widget/Label.h src/widget/Widget.h src/helper/Font.cpp
		src/helper/Font.h src/widget/NuklearCtx.cpp src/widget/NuklearCtx.h src/widget/GameMenu.cpp src/widget/GameMenu.h
		src/widget/PlayerHud.cpp src/widget/PlayerHud.h src/entity/Player.cpp src/entity/Player.h
		src/widget/DebugTextHud.cpp src/widget/DebugTextHud.h src/object3d/BulletObject.cpp src/object3d/BulletObject.h
		src/object3d/Light.h src/Scene.cpp src/Scene.h src/widget/HelpMenu.cpp src/widget/HelpMenu.h src/helper/QuatUtils.h
		src/helper/ImageGenerator.cpp src/helper/ImageGenerator.h src/CubemapTexture.cpp src/CubemapTexture.h
		src/object3d/Skybox.cpp src/object3d/Skybox.h src/entity/ScriptedEntity.cpp src/entity/ScriptedEntity.h
        src/object3d/ParticleSystem.cpp src/object3d/ParticleSystem.h src/helper/CompilerMacros.h src/entity/InstancedProjectile.cpp
		src/entity/InstancedProjectile.h src/object3d/particlesystem/ScriptedParticleSystem.cpp src/object3d/particlesystem/ScriptedParticleSystem.h
		src/entity/ScriptedObject.cpp src/entity/ScriptedObject.h src/entity/AbstractScriptedObject.h
        src/manager/MenuManager.cpp src/manager/MenuManager.h src/LaunchScreen.cpp src/LaunchScreen.h src/widget/Image.cpp src/widget/Image.h src/widget/SettingsMenu.cpp src/widget/SettingsMenu.h src/widget/LevelSelectMenu.cpp src/widget/LevelSelectMenu.h src/GlobalGameState.cpp src/GlobalGameState.h src/widget/BackToMenuWindow.cpp src/widget/BackToMenuWindow.h src/object3d/Trigger.cpp src/object3d/Trigger.h src/widget/TextBoxWindow.cpp src/widget/TextBoxWindow.h src/manager/AudioManager.cpp src/manager/AudioManager.h)

set_target_properties("${BINARY}" PROPERTIES LINKER_LANGUAGE CXX)	
	
target_link_libraries(
		"${BINARY}"
		${GLFW_STATIC_LIBRARIES}
		${BULLET_LIBRARIES}
		${LUA_LIBRARIES}
		glad
        Threads::Threads
        ${FREETYPE_LIBRARIES}
		${IRRKLANG_LIB}
	)

if(NOT BULLET_FOUND)	
	add_dependencies(${BINARY} Bullet)	
endif()
if(NOT GLM_FOUND)
	add_dependencies(${BINARY} GLM)
endif()