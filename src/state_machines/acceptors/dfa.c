#include "internal/state_machines/acceptors/dfa.h"
#include "internal/state_machines/acceptors/acceptor_runnable.h"
#include "internal/state_machines/fsm_modifiable.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/deterministic_transition.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"
#include "core/match.h"

struct _Dfa
{
  Fsm parent_instance;
};

typedef struct
{
  State    *current_state;
  gboolean  is_input_exhausted;
} DfaPrivate;

static void       dfa_fsm_modifiable_interface_init                   (FsmModifiableInterface    *iface);

static void       dfa_acceptor_runnable_interface_init                (AcceptorRunnableInterface *iface);

static void       dfa_minimize                                        (FsmModifiable             *self);

static void       dfa_complement                                      (FsmModifiable             *self);

static GPtrArray *dfa_run                                             (AcceptorRunnable          *self,
                                                                       const gchar               *input);

static gboolean   dfa_can_accept                                      (AcceptorRunnable          *self);

static void       dfa_reset                                           (Dfa                       *self);

static State     *dfa_transition_to_next_state                        (Dfa                       *self,
                                                                       gchar                      input_character);

static void       dfa_remove_unreachable_states_if_needed             (Dfa                       *self);

static GArray    *dfa_fetch_unreachable_states_from                   (GPtrArray                 *all_states);

static void       dfa_compose_equivalent_states_if_needed             (Dfa                       *self);

static GPtrArray *dfa_fetch_equivalence_classes_from                  (GPtrArray                 *input_equivalence_classes,
                                                                       GSList                    *alphabet);

static gboolean   dfa_can_states_transition_to_same_equivalence_class (State                     *first_state,
                                                                       State                     *second_state,
                                                                       gchar                      expected_character,
                                                                       GPtrArray                 *equivalence_classes);

static GPtrArray *dfa_fetch_matched_equivalence_class_from            (GPtrArray                 *input_equivalence_class,
                                                                       GPtrArray                 *all_equivalence_classes,
                                                                       gchar                      expected_character);

static void       dfa_dispose                                         (GObject                   *object);

G_DEFINE_TYPE_WITH_CODE (Dfa, dfa, STATE_MACHINES_TYPE_FSM,
                         G_ADD_PRIVATE (Dfa)
                         G_IMPLEMENT_INTERFACE (STATE_MACHINES_TYPE_FSM_MODIFIABLE,
                                                dfa_fsm_modifiable_interface_init)
                         G_IMPLEMENT_INTERFACE (ACCEPTORS_TYPE_ACCEPTOR_RUNNABLE,
                                                dfa_acceptor_runnable_interface_init))

static void
dfa_class_init (DfaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = dfa_dispose;
}

static void
dfa_init (Dfa *self)
{
  /* NOP */
}

static void
dfa_fsm_modifiable_interface_init (FsmModifiableInterface *iface)
{
  iface->minimize = dfa_minimize;
  iface->complement = dfa_complement;
}

static void
dfa_acceptor_runnable_interface_init (AcceptorRunnableInterface *iface)
{
  iface->run = dfa_run;
  iface->can_accept = dfa_can_accept;
}

static void
dfa_minimize (FsmModifiable *self)
{
  g_return_if_fail (ACCEPTORS_IS_DFA (self));

  Dfa *dfa = ACCEPTORS_DFA (self);

  /* Performing minimization firstly by removing unreachable states (if they exist) - i.e., states in to which
   * no other state can transition.
   */
  dfa_remove_unreachable_states_if_needed (dfa);

  /* Performing further minimization using the equivalence theorem - i.e., by composing classes containing
   * equivalent states (if they exist) into their corresponding composite states.
   */
  dfa_compose_equivalent_states_if_needed (dfa);
}

