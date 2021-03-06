#include "internal/state_machines/transitions/transition.h"
#include "internal/common/helpers.h"

typedef struct
{
  gchar                 expected_character;
  gboolean              requires_input;
  EqualityConditionType condition_type;
} TransitionPrivate;

enum
{
  PROP_EXPECTED_CHARACTER = 1,
  PROP_REQUIRES_INPUT,
  PROP_EQUALITY_CONDITION_TYPE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static GEqualFunc transition_discern_equality_function (EqualityConditionType  condition_type);

static gboolean   transition_char_any                  (gconstpointer          a,
                                                        gconstpointer          b);

static gboolean   transition_char_equal                (gconstpointer          a,
                                                        gconstpointer          b);

static gboolean   transition_char_not_equal            (gconstpointer          a,
                                                        gconstpointer          b);

static void       transition_get_property              (GObject               *object,
                                                        guint                  property_id,
                                                        GValue                *value,
                                                        GParamSpec            *pspec);

static void       transition_set_property              (GObject               *object,
                                                        guint                  property_id,
                                                        const GValue          *value,
                                                        GParamSpec            *pspec);

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (Transition, transition, G_TYPE_OBJECT)

static void
transition_class_init (TransitionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = transition_get_property;
  object_class->set_property = transition_set_property;

  obj_properties[PROP_EXPECTED_CHARACTER] =
    g_param_spec_char (PROP_TRANSITION_EXPECTED_CHARACTER,
                       "Expected character",
                       "Expected character used to check if the equality condition is met.",
                       0,
                       G_MAXINT8,
                       0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  obj_properties[PROP_REQUIRES_INPUT] =
    g_param_spec_boolean (PROP_TRANSITION_REQUIRES_INPUT,
                          "Requires input",
                          "Describes whether or not an actual input is required in order for"
                            "the transition to occur.",
                          TRUE,
                          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  obj_properties[PROP_EQUALITY_CONDITION_TYPE] =
    g_param_spec_uint (PROP_TRANSITION_EQUALITY_CONDITION_TYPE,
                       "Equality condition type",
                       "Type of equality condition that needs to be satisfied in order for the transition to occur.",
                       EQUALITY_CONDITION_TYPE_UNDEFINED,
                       EQUALITY_CONDITION_TYPE_NOT_EQUAL,
                       EQUALITY_CONDITION_TYPE_EQUAL,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
transition_init (Transition *self)
{
  /* NOP */
}

void
transition_supplement_states_array_with_output (Transition *self,
                                                GPtrArray  *states_array)
{
  g_return_if_fail (TRANSITIONS_IS_TRANSITION (self));

  TransitionClass *klass = TRANSITIONS_TRANSITION_GET_CLASS (self);

  g_return_if_fail (klass->supplement_states_array_with_output != NULL);

  klass->supplement_states_array_with_output (self, states_array);
}

gboolean
transition_is_possible (Transition *self,
                        gchar       input_character)
{
  g_return_val_if_fail (TRANSITIONS_IS_TRANSITION (self), FALSE);

  if (!transition_is_epsilon (self))
    {
      TransitionPrivate *priv = transition_get_instance_private (self);
      gchar expected_character = priv->expected_character;
      EqualityConditionType condition_type = priv->condition_type;
      GEqualFunc equality_function = transition_discern_equality_function (condition_type);

      g_return_val_if_fail (equality_function != NULL, FALSE);

      return equality_function (GINT_TO_POINTER (expected_character),
                                GINT_TO_POINTER (input_character));
    }

  return FALSE;
}

gboolean
transition_is_allowed (Transition *self,
                       gchar       input_character)
{
  g_return_val_if_fail (TRANSITIONS_IS_TRANSITION (self), FALSE);

  if (transition_is_possible (self, input_character))
    {
      TransitionPrivate *priv = transition_get_instance_private (self);
      EqualityConditionType condition_type = priv->condition_type;

      switch (condition_type)
        {
        case EQUALITY_CONDITION_TYPE_ANY:
          {
            /* The start and end of text special characters are not allowed
             * as they shouldn't be covered by this type of equality.
             */
            gchar disallowed_chars[] = { START, END };
            gsize disallowed_chars_count = G_N_ELEMENTS (disallowed_chars);

            for (guint i = 0; i < disallowed_chars_count; ++i)
              {
                gchar disallowed_char = disallowed_chars[i];

                if (input_character == disallowed_char)
                  return FALSE;
              }

            return TRUE;
          }

        default:
          return TRUE;
        }
    }

  return FALSE;
}

gboolean
transition_is_epsilon (Transition *self)
{
  g_return_val_if_fail (TRANSITIONS_IS_TRANSITION (self), FALSE);

  TransitionPrivate *priv = transition_get_instance_private (self);

  return priv->expected_character == EPSILON &&
         priv->requires_input == FALSE &&
         priv->condition_type == EQUALITY_CONDITION_TYPE_ANY;
}

void
transition_convert_to_epsilon (Transition *self)
{
  g_return_if_fail (TRANSITIONS_IS_TRANSITION (self));

  TransitionPrivate *priv = transition_get_instance_private (self);

  if (!transition_is_epsilon (self))
    {
      priv->expected_character = EPSILON;
      priv->requires_input = FALSE;
      priv->condition_type = EQUALITY_CONDITION_TYPE_ANY;
    }
}

gint
transition_compare_equality_condition_type (Transition *a,
                                            Transition *b)
{
  Transition **a_ptr = (Transition **) a;
  Transition **b_ptr = (Transition **) b;

  g_return_val_if_fail (TRANSITIONS_IS_TRANSITION (*a_ptr), 0);
  g_return_val_if_fail (TRANSITIONS_IS_TRANSITION (*b_ptr), 0);

  TransitionPrivate *a_priv = transition_get_instance_private (TRANSITIONS_TRANSITION (*a_ptr));
  TransitionPrivate *b_priv = transition_get_instance_private (TRANSITIONS_TRANSITION (*b_ptr));

  EqualityConditionType a_condition_type = a_priv->condition_type;
  EqualityConditionType b_condition_type = b_priv->condition_type;

  return -((a_condition_type > b_condition_type) - (a_condition_type < b_condition_type));
}

static GEqualFunc
transition_discern_equality_function (EqualityConditionType condition_type)
{
  switch (condition_type)
    {
    case EQUALITY_CONDITION_TYPE_ANY:
      return transition_char_any;

    case EQUALITY_CONDITION_TYPE_EQUAL:
      return transition_char_equal;

    case EQUALITY_CONDITION_TYPE_NOT_EQUAL:
      return transition_char_not_equal;

    default:
      return NULL;
    }
}

static gboolean
transition_char_any (gconstpointer a,
                     gconstpointer b)
{
  return TRUE;
}

static gboolean
transition_char_equal (gconstpointer a,
                       gconstpointer b)
{
  const gchar a_char = (const gchar) GPOINTER_TO_INT (a);
  const gchar b_char = (const gchar) GPOINTER_TO_INT (b);

  return a_char == b_char;
}

static gboolean
transition_char_not_equal (gconstpointer a,
                           gconstpointer b)
{
  const gchar a_char = (const gchar) GPOINTER_TO_INT (a);
  const gchar b_char = (const gchar) GPOINTER_TO_INT (b);

  return a_char != b_char;
}

static void
transition_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  TransitionPrivate *priv = transition_get_instance_private (TRANSITIONS_TRANSITION (object));

  switch (property_id)
    {
    case PROP_EXPECTED_CHARACTER:
      g_value_set_schar (value, priv->expected_character);
      break;

    case PROP_REQUIRES_INPUT:
      g_value_set_boolean (value, priv->requires_input);
      break;

    case PROP_EQUALITY_CONDITION_TYPE:
      g_value_set_uint (value, priv->condition_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
transition_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  TransitionPrivate *priv = transition_get_instance_private (TRANSITIONS_TRANSITION (object));

  switch (property_id)
    {
    case PROP_EXPECTED_CHARACTER:
      priv->expected_character = g_value_get_schar (value);
      break;

    case PROP_REQUIRES_INPUT:
      priv->requires_input = g_value_get_boolean (value);
      break;

    case PROP_EQUALITY_CONDITION_TYPE:
      priv->condition_type = (EqualityConditionType) g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
