%{
#include "y.tab.h"
%}

%%
(cd)              {yylval.str = strdup(yytext); return (CD);}
(exit)            {yylval.str = strdup(yytext);return (EXIT);}
(echo)            {yylval.str = strdup(yytext);return (AECHO);}
(&)               return (BG);
(\|)              return (PIPE);
(\>)               return (OUT);
">>"              return (APPEND);
(\<)               return (IN);
[a-zA-Z0-9\/.\^-]+[ \ta-zA-Z0-9\/.\^-]*[a-zA-Z0-9\/.\^-]+ {
  yylval.str = strdup(yytext);;
  return (AFILE);
}
[a-zA-Z0-9\/.\^-]+ {  yylval.str = strdup(yytext);;
  return (AFILE);
}
[ \t]            ;
[\n]             return 0;
.                 printf("Syntax Error: %s\n", yytext);
%%

int yywrap(void) {
     return 1;
}
