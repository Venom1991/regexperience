#ifndef REGEXPERIENCE_AST_NODE_FACTORY_H
#define REGEXPERIENCE_AST_NODE_FACTORY_H

#include "ast_nodes/ast_node.h"

AstNode *create_constant        (GNode        *cst_context);

AstNode *create_anchor          (GNode        *cst_context,
                                 AstNode      *anchored_node);

AstNode *create_unary_operator  (OperatorType  operator_type,
                                 AstNode      *operand);

AstNode *create_binary_operator (OperatorType  operator_type,
                                 AstNode      *left_operand,
                                 AstNode      *right_operand);

#endif /* REGEXPERIENCE_AST_NODE_FACTORY_H */
