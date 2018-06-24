#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/state_machines/transitions/nondeterministic_transition.h"
#include <internal/state_machines/transitions/mealy_transition.h>

Transition *
create_nondeterministic_transition (gchar      expected_character,
                                    GPtrArray *output_states)
{
  EqualityConditionType equality_condition_type = EQUALITY_CONDITION_TYPE_UNDEFINED;

  if (expected_character == EPSILON)
    equality_condition_type = EQUALITY_CONDITION_TYPE_ANY;
  else
    equality_condition_type = EQUALITY_CONDITION_TYPE_EQUAL;

  return nondeterministic_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                                          PROP_TRANSITION_EQUALITY_CONDITION_TYPE, equality_condition_type,
                                          PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES, output_states);
}

Transition *
create_nondeterministic_epsilon_transition (GPtrArray *output_states)
{
  return nondeterministic_transition_new (PROP_TRANSITION_REQUIRES_INPUT, FALSE,
                                          PROP_TRANSITION_EQUALITY_CONDITION_TYPE, EQUALITY_CONDITION_TYPE_ANY,
                                          PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES, output_states);
}

Transition *
create_deterministic_transition (gchar  expected_character,
                                 State *output_state)
{
  EqualityConditionType equality_condition_type = EQUALITY_CONDITION_TYPE_UNDEFINED;

  if (expected_character == EPSILON)
    equality_condition_type = EQUALITY_CONDITION_TYPE_ANY;
  else
    equality_condition_type = EQUALITY_CONDITION_TYPE_EQUAL;

  return deterministic_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                                       PROP_TRANSITION_EQUALITY_CONDITION_TYPE, equality_condition_type,
                                       PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state);
}

Transition *
create_deterministic_epsilon_transition (State *output_state)
{
  return deterministic_transition_new (PROP_TRANSITION_REQUIRES_INPUT, FALSE,
                                       PROP_TRANSITION_EQUALITY_CONDITION_TYPE, EQUALITY_CONDITION_TYPE_ANY,
                                       PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state);
}

Transition *
create_mealy_transition (gchar     expected_character,
                         State    *output_state,
                         gpointer  output_data)
{
  EqualityConditionType equality_condition_type = EQUALITY_CONDITION_TYPE_UNDEFINED;

  if (expected_character == EPSILON)
    equality_condition_type = EQUALITY_CONDITION_TYPE_ANY;
  else
    equality_condition_type = EQUALITY_CONDITION_TYPE_EQUAL;

  return mealy_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                               PROP_TRANSITION_EQUALITY_CONDITION_TYPE, equality_condition_type,
                               PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state,
                               PROP_MEALY_TRANSITION_OUTPUT_DATA, output_data);
}
