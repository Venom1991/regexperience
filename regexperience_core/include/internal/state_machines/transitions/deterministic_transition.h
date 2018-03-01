#ifndef REGEXPERIENCE_CORE_DETERMINISTIC_TRANSITION_H
#define REGEXPERIENCE_CORE_DETERMINISTIC_TRANSITION_H

#include <glib-object.h>

#include "transition.h"

G_BEGIN_DECLS

#define TRANSITIONS_TYPE_DETERMINISTIC_TRANSITION (deterministic_transition_get_type ())
#define deterministic_transition_new(...) (g_object_new (TRANSITIONS_TYPE_DETERMINISTIC_TRANSITION, ##__VA_ARGS__, NULL))

G_DECLARE_DERIVABLE_TYPE (DeterministicTransition,
                          deterministic_transition,
                          TRANSITIONS,
                          DETERMINISTIC_TRANSITION,
                          Transition)

struct _DeterministicTransitionClass
{
    TransitionClass parent_class;
};

#define PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE "output-state"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_DETERMINISTIC_TRANSITION_H */
