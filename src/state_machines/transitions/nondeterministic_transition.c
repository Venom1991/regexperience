#include "internal/state_machines/transitions/nondeterministic_transition.h"
#include "internal/state_machines/state.h"
#include "internal/common/helpers.h"

struct _NondeterministicTransition
{
    Transition parent_instance;
};

typedef struct
{
    GWeakRef *output_states;
    guint     count;
} NondeterministicTransitionPrivate;

enum
{
    PROP_OUTPUT_STATES = 1,
    N_PROPERTIES
};

typedef enum
{
    MANIPULATE_OUTPUT_STATES_MODE_INITIALIZATION,
    MANIPULATE_OUTPUT_STATES_MODE_CLEARING
} ManipulateOutputStatesMode;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void       nondeterministic_transition_supplement_states_array_with_output (Transition                        *self,
                                                                                   GPtrArray                         *states_array);

static void       nondeterministic_transition_prepare_output_states               (NondeterministicTransitionPrivate *priv,
                                                                                   guint                              new_count);

static GPtrArray *nondeterministic_transition_output_states_to_g_ptr_array        (NondeterministicTransitionPrivate *priv);

static void       nondeterministic_transition_output_states_from_g_ptr_array      (NondeterministicTransitionPrivate *priv,
                                                                                   GPtrArray                         *ptr_array);

static void       nondeterministic_transition_manipulate_output_states            (NondeterministicTransitionPrivate *priv,
                                                                                   ManipulateOutputStatesMode         mode);

static void       nondeterministic_transition_get_property                        (GObject                           *object,
                                                                                   guint                              property_id,
                                                                                   GValue                            *value,
                                                                                   GParamSpec                        *pspec);

static void       nondeterministic_transition_set_property                        (GObject                           *object,
                                                                                   guint                              property_id,
                                                                                   const GValue                      *value,
                                                                                   GParamSpec                        *pspec);

static void       nondeterministic_transition_dispose                             (GObject                           *object);

G_DEFINE_TYPE_WITH_PRIVATE (NondeterministicTransition, nondeterministic_transition, TRANSITIONS_TYPE_TRANSITION)

static void
nondeterministic_transition_class_init (NondeterministicTransitionClass *klass)
{
  TransitionClass *transition_class = TRANSITIONS_TRANSITION_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  transition_class->supplement_states_array_with_output = nondeterministic_transition_supplement_states_array_with_output;

  object_class->get_property = nondeterministic_transition_get_property;
  object_class->set_property = nondeterministic_transition_set_property;
  object_class->dispose = nondeterministic_transition_dispose;

  obj_properties[PROP_OUTPUT_STATES] =
    g_param_spec_boxed (PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES,
                        "Output states",
                        "Output states (one or more) in which an NFA or an epsilon NFA could transition itself.",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
nondeterministic_transition_init (NondeterministicTransition *self)
{
  /* NOP */
}

static void
nondeterministic_transition_supplement_states_array_with_output (Transition *self,
                                                                 GPtrArray  *states_array)
{
  g_return_if_fail (TRANSITIONS_IS_NONDETERMINISTIC_TRANSITION (self) != NULL);
  g_return_if_fail (states_array != NULL);

  g_autoptr (GPtrArray) output_states = NULL;
  GCompareFunc state_equal_func = g_direct_equal;

  g_object_get (self,
                PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES, &output_states,
                NULL);

  g_ptr_array_add_range_distinct (states_array,
                                  output_states,
                                  state_equal_func,
                                  NULL);
}

static void
nondeterministic_transition_prepare_output_states (NondeterministicTransitionPrivate *priv,
                                                   guint                              new_count)
{
  GWeakRef *output_states = NULL;

  if (priv->output_states == NULL)
    {
      output_states = g_try_new (GWeakRef, new_count);
    }
  else
    {
      nondeterministic_transition_manipulate_output_states (priv,
                                                            MANIPULATE_OUTPUT_STATES_MODE_CLEARING);

      output_states = g_try_renew (GWeakRef, priv->output_states, new_count);
    }

  if (output_states != NULL)
    {
      priv->output_states = output_states;
      priv->count = new_count;

      nondeterministic_transition_manipulate_output_states (priv,
                                                            MANIPULATE_OUTPUT_STATES_MODE_INITIALIZATION);
    }
}

static GPtrArray *
nondeterministic_transition_output_states_to_g_ptr_array (NondeterministicTransitionPrivate *priv)
{
  if (priv->output_states == NULL)
    return NULL;

  GPtrArray *result = g_ptr_array_new_with_free_func (g_object_unref);
  guint count = priv->count;
  GWeakRef *output_states = priv->output_states;

  for (guint i = 0; i < count; ++i)
    {
      GWeakRef weak_ref = output_states[i];
      State *state = g_weak_ref_get (&weak_ref);

      g_ptr_array_add (result, state);
    }

  return result;
}

static void
nondeterministic_transition_output_states_from_g_ptr_array (NondeterministicTransitionPrivate *priv,
                                                            GPtrArray                         *ptr_array)
{
  g_return_if_fail (g_ptr_array_has_items (ptr_array));

  guint count = ptr_array->len;

  nondeterministic_transition_prepare_output_states (priv, count);

  GWeakRef *output_states = priv->output_states;

  for (guint i = 0; i < count; ++i)
    {
      State *state = g_ptr_array_index (ptr_array, i);

      g_weak_ref_set (&output_states[i], state);
    }
}

static void
nondeterministic_transition_manipulate_output_states (NondeterministicTransitionPrivate *priv,
                                                      ManipulateOutputStatesMode         mode)
{
  g_return_if_fail (priv->output_states != NULL);

  guint count = priv->count;
  GWeakRef *output_states = priv->output_states;

  for (guint i = 0; i < count; ++i)
    {
      switch (mode)
        {
        case MANIPULATE_OUTPUT_STATES_MODE_INITIALIZATION:
          g_weak_ref_init (&output_states[i], NULL);
          break;

        case MANIPULATE_OUTPUT_STATES_MODE_CLEARING:
          g_weak_ref_clear (&output_states[i]);
          break;
        }
    }
}

static void
nondeterministic_transition_get_property (GObject    *object,
                                          guint       property_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  NondeterministicTransitionPrivate *priv =
    nondeterministic_transition_get_instance_private (TRANSITIONS_NONDETERMINISTIC_TRANSITION (object));

  switch (property_id)
    {
    case PROP_OUTPUT_STATES:
      g_value_take_boxed (value, nondeterministic_transition_output_states_to_g_ptr_array (priv));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
nondeterministic_transition_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  NondeterministicTransitionPrivate *priv =
    nondeterministic_transition_get_instance_private (TRANSITIONS_NONDETERMINISTIC_TRANSITION (object));

  switch (property_id)
    {
    case PROP_OUTPUT_STATES:
      nondeterministic_transition_output_states_from_g_ptr_array (priv, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
nondeterministic_transition_dispose (GObject *object)
{
  NondeterministicTransitionPrivate *priv =
    nondeterministic_transition_get_instance_private (TRANSITIONS_NONDETERMINISTIC_TRANSITION (object));

  if (priv->output_states != NULL)
    {
      nondeterministic_transition_manipulate_output_states (priv,
                                                            MANIPULATE_OUTPUT_STATES_MODE_CLEARING);
      g_clear_pointer (&priv->output_states, g_free);
    }

  G_OBJECT_CLASS (nondeterministic_transition_parent_class)->dispose (object);
}
