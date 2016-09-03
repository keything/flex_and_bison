%option noyywrap
%{
    enum yytokentype {
        NUMBER  = 258,
        ADD     = 259,
        SUB     = 260,
        MUL     = 261,
        DIV     = 262,
        ABS     = 263,
        EOL     = 264
    };
    int yylval; /*使用yylval用来存储记号值**/
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
[0-9]+ {yylval = atoi(yytext); return NUMBER;}
\n {return EOL;}
[ \t]  {}
.  {printf("Mystery character:%c\n", *yytext);}
%%
int main(int argc, char **argv)
{
    int tok;
    while (tok = yylex()) {
        printf("%d\n", tok);
        if (tok == NUMBER) {
            printf(" = %d\n", yylval);
        } else {
            printf("\n");
        }
    }
}
