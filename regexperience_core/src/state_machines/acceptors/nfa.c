#include "internal/state_machines/acceptors/nfa.h"
#include "internal/state_machines/acceptors/dfa.h"
#include "internal/state_machines/state_machine_initializable.h"
#include "internal/state_machines/state_machine_convertible.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _Nfa
{
    Acceptor parent_instance;
};

static void nfa_state_machine_convertible_interface_init (StateMachineConvertibleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (Nfa, nfa, STATE_MACHINES_TYPE_ACCEPTOR,
                         G_IMPLEMENT_INTERFACE (STATE_MACHINES_TYPE_CONVERTIBLE,
                                                nfa_state_machine_convertible_interface_init))

static StateMachineConvertible *nfa_compute_epsilon_closures (StateMachineConvertible *self);

static StateMachineModifiable *nfa_construct_subset (StateMachineConvertible *self);

static void nfa_define_dfa_states_from_scalar (State     *input_state,
                                               GSList    *alphabet,
                                               GPtrArray *dfa_states);

static void nfa_define_dfa_states_from_composite (State                 *input_state,
                                                  GPtrArray             *composed_from_states,
                                                  GSList                *alphabet,
                                                  GPtrArray             *dfa_states);

static void nfa_define_transitions_for_dfa_state (GPtrArray             *output_states,
                                                  GSList                *alphabet,
                                                  gchar                  expected_character,
                                                  GPtrArray             *dfa_states,
                                                  GPtrArray             *dfa_transitions);

static void
nfa_class_init (NfaClass *klass)
{
  /* NOP */
}

static void
nfa_init (Nfa *self)
{
  /* NOP */
}

static void
nfa_state_machine_convertible_interface_init (StateMachineConvertibleInterface *iface)
{
  iface->compute_epsilon_closures = nfa_compute_epsilon_closures;
  iface->construct_subset = nfa_construct_subset;
}

static StateMachineConvertible *
nfa_compute_epsilon_closures (StateMachineConvertible *self)
{
  g_return_val_if_fail (STATE_MACHINES_IS_NFA (self), NULL);

  g_return_val_if_reached (NULL);
}

static StateMachineModifiable *
nfa_construct_subset (StateMachineConvertible *self)
{
  g_return_val_if_fail (STATE_MACHINES_IS_NFA (self), NULL);

  GSList *alphabet = NULL;
  g_autoptr (State) start_state = NULL;
  g_autoptr (GPtrArray) dfa_states = g_ptr_array_new_with_free_func (g_object_unref);

  g_object_get (self,
                PROP_STATE_MACHINE_INITIALIZABLE_ALPHABET, &alphabet,
                PROP_STATE_MACHINE_INITIALIZABLE_START_STATE, &start_state,
                NULL);

  /* Beginning the conversion using the NFA's start state. */
  nfa_define_dfa_states_from_scalar (start_state,
                                     alphabet,
                                     dfa_states);

  return dfa_new (PROP_STATE_MACHINE_INITIALIZABLE_ALL_STATES, dfa_states);
}

static void
nfa_define_dfa_states_from_scalar (State     *input_state,
                                   GSList    *alphabet,
                                   GPtrArray *dfa_states)
{
  /* Increasing the reference count due to the input state actually being a reused NFA state. */
  g_ptr_array_add (dfa_states, g_object_ref (input_state));

  guint alphabet_length = g_slist_length (alphabet);
  g_autoptr (GPtrArray) dfa_transitions = g_ptr_array_new_with_free_func (g_object_unref);

  for (guint i = 0; i < alphabet_length; ++i)
    {
      gchar expected_character = (gchar) GPOINTER_TO_INT (g_slist_nth_data (alphabet, i));
      EqualityConditionType condition_type = EQUALITY_CONDITION_TYPE_EQUAL;
      g_autoptr (GPtrArray) output_states = acceptor_fetch_output_states_from_single (input_state,
                                                                                      expected_character);

      nfa_define_transitions_for_dfa_state (output_states,
                                            alphabet,
                                            expected_character,
                                            dfa_states,
                                            dfa_transitions);
    }

  g_object_set (input_state,
                PROP_STATE_TRANSITIONS, dfa_transitions,
                NULL);
}

static void
nfa_define_dfa_states_from_composite (State                 *input_state,
                                      GPtrArray             *composed_from_states,
                                      GSList                *alphabet,
                                      GPtrArray             *dfa_states)
{
  guint alphabet_length = g_slist_length (alphabet);
  g_autoptr (GPtrArray) dfa_transitions = g_ptr_array_new_with_free_func (g_object_unref);

  for (guint i = 0; i < alphabet_length; ++i)
    {
      gchar expected_character = (gchar) GPOINTER_TO_INT (g_slist_nth_data (alphabet, i));

      /* Fetching the union of states that represent the output of each state (on a given input)
       * found in states which were used to construct the composite state.
       */
      g_autoptr (GPtrArray) composed_from_states_output_states =
          acceptor_fetch_output_states_from_multiple (composed_from_states,
                                                      expected_character);

      nfa_define_transitions_for_dfa_state (composed_from_states_output_states,
                                            alphabet,
                                            expected_character,
                                            dfa_states,
                                            dfa_transitions);
    }

  g_object_set (input_state,
                PROP_STATE_TRANSITIONS, dfa_transitions,
                NULL);
}

static void
nfa_define_transitions_for_dfa_state (GPtrArray             *output_states,
                                      GSList                *alphabet,
                                      gchar                  expected_character,
                                      GPtrArray             *dfa_states,
                                      GPtrArray             *dfa_transitions)
{
  if (g_ptr_array_has_items (output_states))
    {
      const guint acceptable_scalar_output_states_count = 1;

      if (output_states->len == acceptable_scalar_output_states_count)
        {
          State *output_state = g_ptr_array_index (output_states, 0);

          Transition *dfa_transition = create_deterministic_transition (expected_character,
                                                                        output_state);

          g_ptr_array_add (dfa_transitions, dfa_transition);

          /* Avoid defining states from a scalar state that is already found in states intended for the DFA. */
          if (!g_ptr_array_find (dfa_states, output_state, NULL))
            nfa_define_dfa_states_from_scalar (output_state,
                                               alphabet,
                                               dfa_states);
        }
      else
        {
          /* Constructing a new composite state or getting an existing one which was constructed
           * using the exact same output states.
           */
          gboolean already_existed;
          State *composite_state = acceptor_get_or_create_composite_state (dfa_states,
                                                                           output_states,
                                                                           COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_MODE_FINAL,
                                                                           &already_existed);

          Transition *dfa_transition = create_deterministic_transition (expected_character,
                                                                        composite_state);

          g_ptr_array_add (dfa_transitions, dfa_transition);

          /* Avoid defining states from a composite state that is already found in states intended for the DFA. */
          if (!already_existed)
            nfa_define_dfa_states_from_composite (composite_state,
                                                  output_states,
                                                  alphabet,
                                                  dfa_states);
        }
    }
  else
    {
      /* Defining a transition to a dead state in case there are no output states available.
       * At most one dead state is required for any given state machine.
       */
      State *dead_state = acceptor_get_or_create_dead_state (dfa_states);
      Transition *dfa_transition = create_deterministic_transition (expected_character,
                                                                    dead_state);

      g_ptr_array_add (dfa_transitions, dfa_transition);
    }
}