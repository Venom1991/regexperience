#include "internal/state_machines/state_factory.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

State *create_composite_state (GPtrArray                      *composed_from_states,
                               CompositeStateResolveTypeFlags  resolve_type_flags_mode)
{
  g_return_val_if_fail (g_ptr_array_has_items (composed_from_states), NULL);

  State *composite_state = composite_state_new (PROP_COMPOSITE_STATE_COMPOSED_FROM_STATES, composed_from_states,
                                                PROP_COMPOSITE_STATE_RESOLVE_TYPE_FLAGS, resolve_type_flags_mode);

  return composite_state;
}

State *create_dead_state (void)
{
  State *dead_state = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT,
                                 PROP_STATE_IS_DEAD, TRUE);
  g_autoptr (GPtrArray) transitions = g_ptr_array_new_with_free_func (g_object_unref);
  Transition *transition = create_deterministic_transition (ANY, dead_state);

  g_ptr_array_add (transitions, transition);
  g_object_set (dead_state,
                PROP_STATE_TRANSITIONS, transitions,
                NULL);

  return dead_state;
}
