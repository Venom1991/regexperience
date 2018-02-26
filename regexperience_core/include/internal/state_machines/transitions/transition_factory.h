#ifndef REGEXPERIENCE_CORE_TRANSITION_FACTORY_H
#define REGEXPERIENCE_CORE_TRANSITION_FACTORY_H

#include "internal/state_machines/transitions/transition.h"
#include "internal/state_machines/state.h"

Transition *create_deterministic_transition (gchar  expected_character,
                                             State *output_state);

Transition *create_deterministic_epsilon_transition (State *output_state);

Transition *create_deterministic_unconditional_transition (State *output_state);

Transition *create_nondeterministic_transition (gchar      expected_character,
                                                GPtrArray *output_states);

Transition *create_nondeterministic_epsilon_transition (GPtrArray *output_states);

#endif /* REGEXPERIENCE_CORE_TRANSITION_FACTORY_H */