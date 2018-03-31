#ifndef REGEXPERIENCE_CORE_STATE_FACTORY_H
#define REGEXPERIENCE_CORE_STATE_FACTORY_H

#include "internal/state_machines/state.h"
#include "internal/state_machines/composite_state.h"

State *create_composite_state (GPtrArray                          *composed_from_states,
                               CompositeStateResolveTypeFlags  resolve_type_flags_mode);

State *create_dead_state      (void);

#endif /* REGEXPERIENCE_CORE_STATE_FACTORY_H */
