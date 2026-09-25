file(GLOB SRC_FILES "src/*.cpp")
add_library(PRAY SHARED ${SRC_FILES})

find_program(DXC_EXECUTABLE NAMES dxc.exe PATHS "${CMAKE_CURRENT_SOURCE_DIR}/libs/DXC/bin/x64" REQUIRED)

file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl")
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated_shaders")
file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")

set(SHADER_BINARY_OUTPUTS)
foreach(shader_file IN LISTS SHADER_SOURCES)
    get_filename_component(shader_name "${shader_file}" NAME_WE)
    set(output_spv "${SHADER_OUTPUT_DIR}/${shader_name}.spv")

    if(shader_name STREQUAL "vertex")
        set(shader_profile "vs_6_0")
    elseif(shader_name STREQUAL "fragment")
        set(shader_profile "ps_6_0")
    else()
        set(shader_profile "cs_6_0")
    endif()

    add_custom_command(
        OUTPUT "${output_spv}"
        COMMAND powershell -NoLogo -NoProfile -Command "& '${DXC_EXECUTABLE}' -spirv -T ${shader_profile} -E main -Fo '${output_spv}' '${shader_file}'"
        DEPENDS "${shader_file}"
        VERBATIM
    )

    list(APPEND SHADER_BINARY_OUTPUTS "${output_spv}")
endforeach()

add_custom_target(PRAY_ShaderBinaries DEPENDS ${SHADER_BINARY_OUTPUTS})
add_dependencies(PRAY PRAY_ShaderBinaries)

target_compile_definitions(PRAY PRIVATE PRAY_EXPORTS)

target_include_directories(PRAY PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/ShaderCross/include
)

target_link_directories(PRAY PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL/lib)
target_link_directories(PRAY PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/ShaderCross/lib)

# target_include_directories(PRAY PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/DXC/include)
# target_link_directories(PRAY PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/DXC/lib)

target_link_libraries(PRAY PUBLIC SDL3)
target_link_libraries(PRAY PUBLIC SDL3_shadercross)

set_target_properties(PRAY PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)