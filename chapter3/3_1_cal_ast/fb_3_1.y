/**基于ast的计算器*/

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "fb_3_1.h"
%}

/*第一部分使用%union来生命语法分析器中符号值的类型
一旦联合类型被定义，我们需要告诉bison每种语法符号使用的值类型，这通过放置在尖括号<>中的联合类型的响应成员名字来确定。
新的声明 %type把值<a>赋给exp, factor, term，当我们创建ast时会用到他们
如果你不使用记号或者非终结符的值，你并不需要为他们声明类型。
当声明中存在%union时，如果你试图使用一个没有被赋予类型的符号值，bison将会报错。
*/
%union {
    struct ast *a;
    double d;
}

%token <d> NUMBER
%token EOL
%type <a> exp factor term

%%
calclist:/*空规则*/
    | calclist exp EOL { 
        printf("=%4.4g\n", eval($2)); 
        treefree($2);
        printf("> ");
    }
    | calclist EOL { printf(">");} /*空行或注释*/
    ;

exp: factor 
   | exp '+' factor { $$ = newast('+', $1, $3);}
   | exp '-' factor { $$ = newast('-', $1, $3);}
   ;
factor: term  
   | factor '*' term { $$ = newast('*', $1, $3); }
   | factor '/' term { $$ = newast('/', $1, $3); }
   ;
term: NUMBER { $$ = newnum($1); }
    | '|' term {$$ = newast('|', $2, NULL);}
    |'(' exp ')' {$$ = $2;}
    | '-' term {$$ = newast('M', $2, NULL);}
    ;
%% 

void
yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}
int
main(int argc, char **argv)
{
    printf("> ");
    return yyparse();
}
