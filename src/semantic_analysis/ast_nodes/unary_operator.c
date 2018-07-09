#include "internal/semantic_analysis/ast_nodes/unary_operator.h"

typedef struct
{
  AstNode *operand;
} UnaryOperatorPrivate;

enum
{
  PROP_OPERAND = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static gboolean unary_operator_is_valid     (AstNode       *self,
                                             GError       **error);

static void     unary_operator_get_property (GObject       *object,
                                             guint          property_id,
                                             GValue        *value,
                                             GParamSpec    *pspec);

static void     unary_operator_set_property (GObject       *object,
                                             guint          property_id,
                                             const GValue  *value,
                                             GParamSpec    *pspec);

static void     unary_operator_dispose      (GObject       *object);

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (UnaryOperator, unary_operator, AST_NODES_TYPE_AST_NODE)

static void
unary_operator_class_init (UnaryOperatorClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  ast_node_class->is_valid = unary_operator_is_valid;

  object_class->get_property = unary_operator_get_property;
  object_class->set_property = unary_operator_set_property;
  object_class->dispose = unary_operator_dispose;

  obj_properties[PROP_OPERAND] =
      g_param_spec_object (PROP_UNARY_OPERATOR_OPERAND,
                           "Operand",
                           "Sole operand of the operation.",
                           AST_NODES_TYPE_AST_NODE,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
unary_operator_init (UnaryOperator *self)
{
  /* NOP */
}

static
gboolean unary_operator_is_valid (AstNode  *self,
                                  GError  **error)
{
  g_return_val_if_fail (AST_NODES_IS_UNARY_OPERATOR (self), FALSE);

  UnaryOperatorPrivate *priv = unary_operator_get_instance_private (AST_NODES_UNARY_OPERATOR (self));
  AstNode *operand = priv->operand;

  return ast_node_is_valid (operand, error);
}

static void
unary_operator_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  UnaryOperatorPrivate *priv = unary_operator_get_instance_private (AST_NODES_UNARY_OPERATOR (object));

  switch (property_id)
    {
    case PROP_OPERAND:
      g_value_set_object (value, priv->operand);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
unary_operator_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  UnaryOperatorPrivate *priv = unary_operator_get_instance_private (AST_NODES_UNARY_OPERATOR (object));

  switch (property_id)
    {
    case PROP_OPERAND:
      if (priv->operand != NULL)
        g_object_unref (priv->operand);

      priv->operand = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
unary_operator_dispose (GObject *object)
{
  UnaryOperatorPrivate *priv = unary_operator_get_instance_private (AST_NODES_UNARY_OPERATOR (object));

  if (priv->operand != NULL)
    g_clear_object (&priv->operand);

  G_OBJECT_CLASS (unary_operator_parent_class)->dispose (object);
}
