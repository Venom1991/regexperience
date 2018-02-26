#ifndef REGEXPERIENCE_CORE_STATE_MACHINE_MODIFIABLE_H
#define REGEXPERIENCE_CORE_STATE_MACHINE_MODIFIABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_MODIFIABLE (state_machine_modifiable_get_type ())

G_DECLARE_INTERFACE (StateMachineModifiable, state_machine_modifiable, STATE_MACHINES, MODIFIABLE, GObject)

struct _StateMachineModifiableInterface
{
    GTypeInterface parent_iface;

    void (*minimize) (StateMachineModifiable  *self);
    void (*complement) (StateMachineModifiable  *self);
};

void state_machine_modifiable_minimize (StateMachineModifiable *self);

void state_machine_modifiable_complement (StateMachineModifiable *self);

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_STATE_MACHINE_MODIFIABLE_H */