static void
dfa_complement (FsmModifiable *self)
{
  g_return_if_fail (ACCEPTORS_IS_DFA (self));

  g_autoptr (GPtrArray) all_states = NULL;

  g_object_get (self,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &all_states,
                NULL);

  if (g_collection_has_items (all_states))
    {
      for (guint i = 0; i < all_states->len; ++i)
        {
          State *state = g_ptr_array_index (all_states, i);
          StateTypeFlags state_type_flags = STATE_TYPE_UNDEFINED;

          g_object_get (state,
                        PROP_STATE_TYPE_FLAGS, &state_type_flags,
                        NULL);

          StateTypeFlags complement_state_type_flags = state_type_flags;

          if ((state_type_flags & STATE_TYPE_DEFAULT) || (state_type_flags & STATE_TYPE_START))
            {
              complement_state_type_flags &= ~STATE_TYPE_DEFAULT;
              complement_state_type_flags |= STATE_TYPE_FINAL;
            }

          if (state_type_flags & STATE_TYPE_FINAL)
            {
              complement_state_type_flags &= ~STATE_TYPE_FINAL;

              /* No need to set the default flag for the start state as it is already a non-final state. */
              if (!(state_type_flags & STATE_TYPE_START))
                complement_state_type_flags |= STATE_TYPE_DEFAULT;
            }

          g_object_set (state,
                        PROP_STATE_TYPE_FLAGS, complement_state_type_flags,
                        NULL);
        }

      /* Reinitializing the DFA's states. */
      g_object_set (self,
                    PROP_FSM_INITIALIZABLE_ALL_STATES, all_states,
                    NULL);
    }
}

static GPtrArray *
dfa_run (AcceptorRunnable *self,
         const gchar      *input)
{
  g_return_val_if_fail (ACCEPTORS_IS_DFA (self), NULL);
  g_return_val_if_fail (input != NULL, NULL);

  Dfa *dfa = ACCEPTORS_DFA (self);
  DfaPrivate *priv = dfa_get_instance_private (dfa);

  dfa_reset (dfa);
  g_return_val_if_fail (priv->current_state != NULL, NULL);

  gchar start_of_text[] = { START, END_OF_STRING };
  gchar end_of_text[] = { END, END_OF_STRING };
  g_autofree gchar *adjusted_input = g_strconcat (start_of_text,
                                                  input,
                                                  end_of_text,
                                                  NULL);
  guint begin = 0, end = 0;
  GPtrArray *matches = NULL;

  while (TRUE)
    {
      gchar previous_character = 0;

      if (end != 0)
        {
          previous_character = adjusted_input[end - 1];

          if (previous_character == END_OF_STRING)
            {
              priv->is_input_exhausted = TRUE;

              break;
            }
        }

      gchar current_character = adjusted_input[end];
      State *current_state = priv->current_state;
      StateTypeFlags current_state_type_flags = STATE_TYPE_UNDEFINED;
      State *next_state = dfa_transition_to_next_state (dfa, current_character);
      gboolean next_state_is_dead = FALSE;

      g_object_get (current_state,
                    PROP_STATE_TYPE_FLAGS, &current_state_type_flags,
                    NULL);

      gboolean current_state_is_start = (current_state_type_flags & STATE_TYPE_START);
      gboolean current_state_is_final = (current_state_type_flags & STATE_TYPE_FINAL);

      g_object_get (next_state,
                    PROP_STATE_IS_DEAD, &next_state_is_dead,
                    NULL);

      guint distance = end - begin;
      g_autoptr (GString) match_value = NULL;
      guint match_range_begin = 0, match_range_end = 0;

      /* Handling empty matches. */
      if (current_state_is_final)
        {
          /* Ignoring empty matches that are a result of the start and end of text special characters. */
          gboolean is_valid_empty_match =
            (distance == 0 &&
             (current_character != START && previous_character != END));

          if (is_valid_empty_match)
            {
              g_autofree gchar *value = g_strdup (EMPTY_STRING);

              match_value = g_string_new (value);
              match_range_begin = match_range_end = end - 1;
            }
        }

      /* Handling non-empty matches. */
      if (next_state_is_dead)
        {
          /* Discerning the current run's result in case the DFA transitioned
           * to the dead state from a final state.
           */
          gboolean is_valid_non_empty_match =
            (distance != 0 && current_state_is_final);

          if (is_valid_non_empty_match)
            {
              g_autofree gchar *value = g_strndup (adjusted_input + begin, distance);

              match_value = g_string_new (value);
              match_range_begin = begin - 1;
              match_range_end = end - 1;
            }

          /* Moving onto the next character in the input in case the dead state was reached
           * straight from the start state without consuming a single character.
           */
          if (current_state_is_start && distance == 0)
            end++;

          /* Preparing for a new run. */
          begin = end;

          dfa_reset (dfa);
        }
      else
        {
          /* Moving onto the next character in the input in case the DFA can continue with the current run. */
          end++;
        }

      /* Adding a new match if possible. */
      if (match_value != NULL)
        {
          if (matches == NULL)
            matches = g_ptr_array_new_with_free_func (g_object_unref);

          Match *match = match_new (PROP_MATCH_VALUE, match_value,
                                    PROP_MATCH_RANGE_BEGIN, match_range_begin,
                                    PROP_MATCH_RANGE_END, match_range_end);

          g_ptr_array_add (matches, match);
        }
    }

  return matches;
}

