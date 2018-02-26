#ifndef REGEXPERIENCE_CORE_DFA_H
#define REGEXPERIENCE_CORE_DFA_H

#include <glib-object.h>

#include "acceptor.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_DFA (dfa_get_type ())
#define dfa_new(...) (g_object_new (STATE_MACHINES_TYPE_DFA, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Dfa, dfa, STATE_MACHINES, DFA, Acceptor)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_DFA_H */
