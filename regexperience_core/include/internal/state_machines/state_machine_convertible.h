#ifndef REGEXPERIENCE_CORE_STATE_MACHINE_CONVERTIBLE_H
#define REGEXPERIENCE_CORE_STATE_MACHINE_CONVERTIBLE_H

#include <glib-object.h>

#include "internal/state_machines/state_machine_modifiable.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_CONVERTIBLE (state_machine_convertible_get_type ())

G_DECLARE_INTERFACE (StateMachineConvertible, state_machine_convertible, STATE_MACHINES, CONVERTIBLE, GObject)

struct _StateMachineConvertibleInterface
{
    GTypeInterface parent_iface;

    StateMachineConvertible * (*compute_epsilon_closures) (StateMachineConvertible  *self);
    StateMachineModifiable * (*construct_subset) (StateMachineConvertible  *self);
};

StateMachineConvertible *state_machine_convertible_compute_epsilon_closures (StateMachineConvertible *self);

StateMachineModifiable *state_machine_convertible_construct_subset (StateMachineConvertible *self);

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_STATE_MACHINE_CONVERTIBLE_H */
