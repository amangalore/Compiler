#include <string>
#include <vector>
using namespace std;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <wait.h>
#include <unistd.h>

#include <alloca.h>
#include <cstring>

#include "stringset.h"
#include "auxlib.h"
#include "lyutils.h"
#include "astree.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;
char* flagD;
bool condD = false; 
int yyydebug = 0;
string yyin_cpp_command;
void yyin_cpp_popen (const char* filename) {
   yyin_cpp_command = CPP;
   yyin_cpp_command += " ";
   yyin_cpp_command += filename;
   yyin = popen (yyin_cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (yyin_cpp_command.c_str());
      exit (get_exitstatus());
   }
}
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}
void cpplines() {
   for ( int token = yylex(); token != YYEOF ; token = yylex()){
      intern_stringset(yytext); 
   }
}

void scan_opts (int argc, char** argv) {
   int option;
   yy_flex_debug = 0;
   yydebug = 0;
   for(;;) {
      option = getopt (argc, argv, "@:D:ly");
      if (option == EOF) break;
      switch (option) {
         case '@': set_debugflags (optarg);      break;
         case 'D': flagD = optarg; condD = true; break; 
         case 'l': yy_flex_debug = 1;            break;
         case 'y': yydebug = 1;                  break;
         default:  errprintf ("%:bad option (%c)\n", optopt); break;
      }
   } 
   if (optind > argc) {
      errprintf ("Usage: %s [-ly] [filename]\n", get_execname());
      exit (get_exitstatus());
   }  
}

string basename_ext(char** argv, int argc) { 
  char* base = basename(argv[argc - 1]);    
  char* ext = strchr(base, '.');                       
  std::string basestr = std::string(base);  
 
  if(ext == NULL || strcmp(ext + 1, "oc") != 0) {      
    fprintf(stderr, "Error: Invalid extension on file\n");
    exit(1);
  } else {
    size_t dotext = basestr.find_last_of('.');   
    string filename = basestr.substr(0, dotext); 
    return filename; 
  }
}

int main (int argc, char **argv) {
   set_execname(argv[0]); 
   
   
   string strfilename = basename_ext(argv, argc);
   string tokfilename = strfilename;
   string astfilename = strfilename;
   strfilename = strfilename.append(".str"); 
   tokfilename = tokfilename.append(".tok");   
   astfilename = astfilename.append(".ast"); 
   scan_opts(argc, argv);
   FILE *strfile;
   tokfile = fopen(tokfilename.c_str(), "w");
   FILE *astfile;

   char* filename = argv[argc-1];
  
   string command; 
   command = (condD == true)? CPP + " -D" + flagD + " " + filename : 
     CPP + " " + filename;

   yyin = popen (command.c_str(), "r");
   if (yyin == NULL) {
     syserrprintf (command.c_str());
   }else {
     yyparse();
     int pclose_rc = pclose (yyin);
     eprint_status (command.c_str(), pclose_rc);
   } 
  
   strfile = fopen(strfilename.c_str(), "w"); 
   astfile = fopen(astfilename.c_str(), "w");
   dump_stringset(strfile);
   dump_astree(astfile, yyparse_astree);
   fclose(strfile);
   fclose(tokfile);
   fclose(astfile);
   return get_exitstatus();

}
