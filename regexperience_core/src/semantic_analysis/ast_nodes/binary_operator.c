#include "internal/semantic_analysis/ast_nodes/binary_operator.h"

typedef struct
{
    AstNode *left_operand;
    AstNode *right_operand;
} BinaryOperatorPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (BinaryOperator, binary_operator, AST_NODES_TYPE_AST_NODE)

enum
{
    PROP_LEFT_OPERAND = 1,
    PROP_RIGHT_OPERAND = 2,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static gboolean binary_operator_is_valid (AstNode  *self,
                                          GError  **error);

static void binary_operator_get_property (GObject    *object,
                                          guint       property_id,
                                          GValue     *value,
                                          GParamSpec *pspec);

static void binary_operator_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec);

static void binary_operator_dispose (GObject *object);

static void
binary_operator_class_init (BinaryOperatorClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  ast_node_class->is_valid = binary_operator_is_valid;

  object_class->get_property = binary_operator_get_property;
  object_class->set_property = binary_operator_set_property;
  object_class->dispose = binary_operator_dispose;

  obj_properties[PROP_LEFT_OPERAND] =
      g_param_spec_object (PROP_BINARY_OPERATOR_LEFT_OPERAND,
                           "Left operand",
                           "Operand representing the left side of the operation.",
                           AST_NODES_TYPE_AST_NODE,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_RIGHT_OPERAND] =
      g_param_spec_object (PROP_BINARY_OPERATOR_RIGHT_OPERAND,
                           "Right operand",
                           "Operand representing the right side of the operation.",
                           AST_NODES_TYPE_AST_NODE,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
binary_operator_init (BinaryOperator *self)
{
  /* NOP */
}

static
gboolean binary_operator_is_valid (AstNode  *self,
                                   GError  **error)
{
  g_return_val_if_fail (AST_NODES_IS_BINARY_OPERATOR (self), FALSE);

  BinaryOperatorPrivate *priv = binary_operator_get_instance_private (AST_NODES_BINARY_OPERATOR (self));

  AstNode *left_operand = priv->left_operand;
  AstNode *right_operand = priv->right_operand;

  return ast_node_is_valid (left_operand, error) &&
         ast_node_is_valid (right_operand, error);
}

static void
binary_operator_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  BinaryOperatorPrivate *priv = binary_operator_get_instance_private (AST_NODES_BINARY_OPERATOR (object));

  switch (property_id)
    {
    case PROP_LEFT_OPERAND:
      g_value_set_object (value, priv->left_operand);
      break;

    case PROP_RIGHT_OPERAND:
      g_value_set_object (value, priv->right_operand);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
binary_operator_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  BinaryOperatorPrivate *priv = binary_operator_get_instance_private (AST_NODES_BINARY_OPERATOR (object));

  switch (property_id)
    {
    case PROP_LEFT_OPERAND:
      if (priv->left_operand != NULL)
        g_object_unref (priv->left_operand);

      priv->left_operand = g_value_dup_object (value);
      break;

    case PROP_RIGHT_OPERAND:
      if (priv->right_operand != NULL)
        g_object_unref (priv->right_operand);

      priv->right_operand = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
binary_operator_dispose (GObject *object)
{
  BinaryOperatorPrivate *priv = binary_operator_get_instance_private (AST_NODES_BINARY_OPERATOR (object));

  if (priv->left_operand != NULL)
    g_clear_object (&priv->left_operand);

  if (priv->right_operand != NULL)
    g_clear_object (&priv->right_operand);

  G_OBJECT_CLASS (binary_operator_parent_class)->dispose (object);
}
