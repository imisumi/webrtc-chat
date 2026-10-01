cmake_minimum_required(VERSION 3.18)
project(MyApp CXX)

# Clear default flags for all compilers to ensure our settings take precedence.
if(MSVC)
    set(CMAKE_CXX_FLAGS_INIT "")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "/Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "")
else()
    # Also clear the flags for GCC/Clang.
    set(CMAKE_CXX_FLAGS_INIT "")
    # CMake's default adds '-g'. We add it back here to ensure debug symbols.
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
    # Release flags like -O3 and -DNDEBUG are set later, so we clear this.
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "")
endif()

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(glm CONFIG REQUIRED)
#find_package(Stb REQUIRED)
#find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(LibDataChannel CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB SOURCES "src/*.cpp" "main.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

if(MSVC)
    message(STATUS "Configuring for MSVC compiler")
    # wd9025 disables override warning for compiler arguments
    set(COMMON_COMPILER_FLAGS /W4 /external:W3 /permissive- /GS /MP  /wd9025)
    set(DEBUG_COMPILER_FLAGS /MDd)
    set(RELEASE_COMPILER_FLAGS /MD /O2 /Oi /Gy /arch:AVX2)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Configuring for GCC compiler")
    set(COMMON_COMPILER_FLAGS -Wall -Wextra -Wpedantic -fstack-protector-strong)
    set(DEBUG_COMPILER_FLAGS "") # '-g' is now handled by CMAKE_CXX_FLAGS_DEBUG_INIT
    set(RELEASE_COMPILER_FLAGS -O2 -march=native)

else()
    message(WARNING "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}. No specific flags will be set.")
    set(COMMON_COMPILER_FLAGS "")
    set(DEBUG_COMPILER_FLAGS "")
    set(RELEASE_COMPILER_FLAGS "")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "inc")

target_compile_options(${PROJECT_NAME} PRIVATE
        ${COMMON_COMPILER_FLAGS}
        $<$<CONFIG:Debug>:${DEBUG_COMPILER_FLAGS}>
        $<$<CONFIG:Release>:${RELEASE_COMPILER_FLAGS}>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

target_link_libraries(${PROJECT_NAME} PRIVATE
        nlohmann_json::nlohmann_json
        LibDataChannel::LibDataChannel
        imgui::imgui
        glfw
        OpenGL::GL
)
