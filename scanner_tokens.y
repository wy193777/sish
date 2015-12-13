%{
#include "sish.h"
struct task *top_task = NULL;
struct task *curr_task = NULL;
%}
%token <str> CD
%token <str> EXIT
%token <str> AECHO
%token <str> AFILE
%token OUT
%token APPEND
%token IN
%token PIPE
%token BG

%union {
    char * str;
    struct task *tsk;
    struct out_append *oas;
    struct  redirect *red;
}
%type <tsk> expr
%type <str> command  in_file
%type <oas> out_file
%type <red> any_files

%start line
%%
line : expr
       {
         if (top_task == NULL) {
           top_task = $1;
           curr_task = top_task;
         }
         else {
           curr_task->next = $1;
           curr_task = curr_task->next;
         }
       }
     | line PIPE expr
       {
         curr_task->next = $3;
         curr_task = curr_task->next;
       }
     | '\n' {printf("Finish");}
     ;

expr : command any_files
        {
          $$ = new_task($1, $2->in, $2->out, $2->append, 0, NULL);
        }
     | command any_files BG
        {
            $$ = new_task($1, $2->in, $2->out, $2->append, 1, NULL);
        }
     ;


out_file   :       OUT AFILE {$<oas>$ = new_o_a($2, NULL);}
           |       APPEND AFILE {$<oas>$ = new_o_a(NULL, $2);}
           ;

in_file   :        IN AFILE {$<str>$ = $2;}
          ;

any_files  :       /* empty */ { $<red>$ = new_redirect(NULL, NULL, NULL);}
           |       in_file {$<red>$ = new_redirect($1, NULL, NULL);}
           |       out_file {$<red>$ = new_redirect(NULL, $1->out, $1->append);}
           |       in_file out_file
                   {
                     $<red>$ = new_redirect($1, $2->out, $2->append);
                   }
           |       out_file in_file
                   {
                     $<red>$ = new_redirect($2, $1->out, $1->append);
                   }
           ;

command    :       AFILE  {$$ = $1;}
           |       CD     {$$ = $1;}
           |       AECHO  {$$ = $1;}
           |       EXIT   {$$ = $1;}
           ;


%%
