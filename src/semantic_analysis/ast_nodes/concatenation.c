#include "internal/semantic_analysis/ast_nodes/concatenation.h"
#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _Concatenation
{
  BinaryOperator parent_instance;
};

static FsmConvertible *concatenation_build_acceptor (AstNode *self);

G_DEFINE_TYPE (Concatenation, concatenation, AST_NODES_TYPE_BINARY_OPERATOR)

static void
concatenation_class_init (ConcatenationClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);

  ast_node_class->build_acceptor = concatenation_build_acceptor;
}

static void
concatenation_init (Concatenation *self)
{
  /* NOP */
}

static FsmConvertible *
concatenation_build_acceptor (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_CONCATENATION (self), NULL);

  g_autoptr (AstNode) left_operand = NULL;
  g_autoptr (AstNode) right_operand = NULL;

  g_object_get (self,
                PROP_BINARY_OPERATOR_LEFT_OPERAND, &left_operand,
                PROP_BINARY_OPERATOR_RIGHT_OPERAND, &right_operand,
                NULL);

  g_autoptr (FsmConvertible) left_operand_acceptor = ast_node_build_acceptor (left_operand);
  g_autoptr (FsmConvertible) right_operand_acceptor = ast_node_build_acceptor (right_operand);

  g_autoptr (GPtrArray) left_all_states = NULL;
  g_autoptr (GPtrArray) right_all_states = NULL;
  g_autoptr (State) left_final = NULL;
  g_autoptr (State) right_start = NULL;
  g_autoptr (GPtrArray) concatenation_all_states = g_ptr_array_new_with_free_func (g_object_unref);

  g_object_get (left_operand_acceptor,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &left_all_states,
                PROP_EPSILON_NFA_FINAL_STATE, &left_final,
                NULL);
  g_object_get (right_operand_acceptor,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &right_all_states,
                PROP_FSM_INITIALIZABLE_START_STATE, &right_start,
                NULL);

  g_autoptr (GPtrArray) left_final_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  Transition *left_on_epsilon = create_deterministic_transition (EPSILON, right_start);

  g_ptr_array_add (left_final_transitions, left_on_epsilon);

  g_object_set (left_final,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                PROP_STATE_TRANSITIONS, left_final_transitions,
                NULL);
  g_object_set (right_start,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                NULL);

  g_ptr_array_add_range (concatenation_all_states,
                         left_all_states,
                         g_object_ref);
  g_ptr_array_add_range (concatenation_all_states,
                         right_all_states,
                         g_object_ref);

  return epsilon_nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, concatenation_all_states);
}
