#ifndef REGEXPERIENCE_CORE_NONDETERMINISTIC_TRANSITION_H
#define REGEXPERIENCE_CORE_NONDETERMINISTIC_TRANSITION_H

#include <glib-object.h>

#include "transition.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_NONDETERMINISTIC_TRANSITION (nondeterministic_transition_get_type ())
#define nondeterministic_transition_new(...) (g_object_new (STATE_MACHINES_TYPE_NONDETERMINISTIC_TRANSITION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (NondeterministicTransition,
                      nondeterministic_transition,
                      STATE_MACHINES,
                      NONDETERMINISTIC_TRANSITION,
                      Transition)

#define PROP_NONDETERMINISTIC_TRANSITION_OUTPUT_STATES "output-states"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_NONDETERMINISTIC_TRANSITION_H */
