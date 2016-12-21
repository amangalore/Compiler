
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"

astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = intern_stringset (lexinfo);
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo->c_str());
   return tree;
}

astree* new_proto (astree* identdecl, astree* paramlist) {
   astree* func = new_astree (TOK_PROTOTYPE, 
                              identdecl->filenr, 
                              identdecl->linenr, 
                              identdecl->offset, "");
   return adopt2(func, identdecl, paramlist); 
}

astree* new_function (astree* identdecl, astree* paramlist,
                       astree* block){
   if (!string(";").compare(*block->lexinfo)) {
      return new_proto(identdecl, paramlist);
   }
   astree* func = new_astree(TOK_FUNCTION, identdecl->filenr,
                             identdecl->linenr, 
                             identdecl->offset, "");
   func = adopt2(func, identdecl, paramlist); 
   return adopt1(func, block);
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}


static void dump_node (FILE* outfile, astree* node) {
   fprintf (outfile, "%p->{%s(%d) %ld:%ld.%03ld \"%s\" [",
            node, get_yytname (node->symbol), node->symbol,
            node->filenr, node->linenr, node->offset,
            node->lexinfo->c_str());
   bool need_space = false;
   for (size_t child = 0; child < node->children.size();
        ++child) {
      if (need_space) fprintf (outfile, " ");
      need_space = true;
      fprintf (outfile, "%p", node->children.at(child));
   }
   fprintf (outfile, "]}");
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   fprintf (outfile, "%*s%s ", depth * 3, "", root->lexinfo->c_str());
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child) {
      dump_astree_rec (outfile, root->children[child], depth + 1);
   }
}

static void ast_node (FILE* outfile, astree* node) {
   char *tname;
   tname = (char*) get_yytname (node->symbol);
   if (strstr (tname, "TOK_") == tname) tname += 4;

   fprintf (outfile, "%s \"%s\" %ld.%ld.%ld",
            tname, node->lexinfo->c_str(),
            node->filenr, node->linenr, node->offset);
}

static void ast_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   for (int i = 0; i <= depth; i++) {
      if (i == 0) fprintf(outfile, "  ");
      else fprintf(outfile, "|  ");
   }
   ast_node (outfile, root);
   fprintf (outfile, "\n");

   for (size_t child = 0; child < root->children.size(); ++child) {
         ast_rec (outfile, root->children[child], depth + 1);
   }
}

static void dump_ast (FILE* outfile, astree* root,
                             int depth) {
   if (root == NULL) return;
   fprintf (outfile, "%*s%s ", depth * 3, "",
            root->lexinfo->c_str());
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size();
        ++child) {
      dump_astree_rec (outfile, root->children[child],
                       depth + 1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep) {
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n",
               get_yytname (toknum), toknum);
   }
   fflush (NULL);
}

void dump_ast (FILE* outfile, astree* root) {
   ast_rec (outfile, root, 0);
   fflush (NULL);
}


void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%p]-> %d:%d.%d: %s: \"%s\")\n",
           root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

void free_ast3 (astree* tree1, astree* tree2, astree* tree3) {
   free_ast (tree1);
   free_ast (tree2);
   free_ast (tree3);
}

astree* adopt3 (astree* root, astree* c1, astree* c2, astree* c3) {
   adopt1 (root, c1);
   adopt1 (root, c2);
   adopt1 (root, c3);
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}
astree* adopt2sym (astree* root, astree* c1, astree* c2, int symbol) {
   adopt1 (root, c1);
   adopt1 (root, c2);
   root->symbol = symbol;
   return root;
}

astree* changesym (astree* root, int symbol) {
   root->symbol = symbol;
   return root;
}

static void tok_node (FILE* outfile, astree* node) {
   fprintf (outfile, "%4ld%4ld.%03ld %4d  %-16s (%s)",
            node->filenr,  node->linenr, node->offset, node->symbol,
            get_yytname (node->symbol), node->lexinfo->c_str());
}
static void tok_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   tok_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child) {
      tok_rec (outfile, root->children[child], depth + 1);
   }
}
void dump_tok (FILE* outfile, astree* root) {
   tok_rec (outfile, root, 0);
   fflush (NULL);
}



RCSC("$Id: astree.cpp,v 1.6 2015-04-09 19:31:47-07 - - $")

