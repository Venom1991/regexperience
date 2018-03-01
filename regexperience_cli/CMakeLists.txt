cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(regexperience_cli VERSION 1.0 LANGUAGES C)

set(SOURCE_FILES main.c)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${regexperience_core_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} regexperience_core ${GLIB_GOBJECT_LIBRARIES} ${GLIB_GOBJECT_LDFLAGS})