static gboolean
dfa_can_accept (AcceptorRunnable *self)
{
  g_return_val_if_fail (ACCEPTORS_IS_DFA (self), FALSE);

  Dfa *dfa = ACCEPTORS_DFA (self);
  DfaPrivate *priv = dfa_get_instance_private (dfa);

  return priv->is_input_exhausted;
}

static void
dfa_reset (Dfa *self)
{
  DfaPrivate *priv = dfa_get_instance_private (self);

  if (priv->current_state != NULL)
    g_object_unref (priv->current_state);

  State *start_state = NULL;

  g_object_get (self,
                PROP_FSM_INITIALIZABLE_START_STATE, &start_state,
                NULL);

  priv->current_state = start_state;
  priv->is_input_exhausted = FALSE;
}

static State *
dfa_transition_to_next_state (Dfa   *self,
                              gchar  input_character)
{
  DfaPrivate *priv = dfa_get_instance_private (self);
  State *current_state = priv->current_state;
  g_autoptr (GPtrArray) transitions = NULL;

  g_object_get (current_state,
                PROP_STATE_TRANSITIONS, &transitions,
                NULL);

  g_return_val_if_fail (g_collection_has_items (transitions), FALSE);
  g_object_unref (current_state);

  /* Ignoring the null terminator. */
  if (input_character != END_OF_STRING)
    {
      /* Trying to find an eligible transition found among the current state's transitions. */
      for (guint i = 0; i < transitions->len; ++i)
        {
          Transition *transition = g_ptr_array_index (transitions, i);

          g_return_val_if_fail (TRANSITIONS_IS_DETERMINISTIC_TRANSITION (transition), FALSE);

          if (transition_is_allowed (transition, input_character))
            {
              State *next_state = NULL;

              g_object_get (transition,
                            PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, &next_state,
                            NULL);

              priv->current_state = next_state;

              return next_state;
            }
        }
    }

  /* Transitioning to the dead state in case the input character is unrecognized.
   * This means that it does not belong to the state machine's alphabet.
   */
  g_autoptr (GPtrArray) all_states = NULL;

  g_object_get (self,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &all_states,
                NULL);

  State *dead_state = fsm_get_or_create_dead_state (all_states);

  priv->current_state = g_object_ref (dead_state);

  return dead_state;
}

static void
dfa_remove_unreachable_states_if_needed (Dfa *self)
{
  g_autoptr (GPtrArray) all_states = NULL;

  g_object_get (self,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &all_states,
                NULL);

  g_autoptr (GArray) unreachable_states = dfa_fetch_unreachable_states_from (all_states);

  if (g_collection_has_items (unreachable_states))
    {
      for (guint i = 0; i < unreachable_states->len; ++i)
        {
          gpointer unreachable_state = g_array_index (unreachable_states, gpointer, i);

          g_ptr_array_remove_fast (all_states, unreachable_state);
        }

      /* Reinitializing the DFA's states. */
      g_object_set (self,
                    PROP_FSM_INITIALIZABLE_ALL_STATES, all_states,
                    NULL);
    }
}

