#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/acceptors/nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/fsm_convertible.h"
#include "internal/state_machines/transitions/deterministic_transition.h"
#include "internal/state_machines/transitions/nondeterministic_transition.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _EpsilonNfa
{
  Fsm parent_instance;
};

enum
{
  PROP_FINAL_STATE = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

typedef enum
{
  INITIALIZE_OUTPUT_STATES_MODE_UNDEFINED,
  INITIALIZE_OUTPUT_STATES_MODE_EPSILON_INITIAL,
  INITIALIZE_OUTPUT_STATES_MODE_EXPLICIT_CHARACTER,
  INITIALIZE_OUTPUT_STATES_MODE_EPSILON_SUBSEQUENT,
} InitializeOutputStatesMode;

static void epsilon_nfa_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec);

static void            epsilon_nfa_fsm_convertible_interface_init          (FsmConvertibleInterface     *iface);

static FsmConvertible *epsilon_nfa_compute_epsilon_closures                (FsmConvertible              *self);

static FsmModifiable  *epsilon_nfa_construct_subset                        (FsmConvertible              *self);

static gboolean        epsilon_nfa_has_epsilon_transitions                 (GPtrArray                   *all_states);

static Transition     *epsilon_nfa_build_epsilon_closed_transition         (State                       *state,
                                                                            gchar                        explicit_character,
                                                                            GHashTable                  *input_output_combinations);

static void            epsilon_nfa_initialize_epsilon_closed_output_states (GPtrArray                   *input_states,
                                                                            GPtrArray                  **epsilon_closed_transition_output_states,
                                                                            gchar                        explicit_character,
                                                                            GPtrArray                   *visited_states,
                                                                            GHashTable                  *input_output_combinations,
                                                                            InitializeOutputStatesMode   initialize_output_states_mode);

static void            mark_input_state_as_final_if_needed                 (State                       *input_state,
                                                                            GPtrArray                   *transitive_epsilon_closed_output_states);

G_DEFINE_TYPE_WITH_CODE (EpsilonNfa, epsilon_nfa, STATE_MACHINES_TYPE_FSM,
                         G_IMPLEMENT_INTERFACE (STATE_MACHINES_TYPE_FSM_CONVERTIBLE,
                                                epsilon_nfa_fsm_convertible_interface_init))

