#ifndef REGEXPERIENCE_CORE_FSM_H
#define REGEXPERIENCE_CORE_FSM_H

#include <glib-object.h>

#include "internal/state_machines/state.h"
#include "internal/state_machines/composite_state.h"

G_BEGIN_DECLS

#define STATE_MACHINES_TYPE_FSM (fsm_get_type ())

G_DECLARE_DERIVABLE_TYPE (Fsm, fsm, STATE_MACHINES, FSM, GObject)

struct _FsmClass
{
    GObjectClass parent_class;
};

GPtrArray *fsm_fetch_output_states_from_single   (State                              *input_state,
                                                  gchar                               expected_character);

GPtrArray *fsm_fetch_output_states_from_multiple (GPtrArray                          *input_states,
                                                  gchar                               expected_character);

State     *fsm_get_or_create_composite_state     (GPtrArray                          *all_states,
                                                  GPtrArray                          *composed_from_states,
                                                  CompositeStateResolveTypeFlagsMode  resolve_type_flags_mode,
                                                  gboolean                           *already_existed);

State     *fsm_get_or_create_dead_state          (GPtrArray                          *all_states);

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_FSM_H */
