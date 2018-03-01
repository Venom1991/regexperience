#include "internal/state_machines/fsm_convertible.h"
#include "internal/state_machines/fsm_initializable.h"

G_DEFINE_INTERFACE (FsmConvertible, fsm_convertible, STATE_MACHINES_TYPE_FSM_INITIALIZABLE)

static void
fsm_convertible_default_init (FsmConvertibleInterface *iface)
{
  /* NOP */
}

FsmConvertible *
fsm_convertible_compute_epsilon_closures (FsmConvertible *self)
{
  FsmConvertibleInterface *iface;

  g_return_val_if_fail (STATE_MACHINES_IS_FSM_CONVERTIBLE (self), NULL);

  iface = STATE_MACHINES_FSM_CONVERTIBLE_GET_IFACE (self);

  g_return_val_if_fail (iface->compute_epsilon_closures != NULL, NULL);

  return iface->compute_epsilon_closures (self);
}

FsmModifiable *
fsm_convertible_construct_subset (FsmConvertible *self)
{
  FsmConvertibleInterface *iface;

  g_return_val_if_fail (STATE_MACHINES_IS_FSM_CONVERTIBLE (self), NULL);

  iface = STATE_MACHINES_FSM_CONVERTIBLE_GET_IFACE (self);

  g_return_val_if_fail (iface->construct_subset != NULL, NULL);

  return iface->construct_subset (self);
}