static GArray *
dfa_fetch_unreachable_states_from (GPtrArray *all_states)
{
  GArray *unreachable_states = NULL;

  if (g_collection_has_items (all_states))
    {
      g_autoptr (GPtrArray) transition_output_states = g_ptr_array_new ();
      GEqualFunc state_equal_func = g_direct_equal;

      for (guint i = 0; i < all_states->len; ++i)
        {
          State *current_state = g_ptr_array_index (all_states, i);
          g_autoptr (GPtrArray) transitions = NULL;

          g_object_get (current_state,
                        PROP_STATE_TRANSITIONS, &transitions,
                        NULL);

          for (guint j = 0; j < transitions->len; ++j)
            {
              Transition *transition = g_ptr_array_index (transitions, j);
              g_autoptr (State) transition_output_state = NULL;

              g_object_get (transition,
                            PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, &transition_output_state,
                            NULL);

              g_ptr_array_add_if_not_exists (transition_output_states,
                                             transition_output_state,
                                             state_equal_func,
                                             NULL);
            }
        }

      for (guint i = 0; i < all_states->len; ++i)
        {
          State *current_state = g_ptr_array_index (all_states, i);

          if (!g_ptr_array_find_with_equal_func (transition_output_states,
                                                 current_state,
                                                 state_equal_func,
                                                 NULL))
            {
              StateTypeFlags current_state_type_flags = STATE_TYPE_UNDEFINED;

              g_object_get (current_state,
                            PROP_STATE_TYPE_FLAGS, &current_state_type_flags,
                            NULL);

              /* Disregarding the start state (even if it does not have any incoming transitions)
               * due to it being reachable as is.
               */
              if (!(current_state_type_flags & STATE_TYPE_START))
                {
                  if (unreachable_states == NULL)
                    unreachable_states = g_array_new (FALSE, FALSE, sizeof (gpointer));

                  g_array_append_val (unreachable_states, current_state);
                }
            }
        }
    }

  return unreachable_states;
}

static void
dfa_compose_equivalent_states_if_needed (Dfa *self)
{
  GSList *alphabet = NULL;
  g_autoptr (GPtrArray) final_states = NULL;
  g_autoptr (GPtrArray) non_final_states = NULL;

  g_object_get (self,
                PROP_FSM_INITIALIZABLE_ALPHABET, &alphabet,
                PROP_FSM_INITIALIZABLE_FINAL_STATES, &final_states,
                PROP_FSM_INITIALIZABLE_NON_FINAL_STATES, &non_final_states,
                NULL);

  GPtrArray *initial_equivalence_classes = g_ptr_array_new ();

  /* Setting up two initial equivalence classes, those containing final and non-final states. */
  g_ptr_array_add_multiple (initial_equivalence_classes,
                            final_states, non_final_states,
                            NULL);

  g_autoptr (GPtrArray) final_equivalence_classes = dfa_fetch_equivalence_classes_from (initial_equivalence_classes,
                                                                                        alphabet);
  const guint acceptable_scalar_equivalence_class_size = 1;
  gboolean should_compose_equivalence_classes = FALSE;

  for (guint i = 0; i < final_equivalence_classes->len; ++i)
    {
      GPtrArray *equivalence_class = g_ptr_array_index (final_equivalence_classes, i);

      if (g_collection_has_items (equivalence_class) &&
          equivalence_class->len > acceptable_scalar_equivalence_class_size)
        {
          should_compose_equivalence_classes = TRUE;

          break;
        }
    }

  /* Avoiding the composure of new states in case no final equivalence class contains more than one state.
   * This means that the DFA is already minimal.
   */
  if (should_compose_equivalence_classes)
    {
      guint alphabet_length = g_slist_length (alphabet);
      g_autoptr (GPtrArray) all_states = g_ptr_array_new_with_free_func (g_object_unref);

      for (guint i = 0; i < final_equivalence_classes->len; ++i)
        {
          GPtrArray *equivalence_class = g_ptr_array_index (final_equivalence_classes, i);

          if (g_collection_has_items (equivalence_class))
            {
              State *input_state = NULL;
              g_autoptr (GPtrArray) transitions = g_ptr_array_new_with_free_func (g_object_unref);

              if (equivalence_class->len == acceptable_scalar_equivalence_class_size)
                {
                  input_state = g_ptr_array_index (equivalence_class, 0);

                  g_ptr_array_add (all_states, g_object_ref (input_state));
                }
              else
                {
                  input_state = fsm_get_or_create_composite_state (all_states,
                                                                   equivalence_class,
                                                                   COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_ALL,
                                                                   NULL);
                }

              /* Defining new transitions for the input state. */
              for (guint j = 0; j < alphabet_length; ++j)
                {
                  gchar expected_character = (gchar) GPOINTER_TO_INT (g_slist_nth_data (alphabet, j));
                  State *output_state = NULL;
                  GPtrArray *matched_equivalence_class =
                    dfa_fetch_matched_equivalence_class_from (equivalence_class,
                                                              final_equivalence_classes,
                                                              expected_character);

                  g_return_if_fail (matched_equivalence_class != NULL);

                  if (matched_equivalence_class->len == acceptable_scalar_equivalence_class_size)
                    output_state = g_ptr_array_index (matched_equivalence_class, 0);
                  else
                    output_state = fsm_get_or_create_composite_state (all_states,
                                                                      matched_equivalence_class,
                                                                      COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_ALL,
                                                                      NULL);

                  Transition *transition = create_deterministic_transition (expected_character,
                                                                            output_state);

                  g_ptr_array_add (transitions, transition);
                }

              g_object_set (input_state,
                            PROP_STATE_TRANSITIONS, transitions,
                            NULL);
            }
        }

      /* Reinitializing the DFA's states. */
      g_object_set (self,
                    PROP_FSM_INITIALIZABLE_ALL_STATES, all_states,
                    NULL);
    }
}

