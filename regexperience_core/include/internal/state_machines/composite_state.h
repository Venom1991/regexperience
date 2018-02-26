#ifndef REGEXPERIENCE_CORE_COMPOSITE_STATE_H
#define REGEXPERIENCE_CORE_COMPOSITE_STATE_H

#include <glib-object.h>

#include "state.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_COMPOSITE_STATE (composite_state_get_type ())
#define composite_state_new(...) (g_object_new (STATE_MACHINES_TYPE_COMPOSITE_STATE, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (CompositeState, composite_state, STATE_MACHINES, COMPOSITE_STATE, State)

#define PROP_COMPOSITE_STATE_COMPOSED_FROM_STATES    "composed-from-states"
#define PROP_COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_MODE "resolve-type-flags-mode"

typedef enum _CompositeStateResolveTypeFlagsMode
{
    COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_MODE_START = 1 << 0,
    COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_MODE_FINAL = 1 << 1,
    COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_MODE_ALL = COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_MODE_START |
                                                  COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_MODE_FINAL
} CompositeStateResolveTypeFlagsMode;

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_COMPOSITE_STATE_H */
