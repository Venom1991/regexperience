#include "internal/semantic_analysis/ast_nodes/anchor.h"
#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _Anchor
{
  UnaryOperator parent_instance;
};

typedef struct
{
  AnchorType start_type;
  AnchorType end_type;
} AnchorPrivate;

enum
{
  PROP_START_TYPE = 1,
  PROP_END_TYPE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static FsmConvertible *anchor_build_acceptor (AstNode        *self,
                                              FsmConvertible *operand_acceptor);

static void            anchor_set_property   (GObject        *object,
                                              guint           property_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec);

G_DEFINE_TYPE_WITH_PRIVATE (Anchor, anchor, AST_NODES_TYPE_UNARY_OPERATOR)

static void
anchor_class_init (AnchorClass *klass)
{
  UnaryOperatorClass *unary_operator_class = AST_NODES_UNARY_OPERATOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  unary_operator_class->build_acceptor = anchor_build_acceptor;

  object_class->set_property = anchor_set_property;

  obj_properties[PROP_START_TYPE] =
    g_param_spec_uint (PROP_ANCHOR_START_TYPE,
                       "Start type",
                       "Start type of the anchor (anchored or unanchored to the beginning of the input).",
                       ANCHOR_TYPE_UNDEFINED,
                       ANCHOR_TYPE_UNANCHORED,
                       ANCHOR_TYPE_UNDEFINED,
                       G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  obj_properties[PROP_END_TYPE] =
    g_param_spec_uint (PROP_ANCHOR_END_TYPE,
                       "End type",
                       "End type of the anchor (anchored or unanchored to the ending of the input).",
                       ANCHOR_TYPE_UNDEFINED,
                       ANCHOR_TYPE_UNANCHORED,
                       ANCHOR_TYPE_UNDEFINED,
                       G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
anchor_init (Anchor *self)
{
  /* NOP */
}

static FsmConvertible *
anchor_build_acceptor (AstNode        *self,
                       FsmConvertible *operand_acceptor)
{
  g_return_val_if_fail (AST_NODES_IS_ANCHOR (self), NULL);
  g_return_val_if_fail (operand_acceptor != NULL, NULL);

  AnchorPrivate *priv = anchor_get_instance_private (AST_NODES_ANCHOR (self));
  AnchorType start_type = priv->start_type;
  AnchorType end_type = priv->end_type;
  gboolean should_convert_inner_anchors = FALSE;

  g_autoptr (GPtrArray) all_states = NULL;
  g_autoptr (State) start = NULL;
  g_autoptr (State) final = NULL;
  g_autoptr (GPtrArray) final_transitions = NULL;

  g_autoptr (GPtrArray) anchor_all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *start_anchor_start = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START);
  State *end_anchor_final = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_FINAL);

  g_object_get (operand_acceptor,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &all_states,
                PROP_FSM_INITIALIZABLE_START_STATE, &start,
                PROP_EPSILON_NFA_FINAL_STATE, &final,
                NULL);
  g_object_get (final,
                PROP_STATE_TRANSITIONS, &final_transitions,
                NULL);

  g_autoptr (GPtrArray) start_anchor_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  g_autoptr (GPtrArray) end_anchor_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  Transition *start_anchor_start_transition = NULL;
  Transition *end_anchor_final_transition = NULL;

  switch (start_type)
    {
    case ANCHOR_TYPE_ANCHORED:
      g_object_set (start_anchor_start,
                    PROP_STATE_IS_START_ANCHOR, TRUE,
                    NULL);

      should_convert_inner_anchors = TRUE;
      start_anchor_start_transition = create_deterministic_transition (START, start);
      break;
    case ANCHOR_TYPE_UNANCHORED:
      start_anchor_start_transition = create_deterministic_transition (EPSILON, start);
      break;
    default:
      g_return_val_if_reached (NULL);
    }

  g_ptr_array_add (start_anchor_transitions, start_anchor_start_transition);

  switch (end_type)
    {
    case ANCHOR_TYPE_ANCHORED:
      g_object_set (final,
                    PROP_STATE_IS_END_ANCHOR, TRUE,
                    NULL);

      should_convert_inner_anchors = TRUE;
      end_anchor_final_transition = create_deterministic_transition (END, end_anchor_final);
      break;
    case ANCHOR_TYPE_UNANCHORED:
      end_anchor_final_transition = create_deterministic_transition (EPSILON, end_anchor_final);
      break;
    default:
      g_return_val_if_reached (NULL);
    }

  /* Converting previously initialized START and END transitions (if they exists)
   * to epsilon ones as they were made redundant by the newly initialized ones.
   */
  if (should_convert_inner_anchors)
    {
      for (guint i = 0; i < all_states->len; ++i)
        {
          State *state = g_ptr_array_index (all_states, i);
          gboolean state_is_start_anchor = FALSE;
          gboolean state_is_end_anchor = FALSE;

          if (start_type == ANCHOR_TYPE_ANCHORED)
            g_object_get (state,
                          PROP_STATE_IS_START_ANCHOR, &state_is_start_anchor,
                          NULL);

          if (end_type == ANCHOR_TYPE_ANCHORED)
            g_object_get (state,
                          PROP_STATE_IS_END_ANCHOR, &state_is_end_anchor,
                          NULL);

          if (state_is_start_anchor || state_is_end_anchor)
            {
              g_autoptr (GPtrArray) transitions = NULL;

              g_object_get (state,
                            PROP_STATE_TRANSITIONS, &transitions,
                            NULL);

              if (g_collection_has_items (transitions))
                {
                  for (guint j = 0; j < transitions->len; ++j)
                    {
                      Transition *transition = g_ptr_array_index (transitions, j);

                      transition_convert_to_epsilon (transition);
                    }
                }
            }
        }
    }

  g_ptr_array_add (end_anchor_transitions, end_anchor_final_transition);

  if (g_collection_has_items (final_transitions))
    g_ptr_array_add_range (end_anchor_transitions,
                           final_transitions,
                           g_object_ref);

  g_object_set (start_anchor_start,
                PROP_STATE_TRANSITIONS, start_anchor_transitions,
                NULL);
  g_object_set (start,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                NULL);
  g_object_set (final,
                PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                PROP_STATE_TRANSITIONS, end_anchor_transitions,
                NULL);

  g_ptr_array_add_multiple (anchor_all_states,
                            start_anchor_start, end_anchor_final,
                            NULL);
  g_ptr_array_add_range (anchor_all_states,
                         all_states,
                         g_object_ref);

  return epsilon_nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, anchor_all_states);
}

static void
anchor_set_property (GObject      *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
  AnchorPrivate *priv = anchor_get_instance_private (AST_NODES_ANCHOR (object));

  switch (property_id)
    {
    case PROP_START_TYPE:
      priv->start_type = (AnchorType) g_value_get_uint (value);
      break;

    case PROP_END_TYPE:
      priv->end_type = (AnchorType) g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
