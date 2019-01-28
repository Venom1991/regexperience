#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/state_machines/transitions/nondeterministic_transition.h"
#include "internal/state_machines/transitions/mealy_transition.h"

void
initialize_acceptor_transition_properties (gchar                  expected_character,
                                           EqualityConditionType *condition_type,
                                           gboolean              *requires_input)
{
  g_return_if_fail (condition_type != NULL);
  g_return_if_fail (requires_input != NULL);

  switch (expected_character)
    {
    case EPSILON:
      *condition_type = EQUALITY_CONDITION_TYPE_ANY;
      *requires_input = FALSE;
      break;

    case ANY:
      *condition_type = EQUALITY_CONDITION_TYPE_ANY;
      *requires_input = TRUE;
      break;

    default:
      *condition_type = EQUALITY_CONDITION_TYPE_EQUAL;
      *requires_input = TRUE;
      break;
    }
}

Transition *
create_nondeterministic_transition (gchar      expected_character,
                                    GPtrArray *output_states)
{
  EqualityConditionType condition_type = EQUALITY_CONDITION_TYPE_UNDEFINED;
  gboolean requires_input = FALSE;

  initialize_acceptor_transition_properties (expected_character,
                                             &condition_type,
                                             &requires_input);

  return nondeterministic_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                                          PROP_TRANSITION_REQUIRES_INPUT, requires_input,
                                          PROP_TRANSITION_EQUALITY_CONDITION_TYPE, condition_type,
                                          PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES, output_states);
}

Transition *
create_deterministic_transition (gchar  expected_character,
                                 State *output_state)
{
  EqualityConditionType condition_type = EQUALITY_CONDITION_TYPE_UNDEFINED;
  gboolean requires_input = FALSE;

  initialize_acceptor_transition_properties (expected_character,
                                             &condition_type,
                                             &requires_input);

  return deterministic_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                                       PROP_TRANSITION_REQUIRES_INPUT, requires_input,
                                       PROP_TRANSITION_EQUALITY_CONDITION_TYPE, condition_type,
                                       PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state);
}

Transition *
create_mealy_transition (gchar     expected_character,
                         State    *output_state,
                         gpointer  output_data)
{
  EqualityConditionType condition_type = EQUALITY_CONDITION_TYPE_UNDEFINED;

  if (expected_character == ANY)
    condition_type = EQUALITY_CONDITION_TYPE_ANY;
  else
    condition_type = EQUALITY_CONDITION_TYPE_EQUAL;

  return mealy_transition_new (PROP_TRANSITION_EXPECTED_CHARACTER, expected_character,
                               PROP_TRANSITION_REQUIRES_INPUT, TRUE,
                               PROP_TRANSITION_EQUALITY_CONDITION_TYPE, condition_type,
                               PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, output_state,
                               PROP_MEALY_TRANSITION_OUTPUT_DATA, output_data);
}
