#ifndef REGEXPERIENCE_CORE_FSM_CONVERTIBLE_H
#define REGEXPERIENCE_CORE_FSM_CONVERTIBLE_H

#include <glib-object.h>

#include "internal/state_machines/fsm_modifiable.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_FSM_CONVERTIBLE (fsm_convertible_get_type ())

G_DECLARE_INTERFACE (FsmConvertible, fsm_convertible, STATE_MACHINES, FSM_CONVERTIBLE, GObject)

struct _FsmConvertibleInterface
{
    GTypeInterface parent_iface;

    FsmConvertible * (*compute_epsilon_closures) (FsmConvertible  *self);
    FsmModifiable  * (*construct_subset)         (FsmConvertible  *self);
};

FsmConvertible *fsm_convertible_compute_epsilon_closures (FsmConvertible *self);

FsmModifiable  *fsm_convertible_construct_subset         (FsmConvertible *self);

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_FSM_CONVERTIBLE_H */
