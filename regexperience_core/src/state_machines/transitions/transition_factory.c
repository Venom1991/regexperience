#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/state_machines/transitions/deterministic_transition.h"
#include "internal/state_machines/transitions/nondeterministic_transition.h"

Transition *create_deterministic_transition (gchar  expected_character,
                                             State *output_state)
{
  return deterministic_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                                       PROP_TRANSITION_EQUALITY_CONDITION_TYPE, EQUALITY_CONDITION_TYPE_EQUAL,
                                       PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state);
}

Transition *create_deterministic_epsilon_transition (State *output_state)
{
  return deterministic_transition_new (PROP_TRANSITION_REQUIRES_INPUT, FALSE,
                                       PROP_TRANSITION_EQUALITY_CONDITION_TYPE, EQUALITY_CONDITION_TYPE_ANY,
                                       PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state);
}

Transition *create_deterministic_unconditional_transition (State *output_state)
{
  return deterministic_transition_new (PROP_TRANSITION_EQUALITY_CONDITION_TYPE, EQUALITY_CONDITION_TYPE_ANY,
                                       PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state);
}

Transition *create_nondeterministic_transition (gchar      expected_character,
                                                GPtrArray *output_states)
{
  return nondeterministic_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                                          PROP_TRANSITION_EQUALITY_CONDITION_TYPE, EQUALITY_CONDITION_TYPE_EQUAL,
                                          PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES, output_states);
}

Transition *create_nondeterministic_epsilon_transition (GPtrArray *output_states)
{
  return nondeterministic_transition_new (PROP_TRANSITION_REQUIRES_INPUT, FALSE,
                                          PROP_TRANSITION_EQUALITY_CONDITION_TYPE, EQUALITY_CONDITION_TYPE_ANY,
                                          PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES, output_states);
}
