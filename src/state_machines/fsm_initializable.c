#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/state.h"

G_DEFINE_INTERFACE (FsmInitializable, fsm_initializable, G_TYPE_OBJECT)

static void
fsm_initializable_default_init (FsmInitializableInterface *iface)
{
  g_object_interface_install_property (iface,
                                       g_param_spec_boxed (PROP_FSM_INITIALIZABLE_ALL_STATES,
                                                           "All states",
                                                           "All states (one or more) in which the state machine can"
                                                             "find itself at any given moment.",
                                                           G_TYPE_PTR_ARRAY,
                                                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_interface_install_property (iface,
                                       g_param_spec_object (PROP_FSM_INITIALIZABLE_START_STATE,
                                                            "Start state",
                                                            "Start state in which the state machine initially"
                                                              "finds itself.",
                                                            STATE_MACHINES_TYPE_STATE,
                                                            G_PARAM_READABLE));

  g_object_interface_install_property (iface,
                                       g_param_spec_boxed (PROP_FSM_INITIALIZABLE_FINAL_STATES,
                                                           "Final state",
                                                           "Final states that mark the acceptance of the input if at"
                                                             "the moment of exhausting it the state machine finds"
                                                             "itself in any of these states.",
                                                           G_TYPE_PTR_ARRAY,
                                                           G_PARAM_READABLE));

  g_object_interface_install_property (iface,
                                       g_param_spec_boxed (PROP_FSM_INITIALIZABLE_NON_FINAL_STATES,
                                                           "Non-final states",
                                                           "Non-final states (one or more) that mark the rejection of"
                                                             "the input if at the moment of exhausting it the state"
                                                             "machine finds itself in any one of these states.",
                                                           G_TYPE_PTR_ARRAY,
                                                           G_PARAM_READABLE));

  g_object_interface_install_property (iface,
                                       g_param_spec_pointer (PROP_FSM_INITIALIZABLE_ALPHABET,
                                                             "Alphabet",
                                                             "Finite set of characters that the state machine"
                                                               "explicitly recognizes.",
                                                             G_PARAM_READABLE));
}
