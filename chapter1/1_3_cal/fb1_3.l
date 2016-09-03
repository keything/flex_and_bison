%option noyywrap
%{
%}

/**前5个模式就是操作符本身，用引号引起。引号告诉flex使用引号内文本的原义，而不是解释为正则表达式*/
%%
"+" {printf("PLUS\n");}
"-" {printf("MINUS\n");}
"*" {printf("MUL\n");}
"/" {printf("DIVIDE\n");}
"|" {printf("ABS\n");}
[0-9]+ {printf("NUMBER %s\n", yytext);}
\n {printf("NEWLINE\n");}
[ \t]  {printf("WHITE SPACE\n");}
.  {printf("Mystery character:%s\n", yytext);}
%%
/**
大多数包含flex词法分析器的程序使用词法分析器来获得一个记号流。
每当程序需要一个记号时，调用yylex()来读取一小部分输入然后返回相应的记号。
当程序需要下一个记号时，yylex会再次调用。

*/
int main(int argc, char **argv)
{
    yylex();
}
