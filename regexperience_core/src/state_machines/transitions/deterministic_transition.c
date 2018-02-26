#include "internal/state_machines/transitions/deterministic_transition.h"
#include "internal/state_machines/state.h"

typedef struct
{
    GWeakRef output_state;
} DeterministicTransitionPrivate;

struct _DeterministicTransition
{
    Transition parent_instance;
};

G_DEFINE_TYPE_WITH_PRIVATE (DeterministicTransition, deterministic_transition, STATE_MACHINES_TYPE_TRANSITION)

enum
{
    PROP_OUTPUT_STATE = 1,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void deterministic_transition_get_property (GObject    *object,
                                                   guint       property_id,
                                                   GValue     *value,
                                                   GParamSpec *pspec);

static void deterministic_transition_set_property (GObject      *object,
                                                   guint         property_id,
                                                   const GValue *value,
                                                   GParamSpec   *pspec);

static void deterministic_transition_dispose (GObject *object);

static void
deterministic_transition_class_init (DeterministicTransitionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = deterministic_transition_get_property;
  object_class->set_property = deterministic_transition_set_property;
  object_class->dispose = deterministic_transition_dispose;

  obj_properties[PROP_OUTPUT_STATE] =
      g_param_spec_object (PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE,
                           "Output state",
                           "Output state in which a DFA could transition itself.",
                           STATE_MACHINES_TYPE_STATE,
                           G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
deterministic_transition_init (DeterministicTransition *self)
{
  DeterministicTransitionPrivate *priv =
      deterministic_transition_get_instance_private (self);

  g_weak_ref_init (&priv->output_state, NULL);
}

static void
deterministic_transition_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  DeterministicTransitionPrivate *priv =
      deterministic_transition_get_instance_private (STATE_MACHINES_DETERMINISTIC_TRANSITION (object));

  switch (property_id)
    {
    case PROP_OUTPUT_STATE:
      {
        State *output_state = g_weak_ref_get (&priv->output_state);

        g_value_take_object (value, output_state);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
deterministic_transition_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  DeterministicTransitionPrivate *priv =
      deterministic_transition_get_instance_private (STATE_MACHINES_DETERMINISTIC_TRANSITION (object));

  switch (property_id)
    {
    case PROP_OUTPUT_STATE:
      {
        State *output_state = g_value_get_object (value);

        g_weak_ref_set (&priv->output_state, output_state);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
deterministic_transition_dispose (GObject *object)
{
  DeterministicTransitionPrivate *priv =
      deterministic_transition_get_instance_private (STATE_MACHINES_DETERMINISTIC_TRANSITION (object));

  g_weak_ref_clear (&priv->output_state);

  G_OBJECT_CLASS (deterministic_transition_parent_class)->dispose (object);
}
