#include "internal/state_machines/state_machine_modifiable.h"
#include "internal/state_machines/state_machine_initializable.h"

G_DEFINE_INTERFACE (StateMachineModifiable, state_machine_modifiable, STATE_MACHINES_TYPE_INITIALIZABLE)

static void
state_machine_modifiable_default_init (StateMachineModifiableInterface *iface)
{
  /* NOP */
}

void
state_machine_modifiable_minimize (StateMachineModifiable *self)
{
  StateMachineModifiableInterface *iface;

  g_return_if_fail (STATE_MACHINES_IS_MODIFIABLE (self));

  iface = STATE_MACHINES_MODIFIABLE_GET_IFACE (self);

  g_return_if_fail (iface->minimize != NULL);

  iface->minimize (self);
}

void
state_machine_modifiable_complement (StateMachineModifiable *self)
{
  StateMachineModifiableInterface *iface;

  g_return_if_fail (STATE_MACHINES_IS_MODIFIABLE (self));

  iface = STATE_MACHINES_MODIFIABLE_GET_IFACE (self);

  g_return_if_fail (iface->complement != NULL);

  iface->complement (self);
}

