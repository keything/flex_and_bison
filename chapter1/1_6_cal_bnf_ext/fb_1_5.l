%{
#include "fb_1_5.tab.h"
%}

/**
前5个模式就是操作符本身，用引号引起。引号告诉flex使用引号内文本的原义，而不是解释为正则表达式
*/
%%
"+" {return ADD;}
"-" {return SUB;}
"*" {return MUL;}
"/" {return DIV;}
"|" {return ABS; }
"(" {return LB; }
")" {return RB; }
[0-9]+ {yylval = atoi(yytext); return NUMBER;}
\n {return EOL;}
[ \t]  {}
.  {printf("Mystery character:%c\n", *yytext);}
%%
