#ifndef REGEXPERIENCE_CORE_DETERMINISTIC_TRANSITION_H
#define REGEXPERIENCE_CORE_DETERMINISTIC_TRANSITION_H

#include <glib-object.h>

#include "transition.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_DETERMINISTIC_TRANSITION (deterministic_transition_get_type ())
#define deterministic_transition_new(...) (g_object_new (STATE_MACHINES_TYPE_DETERMINISTIC_TRANSITION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (DeterministicTransition,
                      deterministic_transition,
                      STATE_MACHINES,
                      DETERMINISTIC_TRANSITION,
                      Transition)

#define PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE "output-state"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_DETERMINISTIC_TRANSITION_H */
