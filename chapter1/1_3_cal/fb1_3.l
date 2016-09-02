%{
%}

/**
前5个模式就是操作符本身，用引号引起。引号告诉flex使用引号内文本的原义，而不是解释为正则表达式
*/
%%
"+" {printf("PLUS");}
"-" {printf("MINUS");}
"*" {printf("MUL");}
"/" {printf("DIVIDE");}
"|" {printf("ABS");}
[0-9]+ {printf("NUMBER %s", yytext);}
\n {printf("NEWLINE");}
[ \t]  {printf("WHITE SPACE");}
.  {printf("Mystery character:%s", yytext);}
%%