static GPtrArray *
dfa_fetch_equivalence_classes_from (GPtrArray *input_equivalence_classes,
                                    GSList    *alphabet)
{
  GPtrArray *current_equivalence_classes = g_ptr_array_new_with_free_func ((GDestroyNotify) g_ptr_array_unref);
  g_autoptr (GHashTable) states_for_removal = g_hash_table_new_full (g_direct_hash,
                                                                     g_direct_equal,
                                                                     NULL,
                                                                     (GDestroyNotify) g_array_unref);
  guint alphabet_length = g_slist_length (alphabet);

  for (guint i = 0; i < input_equivalence_classes->len; ++i)
    {
      GPtrArray *input_equivalence_class = g_ptr_array_index (input_equivalence_classes, i);

      if (g_collection_has_items (input_equivalence_class))
        {
          const guint candidate_class_states_count = 1;

          if (input_equivalence_class->len > candidate_class_states_count)
            {
              State *current_state = g_ptr_array_index (input_equivalence_class, 0);
              gboolean equivalence_class_difference_found = FALSE;
              GPtrArray *additional_equivalence_class = NULL;
              GArray *current_states_for_removal = NULL;

              for (guint j = 1; j < input_equivalence_class->len; ++j)
                {
                  State *next_state = g_ptr_array_index (input_equivalence_class, j);

                  for (guint k = 0; k < alphabet_length; ++k)
                    {
                      gchar expected_character = (gchar) GPOINTER_TO_INT (g_slist_nth_data (alphabet, k));

                      /* Moving the next state into an additional equivalence class and also marking it for removal
                       * from the input equivalence class in case it cannot transition to a member of the same
                       * equivalence class as the current state.
                       */
                      if (!dfa_can_states_transition_to_same_equivalence_class (current_state,
                                                                                next_state,
                                                                                expected_character,
                                                                                input_equivalence_classes))
                        {
                          if (!equivalence_class_difference_found)
                            {
                              additional_equivalence_class = g_ptr_array_new ();
                              current_states_for_removal = g_array_new (FALSE, FALSE, sizeof (gpointer));

                              equivalence_class_difference_found = TRUE;
                            }

                          g_ptr_array_add (additional_equivalence_class, next_state);
                          g_array_append_val (current_states_for_removal, next_state);

                          break;
                        }
                    }
                }

              if (equivalence_class_difference_found)
                {
                  g_ptr_array_add (current_equivalence_classes, additional_equivalence_class);
                  g_hash_table_insert (states_for_removal,
                                       input_equivalence_class,
                                       current_states_for_removal);
                }
            }
        }
    }

  if (g_collection_has_items (states_for_removal))
    for (guint i = 0; i < input_equivalence_classes->len; ++i)
      {
        GPtrArray *input_equivalence_class = g_ptr_array_index (input_equivalence_classes, i);
        GArray *current_states_for_removal = g_hash_table_lookup (states_for_removal,
                                                                  input_equivalence_class);

        if (g_collection_has_items (current_states_for_removal))
          for (guint j = 0; j < current_states_for_removal->len; ++j)
            {
              gpointer state_for_removal = g_array_index (current_states_for_removal, gpointer, j);

              g_ptr_array_remove_fast (input_equivalence_class, state_for_removal);
            }
      }

  g_ptr_array_add_range (current_equivalence_classes,
                         input_equivalence_classes,
                         (GRefFunc) g_ptr_array_ref);

  gboolean are_current_and_input_classes_equal = g_ptr_array_equal_with_equal_func (current_equivalence_classes,
                                                                                    input_equivalence_classes,
                                                                                    g_ptr_array_equal);

  g_ptr_array_unref (input_equivalence_classes);

  /* Returning the current call's result in case it is equal to the previous call's result. */
  if (are_current_and_input_classes_equal)
    return current_equivalence_classes;

  return dfa_fetch_equivalence_classes_from (current_equivalence_classes,
                                             alphabet);
}

