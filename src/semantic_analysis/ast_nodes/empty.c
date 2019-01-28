#include "internal/semantic_analysis/ast_nodes/empty.h"
#include "internal/state_machines/acceptors/epsilon_nfa.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/state_factory.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"

struct _Empty
{
  Constant parent_instance;
};

static FsmConvertible *empty_build_acceptor (AstNode *self);

G_DEFINE_TYPE (Empty, empty, AST_NODES_TYPE_CONSTANT)

static void
empty_class_init (EmptyClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);

  ast_node_class->build_acceptor = empty_build_acceptor;
}

static void
empty_init (Empty *self)
{
  /* NOP */
}

static FsmConvertible *
empty_build_acceptor (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_EMPTY (self), NULL);

  g_autoptr (GPtrArray) all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *empty = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START | STATE_TYPE_FINAL);
  State *non_empty = create_dead_state ();
  g_autoptr (GPtrArray) empty_transitions = g_ptr_array_new_with_free_func (g_object_unref);
  Transition *empty_on_any = create_deterministic_transition (ANY, non_empty);

  g_ptr_array_add (empty_transitions, empty_on_any);
  g_object_set (empty,
                PROP_STATE_TRANSITIONS, empty_transitions,
                NULL);

  g_ptr_array_add_multiple (all_states,
                            empty, non_empty,
                            NULL);

  return epsilon_nfa_new (PROP_FSM_INITIALIZABLE_ALL_STATES, all_states);
}
