#ifndef REGEXPERIENCE_MEALY_H
#define REGEXPERIENCE_MEALY_H

#include <glib-object.h>

#include "internal/state_machines/fsm.h"

G_BEGIN_DECLS

#define ACCEPTORS_TYPE_MEALY (mealy_get_type ())
#define mealy_new(...) (g_object_new (ACCEPTORS_TYPE_MEALY, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Mealy, mealy, TRANSDUCERS, MEALY, Fsm)

G_END_DECLS

#endif /* REGEXPERIENCE_MEALY_H */
