#ifndef REGEXPERIENCE_FSM_MODIFIABLE_H
#define REGEXPERIENCE_FSM_MODIFIABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_FSM_MODIFIABLE (fsm_modifiable_get_type ())

G_DECLARE_INTERFACE (FsmModifiable, fsm_modifiable, STATE_MACHINES, FSM_MODIFIABLE, GObject)

struct _FsmModifiableInterface
{
    GTypeInterface parent_iface;

    void (*minimize)   (FsmModifiable  *self);
    void (*complement) (FsmModifiable  *self);
};

void fsm_modifiable_minimize (FsmModifiable *self);

void fsm_modifiable_complement (FsmModifiable *self);

G_END_DECLS

#endif /* REGEXPERIENCE_FSM_MODIFIABLE_H */
