#ifndef REGEXPERIENCE_COMPOSITE_STATE_H
#define REGEXPERIENCE_COMPOSITE_STATE_H

#include <glib-object.h>

#include "state.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_COMPOSITE_STATE (composite_state_get_type ())
#define composite_state_new(...) (g_object_new (STATE_MACHINES_TYPE_COMPOSITE_STATE, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (CompositeState, composite_state, STATE_MACHINES, COMPOSITE_STATE, State)

typedef enum
{
    COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_START = 1 << 0,
    COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_FINAL = 1 << 1,
    COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_ALL = COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_START |
                                             COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_FINAL
} CompositeStateResolveTypeFlags;

#define PROP_COMPOSITE_STATE_COMPOSED_FROM_STATES "composed-from-states"
#define PROP_COMPOSITE_STATE_RESOLVE_TYPE_FLAGS   "resolve-type-flags"

G_END_DECLS

#endif /* REGEXPERIENCE_COMPOSITE_STATE_H */