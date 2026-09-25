cmake_minimum_required(VERSION 4.0.0)
project(PRAY VERSION 0.1.0)

set(SANDBOX_NAME Sandbox)
set(ENGINE_NAME Engine)

add_subdirectory(Engine)
add_subdirectory(Sandbox)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY Sandbox)