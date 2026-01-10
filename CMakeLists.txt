cmake_minimum_required(VERSION 3.16)
project(RacingGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Setup SFML
# We will try to find it first, if not found, we fetch it (optional, but finding is standard)
# For this "Configured Project" deliverable, FetchContent is safer ensuring it works out of the box
# if the user has internet access and no local SFML.
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

# Define sources
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Create Executable
add_executable(RacingGame ${SOURCES})

# Link SFML
target_link_libraries(RacingGame PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)

# Copy assets to build directory (Post_build)
add_custom_command(
    TARGET RacingGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:RacingGame>/assets
    COMMENT "Copying assets to output directory..."
)

add_executable(SoundGen sound_gen.cpp)
