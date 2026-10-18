cmake_minimum_required(VERSION 3.14)
project(DesktopAquarium CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(DesktopAquarium WIN32
    main.cpp
    Aquarium.h
    Aquarium.cpp
    Fish.h
    Fish.cpp
    Renderer.h
    Renderer.cpp
    ContextMenu.h
    ContextMenu.cpp
)

target_link_libraries(DesktopAquarium PRIVATE
    gdi32
    gdiplus
    user32
    shell32
)
