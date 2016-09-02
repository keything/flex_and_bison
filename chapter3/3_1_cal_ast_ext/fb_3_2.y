/**基于ast的计算器*/

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "fb_3_2.h"
%}

/*
第一部分使用%union来生命语法分析器中符号值的类型
一旦联合类型被定义，我们需要告诉bison每种语法符号使用的值类型，这通过放置在尖括号<>中的联合类型的响应成员名字来确定。
新的声明 %type把值<a>赋给exp, factor, term，当我们创建ast时会用到他们
如果你不使用记号或者非终结符的值，你并不需要为他们声明类型。
当声明中存在%union时，如果你试图使用一个没有被赋予类型的符号值，bison将会报错。
*/
%union {
    struct ast *a;
    double d;
    struct symbol *s; /*指定符号*/
    struct symlist *sl; 
    int fn;           /*指定函数*/
}
/**正如符号（symbol）值可以指向抽象语法树的指针或者一个具体的数值那样，符号值也可以指向符号表中特定用户符号user symbol,符号列表，比较子类型和函数记号的指针

这儿有一个新记号 FUNC表示内置函数，它的值确定了具体的某个函数，另外还有6个保留字，从IF到LET。记号CMP是6中比较操作符之一，它的值确定了具体的操作符。

优先级声明列表从新的CMP和=操作符开始

*/

%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token EOL

%token IF THEN ELSE WHILE DO LET

/*优先级声明*/
%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS

%type <a> exp stmt list explist
%type <sl> symlist

/**%start 声明定义了顶层规则，所以我们不需要把这条规则放到语法分析器的开始部分*/
%start calclist
%%

/**语法区分了语句stmt和表达式exp。语句是一个控制流(if/then/else或者while/do）或者是一个表达式。
if和while语句具有一连串语句，这里面的语句都用分号结尾。每当规则匹配一条语句时，它调用相应的函数来创建合适的抽象语法树节点
*/
/*stmt和list是计算器语句的语法*/
stmt: IF exp THEN list { $$ = newflow('I', $2, $4, NULL); }
    | IF exp THEN list ELSE list { $$ = newflow('I', $2, $4, $6); }
    | WHILE exp DO list { $$ = newflow('W', $2, $4, NULL); }
    | exp
    ;
/**
list的定义是右递归的，也就是说是stmt;list 而不是list stmt ;这对于语言识别没有任何差异，但是我们因此会更容易构造一个从头到尾而不是相反方向的语句链表。
每当stmt; list规则被归约时，它创建一个链接来把语句添加到当前链表的头部。如果规则是list stmt ; 语句需要被添加到链表的尾部，这就要求我们使用一个更复杂的循环链表或者反转整个链表

使用右递归而不是左递归的缺点是它把所有需要被归约的语句都放在语法分析器的堆栈里，直到链表的尾部才进行归约，而左递归在分析输入时每次遇到语句都会like将其添加到链表中。
左递归可以避免堆栈溢出并且容易调试

*/
list: /*空*/ { $$ = NULL;}
    | stmt ';' list { if ($3 == NULL)
                        $$ = $1;
                        else 
                        $$ = newast('L', $1, $3);
                    }
    ;
/*计算器表达式的语法*/
exp: exp CMP exp { $$ = newcmp($2, $1, $3);}
   | exp '+' exp { $$ = newast('+', $1, $3);}
   | exp '-' exp { $$ = newast('-', $1, $3);}
   | exp '*' exp { $$ = newast('*', $1, $3);}
   | exp '/' exp { $$ = newast('/', $1, $3);}
    | '|' exp    { $$ = newast('|', $2, NULL);}
    |'(' exp ')' { $$ = $2;}
    | '-' exp %prec UMINUS { $$ = newast('M', $2, NULL);}
    | NUMBER { $$ = newnum($1); }
    | NAME   { $$ = newref($1); }
    | NAME '=' exp  { $$ = newasgn($1, $3); }
    | FUNC '(' explist ')' { $$ = newfunc($1, $3); }
    | NAME '(' explist ')' { $$ = newcall($1, $3); }
;
explist: exp
       | exp ',' explist { $$ = newast('L', $1, $3); }
   ;
/**规则explist定义了一个表达式列表，它创建这些表达式的抽象语法树来作为函数调用所需要的实际参数
规则symlist定义了一个符号列表，它创建了这些符号的链表用于函数定义时所需要的虚拟参数。他们都是右递归的，以方便基于期望的顺序创建链表*/
symlist: NAME { $$ = newsymlist($1, NULL); }
       | NAME ',' symlist { $$ = newsymlist($1, $3);}
   ;

/**
计算器的顶层规则
像前面那样，顶层规则计算每条语句的抽象语法树，打印结果，然后释放抽象语法树。
当前，保存函数定义只是便于后续使用
*/
calclist: /*空*/
        | calclist stmt EOL {
            printf("= %4.4g\n>", eval($2));
            treefree($2);
            }
        | calclist LET NAME '(' symlist ')' '=' list EOL {
            dodef($3, $5, $8);
            printf("Defined %s\n>", $3->name);
            }
        | calclist error EOL { yyerrok; printf("> "); }
        ;
/**
我们至少有可能在错误发生时把语法分析器恢复到可以继续工作的状态。在这里，特殊的伪记号error确定了错误恢复点。
当bison语法分析器遇到一个错误时，它开始从语法分析器堆栈里放弃各种语法符号，直到它到达一个记号error为有效的点；
接着它开始忽略后续输入记号，直到它找到一个在当前状态可以被移进的记号，然后从这一点开始继续分析。
为了避免大段误导性的错误信息，语法分析器通常在第一个错误产生后就抑制后续的分析错误消息，直到它能够成功的在一行里移进三个记号。
动作中的宏yyerrok告诉语法分析器恢复已经完成，这样后续的错误消息可以顺利产生。



*/
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
