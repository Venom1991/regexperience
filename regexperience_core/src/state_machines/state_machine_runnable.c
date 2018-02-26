#include "internal/state_machines/state_machine_runnable.h"
#include "internal/state_machines/state_machine_initializable.h"

G_DEFINE_INTERFACE (StateMachineRunnable, state_machine_runnable, STATE_MACHINES_TYPE_INITIALIZABLE)

static void
state_machine_runnable_default_init (StateMachineRunnableInterface *iface)
{
  /* NOP */
}

void
state_machine_runnable_run (StateMachineRunnable *self, const gchar *input)
{
  StateMachineRunnableInterface *iface;

  g_return_if_fail (STATE_MACHINES_IS_RUNNABLE (self));

  iface = STATE_MACHINES_RUNNABLE_GET_IFACE (self);

  g_return_if_fail (iface->run != NULL);

  iface->run (self, input);
}

gboolean
state_machine_runnable_can_accept (StateMachineRunnable *self)
{
  StateMachineRunnableInterface *iface;

  g_return_val_if_fail (STATE_MACHINES_IS_RUNNABLE (self), FALSE);

  iface = STATE_MACHINES_RUNNABLE_GET_IFACE (self);

  g_return_val_if_fail (iface->run != NULL, FALSE);

  return iface->can_accept (self);
}
