/**
Flex词法分析器中的文件I/O操作
词法分析器总是通过名为yyin的文件句柄读取输入。
为了让它读取一个单一文件，只需要在第一次调用yylex之前重新设定yyin。

关于flex库：
    flex和lex总是提供一个小型的库，叫做-lfl,它定义了默认的main程序和早期lex遗留至今的鸡肋---默认版本的yywrap。
    当lex词法分析器到达yyin的结束位置时，将调用yywrap()。这样做的目的是：当有另一个输入文件时，yywrap可以调整yyin的值并且通过返回0来重新开始词法分析。
    如果当前就是真正的输入结束位置，可以通过返回1来完成分析。
    

    flex 的比较新的版本允许你在词法分析器的开头设定%option noyywrap来要求不使用yywrap.

*/

%option noyywrap
%{
int chars = 0;
int words = 0;
int lines = 0;
%}

%% 
[a-zA-Z]+ {words++;chars += strlen(yytext);}
\n        {chars++; lines++;}
.         {chars++;}

%%
int main(int argc, char ** argv)
{
    if (argc >1) {
        if (! (yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return (1);
        }
    }
    yylex();
    printf("%8d%8d%8d\n", lines, words, chars);
}
