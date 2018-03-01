#include "internal/state_machines/fsm.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/deterministic_transition.h"
#include "internal/state_machines/transitions/nondeterministic_transition.h"
#include "internal/state_machines/state_factory.h"
#include "internal/common/helpers.h"

typedef struct
{
    GPtrArray *all_states;
    State     *start_state;
    GPtrArray *final_states;
    GPtrArray *non_final_states;
    GSList    *alphabet;
} FsmPrivate;

static void fsm_fsm_initializable_interface_init (FsmInitializableInterface *iface);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (Fsm, fsm, G_TYPE_OBJECT,
                                  G_ADD_PRIVATE (Fsm)
                                  G_IMPLEMENT_INTERFACE (STATE_MACHINES_TYPE_FSM_INITIALIZABLE,
                                                         fsm_fsm_initializable_interface_init))

enum
{
    PROP_ALL_STATES = 1,
    PROP_START_STATE,
    PROP_FINAL_STATES,
    PROP_NON_FINAL_STATES,
    PROP_ALPHABET,
    N_PROPERTIES
};

static void fsm_prepare_states (FsmPrivate *priv);

static void fsm_prepare_alphabet (FsmPrivate *priv);

static void fsm_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec);

static void fsm_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec);

static void fsm_dispose (GObject *object);

static void
fsm_class_init (FsmClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = fsm_get_property;
  object_class->set_property = fsm_set_property;
  object_class->dispose = fsm_dispose;

  g_object_class_override_property (object_class,
                                    PROP_ALL_STATES,
                                    PROP_FSM_INITIALIZABLE_ALL_STATES);
  g_object_class_override_property (object_class,
                                    PROP_START_STATE,
                                    PROP_FSM_INITIALIZABLE_START_STATE);
  g_object_class_override_property (object_class,
                                    PROP_FINAL_STATES,
                                    PROP_FSM_INITIALIZABLE_FINAL_STATES);
  g_object_class_override_property (object_class,
                                    PROP_NON_FINAL_STATES,
                                    PROP_FSM_INITIALIZABLE_NON_FINAL_STATES);
  g_object_class_override_property (object_class,
                                    PROP_ALPHABET,
                                    PROP_FSM_INITIALIZABLE_ALPHABET);
}

static void
fsm_init (Fsm *self)
{
  /* NOP */
}

GPtrArray *
fsm_fetch_output_states_from_single (State *input_state,
                                     gchar  expected_character)
{
  g_autoptr (GPtrArray) input_states = g_ptr_array_new ();

  g_ptr_array_add (input_states, input_state);

  return fsm_fetch_output_states_from_multiple (input_states,
                                                     expected_character);
}

GPtrArray *
fsm_fetch_output_states_from_multiple (GPtrArray *input_states,
                                       gchar      expected_character)
{
  GPtrArray *output_states = g_ptr_array_new ();
  GCompareFunc state_equal_func = g_direct_equal;

  for (guint i = 0; i < input_states->len; ++i)
    {
      State *input_state = g_ptr_array_index (input_states, i);
      g_autoptr (GPtrArray) transitions = NULL;

      g_object_get (input_state,
                    PROP_STATE_TRANSITIONS, &transitions,
                    NULL);

      if (g_ptr_array_has_items (transitions))
        {
          for (guint j = 0; j < transitions->len; ++j)
            {
              Transition *transition = g_ptr_array_index (transitions, j);

              if (transition_is_possible (transition, expected_character))
                {
                  if (TRANSITIONS_IS_DETERMINISTIC_TRANSITION (transition))
                    {
                      g_autoptr (State) deterministic_transition_output_state = NULL;

                      g_object_get (transition,
                                    PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, &deterministic_transition_output_state,
                                    NULL);

                      g_ptr_array_add_if_not_exists (output_states,
                                                     deterministic_transition_output_state,
                                                     state_equal_func);
                    }
                  else if (TRANSITIONS_IS_NONDETERMINISTIC_TRANSITION (transition))
                    {
                      g_autoptr (GPtrArray) nondeterministic_transition_output_states = NULL;

                      g_object_get (transition,
                                    PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES, &nondeterministic_transition_output_states,
                                    NULL);

                      g_ptr_array_add_range_distinct (output_states,
                                                      nondeterministic_transition_output_states,
                                                      state_equal_func);
                    }
                }
            }
        }
    }

  return output_states;
}

State *
fsm_get_or_create_composite_state (GPtrArray                          *all_states,
                                   GPtrArray                          *composed_from_states,
                                   CompositeStateResolveTypeFlagsMode  resolve_type_flags_mode,
                                   gboolean                           *already_existed)
{
  State *composite_state = NULL;

  if (g_ptr_array_has_items (all_states))
    {
      for (guint i = 0; i < all_states->len; ++i)
        {
          State *current_state = g_ptr_array_index (all_states, i);

          if (state_is_composed_from (current_state, composed_from_states))
            {
              composite_state = current_state;

              if (already_existed != NULL)
                *already_existed = TRUE;

              break;
            }
        }
    }

  if (composite_state == NULL)
    {
      composite_state = create_composite_state (composed_from_states,
                                                resolve_type_flags_mode);

      if (already_existed != NULL)
        *already_existed = FALSE;

      g_ptr_array_add (all_states, composite_state);
    }

  return composite_state;
}

