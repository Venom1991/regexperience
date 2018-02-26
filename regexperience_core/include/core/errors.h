#ifndef REGEXPERIENCE_CORE_ERRORS_H
#define REGEXPERIENCE_CORE_ERRORS_H

typedef enum _CoreRegexperienceError
{
    CORE_REGEXPERIENCE_ERROR_UNDEFINED = 0,
    CORE_REGEXPERIENCE_ERROR_REGULAR_EXPRESSION_NOT_COMPILED,
    CORE_REGEXPERIENCE_ERROR_INPUT_NULL,
    CORE_REGEXPERIENCE_ERROR_INPUT_NOT_ASCII,
    CORE_REGEXPERIENCE_N_ERRORS
} CoreRegexperienceError;

typedef enum _SyntacticAnalysisLexerError
{
    SYNTACTIC_ANALYSIS_LEXER_ERROR_UNDEFINED = CORE_REGEXPERIENCE_N_ERRORS,
    SYNTACTIC_ANALYSIS_LEXER_ERROR_INPUT_NULL,
    SYNTACTIC_ANALYSIS_LEXER_ERROR_INPUT_EMPTY,
    SYNTACTIC_ANALYSIS_LEXER_ERROR_INPUT_NOT_ASCII,
    SYNTACTIC_ANALYSIS_LEXER_N_ERRORS
} SyntacticAnalysisLexerError;

typedef enum _SyntacticAnalysisParserError
{
    SYNTACTIC_ANALYSIS_PARSER_ERROR_UNDEFINED = SYNTACTIC_ANALYSIS_LEXER_N_ERRORS,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_UNEXPECTED_CHARACTER,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_ALTERNATION_OPERATOR,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_QUANTIFICATION_METACHARACTER,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_RANGE_OPERATOR,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_METACHARACTER_ESCAPE,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_OPEN_PARENTHESIS,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_CLOSE_PARENTHESIS,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_OPEN_BRACKET,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_EMPTY_GROUP,
    SYNTACTIC_ANALYSIS_PARSER_ERROR_EMPTY_BRACKET_EXPRESSION,
    SYNTACTIC_ANALYSIS_PARSER_N_ERRORS
} SyntacticAnalysisParserError;

typedef enum _SemanticAnalysisRangeError
{
    SEMANTIC_ANALYSIS_RANGE_ERROR_INVALID_VALUES = SYNTACTIC_ANALYSIS_PARSER_N_ERRORS
} SemanticAnalysisAnalyzerError;

#endif /* REGEXPERIENCE_CORE_ERRORS_H */