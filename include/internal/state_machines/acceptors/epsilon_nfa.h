#ifndef REGEXPERIENCE_EPSILON_NFA_H
#define REGEXPERIENCE_EPSILON_NFA_H

#include "internal/state_machines/fsm.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define ACCEPTORS_TYPE_EPSILON_NFA (epsilon_nfa_get_type ())
#define epsilon_nfa_new(...) (g_object_new (ACCEPTORS_TYPE_EPSILON_NFA, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (EpsilonNfa, epsilon_nfa, ACCEPTORS, EPSILON_NFA, Fsm)

#define PROP_EPSILON_NFA_FINAL_STATE "final-state"

G_END_DECLS

#endif /* REGEXPERIENCE_EPSILON_NFA_H */
