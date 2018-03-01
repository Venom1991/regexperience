#include "core/regexperience.h"
#include "internal/syntactic_analysis/lexer.h"
#include "internal/syntactic_analysis/parser.h"
#include "internal/semantic_analysis/analyzer.h"
#include "internal/state_machines/acceptors/acceptor_runnable.h"
#include "internal/common/helpers.h"
#include "core/errors.h"

struct _Regexperience
{
    GObject parent_instance;
};

typedef struct
{
    /* Compilation */
    Lexer    *lexer;
    Parser   *parser;
    Analyzer *analyzer;

    /* Matching */
    AcceptorRunnable *acceptor;
} RegexperiencePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Regexperience, regexperience, G_TYPE_OBJECT)

G_DEFINE_QUARK (core-regexperience-error-quark, core_regexperience_error)
#define CORE_REGEXPERIENCE_ERROR (core_regexperience_error_quark())

static void regexperience_dispose (GObject *object);

static void
regexperience_class_init (RegexperienceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = regexperience_dispose;
}

static void
regexperience_init (Regexperience *self)
{
  RegexperiencePrivate *priv = regexperience_get_instance_private (self);

  priv->lexer = lexer_new ();
  priv->parser = parser_new ();
  priv->analyzer = analyzer_new ();
}

void regexperience_compile (Regexperience  *self,
                            const gchar    *regular_expression,
                            GError        **error)
{
  g_return_if_fail (error == NULL || *error == NULL);

  RegexperiencePrivate *priv = regexperience_get_instance_private (self);

  Lexer *lexer = priv->lexer;
  Parser *parser = priv->parser;
  Analyzer *analyzer = priv->analyzer;
  GError *temporary_error = NULL;

  g_autoptr (GPtrArray) tokens = lexer_tokenize (lexer,
                                                 regular_expression,
                                                 &temporary_error);

  if (temporary_error != NULL)
    {
      g_propagate_error (error, temporary_error);

      return;
    }

  GNode *concrete_syntax_tree = parser_build_concrete_syntax_tree (parser,
                                                                   tokens,
                                                                   &temporary_error);

  if (temporary_error != NULL)
    {
      g_propagate_error (error, temporary_error);

      return;
    }

  g_autoptr (AstNode) abstract_syntax_tree = analyzer_build_abstract_syntax_tree (analyzer,
                                                                                  concrete_syntax_tree,
                                                                                  &temporary_error);

  /* Manually decreasing the reference count of every object found
   * in the concrete syntax tree and finally destroying the GNode itself.
   */
  g_node_unref_g_objects (concrete_syntax_tree);

  if (temporary_error != NULL)
    {
      g_propagate_error (error, temporary_error);

      return;
    }

  if (priv->acceptor != NULL)
    g_object_unref (priv->acceptor);

  g_autoptr (FsmConvertible) epsilon_nfa = ast_node_build_fsm (abstract_syntax_tree);
  g_autoptr (FsmConvertible) nfa = fsm_convertible_compute_epsilon_closures (epsilon_nfa);
  FsmModifiable *dfa = fsm_convertible_construct_subset (nfa);

  state_machine_modifiable_minimize (dfa);

  priv->acceptor = ACCEPTORS_ACCEPTOR_RUNNABLE (dfa);
}

gboolean regexperience_match (Regexperience  *self,
                              const gchar    *input,
                              GError        **error)
{
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  RegexperiencePrivate *priv = regexperience_get_instance_private (self);

  AcceptorRunnable *acceptor = priv->acceptor;
  CoreRegexperienceError error_code = CORE_REGEXPERIENCE_ERROR_UNDEFINED;
  const gchar *error_message = NULL;

  if (acceptor == NULL)
    {
      error_message = "The regular expression must be compiled beforehand";
      error_code = CORE_REGEXPERIENCE_ERROR_REGULAR_EXPRESSION_NOT_COMPILED;
    }
  else if (input == NULL)
    {
      error_message = "The input must not be NULL";
      error_code = CORE_REGEXPERIENCE_ERROR_INPUT_NULL;
    }
  else if (!g_str_is_ascii (input))
    {
      error_message = "The input must be an ASCII string";
      error_code = CORE_REGEXPERIENCE_ERROR_INPUT_NOT_ASCII;
    }

  if (error_message != NULL)
    {
      g_set_error (error,
                   CORE_REGEXPERIENCE_ERROR,
                   error_code,
                   error_message);

      return FALSE;
    }

  acceptor_runnable_run (acceptor, input);

  return acceptor_runnable_can_accept (acceptor);
}

static void
regexperience_dispose (GObject *object)
{
  RegexperiencePrivate *priv = regexperience_get_instance_private (CORE_REGEXPERIENCE (object));

  if (priv->lexer != NULL)
    g_clear_object (&priv->lexer);

  if (priv->parser != NULL)
    g_clear_object (&priv->parser);

  if (priv->analyzer != NULL)
    g_clear_object (&priv->analyzer);

  if (priv->acceptor != NULL)
    g_clear_object (&priv->acceptor);

  G_OBJECT_CLASS (regexperience_parent_class)->dispose (object);
}
