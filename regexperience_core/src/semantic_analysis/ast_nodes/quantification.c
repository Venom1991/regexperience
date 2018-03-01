#include "internal/semantic_analysis/ast_nodes/quantification.h"
#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _Quantification
{
    UnaryOperator parent_instance;
};

typedef struct
{
    QuantificationBoundType lower_bound;
    QuantificationBoundType upper_bound;
} QuantificationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Quantification, quantification, AST_NODES_TYPE_UNARY_OPERATOR)

enum
{
    PROP_LOWER_BOUND = 1,
    PROP_UPPER_BOUND,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static FsmConvertible *quantification_build_fsm (AstNode *self);

static void quantification_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);

static void
quantification_class_init (QuantificationClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  ast_node_class->build_fsm = quantification_build_fsm;

  object_class->set_property = quantification_set_property;

  obj_properties[PROP_LOWER_BOUND] =
      g_param_spec_uint (PROP_QUANTIFICATION_LOWER_BOUND,
                         "Lower bound",
                         "Lower bound of the quantification operator.",
                         QUANTIFICATION_BOUND_TYPE_UNDEFINED,
                         QUANTIFICATION_BOUND_TYPE_INFINITY,
                         QUANTIFICATION_BOUND_TYPE_UNDEFINED,
                         G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  obj_properties[PROP_UPPER_BOUND] =
      g_param_spec_uint (PROP_QUANTIFICATION_UPPER_BOUND,
                         "Upper bound",
                         "Upper of the quantification operator.",
                         QUANTIFICATION_BOUND_TYPE_UNDEFINED,
                         QUANTIFICATION_BOUND_TYPE_INFINITY,
                         QUANTIFICATION_BOUND_TYPE_UNDEFINED,
                         G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
quantification_init (Quantification *self)
{
  /* NOP */
}

static FsmConvertible *
quantification_build_fsm (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_QUANTIFICATION (self), NULL);

  QuantificationPrivate *priv = quantification_get_instance_private (AST_NODES_QUANTIFICATION (self));

  QuantificationBoundType lower_bound = priv->lower_bound;
  QuantificationBoundType upper_bound = priv->upper_bound;

  g_autoptr (AstNode) operand = NULL;

  g_object_get (self,
                PROP_UNARY_OPERATOR_OPERAND, &operand,
                NULL);

  g_autoptr (FsmConvertible) operand_fsm = ast_node_build_fsm (operand);

  g_autoptr (GPtrArray) all_states = NULL;
  g_autoptr (State) start = NULL;
  g_autoptr (State) final = NULL;

  g_autoptr (GPtrArray) quantification_all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *quantification_start = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START);
  State *quantification_final = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_FINAL);

  g_object_get (operand_fsm,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &all_states,
                PROP_FSM_INITIALIZABLE_START_STATE, &start,
                PROP_EPSILON_NFA_FINAL_STATE, &final,
                NULL);

  g_autoptr (GPtrArray) quantification_start_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  g_autoptr (GPtrArray) final_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  Transition *quantification_start_transition_on_epsilon = NULL;
  Transition *final_transition_on_epsilon = NULL;

  switch (lower_bound)
    {
    case QUANTIFICATION_BOUND_TYPE_ZERO:
      {
        g_autoptr (GPtrArray) quantification_start_output_states = g_ptr_array_new ();

        g_ptr_array_add_multiple (quantification_start_output_states,
                                  start, quantification_final,
                                  NULL);

        quantification_start_transition_on_epsilon = create_nondeterministic_epsilon_transition (quantification_start_output_states);
      }
      break;

    case QUANTIFICATION_BOUND_TYPE_ONE:
      quantification_start_transition_on_epsilon = create_deterministic_epsilon_transition (start);
      break;

    default:
      g_return_val_if_reached (NULL);
    }

  g_ptr_array_add (quantification_start_transitions, quantification_start_transition_on_epsilon);

  g_object_set (quantification_start,
                PROP_STATE_TRANSITIONS, quantification_start_transitions,
                NULL);

  switch (upper_bound)
    {
    case QUANTIFICATION_BOUND_TYPE_INFINITY:
      {
        g_autoptr (GPtrArray) final_output_states = g_ptr_array_new ();

        g_ptr_array_add_multiple (final_output_states,
                                  start, quantification_final,
                                  NULL);

        final_transition_on_epsilon = create_nondeterministic_epsilon_transition (final_output_states);
      }
      break;

    case QUANTIFICATION_BOUND_TYPE_ONE:
      final_transition_on_epsilon = create_deterministic_epsilon_transition (quantification_final);
      break;

    default:
      g_return_val_if_reached (NULL);
    }

  g_ptr_array_add (final_transitions, final_transition_on_epsilon);

  g_object_set (start,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                NULL);
  g_object_set (final,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                PROP_STATE_TRANSITIONS, final_transitions,
                NULL);

  g_ptr_array_add_multiple (quantification_all_states,
                            quantification_start, quantification_final,
                            NULL);
  g_ptr_array_add_range (quantification_all_states,
                         all_states,
                         g_object_ref);

  return epsilon_nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, quantification_all_states);
}

static void
quantification_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  QuantificationPrivate *priv = quantification_get_instance_private (AST_NODES_QUANTIFICATION (object));

  switch (property_id)
    {
    case PROP_LOWER_BOUND:
      priv->lower_bound = (QuantificationBoundType) g_value_get_uint (value);
      break;

    case PROP_UPPER_BOUND:
      priv->upper_bound = (QuantificationBoundType) g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
