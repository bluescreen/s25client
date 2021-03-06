find_package(SDL 1.2 REQUIRED)

include(CMakeMacroCorrectLib)	
correct_lib(SDL_LIBRARY SDL)

add_library(videoMockup STATIC MockupVideoDriver.h MockupVideoDriver.cpp)
target_link_libraries(videoMockup PUBLIC videodrv s25util ${SDL_LIBRARY})
target_include_directories(videoMockup INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(videoMockup SYSTEM PUBLIC ${SDL_INCLUDE_DIR})

add_library(audioMockup STATIC MockupAudioDriver.h MockupAudioDriver.cpp)
target_link_libraries(audioMockup PUBLIC audiodrv)
target_include_directories(audioMockup INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(WIN32)
	include(GatherDll)
	gather_dll_by_name(SDL SDL.dll)
endif()

if(ClangFormat_FOUND)
    add_clangFormat_files(MockupVideoDriver.h MockupVideoDriver.cpp MockupAudioDriver.h MockupAudioDriver.cpp)
endif()
