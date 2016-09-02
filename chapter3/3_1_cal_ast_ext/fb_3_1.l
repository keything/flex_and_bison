/**识别计算器的记号*/

%option noyywrap nodefault yylineno
%{
#include "fb_3_1.h"
#include "fb_3_1.tab.h"
%}
/*浮点数指数部分*/

EXP ([Ee][-+]?[0-9]+)

%%

"+"|
"-"|
"*"|
"/"|
"("|
")" {return yytext[0];}
[0-9]+"."[0-9]*{EXP}?|
"."?[0-9]+{EXP}?{yylval.d = atof(yytext);return NUMBER;}

\n {return EOL;}
"//".*
[	\t]{/*忽略空白字符*/}
. {yyerror("Mystery character %c\n",*yytext);}
%%
