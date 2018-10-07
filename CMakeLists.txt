cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(regexperience VERSION 1.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(SOURCE_FILES
    include/core/errors.h
    include/core/regexperience.h
    include/internal/common/helpers.h
    include/internal/lexical_analysis/lexeme.h
    include/internal/lexical_analysis/lexer.h
    include/internal/lexical_analysis/token.h
    include/internal/semantic_analysis/analyzer.h
    include/internal/semantic_analysis/ast_node_factory.h
    include/internal/semantic_analysis/ast_nodes/alternation.h
    include/internal/semantic_analysis/ast_nodes/ast_node.h
    include/internal/semantic_analysis/ast_nodes/binary_operator.h
    include/internal/semantic_analysis/ast_nodes/concatenation.h
    include/internal/semantic_analysis/ast_nodes/constant.h
    include/internal/semantic_analysis/ast_nodes/empty.h
    include/internal/semantic_analysis/ast_nodes/quantification.h
    include/internal/semantic_analysis/ast_nodes/range.h
    include/internal/semantic_analysis/ast_nodes/unary_operator.h
    include/internal/state_machines/acceptors/acceptor_runnable.h
    include/internal/state_machines/acceptors/dfa.h
    include/internal/state_machines/acceptors/epsilon_nfa.h
    include/internal/state_machines/acceptors/nfa.h
    include/internal/state_machines/composite_state.h
    include/internal/state_machines/fsm.h
    include/internal/state_machines/fsm_convertible.h
    include/internal/state_machines/fsm_initializable.h
    include/internal/state_machines/fsm_modifiable.h
    include/internal/state_machines/state.h
    include/internal/state_machines/state_factory.h
    include/internal/state_machines/transducers/mealy.h
    include/internal/state_machines/transducers/transducer_runnable.h
    include/internal/state_machines/transitions/deterministic_transition.h
    include/internal/state_machines/transitions/mealy_transition.h
    include/internal/state_machines/transitions/nondeterministic_transition.h
    include/internal/state_machines/transitions/transition.h
    include/internal/state_machines/transitions/transition_factory.h
    include/internal/syntactic_analysis/derivation_item.h
    include/internal/syntactic_analysis/grammar.h
    include/internal/syntactic_analysis/occurrence.h
    include/internal/syntactic_analysis/parser.h
    include/internal/syntactic_analysis/parsing_table_key.h
    include/internal/syntactic_analysis/production.h
    include/internal/syntactic_analysis/rule.h
    include/internal/syntactic_analysis/symbols/non_terminal.h
    include/internal/syntactic_analysis/symbols/symbol.h
    include/internal/syntactic_analysis/symbols/terminal.h
    src/common/helpers.c
    src/lexical_analysis/lexeme.c
    src/lexical_analysis/lexer.c
    src/lexical_analysis/token.c
    src/regexperience.c
    src/semantic_analysis/analyzer.c
    src/semantic_analysis/ast_node_factory.c
    src/semantic_analysis/ast_nodes/alternation.c
    src/semantic_analysis/ast_nodes/ast_node.c
    src/semantic_analysis/ast_nodes/binary_operator.c
    src/semantic_analysis/ast_nodes/concatenation.c
    src/semantic_analysis/ast_nodes/constant.c
    src/semantic_analysis/ast_nodes/empty.c
    src/semantic_analysis/ast_nodes/quantification.c
    src/semantic_analysis/ast_nodes/range.c
    src/semantic_analysis/ast_nodes/unary_operator.c
    src/state_machines/acceptors/acceptor_runnable.c
    src/state_machines/acceptors/dfa.c
    src/state_machines/acceptors/epsilon_nfa.c
    src/state_machines/acceptors/nfa.c
    src/state_machines/composite_state.c
    src/state_machines/fsm.c
    src/state_machines/fsm_convertible.c
    src/state_machines/fsm_initializable.c
    src/state_machines/fsm_modifiable.c
    src/state_machines/state.c
    src/state_machines/state_factory.c
    src/state_machines/transducers/mealy.c
    src/state_machines/transducers/transducer_runnable.c
    src/state_machines/transitions/deterministic_transition.c
    src/state_machines/transitions/mealy_transition.c
    src/state_machines/transitions/nondeterministic_transition.c
    src/state_machines/transitions/transition.c
    src/state_machines/transitions/transition_factory.c
    src/syntactic_analysis/derivation_item.c
    src/syntactic_analysis/grammar.c
    src/syntactic_analysis/occurrence.c
    src/syntactic_analysis/parser.c
    src/syntactic_analysis/parsing_table_key.c
    src/syntactic_analysis/production.c
    src/syntactic_analysis/rule.c
    src/syntactic_analysis/symbols/non_terminal.c
    src/syntactic_analysis/symbols/symbol.c
    src/syntactic_analysis/symbols/terminal.c)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PUBLIC include/core)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)

# GLib & GObject
pkg_check_modules(GLIB_GOBJECT glib-2.0>=2.54 gobject-2.0>=2.54)

if(GLIB_GOBJECT_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLIB_GOBJECT_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLIB_GOBJECT_LIBRARIES} ${GLIB_GOBJECT_LDFLAGS})

    add_definitions(${GLIB_GOBJECT_CFLAGS} ${GLIB_GOBJECT_CFLAGS_OTHER})

    message( STATUS "GLIB_GOBJECT_LIBRARIES: " ${GLIB_GOBJECT_LIBRARIES} )
    message( STATUS "GLIB_GOBJECT_LIBRARY_DIRS: " ${GLIB_GOBJECT_LIBRARY_DIRS} )
    message( STATUS "GLIB_GOBJECT_LDFLAGS: " ${GLIB_GOBJECT_LDFLAGS} )
    message( STATUS "GLIB_GOBJECT_LDFLAGS_OTHER: " ${GLIB_GOBJECT_LDFLAGS_OTHER} )
    message( STATUS "GLIB_GOBJECT_INCLUDE_DIRS: " ${GLIB_GOBJECT_INCLUDE_DIRS} )
    message( STATUS "GLIB_GOBJECT_CFLAGS: " ${GLIB_GOBJECT_CFLAGS} )
    message( STATUS "GLIB_GOBJECT_CFLAGS_OTHER: " ${GLIB_GOBJECT_CFLAGS_OTHER} )
else()
    message( FATAL_ERROR "GLib & GObject >= 2.54 are required, exiting." )
endif()

# compiler-dependent options
# setting the desired linker explicitly on Unix systems (GNU linker is used on other systems, by default)
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options (${PROJECT_NAME} PRIVATE -Wall -Wextra)

    if(UNIX)
        # linker (gold)
        target_link_libraries(${PROJECT_NAME} -fuse-ld=gold)
    endif()
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options (${PROJECT_NAME} PRIVATE -Weverything)

    if(UNIX)
        # linker (LLD)
        target_link_libraries(${PROJECT_NAME} -fuse-ld=lld)
    endif()
else()
    message( FATAL_ERROR "Only Clang and GCC are supported, exiting." )
endif()

# common options
target_compile_options (${PROJECT_NAME} PRIVATE -pedantic)

# compiler being used
message( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )

# build type
message( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )
