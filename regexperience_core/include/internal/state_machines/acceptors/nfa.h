#ifndef REGEXPERIENCE_CORE_NFA_H
#define REGEXPERIENCE_CORE_NFA_H

#include <glib-object.h>

#include "acceptor.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_NFA (nfa_get_type ())
#define nfa_new(...) (g_object_new (STATE_MACHINES_TYPE_NFA, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Nfa, nfa, STATE_MACHINES, NFA, Acceptor)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_NFA_H */
