add_executable(Sandbox main.cpp)

target_link_libraries(Sandbox PRIVATE PRAY)

file(GLOB SHADER_SOURCES
    "${CMAKE_SOURCE_DIR}/Engine/shaders/*.hlsl"
    "${CMAKE_SOURCE_DIR}/Sandbox/*.hlsl"
)

# target_include_directories(PRAY PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/ShaderCross/include)
# target_link_libraries(Sandbox PRIVATE ShaderCross)



add_custom_command(
    TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:Sandbox>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SHADER_SOURCES}
    $<TARGET_FILE_DIR:Sandbox>
)

add_custom_command(
    TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/Engine/libs/SDL/bin/SDL3.dll"
    $<TARGET_FILE_DIR:Sandbox>
)

add_custom_command(
    TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/Engine/libs/ShaderCross/bin/SDL3_shadercross.dll"
    $<TARGET_FILE_DIR:Sandbox>
)

# add_custom_command(
#     TARGET Sandbox POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     "${CMAKE_SOURCE_DIR}/Engine/libs/ShaderCross/bin/dxcompiler.dll"
#     $<TARGET_FILE_DIR:Sandbox>
# )

add_custom_command(
    TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/Engine/libs/ShaderCross/bin/spirv-cross-c-shared.dll"
    $<TARGET_FILE_DIR:Sandbox>
)

add_custom_command(
    TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/Engine/libPRAY.dll"
    $<TARGET_FILE_DIR:Sandbox>
)