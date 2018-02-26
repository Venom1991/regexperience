#ifndef REGEXPERIENCE_CORE_STATE_MACHINE_RUNNABLE_H
#define REGEXPERIENCE_CORE_STATE_MACHINE_RUNNABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_RUNNABLE (state_machine_runnable_get_type ())

G_DECLARE_INTERFACE (StateMachineRunnable, state_machine_runnable, STATE_MACHINES, RUNNABLE, GObject)

struct _StateMachineRunnableInterface
{
    GTypeInterface parent_iface;

    void (*run) (StateMachineRunnable  *self,
                 const gchar           *input);
    gboolean (*can_accept) (StateMachineRunnable  *self);
};

void state_machine_runnable_run (StateMachineRunnable *self, const gchar *input);

gboolean state_machine_runnable_can_accept (StateMachineRunnable *self);

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_STATE_MACHINE_RUNNABLE_H */
