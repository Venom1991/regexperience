#ifndef REGEXPERIENCE_CORE_DFA_H
#define REGEXPERIENCE_CORE_DFA_H

#include <glib-object.h>

#include "internal/state_machines/fsm.h"

G_BEGIN_DECLS

#define ACCEPTORS_TYPE_DFA (dfa_get_type ())
#define dfa_new(...) (g_object_new (ACCEPTORS_TYPE_DFA, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Dfa, dfa, ACCEPTORS, DFA, Fsm)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_DFA_H */
