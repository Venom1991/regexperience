#include "internal/state_machines/transitions/mealy_transition.h"

struct _MealyTransition
{
  DeterministicTransition parent_instance;
};

typedef struct
{
  gpointer output_data;
} MealyTransitionPrivate;

enum
{
  PROP_OUTPUT_DATA = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void mealy_transition_get_property (GObject      *object,
                                           guint         property_id,
                                           GValue       *value,
                                           GParamSpec   *pspec);

static void mealy_transition_set_property (GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec);

G_DEFINE_TYPE_WITH_PRIVATE (MealyTransition, mealy_transition, TRANSITIONS_TYPE_DETERMINISTIC_TRANSITION)

static void
mealy_transition_class_init (MealyTransitionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = mealy_transition_get_property;
  object_class->set_property = mealy_transition_set_property;

  obj_properties[PROP_OUTPUT_DATA] =
    g_param_spec_pointer (PROP_MEALY_TRANSITION_OUTPUT_DATA,
                         "Output data",
                         "Arbitrary output data mapped to both the expected character as well as the output state.",
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
mealy_transition_init (MealyTransition *self)
{
  /* NOP */
}

static void
mealy_transition_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  MealyTransitionPrivate *priv =
    mealy_transition_get_instance_private (TRANSITIONS_MEALY_TRANSITION (object));

  switch (property_id)
    {
    case PROP_OUTPUT_DATA:
      g_value_set_pointer (value, priv->output_data);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
mealy_transition_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  MealyTransitionPrivate *priv =
    mealy_transition_get_instance_private (TRANSITIONS_MEALY_TRANSITION (object));

  switch (property_id)
    {
    case PROP_OUTPUT_DATA:
      priv->output_data = g_value_get_pointer (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
