#ifndef REGEXPERIENCE_CORE_STATE_MACHINE_INITIALIZABLE_H
#define REGEXPERIENCE_CORE_STATE_MACHINE_INITIALIZABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_INITIALIZABLE (state_machine_initializable_get_type ())

G_DECLARE_INTERFACE (StateMachineInitializable, state_machine_initializable, STATE_MACHINES, INITIALIZABLE, GObject)

struct _StateMachineInitializableInterface
{
    GTypeInterface parent_iface;
};

#define PROP_STATE_MACHINE_INITIALIZABLE_ALL_STATES       "all-states"
#define PROP_STATE_MACHINE_INITIALIZABLE_START_STATE      "start-state"
#define PROP_STATE_MACHINE_INITIALIZABLE_FINAL_STATES     "final-states"
#define PROP_STATE_MACHINE_INITIALIZABLE_NON_FINAL_STATES "non-final-states"
#define PROP_STATE_MACHINE_INITIALIZABLE_ALPHABET         "alphabet"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_STATE_MACHINE_INITIALIZABLE_H */
