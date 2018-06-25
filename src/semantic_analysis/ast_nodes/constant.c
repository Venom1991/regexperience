#include "internal/semantic_analysis/ast_nodes/constant.h"
#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _Constant
{
    AstNode parent_instance;
};

typedef struct
{
    gchar value;
    guint position;
} ConstantPrivate;

enum
{
    PROP_VALUE = 1,
    PROP_POSITION = 2,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static FsmConvertible *constant_build_acceptor (AstNode      *self);

static void            constant_get_property   (GObject      *object,
                                                guint         property_id,
                                                GValue       *value,
                                                GParamSpec   *pspec);

static void            constant_set_property   (GObject      *object,
                                                guint         property_id,
                                                const GValue *value,
                                                GParamSpec   *pspec);

G_DEFINE_TYPE_WITH_PRIVATE (Constant, constant, AST_NODES_TYPE_AST_NODE)

static void
constant_class_init (ConstantClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  ast_node_class->build_acceptor = constant_build_acceptor;

  object_class->get_property = constant_get_property;
  object_class->set_property = constant_set_property;

  obj_properties[PROP_VALUE] =
    g_param_spec_char (PROP_CONSTANT_VALUE,
                       "Value",
                       "Value of the constant which can be either a letter, a digit, a special character, etc.",
                       0,
                       G_MAXINT8,
                       0,
                       G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_POSITION] =
    g_param_spec_uint (PROP_CONSTANT_POSITION,
                       "Position",
                       "Position of the constant relative to the beginning of the input.",
                       0,
                       G_MAXUINT32,
                       0,
                       G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
constant_init (Constant *self)
{
  /* NOP */
}

static FsmConvertible *
constant_build_acceptor (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_CONSTANT (self), NULL);

  ConstantPrivate *priv = constant_get_instance_private (AST_NODES_CONSTANT (self));
  gchar expected_character = priv->value;
  g_autoptr (GPtrArray) all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *start = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START);
  State *final = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_FINAL);
  g_autoptr (GPtrArray) start_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  Transition *start_on_value = create_deterministic_transition (expected_character, final);

  g_ptr_array_add (start_transitions, start_on_value);
  g_object_set (start,
                PROP_STATE_TRANSITIONS, start_transitions,
                NULL);

  g_ptr_array_add_multiple (all_states,
                            start, final,
                            NULL);

  return epsilon_nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, all_states);
}

static void
constant_get_property (GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  ConstantPrivate *priv = constant_get_instance_private (AST_NODES_CONSTANT (object));

  switch (property_id)
    {
    case PROP_VALUE:
      g_value_set_schar (value, priv->value);
      break;

    case PROP_POSITION:
      g_value_set_uint (value, priv->position);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
constant_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  ConstantPrivate *priv = constant_get_instance_private (AST_NODES_CONSTANT (object));

  switch (property_id)
    {
    case PROP_VALUE:
      priv->value = g_value_get_schar (value);
      break;

    case PROP_POSITION:
      priv->position = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
