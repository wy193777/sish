%{
#include "sish.h"
%}

%%
(cd)              return (CD);
(exit)            return (EXIT);
(echo)            return (AECHO);
(&)               return (BG);
(\|)              return (PIPE);
(>)               return (OUT);
(>>)              return (APPEND);
(<)               return (IN);
[a-zA-Z0-9\/.-^]+[:blank:]*[a-zA-Z0-9\/.-^]+        return (AFILE);
[ \t\n]            ;
.                 printf("Syntax Error");
%%

int yywrap(void) {
     return 1;
}
