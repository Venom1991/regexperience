#ifndef REGEXPERIENCE_FSM_INITIALIZABLE_H
#define REGEXPERIENCE_FSM_INITIALIZABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_FSM_INITIALIZABLE (fsm_initializable_get_type ())

G_DECLARE_INTERFACE (FsmInitializable, fsm_initializable, STATE_MACHINES, FSM_INITIALIZABLE, GObject)

struct _FsmInitializableInterface
{
  GTypeInterface parent_iface;
};

#define PROP_FSM_INITIALIZABLE_ALL_STATES       "all-states"
#define PROP_FSM_INITIALIZABLE_START_STATE      "start-state"
#define PROP_FSM_INITIALIZABLE_FINAL_STATES     "final-states"
#define PROP_FSM_INITIALIZABLE_NON_FINAL_STATES "non-final-states"
#define PROP_FSM_INITIALIZABLE_ALPHABET         "alphabet"

G_END_DECLS

#endif /* REGEXPERIENCE_FSM_INITIALIZABLE_H */
