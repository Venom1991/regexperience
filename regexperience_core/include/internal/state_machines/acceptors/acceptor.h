#ifndef REGEXPERIENCE_CORE_ACCEPTOR_H
#define REGEXPERIENCE_CORE_ACCEPTOR_H

#include <glib-object.h>

#include "internal/state_machines/state.h"
#include "internal/state_machines/composite_state.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_ACCEPTOR (acceptor_get_type())

G_DECLARE_DERIVABLE_TYPE (Acceptor, acceptor, STATE_MACHINES, ACCEPTOR, GObject)

struct _AcceptorClass
{
    GObjectClass parent_class;
};

GPtrArray *acceptor_fetch_output_states_from_single (State *input_state,
                                                     gchar  expected_character);

GPtrArray *acceptor_fetch_output_states_from_multiple (GPtrArray *input_states,
                                                       gchar      expected_character);

State *acceptor_get_or_create_composite_state (GPtrArray                          *all_states,
                                               GPtrArray                          *composed_from_states,
                                               CompositeStateResolveTypeFlagsMode  resolve_type_flags_mode,
                                               gboolean                           *already_existed);

State *acceptor_get_or_create_dead_state (GPtrArray *all_states);

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_ACCEPTOR_H */
