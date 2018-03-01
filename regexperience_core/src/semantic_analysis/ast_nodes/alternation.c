#include "internal/semantic_analysis/ast_nodes/alternation.h"
#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _Alternation
{
    BinaryOperator parent_instance;
};

G_DEFINE_TYPE (Alternation, alternation, AST_NODES_TYPE_BINARY_OPERATOR)

static FsmConvertible *alternation_build_fsm (AstNode *self);

static void
alternation_class_init (AlternationClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);

  ast_node_class->build_fsm = alternation_build_fsm;
}

static void
alternation_init (Alternation *self)
{
  /* NOP */
}

static FsmConvertible *
alternation_build_fsm (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_ALTERNATION (self), NULL);

  g_autoptr (AstNode) left_operand = NULL;
  g_autoptr (AstNode) right_operand = NULL;

  g_object_get (self,
                PROP_BINARY_OPERATOR_LEFT_OPERAND, &left_operand,
                PROP_BINARY_OPERATOR_RIGHT_OPERAND, &right_operand,
                NULL);

  g_autoptr (FsmConvertible) left_operand_fsm = ast_node_build_fsm (left_operand);
  g_autoptr (FsmConvertible) right_operand_fsm = ast_node_build_fsm (right_operand);

  g_autoptr (GPtrArray) left_all_states = NULL;
  g_autoptr (GPtrArray) right_all_states = NULL;
  g_autoptr (State) left_start = NULL;
  g_autoptr (State) left_final = NULL;
  g_autoptr (State) right_start = NULL;
  g_autoptr (State) right_final = NULL;
  g_autoptr (GPtrArray) alternation_all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *alternation_start = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START);
  State *alternation_final = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_FINAL);

  g_object_get (left_operand_fsm,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &left_all_states,
                PROP_FSM_INITIALIZABLE_START_STATE, &left_start,
                PROP_EPSILON_NFA_FINAL_STATE, &left_final,
                NULL);

  g_object_get (right_operand_fsm,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &right_all_states,
                PROP_FSM_INITIALIZABLE_START_STATE, &right_start,
                PROP_EPSILON_NFA_FINAL_STATE, &right_final,
                NULL);

  g_autoptr (GPtrArray) alternation_start_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  g_autoptr (GPtrArray) alternation_start_to_left_and_right_start_output_states = g_ptr_array_new ();

  g_ptr_array_add_multiple (alternation_start_to_left_and_right_start_output_states,
                            left_start, right_start,
                            NULL);

  Transition *alternation_start_to_left_and_right_start_on_epsilon =
      create_nondeterministic_epsilon_transition (alternation_start_to_left_and_right_start_output_states);

  g_ptr_array_add (alternation_start_transitions, alternation_start_to_left_and_right_start_on_epsilon);

  g_object_set (alternation_start,
                PROP_STATE_TRANSITIONS, alternation_start_transitions,
                NULL);

  g_autoptr (GPtrArray) left_final_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  g_autoptr (GPtrArray) right_final_transitions = g_ptr_array_new_with_free_func (g_object_unref);

  Transition *left_final_to_alternation_final_on_epsilon = create_deterministic_epsilon_transition (alternation_final);
  Transition *right_final_to_alternation_final_on_epsilon = create_deterministic_epsilon_transition (alternation_final);

  g_ptr_array_add (left_final_transitions, left_final_to_alternation_final_on_epsilon);
  g_ptr_array_add (right_final_transitions, right_final_to_alternation_final_on_epsilon);

  g_object_set (left_final,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                PROP_STATE_TRANSITIONS, left_final_transitions,
                NULL);
  g_object_set (right_final,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                PROP_STATE_TRANSITIONS, right_final_transitions,
                NULL);

  g_object_set (left_start,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                NULL);
  g_object_set (right_start,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                NULL);

  g_ptr_array_add_multiple (alternation_all_states,
                            alternation_start, alternation_final,
                            NULL);
  g_ptr_array_add_range (alternation_all_states,
                         left_all_states,
                         g_object_ref);
  g_ptr_array_add_range (alternation_all_states,
                         right_all_states,
                         g_object_ref);

  return epsilon_nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, alternation_all_states);
}
