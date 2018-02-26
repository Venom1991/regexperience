#ifndef REGEXPERIENCE_CORE_EPSILON_NFA_H
#define REGEXPERIENCE_CORE_EPSILON_NFA_H

#include <glib-object.h>

#include "acceptor.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_EPSILON_NFA (epsilon_nfa_get_type ())
#define epsilon_nfa_new(...) (g_object_new (STATE_MACHINES_TYPE_EPSILON_NFA, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (EpsilonNfa, epsilon_nfa, STATE_MACHINES, EPSILON_NFA, Acceptor)

#define PROP_EPSILON_NFA_FINAL_STATE "final-state"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_EPSILON_NFA_H */
