#include "internal/state_machines/state.h"
#include "internal/state_machines/transitions/transition.h"
#include "internal/common/helpers.h"

typedef struct
{
  StateTypeFlags  state_type_flags;
  GPtrArray      *transitions;
  gboolean        is_start_anchor;
  gboolean        is_end_anchor;
  gboolean        is_dead;
} StatePrivate;

enum
{
  PROP_TYPE_FLAGS = 1,
  PROP_TRANSITIONS,
  PROP_IS_START_ANCHOR,
  PROP_IS_END_ANCHOR,
  PROP_IS_DEAD,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static gboolean state_default_is_composed_from (State           *self,
                                                const GPtrArray *states);

static void     state_get_property             (GObject         *object,
                                                guint            property_id,
                                                GValue          *value,
                                                GParamSpec      *pspec);

static void     state_set_property             (GObject         *object,
                                                guint            property_id,
                                                const GValue    *value,
                                                GParamSpec      *pspec);

static void     state_dispose                  (GObject         *object);

G_DEFINE_TYPE_WITH_PRIVATE (State, state, G_TYPE_OBJECT)

static void
state_class_init (StateClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  klass->is_composed_from = state_default_is_composed_from;

  object_class->get_property = state_get_property;
  object_class->set_property = state_set_property;
  object_class->dispose = state_dispose;

  obj_properties[PROP_TYPE_FLAGS] =
    g_param_spec_uint (PROP_STATE_TYPE_FLAGS,
                       "Type flags",
                       "Flags describing the purpose of the state as part of a state machine as a whole.",
                       0,
                       G_MAXUINT32,
                       STATE_TYPE_UNDEFINED,
                       G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_TRANSITIONS] =
    g_param_spec_boxed (PROP_STATE_TRANSITIONS,
                        "Transitions",
                        "Transitions that are possible once a state machine reaches the state.",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_IS_START_ANCHOR] =
    g_param_spec_boolean (PROP_STATE_IS_START_ANCHOR,
                          "Is start anchor",
                          "Describes whether or not the state is intended to represent the start anchor.",
                          FALSE,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_IS_END_ANCHOR] =
    g_param_spec_boolean (PROP_STATE_IS_END_ANCHOR,
                          "Is end anchor",
                          "Describes whether or not the state is intended to represent the end anchor.",
                          FALSE,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_IS_DEAD] =
    g_param_spec_boolean (PROP_STATE_IS_DEAD,
                          "Is dead",
                          "Describes whether or not the state has a single transition to itself on any input.",
                          FALSE,
                          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
state_init (State *self)
{
  /* NOP */
}

gboolean
state_is_composed_from (State           *self,
                        const GPtrArray *states)
{
  StateClass *klass;

  g_return_val_if_fail (STATE_MACHINES_IS_STATE (self), FALSE);

  klass = STATE_MACHINES_STATE_GET_CLASS (self);

  g_assert (klass->is_composed_from != NULL);

  return klass->is_composed_from (self, states);
}

gint
state_compare_deadness (State *a,
                        State *b)
{
  State **a_ptr = (State **) a;
  State **b_ptr = (State **) b;

  g_return_val_if_fail (STATE_MACHINES_IS_STATE (*a_ptr), 0);
  g_return_val_if_fail (STATE_MACHINES_IS_STATE (*b_ptr), 0);

  StatePrivate *a_priv = state_get_instance_private (STATE_MACHINES_STATE (*a_ptr));
  StatePrivate *b_priv = state_get_instance_private (STATE_MACHINES_STATE (*b_ptr));

  gboolean a_is_dead = a_priv->is_dead;
  gboolean b_is_dead = b_priv->is_dead;

  return -((a_is_dead > b_is_dead) - (a_is_dead < b_is_dead));
}

static gboolean
state_default_is_composed_from (State           *self,
                                const GPtrArray *states)
{
  return FALSE;
}

static void
state_get_property (GObject    *object,
                    guint       property_id,
                    GValue     *value,
                    GParamSpec *pspec)
{
  StatePrivate *priv = state_get_instance_private (STATE_MACHINES_STATE (object));

  switch (property_id)
    {
    case PROP_TYPE_FLAGS:
      g_value_set_uint (value, priv->state_type_flags);
      break;

    case PROP_TRANSITIONS:
      g_value_set_boxed (value, priv->transitions);
      break;

    case PROP_IS_START_ANCHOR:
      g_value_set_boolean (value, priv->is_start_anchor);
      break;

    case PROP_IS_END_ANCHOR:
      g_value_set_boolean (value, priv->is_end_anchor);
      break;

    case PROP_IS_DEAD:
      g_value_set_boolean (value, priv->is_dead);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
state_set_property (GObject      *object,
                    guint         property_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
  StatePrivate *priv = state_get_instance_private (STATE_MACHINES_STATE (object));

  switch (property_id)
    {
    case PROP_TYPE_FLAGS:
      priv->state_type_flags = (StateTypeFlags) g_value_get_uint (value);
      break;

    case PROP_TRANSITIONS:
      {
        if (priv->transitions != NULL)
          g_ptr_array_unref (priv->transitions);

        GPtrArray *transitions = g_value_dup_boxed (value);

        if (g_collection_has_items (transitions))
          g_ptr_array_sort (transitions, (GCompareFunc) transition_compare_equality_condition_type);

        priv->transitions = transitions;
      }
      break;

    case PROP_IS_START_ANCHOR:
      priv->is_start_anchor = g_value_get_boolean (value);
      break;

    case PROP_IS_END_ANCHOR:
      priv->is_end_anchor = g_value_get_boolean (value);
      break;

    case PROP_IS_DEAD:
      priv->is_dead = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
state_dispose (GObject *object)
{
  StatePrivate *priv = state_get_instance_private (STATE_MACHINES_STATE (object));

  if (priv->transitions != NULL)
    g_clear_pointer (&priv->transitions, g_ptr_array_unref);

  G_OBJECT_CLASS (state_parent_class)->dispose (object);
}