State *
fsm_get_or_create_dead_state (GPtrArray *all_states)
{
  State *dead_state = NULL;

  if (g_ptr_array_has_items (all_states))
    {
      for (guint i = 0; i < all_states->len; ++i)
        {
          State *current_state = g_ptr_array_index (all_states, i);

          if (state_is_dead (current_state))
            {
              dead_state = current_state;

              break;
            }
        }
    }

  if (dead_state == NULL)
    {
      dead_state = create_dead_state ();

      g_ptr_array_add (all_states, dead_state);
    }

  return dead_state;
}

static void
fsm_fsm_initializable_interface_init (FsmInitializableInterface *iface)
{
  /* NOP */
}

static void
fsm_prepare_states (FsmPrivate *priv)
{
  if (priv->start_state != NULL)
    g_object_unref (priv->start_state);

  if (priv->final_states != NULL)
    g_ptr_array_unref (priv->final_states);

  if (priv->non_final_states != NULL)
    g_ptr_array_unref (priv->non_final_states);

  GPtrArray *all_states = priv->all_states;
  State *start_state = NULL;
  GPtrArray *final_states = NULL;
  GPtrArray *non_final_states = NULL;

  if (g_ptr_array_has_items (all_states))
    {
      final_states = g_ptr_array_new_with_free_func (g_object_unref);
      non_final_states = g_ptr_array_new_with_free_func (g_object_unref);

      for (guint i = 0; i < all_states->len; ++i)
        {
          State *current_state = g_object_ref (g_ptr_array_index (all_states, i));
          StateTypeFlags state_type_flags = STATE_TYPE_UNDEFINED;

          g_object_get (current_state,
                        PROP_STATE_TYPE_FLAGS, &state_type_flags,
                        NULL);

          gboolean state_is_start = (state_type_flags & STATE_TYPE_START);
          gboolean state_is_final = (state_type_flags & STATE_TYPE_FINAL);

          if (state_is_start)
            {
              g_return_if_fail (start_state == NULL);

              start_state = g_object_ref (current_state);
            }

          if (state_is_final)
            g_ptr_array_add (final_states, current_state);
          else
            g_ptr_array_add (non_final_states, current_state);
        }
    }

  priv->start_state = start_state;
  priv->final_states = final_states;
  priv->non_final_states = non_final_states;
}

static void
fsm_prepare_alphabet (FsmPrivate *priv)
{
  if (priv->alphabet != NULL)
    g_slist_free (priv->alphabet);

  GSList *alphabet = NULL;
  GPtrArray *all_states = priv->all_states;

  if (g_ptr_array_has_items (all_states))
    {
      for (guint i = 0; i < all_states->len; ++i)
        {
          State *state = g_ptr_array_index (all_states, i);
          g_autoptr (GPtrArray) transitions = NULL;

          g_object_get (state,
                        PROP_STATE_TRANSITIONS, &transitions,
                        NULL);

          if (g_ptr_array_has_items (transitions))
            {
              for (guint j = 0; j < transitions->len; ++j)
                {
                  Transition *transition = g_ptr_array_index (transitions, j);

                  if (!transition_is_epsilon (transition))
                    {
                      gchar expected_character;
                      gpointer expected_character_as_pointer = NULL;

                      g_object_get (transition,
                                    PROP_TRANSITION_EXPECTED_CHARACTER, &expected_character,
                                    NULL);
                      expected_character_as_pointer = GINT_TO_POINTER (expected_character);

                      if (g_slist_find (alphabet, expected_character_as_pointer) == NULL)
                        alphabet = g_slist_append (alphabet, expected_character_as_pointer);
                    }
                }
            }
        }
    }

  priv->alphabet = alphabet;
}

static void
fsm_get_property (GObject    *object,
                  guint       property_id,
                  GValue     *value,
                  GParamSpec *pspec)
{
  FsmPrivate *priv = fsm_get_instance_private (STATE_MACHINES_FSM (object));

  switch (property_id)
    {
      case PROP_ALL_STATES:
        g_value_set_boxed (value, priv->all_states);
      break;

      case PROP_START_STATE:
        g_value_set_object (value, priv->start_state);
      break;

      case PROP_FINAL_STATES:
        g_value_set_boxed (value, priv->final_states);
      break;

      case PROP_NON_FINAL_STATES:
        g_value_set_boxed (value, priv->non_final_states);
      break;

      case PROP_ALPHABET:
        g_value_set_pointer (value, priv->alphabet);
      break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
fsm_set_property (GObject      *object,
                  guint         property_id,
                  const GValue *value,
                  GParamSpec   *pspec)
{
  FsmPrivate *priv = fsm_get_instance_private (STATE_MACHINES_FSM (object));

  switch (property_id)
    {
      case PROP_ALL_STATES:
        if (priv->all_states != NULL)
          g_ptr_array_unref (priv->all_states);

      priv->all_states = g_value_dup_boxed (value);

      fsm_prepare_states (priv);
      fsm_prepare_alphabet (priv);

      break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
fsm_dispose (GObject *object)
{
  FsmPrivate *priv = fsm_get_instance_private (STATE_MACHINES_FSM (object));

  if (priv->all_states != NULL)
    g_clear_pointer (&priv->all_states, g_ptr_array_unref);

  if (priv->start_state != NULL)
    g_clear_object (&priv->start_state);

  if (priv->final_states != NULL)
    g_clear_pointer (&priv->final_states, g_ptr_array_unref);

  if (priv->non_final_states != NULL)
    g_clear_pointer (&priv->non_final_states, g_ptr_array_unref);

  if (priv->alphabet != NULL)
    g_clear_pointer (&priv->alphabet, g_slist_free);

  G_OBJECT_CLASS (fsm_parent_class)->dispose (object);
}
