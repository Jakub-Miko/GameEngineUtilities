add_library(Lua_Jit INTERFACE)

if(UNIX AND NOT APPLE)
        set(LINUX TRUE)
endif()


if(WIN32)

find_library(Lua_Jit_library luajit.lib PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lua_jit/src)
find_library(Lua_library_s lua51.lib PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lua_jit/src)

message(${Lua_Jit_library})
message(${Lua_library_s})


if(NOT Lua_Jit_library AND NOT Lua_library_s)

    message(FATAL_ERROR "Please run build.bat in Visual Studio Command Promt(Windows Search -> \"x64 Native Tools Command Prompt\" OR \"x86 Native Tools Command Prompt\" -> cd \" root directory \" -> \"build.bat\") in the project root directory before running cmake.")

else()
    message(${Lua_Jit_library})
    target_link_libraries(Lua_Jit INTERFACE ${Lua_Jit_library} ${Lua_library_s} )
    target_include_directories(Lua_Jit INTERFACE ./lua_jit/src/)

endif()

execute_process(        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        ${CMAKE_CURRENT_SOURCE_DIR}/lua_jit/src/lua51.dll      
        ${CMAKE_BINARY_DIR})                 


endif()




if(LINUX)

find_library(Lua_Jit_library NAMES libluajit-5.1.a libluajit.a)

message(${Lua_Jit_library})



if(NOT Lua_Jit_library)
    
    message(FATAL_ERROR "Please run \"sudo sh ./build.sh\" in the project root directory before running cmake.")

else()
    message(${Lua_Jit_library})
    target_link_libraries(Lua_Jit INTERFACE ${Lua_Jit_library} )
    
    find_path(Lua_include NAMES lua.h lauxlib.h PATH_SUFFIXES luajit-2.1)
    message(${Lua_include})
    target_include_directories(Lua_Jit INTERFACE ${Lua_include})
    target_link_libraries(Lua_Jit INTERFACE ${CMAKE_DL_LIBS} )

endif()

endif()


