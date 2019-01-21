#include "internal/state_machines/acceptors/acceptor_runnable.h"
#include "internal/state_machines/fsm_initializable.h"

G_DEFINE_INTERFACE (AcceptorRunnable, acceptor_runnable, STATE_MACHINES_TYPE_FSM_INITIALIZABLE)

static void
acceptor_runnable_default_init (AcceptorRunnableInterface *iface)
{
  /* NOP */
}

GPtrArray *
acceptor_runnable_run (AcceptorRunnable *self,
                       const gchar      *input)
{
  AcceptorRunnableInterface *iface;

  g_return_val_if_fail (ACCEPTORS_ACCEPTOR_RUNNABLE (self), NULL);

  iface = ACCEPTORS_ACCEPTOR_RUNNABLE_GET_IFACE (self);

  g_return_val_if_fail (iface->run != NULL, NULL);

  iface->run (self, input);
}

gboolean
acceptor_runnable_can_accept (AcceptorRunnable *self)
{
  AcceptorRunnableInterface *iface;

  g_return_val_if_fail (ACCEPTORS_IS_ACCEPTOR_RUNNABLE (self), FALSE);

  iface = ACCEPTORS_ACCEPTOR_RUNNABLE_GET_IFACE (self);

  g_return_val_if_fail (iface->can_accept != NULL, FALSE);

  return iface->can_accept (self);
}
