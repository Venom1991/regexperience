#ifndef REGEXPERIENCE_MEALY_TRANSITION_H
#define REGEXPERIENCE_MEALY_TRANSITION_H

#include "deterministic_transition.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define TRANSITIONS_TYPE_MEALY_TRANSITION (mealy_transition_get_type ())
#define mealy_transition_new(...) (g_object_new (TRANSITIONS_TYPE_MEALY_TRANSITION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (MealyTransition,
                      mealy_transition,
                      TRANSITIONS,
                      MEALY_TRANSITION,
                      DeterministicTransition)

#define PROP_MEALY_TRANSITION_OUTPUT_DATA "output-data"

G_END_DECLS

#endif /* REGEXPERIENCE_MEALY_TRANSITION_H */
