cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(regexperience_core VERSION 1.1 LANGUAGES C)

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
target_link_libraries(${PROJECT_NAME} ${GLIB_GOBJECT_LIBRARIES} ${GLIB_GOBJECT_LDFLAGS})

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include/core CACHE INTERNAL "${PROJECT_NAME}: include directory")
