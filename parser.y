%{

// Dummy parser for scanner project.

#include "lyutils.h"
#include "astree.h"
#include "assert.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL TOK_DECLID
%token TOK_POS TOK_NEG TOK_PARAMLIST TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_NEWSTRING TOK_ORD TOK_CHR TOK_ROOT TOK_VARDECL TOK_RETURNVOID
%token TOK_INDEX TOK_FUNCTION TOK_PROTOTYPE 

%right TOK_IF TOK_ELSE
%right '='
%left TOK_EQ TOK_NE TOK_GT TOK_GE TOK_LT TOK_LE 
%left '+' '-'
%left '*' '/' '%' 
%right TOK_POS TOK_NEG '!' TOK_NEW TOK_ORD TOK_CHR
%left TOK_ARRAY TOK_IDENT TOK_CALL
%left '[' ']' '(' ')' '.'
%nonassoc TOK_ROOT

%start start 

%%

start        : program                { yyparse_astree = $1; }
             ;

program      : program structdef      { $$ = adopt1 ($1, $2); }
             | program function       { $$ = adopt1 ($1, $2); }
             | program statement      { $$ = adopt1 ($1, $2); }
             | program error '}'      { $$ = $1; }
             | program error ';'      { $$ = $1; }
             |                        { $$ = new_parseroot (); }
             ;

structdef    : TOK_STRUCT TOK_IDENT '{' '}'
               { free_ast2($3, $4);
                 $$ = adopt1($1, changesym($2, TOK_TYPEID)); }
             | TOK_STRUCT TOK_IDENT '{' fieldopt '}'
               { free_ast2($3, $5);
                 $$ = adopt2($1, changesym($2, TOK_TYPEID), $4); }
             ;

fielddecl    : basetype TOK_IDENT
               { $$ = adopt1($1, changesym($2, TOK_FIELD)); }
             | basetype TOK_ARRAY TOK_IDENT
               { $$ = adopt2($1, $2, changesym($3, TOK_FIELD)); }
             ;
fieldopt     : fieldopt fielddecl ';'
               { free_ast($3); $$ = adopt1($1, $2); }
             | fielddecl ';'          { free_ast($2); $$ = $1; }
             ;

basetype     : TOK_VOID               { $$ = $1; }
             | TOK_BOOL               { $$ = $1; }
             | TOK_CHAR               { $$ = $1; }
             | TOK_INT                { $$ = $1; }
             | TOK_STRING             { $$ = $1; }
             | TOK_IDENT
               { $$ = changesym($1, TOK_TYPEID); }

function     : identdecl paramlist ')' block
               { free_ast($3); $$ = new_function($1, $2, $4); }
             ;

identdecl    : basetype TOK_IDENT
               { $$ = adopt1($1, changesym($2, TOK_DECLID)); }
             | basetype TOK_ARRAY TOK_IDENT
               { $$ = adopt2($1, $2, changesym($3, TOK_DECLID)); }
             ;

paramlist    : '(' identdecl
               { $$ = adopt1sym($1, $2, TOK_PARAMLIST); }
             | paramlist ',' identdecl
               { free_ast($2); $$ = adopt1($1, $3); }
             | '('                    { $$ = $1; }
             ;

block        : '{' '}'
               { free_ast($2); changesym($1, TOK_BLOCK); }
             | statementopt '}'       { free_ast($2); $$ = $1; }
             | ';'                    { $$ = $1; }
             ;

statement    : block                  { $$ = $1; }
             | vardecl                { $$ = $1; }
             | while                  { $$ = $1; }
             | ifelse                 { $$ = $1; }
             | return                 { $$ = $1; }
             | expr ';'               { free_ast($2); $$ = $1; }
             ;
statementopt : statementopt statement { $$ = adopt1($1, $2); }
             | '{' statement
               { $$ = adopt1sym($1, $2, TOK_BLOCK); }
             ;

vardecl     : identdecl '=' expr ';'
               { free_ast($4);
                 $$ = adopt2sym($2, $1, $3, TOK_VARDECL); }
             ;

while        : TOK_WHILE '(' expr ')' statement
               { free_ast2($2, $4); $$ = adopt2($1, $3, $5); }
             ;

ifelse       : TOK_IF '(' expr ')' statement %prec TOK_ELSE
               { free_ast2($2, $4); $$ = adopt2($1, $3, $5); }
             | TOK_IF '(' expr ')' statement TOK_ELSE statement 
               { free_ast3($2, $4, $6);
                 $$ = changesym(adopt3($1, $7, $3, $5), TOK_IFELSE); }
             ;

return       : TOK_RETURN ';'
               { free_ast($2);
                 $$ = changesym($1, TOK_RETURNVOID); }
             | TOK_RETURN expr ';'
               { free_ast($3); $$ = adopt1($1, $2); }
             ;

expr         : expr '+' expr          { $$ = adopt2($2, $1, $3); }
             | expr '-' expr          { $$ = adopt2($2, $1, $3); }
             | expr '*' expr          { $$ = adopt2($2, $1, $3); }
             | expr '/' expr          { $$ = adopt2($2, $1, $3); }
             | expr '%' expr          { $$ = adopt2($2, $1, $3); }
             | expr '=' expr          { $$ = adopt2($2, $1, $3); }
             | expr TOK_EQ expr       { $$ = adopt2($2, $1, $3); }
             | expr TOK_NE expr       { $$ = adopt2($2, $1, $3); }
             | expr TOK_GT expr       { $$ = adopt2($2, $1, $3); }
             | expr TOK_LT expr       { $$ = adopt2($2, $1, $3); }
             | expr TOK_GE expr       { $$ = adopt2($2, $1, $3); }
             | expr TOK_LE expr       { $$ = adopt2($2, $1, $3); }
             | '+' expr %prec TOK_POS { $$ = adopt1sym($1,$2,TOK_POS); }
             | '-' expr %prec TOK_NEG { $$ = adopt1sym($1,$2,TOK_NEG); }
             | TOK_ORD expr           { $$ = adopt1($1, $2); }
             | TOK_CHR expr           { $$ = adopt1($1, $2); }
             | '!' expr               { $$ = adopt1($1, $2); }
             | allocator %prec TOK_NEW{ $$ = $1; }
             | call                   { $$ = $1; }
             | variable               { $$ = $1; }
             | constant               { $$ = $1; }
             | '(' expr ')'           { free_ast2($1, $3); $$ = $2; }
             ;

allocator    : TOK_NEW TOK_IDENT '(' ')'
               { free_ast2($3, $4);
                 $$ = adopt1($1, changesym($2, TOK_TYPEID)); }
             | TOK_NEW TOK_STRING '(' expr ')'
               { free_ast3($2, $3, $5);
                 $$ = adopt1(changesym($1, TOK_NEWSTRING), $4); }
             | TOK_NEW basetype '[' expr ']'
               { free_ast2($3, $5);
                 $$ = adopt2sym($1, $2, $4, TOK_NEWARRAY); }
             ;

call         : TOK_IDENT '(' ')'
               { free_ast($3); $$ = adopt1sym($2, $1, TOK_CALL); }
             | TOK_IDENT '(' expropt ')'
               { free_ast ($4); $$ = adopt2sym($2, $1, $3, TOK_CALL); }
             ;
expropt      : expr                   { $$ = $1; }
             | expr expropt2          { $$ = adopt1($1, $2); } 
             ;
expropt2     : expropt2 ',' expr
               { free_ast($2); $$ = adopt1($1, $3); }
             | ',' expr               { free_ast($1); $$ = $2; }
             ;

variable     : TOK_IDENT              { $$ = $1; }
             | expr '[' expr ']'
               { free_ast($4);
                 $$ = adopt2sym($2, $1, $3, TOK_INDEX); }
             | expr '.' TOK_IDENT
               { $$ = adopt2($2, $1, changesym($3, TOK_FIELD)); }
             ;

constant     : TOK_INTCON             { $$ = $1; }
             | TOK_CHARCON            { $$ = $1; }
             | TOK_STRINGCON          { $$ = $1; }
             | TOK_FALSE              { $$ = $1; }
             | TOK_TRUE               { $$ = $1; }
             | TOK_NULL               { $$ = $1; }
             ;
%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != NULL);
   return result;
}

