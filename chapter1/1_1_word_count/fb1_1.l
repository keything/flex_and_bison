
/**
例1-1：字数统计

*/
%option noyywrap
%{
int chars = 0;
int words = 0;
int lines = 0;
%}
/*
1. 变量yytext总是被设为指向本次匹配的输入文本
2. . 在正则表达式中代表任意一个字符

**/
%%
[a-zA-Z]+ { words++; chars += strlen(yytext); }
\n        {chars++; lines++;}
.         {chars++;}
%%

int main(int argc, char **argv)
{
    yylex();
    printf("%8d%8d%8d", lines, words, chars);
}
