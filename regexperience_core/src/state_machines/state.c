#include "internal/state_machines/state.h"
#include "internal/state_machines/transitions/transition.h"
#include "internal/state_machines/transitions/deterministic_transition.h"
#include "internal/common/helpers.h"

typedef struct
{
    StateTypeFlags  state_type_flags;
    GPtrArray      *transitions;
} StatePrivate;

enum
{
    PROP_TYPE_FLAGS = 1,
    PROP_TRANSITIONS,
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

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
state_init (State *self)
{
  /* NOP */
}

static gboolean
state_default_is_composed_from (State           *self,
                                const GPtrArray *states)
{
  return FALSE;
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

gboolean
state_is_dead (State *self)
{
  g_return_val_if_fail (STATE_MACHINES_IS_STATE (self), FALSE);

  StatePrivate *priv = state_get_instance_private (self);
  GPtrArray *transitions = priv->transitions;
  GEqualFunc state_equal_func = g_direct_equal;

  if (g_ptr_array_has_items (transitions))
    {
      const guint dead_state_transitions_count = 1;

      if (transitions->len == dead_state_transitions_count)
        {
          Transition *transition = g_ptr_array_index (transitions, 0);

          if (TRANSITIONS_IS_DETERMINISTIC_TRANSITION (transition))
            {
              EqualityConditionType condition_type = EQUALITY_CONDITION_TYPE_UNDEFINED;
              g_autoptr (State) output_state = NULL;

              g_object_get (transition,
                            PROP_TRANSITION_EQUALITY_CONDITION_TYPE, &condition_type,
                            PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, &output_state,
                            NULL);

              return condition_type == EQUALITY_CONDITION_TYPE_ANY &&
                     state_equal_func (self, output_state);
            }
        }
    }

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
      if (priv->transitions != NULL)
        g_ptr_array_unref (priv->transitions);

      priv->transitions = g_value_dup_boxed (value);
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
