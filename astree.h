#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"

struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
};

// Append one child to the vector of children.
astree* adopt1 (astree* root, astree* child);

// Append two children to the vector of children.
astree* adopt2 (astree* root, astree* left, astree* right);

// Dump an astree to a FILE.
void dump_astree (FILE* outfile, astree* root);

// Debug print an astree.
void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep);

// Recursively free an astree.
void free_ast (astree* tree);

// Recursively free two astrees.
void free_ast2 (astree* tree1, astree* tree2);

astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo);
astree* new_function (astree* identdecl, astree* paramlist,
                      astree* block);
void free_ast3 (astree* tree1, astree* tree2, astree* tree3);
astree* adopt3 (astree* root, astree* c1, astree* c2, astree* c3);
astree* adopt1sym (astree* root, astree* child, int symbol);
astree* adopt2sym (astree* root, astree* c1, astree* c2, int symbol);
astree* changesym (astree* root, int symbol);
void dump_tok (FILE* outfile, astree* root);
void dump_ast (FILE* outfile, astree* root);

RCSH("$Id: astree.h,v 1.4 2015-04-09 19:31:47-07 - - $")
#endif
