#include "internal/state_machines/fsm_modifiable.h"
#include "internal/state_machines/fsm_initializable.h"

G_DEFINE_INTERFACE (FsmModifiable, fsm_modifiable, STATE_MACHINES_TYPE_FSM_INITIALIZABLE)

static void
fsm_modifiable_default_init (FsmModifiableInterface *iface)
{
  /* NOP */
}

void
state_machine_modifiable_minimize (FsmModifiable *self)
{
  FsmModifiableInterface *iface;

  g_return_if_fail (STATE_MACHINES_IS_FSM_MODIFIABLE (self));

  iface = STATE_MACHINES_FSM_MODIFIABLE_GET_IFACE (self);

  g_return_if_fail (iface->minimize != NULL);

  iface->minimize (self);
}

void
state_machine_modifiable_complement (FsmModifiable *self)
{
  FsmModifiableInterface *iface;

  g_return_if_fail (STATE_MACHINES_IS_FSM_MODIFIABLE (self));

  iface = STATE_MACHINES_FSM_MODIFIABLE_GET_IFACE (self);

  g_return_if_fail (iface->complement != NULL);

  iface->complement (self);
}
