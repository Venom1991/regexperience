cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(regexperience_core VERSION 0.1 LANGUAGES C)

set(SOURCE_FILES
        include/core/errors.h
        include/core/regexperience.h
        include/internal/common/helpers.h
        include/internal/common/macros.h
        include/internal/semantic_analysis/analyzer.h
        include/internal/semantic_analysis/ast_node_factory.h
        include/internal/semantic_analysis/ast_nodes/alternation.h
        include/internal/semantic_analysis/ast_nodes/ast_node.h
        include/internal/semantic_analysis/ast_nodes/binary_operator.h
        include/internal/semantic_analysis/ast_nodes/bracket_expression.h
        include/internal/semantic_analysis/ast_nodes/concatenation.h
        include/internal/semantic_analysis/ast_nodes/constant.h
        include/internal/semantic_analysis/ast_nodes/quantification.h
        include/internal/semantic_analysis/ast_nodes/range.h
        include/internal/semantic_analysis/ast_nodes/unary_operator.h
        include/internal/state_machines/acceptors/acceptor.h
        include/internal/state_machines/acceptors/dfa.h
        include/internal/state_machines/acceptors/epsilon_nfa.h
        include/internal/state_machines/acceptors/nfa.h
        include/internal/state_machines/composite_state.h
        include/internal/state_machines/state.h
        include/internal/state_machines/state_factory.h
        include/internal/state_machines/state_machine_convertible.h
        include/internal/state_machines/state_machine_initializable.h
        include/internal/state_machines/state_machine_modifiable.h
        include/internal/state_machines/state_machine_runnable.h
        include/internal/state_machines/transitions/deterministic_transition.h
        include/internal/state_machines/transitions/nondeterministic_transition.h
        include/internal/state_machines/transitions/transition.h
        include/internal/state_machines/transitions/transition_factory.h
        include/internal/syntactic_analysis/derivation_item.h
        include/internal/syntactic_analysis/grammar/grammar.h
        include/internal/syntactic_analysis/grammar/production.h
        include/internal/syntactic_analysis/grammar/rule.h
        include/internal/syntactic_analysis/grammar/symbols/non_terminal.h
        include/internal/syntactic_analysis/grammar/symbols/symbol.h
        include/internal/syntactic_analysis/grammar/symbols/terminal.h
        include/internal/syntactic_analysis/lexeme.h
        include/internal/syntactic_analysis/lexer.h
        include/internal/syntactic_analysis/parser.h
        include/internal/syntactic_analysis/token.h
        src/common/helpers.c
        src/regexperience.c
        src/semantic_analysis/analyzer.c
        src/semantic_analysis/ast_node_factory.c
        src/semantic_analysis/ast_nodes/alternation.c
        src/semantic_analysis/ast_nodes/ast_node.c
        src/semantic_analysis/ast_nodes/binary_operator.c
        src/semantic_analysis/ast_nodes/bracket_expression.c
        src/semantic_analysis/ast_nodes/concatenation.c
        src/semantic_analysis/ast_nodes/constant.c
        src/semantic_analysis/ast_nodes/quantification.c
        src/semantic_analysis/ast_nodes/range.c
        src/semantic_analysis/ast_nodes/unary_operator.c
        src/state_machines/acceptors/acceptor.c
        src/state_machines/acceptors/dfa.c
        src/state_machines/acceptors/epsilon_nfa.c
        src/state_machines/acceptors/nfa.c
        src/state_machines/composite_state.c
        src/state_machines/state.c
        src/state_machines/state_factory.c
        src/state_machines/state_machine_convertible.c
        src/state_machines/state_machine_initializable.c
        src/state_machines/state_machine_modifiable.c
        src/state_machines/state_machine_runnable.c
        src/state_machines/transitions/deterministic_transition.c
        src/state_machines/transitions/nondeterministic_transition.c
        src/state_machines/transitions/transition.c
        src/state_machines/transitions/transition_factory.c
        src/syntactic_analysis/derivation_item.c
        src/syntactic_analysis/grammar/grammar.c
        src/syntactic_analysis/grammar/production.c
        src/syntactic_analysis/grammar/rule.c
        src/syntactic_analysis/grammar/symbols/non_terminal.c
        src/syntactic_analysis/grammar/symbols/symbol.c
        src/syntactic_analysis/grammar/symbols/terminal.c
        src/syntactic_analysis/lexeme.c
        src/syntactic_analysis/lexer.c
        src/syntactic_analysis/parser.c
        src/syntactic_analysis/token.c)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} ${GLIB_GOBJECT_LIBRARIES} ${GLIB_GOBJECT_LDFLAGS})

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include/core CACHE INTERNAL "${PROJECT_NAME}: include directory")