static void
epsilon_nfa_class_init (EpsilonNfaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = epsilon_nfa_get_property;

  obj_properties[PROP_FINAL_STATE] =
    g_param_spec_object (PROP_EPSILON_NFA_FINAL_STATE,
                         "Final state",
                         "Epsilon NFA's sole final state.",
                         STATE_MACHINES_TYPE_STATE,
                         G_PARAM_READABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
epsilon_nfa_init (EpsilonNfa *self)
{
  /* NOP */
}

static void
epsilon_nfa_fsm_convertible_interface_init (FsmConvertibleInterface *iface)
{
  iface->compute_epsilon_closures = epsilon_nfa_compute_epsilon_closures;
  iface->construct_subset = epsilon_nfa_construct_subset;
}

static void
epsilon_nfa_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  switch (property_id)
    {
    case PROP_FINAL_STATE:
      {
        g_autoptr (GPtrArray) final_states = NULL;
        const guint acceptable_epsilon_nfa_final_states_count = 1;

        g_object_get (object,
                      PROP_FSM_INITIALIZABLE_FINAL_STATES, &final_states,
                      NULL);

        g_return_if_fail (g_collection_has_items (final_states));
        g_return_if_fail (final_states->len == acceptable_epsilon_nfa_final_states_count);

        g_value_set_object (value, g_ptr_array_index (final_states, 0));
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static FsmConvertible *
epsilon_nfa_compute_epsilon_closures (FsmConvertible *self)
{
  g_return_val_if_fail (ACCEPTORS_IS_EPSILON_NFA (self), NULL);

  GSList *alphabet = NULL;
  g_autoptr (GPtrArray) all_states = NULL;

  g_object_get (self,
                PROP_FSM_INITIALIZABLE_ALPHABET, &alphabet,
                PROP_FSM_INITIALIZABLE_ALL_STATES, &all_states,
                NULL);

  /* Performing the computations only if there is an actual need to do so - i.e., the state machine contains
   * at least one epsilon transition.
   */
  if (epsilon_nfa_has_epsilon_transitions (all_states))
    {
      guint alphabet_length = g_slist_length (alphabet);
      g_autoptr (GHashTable) input_output_combinations = g_hash_table_new_full (g_str_hash,
                                                                                g_str_equal,
                                                                                g_free,
                                                                                (GDestroyNotify) g_ptr_array_unref);

      for (guint i = 0; i < all_states->len; ++i)
        {
          State *state = g_ptr_array_index (all_states, i);
          g_autoptr (GPtrArray) nfa_transitions = NULL;

          for (guint j = 0; j < alphabet_length; ++j)
            {
              gchar expected_character = (gchar) GPOINTER_TO_INT (g_slist_nth_data (alphabet, j));
              Transition *epsilon_closed_transition =
                epsilon_nfa_build_epsilon_closed_transition (state,
                                                             expected_character,
                                                             input_output_combinations);

              if (epsilon_closed_transition != NULL)
                {
                  if (nfa_transitions == NULL)
                    nfa_transitions = g_ptr_array_new_with_free_func (g_object_unref);

                  g_ptr_array_add (nfa_transitions, epsilon_closed_transition);
                }
            }

          g_object_set (state,
                        PROP_STATE_TRANSITIONS, nfa_transitions,
                        NULL);
        }
    }

  return nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, all_states);
}

static FsmModifiable *
epsilon_nfa_construct_subset (FsmConvertible *self)
{
  g_return_val_if_fail (ACCEPTORS_IS_EPSILON_NFA (self), NULL);
  g_return_val_if_reached (NULL);
}

static gboolean
epsilon_nfa_has_epsilon_transitions (GPtrArray *all_states)
{
  for (guint i = 0; i < all_states->len; ++i)
    {
      State *state = g_ptr_array_index (all_states, i);
      g_autoptr (GPtrArray) transitions = NULL;

      g_object_get (state,
                    PROP_STATE_TRANSITIONS, &transitions,
                    NULL);

      if (g_collection_has_items (transitions))
        {
          for (guint j = 0; j < transitions->len; ++j)
            {
              Transition *transition = g_ptr_array_index (transitions, j);

              if (transition_is_epsilon (transition))
                return TRUE;
            }
        }
    }

  return FALSE;
}

static Transition *
epsilon_nfa_build_epsilon_closed_transition (State      *state,
                                             gchar       explicit_character,
                                             GHashTable *input_output_combinations)
{
  g_autoptr (GPtrArray) initial_input_states = g_ptr_array_new ();
  g_autoptr (GPtrArray) visited_states = g_ptr_array_new ();
  g_autoptr (GPtrArray) epsilon_closed_transition_output_states = NULL;

  g_ptr_array_add (initial_input_states, state);

  epsilon_nfa_initialize_epsilon_closed_output_states (initial_input_states,
                                                       &epsilon_closed_transition_output_states,
                                                       explicit_character,
                                                       visited_states,
                                                       input_output_combinations,
                                                       INITIALIZE_OUTPUT_STATES_MODE_EPSILON_INITIAL);

  if (g_collection_has_items (epsilon_closed_transition_output_states))
    {
      const guint acceptable_deterministic_output_states_count = 1;

      if (epsilon_closed_transition_output_states->len == acceptable_deterministic_output_states_count)
        {
          State *epsilon_closed_transition_output_state = g_ptr_array_index (epsilon_closed_transition_output_states, 0);

          return create_deterministic_transition (explicit_character,
                                                  epsilon_closed_transition_output_state);
        }
      else
        {
          return create_nondeterministic_transition (explicit_character,
                                                     epsilon_closed_transition_output_states);
        }
    }

  return NULL;
}

static void
epsilon_nfa_initialize_epsilon_closed_output_states (GPtrArray                   *input_states,
                                                     GPtrArray                  **epsilon_closed_transition_output_states,
                                                     gchar                        explicit_character,
                                                     GPtrArray                   *visited_states,
                                                     GHashTable                  *input_output_combinations,
                                                     InitializeOutputStatesMode   initialize_output_states_mode)
{
  g_return_if_fail (visited_states != NULL);
  g_return_if_fail (input_output_combinations != NULL);

  gboolean is_epsilon_step = FALSE;
  gboolean is_explicit_character_step = FALSE;
  gboolean is_final_step = FALSE;
  GEqualFunc state_equal_func = g_direct_equal;
  g_autoptr (GPtrArray) current_closure_step_output_states = NULL;

  switch (initialize_output_states_mode)
    {
    case INITIALIZE_OUTPUT_STATES_MODE_EPSILON_INITIAL:
      is_epsilon_step = TRUE;
      break;

    case INITIALIZE_OUTPUT_STATES_MODE_EXPLICIT_CHARACTER:
      is_explicit_character_step = TRUE;
      break;

    case INITIALIZE_OUTPUT_STATES_MODE_EPSILON_SUBSEQUENT:
      is_epsilon_step = TRUE;
      is_final_step = TRUE;
      break;

    default:
      g_return_if_reached ();
    }

  if (g_collection_has_items (input_states))
    {
      gchar input_character = 0;

      current_closure_step_output_states = g_ptr_array_new ();

      if (is_epsilon_step)
        input_character = EPSILON;
      else if (is_explicit_character_step)
        input_character = explicit_character;

      for (guint i = 0; i < input_states->len; ++i)
        {
          State *input_state = g_ptr_array_index (input_states, i);
          g_autofree gchar *input = g_strdup_printf ("%p" DELIMITER "%hhx",
                                                     (gpointer) input_state, input_character);
          GPtrArray *current_iteration_output_states = g_hash_table_lookup (input_output_combinations,
                                                                            input);
          gboolean transition_cycle_detected = FALSE;

          /* Avoiding the computation that was performed in one of the previous calls. */
          if (current_iteration_output_states == NULL)
            {
              current_iteration_output_states = g_ptr_array_new ();

              if (is_epsilon_step)
                {
                  /* Every state can, by definition, reach itself using solely an epsilon transition. */
                  g_ptr_array_add (current_iteration_output_states, input_state);

                  /* Keeping track of previously visited states so as to break transition cycles and
                   * to consequently avoid infinite recursion.
                   */
                  if (g_ptr_array_find_with_equal_func (visited_states,
                                                        input_state,
                                                        state_equal_func,
                                                        NULL))
                    transition_cycle_detected = TRUE;
                  else
                    g_ptr_array_add (visited_states, input_state);
                }

              if (!transition_cycle_detected)
                {
                  g_autoptr (GPtrArray) transitions = NULL;

                  g_object_get (input_state,
                                PROP_STATE_TRANSITIONS, &transitions,
                                NULL);

                  if (g_collection_has_items (transitions))
                    {
                      for (guint j = 0; j < transitions->len; ++j)
                        {
                          Transition *transition = g_ptr_array_index (transitions, j);
                          g_autoptr (GPtrArray) transition_output_states = g_ptr_array_new ();

                          transition_supplement_states_array_with_output (transition,
                                                                          transition_output_states);

                          if (g_collection_has_items (transition_output_states))
                            {
                              if (is_epsilon_step && transition_is_epsilon (transition))
                                {
                                  g_autoptr (GPtrArray) transitive_epsilon_closed_output_states = NULL;

                                  g_ptr_array_add_range_distinct (current_iteration_output_states,
                                                                  transition_output_states,
                                                                  state_equal_func,
                                                                  NULL);

                                  /* Recursively finding all the states that are reachable from the input state
                                   * using epsilon transitions only.
                                   */
                                  epsilon_nfa_initialize_epsilon_closed_output_states (transition_output_states,
                                                                                       &transitive_epsilon_closed_output_states,
                                                                                       EPSILON,
                                                                                       visited_states,
                                                                                       input_output_combinations,
                                                                                       INITIALIZE_OUTPUT_STATES_MODE_EPSILON_SUBSEQUENT);

                                  if (g_collection_has_items (transitive_epsilon_closed_output_states))
                                    {
                                      g_ptr_array_add_range_distinct (current_iteration_output_states,
                                                                      transitive_epsilon_closed_output_states,
                                                                      state_equal_func,
                                                                      NULL);

                                      /* Each state that can reach a final state using only epsilon transitions must
                                       * be marked as final itself.
                                       */
                                      mark_input_state_as_final_if_needed (input_state,
                                                                           transitive_epsilon_closed_output_states);
                                    }
                                }
                              else if (is_explicit_character_step && transition_is_possible (transition, input_character))
                                {
                                  g_ptr_array_add_range_distinct (current_iteration_output_states,
                                                                  transition_output_states,
                                                                  state_equal_func,
                                                                  NULL);
                                }
                            }
                        }
                    }

                  /* Keeping track of performed computations. */
                  g_hash_table_insert (input_output_combinations,
                                       g_strdup (input),
                                       current_iteration_output_states);
                }
            }

          if (g_collection_has_items (current_iteration_output_states))
            g_ptr_array_add_range_distinct (current_closure_step_output_states,
                                            current_iteration_output_states,
                                            state_equal_func,
                                            NULL);

          if (transition_cycle_detected)
            g_ptr_array_unref (current_iteration_output_states);
        }
    }

  /* Initializing the resultant array and moving the final closure output states into it (last recursive call). */
  if (is_final_step)
    {
      if (g_collection_has_items (current_closure_step_output_states))
        {
          *epsilon_closed_transition_output_states = g_ptr_array_new ();

          g_ptr_array_add_range_distinct (*epsilon_closed_transition_output_states,
                                          current_closure_step_output_states,
                                          state_equal_func,
                                          NULL);
        }

      return;
    }

  /* Moving on to the next step using the current step's result. */
  epsilon_nfa_initialize_epsilon_closed_output_states (current_closure_step_output_states,
                                                       epsilon_closed_transition_output_states,
                                                       explicit_character,
                                                       visited_states,
                                                       input_output_combinations,
                                                       ++initialize_output_states_mode);
}

static void
mark_input_state_as_final_if_needed (State     *input_state,
                                     GPtrArray *transitive_epsilon_closed_output_states)
{
  StateTypeFlags input_state_type_flags = STATE_TYPE_UNDEFINED;
  StateTypeFlags default_flag = STATE_TYPE_DEFAULT;
  StateTypeFlags final_flag = STATE_TYPE_FINAL;

  g_object_get (input_state,
                PROP_STATE_TYPE_FLAGS, &input_state_type_flags,
                NULL);

  /* Skipping the checks in case the input state is already a final one. */
  if (!(input_state_type_flags & final_flag))
    {
      for (guint i = 0; i < transitive_epsilon_closed_output_states->len; ++i)
        {
          State *current_state = g_ptr_array_index (transitive_epsilon_closed_output_states, i);
          StateTypeFlags current_state_type_flags = STATE_TYPE_UNDEFINED;

          g_object_get (current_state,
                        PROP_STATE_TYPE_FLAGS, &current_state_type_flags,
                        NULL);

          if (current_state_type_flags & final_flag)
            {
              input_state_type_flags |= final_flag;
              input_state_type_flags &= ~default_flag;

              g_object_set (input_state,
                            PROP_STATE_TYPE_FLAGS, input_state_type_flags,
                            NULL);

              return;
            }
        }
    }
}
