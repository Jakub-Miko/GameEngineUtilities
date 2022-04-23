add_library(Lua_Jit INTERFACE)

if(UNIX AND NOT APPLE)
        set(LINUX TRUE)
endif()


if(WIN32)

find_library(Lua_Jit_library luajit.lib HINT ${CMAKE_CURRENT_SOURCE_DIR}/lua_jit/src)
find_library(Lua_library_s lua51.lib HINT ${CMAKE_CURRENT_SOURCE_DIR}/lua_jit/src)

message(${Lua_Jit_library})
message(${Lua_library_s})


if(NOT Lua_Jit_library AND NOT Lua_library_s AND NOT Lua_library_d)
    execute_process(COMMAND "build.bat" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE Res OUTPUT_VARIABLE OUT_STD)
    message("Building")
    message(${Lua_Jit_library})
    message(${Lua_library_s})

    find_library(Lua_Jit_library luajit.lib HINT ${CMAKE_CURRENT_SOURCE_DIR}/lua_jit/src)
    find_library(Lua_library_s lua51.lib HINT ${CMAKE_CURRENT_SOURCE_DIR}/lua_jit/src)

    target_link_libraries(Lua_Jit INTERFACE ${Lua_Jit_library} ${Lua_library_s}  )
    target_include_directories(Lua_Jit INTERFACE ./lua_jit/src/)

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



endif()


