#include "internal/state_machines/transducers/transducer_runnable.h"
#include "internal/state_machines/fsm_initializable.h"

G_DEFINE_INTERFACE (TransducerRunnable, transducer_runnable, STATE_MACHINES_TYPE_FSM_INITIALIZABLE)

static void
transducer_runnable_default_init (TransducerRunnableInterface *iface)
{
  /* NOP */
}

void
transducer_runnable_reset (TransducerRunnable *self)
{
  TransducerRunnableInterface *iface;

  g_return_if_fail (TRANSDUCERS_IS_TRANSDUCER_RUNNABLE (self));

  iface = TRANSDUCERS_TRANSDUCER_RUNNABLE_GET_IFACE (self);

  g_return_if_fail (iface->run != NULL);

  iface->reset (self);
}

gpointer
transducer_runnable_run (TransducerRunnable *self,
                         gchar               input)
{
  TransducerRunnableInterface *iface;

  g_return_val_if_fail (TRANSDUCERS_IS_TRANSDUCER_RUNNABLE (self), NULL);

  iface = TRANSDUCERS_TRANSDUCER_RUNNABLE_GET_IFACE (self);

  g_return_val_if_fail (iface->run != NULL, NULL);

  return iface->run (self, input);
}
