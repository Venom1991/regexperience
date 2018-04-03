#ifndef REGEXPERIENCE_CORE_STATE_H
#define REGEXPERIENCE_CORE_STATE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_STATE (state_get_type ())
#define state_new(...) (g_object_new (STATE_MACHINES_TYPE_STATE, ##__VA_ARGS__, NULL))

G_DECLARE_DERIVABLE_TYPE (State, state, STATE_MACHINES, STATE, GObject)

typedef enum
{
    STATE_TYPE_UNDEFINED = 0x0000,
    STATE_TYPE_DEFAULT = 1 << 0,
    STATE_TYPE_START = 1 << 1,
    STATE_TYPE_FINAL = 1 << 2
} StateTypeFlags;

struct _StateClass
{
    GObjectClass parent_instance;

    gboolean (*is_composed_from) (State           *self,
                                  const GPtrArray *states);

    gpointer padding[8];
};

gboolean state_is_composed_from (State           *self,
                                 const GPtrArray *states);

gboolean state_is_dead          (State           *self);

#define PROP_STATE_TYPE_FLAGS  "type-flags"
#define PROP_STATE_TRANSITIONS "transitions"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_STATE_H */
