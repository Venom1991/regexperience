#include "internal/state_machines/state_machine_convertible.h"
#include "internal/state_machines/state_machine_initializable.h"

G_DEFINE_INTERFACE (StateMachineConvertible, state_machine_convertible, STATE_MACHINES_TYPE_INITIALIZABLE)

static void
state_machine_convertible_default_init (StateMachineConvertibleInterface *iface)
{
  /* NOP */
}

StateMachineConvertible *
state_machine_convertible_compute_epsilon_closures (StateMachineConvertible *self)
{
  StateMachineConvertibleInterface *iface;

  g_return_val_if_fail (STATE_MACHINES_IS_CONVERTIBLE (self), NULL);

  iface = STATE_MACHINES_CONVERTIBLE_GET_IFACE (self);

  g_return_val_if_fail (iface->compute_epsilon_closures != NULL, NULL);

  return iface->compute_epsilon_closures (self);
}

StateMachineModifiable *
state_machine_convertible_construct_subset (StateMachineConvertible *self)
{
  StateMachineConvertibleInterface *iface;

  g_return_val_if_fail (STATE_MACHINES_IS_CONVERTIBLE (self), NULL);

  iface = STATE_MACHINES_CONVERTIBLE_GET_IFACE (self);

  g_return_val_if_fail (iface->construct_subset != NULL, NULL);

  return iface->construct_subset (self);
}
