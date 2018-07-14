#ifndef REGEXPERIENCE_NFA_H
#define REGEXPERIENCE_NFA_H

#include "internal/state_machines/fsm.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define ACCEPTORS_TYPE_NFA (nfa_get_type ())
#define nfa_new(...) (g_object_new (ACCEPTORS_TYPE_NFA, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Nfa, nfa, ACCEPTORS, NFA, Fsm)

G_END_DECLS

#endif /* REGEXPERIENCE_NFA_H */