static gboolean
dfa_can_states_transition_to_same_equivalence_class (State     *first_state,
                                                     State     *second_state,
                                                     gchar      expected_character,
                                                     GPtrArray *equivalence_classes)
{
  g_autoptr (GPtrArray) input_states = g_ptr_array_new ();

  g_ptr_array_add_multiple (input_states,
                            first_state, second_state,
                            NULL);

  g_autoptr (GPtrArray) output_states = fsm_fetch_output_states_from_multiple (input_states,
                                                                               expected_character);
  gboolean states_transition_to_same_state = output_states->len == 1;

  /* Avoiding further checks if both states output to the same state on a given input. */
  if (!states_transition_to_same_state)
    {
      g_autoptr (GPtrArray) output_states_equivalence_classes = g_ptr_array_new ();
      GEqualFunc state_equal_func = g_direct_equal;
      GEqualFunc equivalence_class_equal_func = g_direct_equal;
      const guint acceptable_equivalence_classes_count = 1;

      for (guint i = 0; i < output_states->len; ++i)
        {
          State *output_state = g_ptr_array_index (output_states, i);

          for (guint j = 0; j < equivalence_classes->len; ++j)
            {
              GPtrArray *equivalence_class = g_ptr_array_index (equivalence_classes, j);

              if (g_ptr_array_find_with_equal_func (equivalence_class,
                                                    output_state,
                                                    state_equal_func,
                                                    NULL))
                {
                  g_ptr_array_add_if_not_exists (output_states_equivalence_classes,
                                                 equivalence_class,
                                                 equivalence_class_equal_func,
                                                 NULL);

                  break;
                }
            }
        }

      return output_states_equivalence_classes->len == acceptable_equivalence_classes_count;
    }

  return TRUE;
}

static GPtrArray *
dfa_fetch_matched_equivalence_class_from (GPtrArray *input_equivalence_class,
                                          GPtrArray *all_equivalence_classes,
                                          gchar      expected_character)
{
  g_autoptr (GPtrArray) output_states = fsm_fetch_output_states_from_multiple (input_equivalence_class,
                                                                               expected_character);
  GEqualFunc state_equal_func = g_direct_equal;

  /* Forcing the dead state (if it exists) to appear at the end of the
   * output states array as other states should take precedence over it.
   */
  g_ptr_array_sort (output_states, (GCompareFunc) state_compare_deadness);

  for (guint i = 0; i < output_states->len; ++i)
    {
      State *output_state = g_ptr_array_index (output_states, i);

      for (guint j = 0; j < all_equivalence_classes->len; ++j)
        {
          GPtrArray *equivalence_class = g_ptr_array_index (all_equivalence_classes, j);

          if (g_ptr_array_find_with_equal_func (equivalence_class,
                                                output_state,
                                                state_equal_func,
                                                NULL))
            return equivalence_class;
        }
    }

  return NULL;
}

static void
dfa_dispose (GObject *object)
{
  DfaPrivate *priv = dfa_get_instance_private (ACCEPTORS_DFA (object));

  if (priv->current_state != NULL)
    g_clear_object (&priv->current_state);

  G_OBJECT_CLASS (dfa_parent_class)->dispose (object);
}
