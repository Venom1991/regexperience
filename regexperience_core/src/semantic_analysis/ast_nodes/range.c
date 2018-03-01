#include "internal/semantic_analysis/ast_nodes/range.h"
#include "internal/semantic_analysis/ast_nodes/constant.h"
#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"
#include "core/errors.h"

struct _Range
{
    BinaryOperator parent_instance;
};

G_DEFINE_TYPE (Range, range, AST_NODES_TYPE_BINARY_OPERATOR)

G_DEFINE_QUARK (semantic-analysis-range-error-quark, semantic_analysis_range_error)
#define SEMANTIC_ANALYSIS_RANGE_ERROR (semantic_analysis_range_error_quark())

static FsmConvertible *range_build_fsm (AstNode *self);

static gboolean range_is_valid (AstNode  *self,
                                GError  **error);

static void
range_class_init (RangeClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);

  ast_node_class->build_fsm = range_build_fsm;
  ast_node_class->is_valid = range_is_valid;
}

static void
range_init (Range *self)
{
  /* NOP */
}

static FsmConvertible *
range_build_fsm (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_RANGE (self), NULL);

  g_autoptr (AstNode) left_operand = NULL;
  g_autoptr (AstNode) right_operand = NULL;

  g_object_get (self,
                PROP_BINARY_OPERATOR_LEFT_OPERAND, &left_operand,
                PROP_BINARY_OPERATOR_RIGHT_OPERAND, &right_operand,
                NULL);

  Constant *left_constant = AST_NODES_CONSTANT (left_operand);
  Constant *right_constant = AST_NODES_CONSTANT (right_operand);

  gchar lower_value = 0;
  gchar upper_value = 0;

  g_object_get (left_constant,
                PROP_CONSTANT_VALUE, &lower_value,
                NULL);
  g_object_get (right_constant,
                PROP_CONSTANT_VALUE, &upper_value,
                NULL);

  g_autoptr (GPtrArray) all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *start = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START);
  State *final = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_FINAL);
  g_autoptr (GPtrArray) start_transitions = g_ptr_array_new_with_free_func (g_object_unref);

  for (gchar expected_character = lower_value; expected_character <= upper_value; ++expected_character)
    {
      Transition *start_to_final_on_value = create_deterministic_transition (expected_character,
                                                                             final);

      g_ptr_array_add (start_transitions, start_to_final_on_value);
    }

  g_object_set (start,
                PROP_STATE_TRANSITIONS, start_transitions,
                NULL);

  g_ptr_array_add_multiple (all_states,
                            start, final,
                            NULL);

  return epsilon_nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, all_states);
}

static gboolean
range_is_valid (AstNode  *self,
                GError  **error)
{
  g_return_val_if_fail (AST_NODES_IS_RANGE (self), FALSE);

  g_autoptr (AstNode) left_operand = NULL;
  g_autoptr (AstNode) right_operand = NULL;

  g_object_get (self,
                PROP_BINARY_OPERATOR_LEFT_OPERAND, &left_operand,
                PROP_BINARY_OPERATOR_RIGHT_OPERAND, &right_operand,
                NULL);

  g_assert (AST_NODES_IS_CONSTANT (left_operand));
  g_assert (AST_NODES_IS_CONSTANT (right_operand));

  Constant *left_constant = AST_NODES_CONSTANT (left_operand);
  Constant *right_constant = AST_NODES_CONSTANT (right_operand);

  gchar left_value = 0;
  gchar right_value = 0;
  guint left_position = 0;
  guint right_position = 0;

  g_object_get (left_constant,
                PROP_CONSTANT_VALUE, &left_value,
                PROP_CONSTANT_POSITION, &left_position,
                NULL);
  g_object_get (right_constant,
                PROP_CONSTANT_VALUE, &right_value,
                PROP_CONSTANT_POSITION, &right_position,
                NULL);

  if (left_value >= right_value)
    {
      guint range_position = (left_position + right_position) / 2;

      g_set_error (error,
                   SEMANTIC_ANALYSIS_RANGE_ERROR,
                   SEMANTIC_ANALYSIS_RANGE_ERROR_INVALID_VALUES,
                   "Invalid range values (position - %d)", range_position);

      return FALSE;
    }

  return TRUE;
}
