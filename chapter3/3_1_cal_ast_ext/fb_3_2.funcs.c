/**
 * 例3-8 高级计算器辅助函数fb_3_2.funcs.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "fb_3_2.h"

/**符号表*/
/*哈希一个符号*/

static unsigned
symhash(char *sym)
{
    unsigned int hash = 0;
    unsigned c;

    while (c = *sym++) {
        hash = hash * 9 ^ c;
    }

    return hash;
}

struct symbol * 
lookup(char *sym)
{
    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH; /**需要查看的个数*/

    while (--scount >= 0) {
        if (sp->name && ! strcmp(sp->name, sym)) {
            return sp;
        }

        if (!sp->name) {/**新条目*/
            sp->name  = strdup(sym);
            sp->value = 0;
            sp->func  = NULL;
            sp->syms  = NULL;
            return sp;
        }

        if (++sp >= symtab + NHASH) {/**尝试下一个条目*/
            sp = symtab; 
        }
    }
    yyerror("symbol table overflow\n");
    abort();/**尝试完所有的条目，符号表已满*/
}

struct ast *
newast(int nodetype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *
newnum(double d)
{
    struct numval *a = malloc(sizeof(struct numval));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = 'K';
    a->number = d;
    return (struct ast *)a;

}

struct ast *
newcmp(int cmptype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = 'O' + cmptype;
    a->l = l;
    a->r = r;
    return a;

}

struct ast *
newfunc(int functype, struct ast *l)
{
    struct fncall *a = malloc(sizeof(struct fncall));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = 'F';
    a->l = l;
    a->functype = functype;
    return (struct ast*)a;
}

struct ast *
newcall(struct symbol *s, struct ast *l)
{
    struct ufncall *a = malloc(sizeof(struct ufncall));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = 'C';
    a->l = l;
    a->s = s;
    return (struct ast*)a;
}

struct ast *
newref(struct symbol *s)
{
    struct symref *a = malloc(sizeof(struct symref));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = 'N';
    a->s = s;
    return (struct ast*)a;
}

struct ast *
newasgn(struct symbol *s, struct ast *v)
{
    struct symasgn *a = malloc(sizeof(struct symasgn));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = '=';
    a->s = s;
    a->v = v;
    return (struct ast*)a;
}

struct ast *
newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
    struct flow *a = malloc(sizeof(struct flow));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;
    return (struct ast*)a;
}

struct symlist *
newsymlist(struct symbol *sym, struct symlist *next)
{
    struct symlist *a = malloc(sizeof(struct symlist));
    if (!a) {
        printf("out of space");
        exit(0);
    }

    a->sym = sym;
    a->next = next;
    return a;
}

void
treefree(struct ast *a)
{
    switch(a->nodetype) {
        /**两颗子树*/
        case '+':
        case '-':
        case '*':
        case '/':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case 'L':
            treefree(a->r);
       /*一颗子树*/
        case '|':
        case 'M':
        case 'C':
        case 'F':
            treefree(a->l);
       /*没有子树*/
        case 'K':
        case 'N':
            break;
        case '=':
            free( ((struct symasgn *)a)->v);
            break;

        case 'I':
        case 'W':
            free(((struct flow*)a)->cond);
            if (((struct flow*)a)->tl) {
                    treefree(((struct flow*)a)->tl);
                    }
            if (((struct flow*)a)->el) {
                    treefree(((struct flow*)a)->el);
                    }
            break;
        default: printf("internal error: bad node %c\n", a->nodetype);
    }
    free(a); /**总是释放节点自身*/
}

/**释放符号列表*/
void
symlistfree(struct symlist *sl)
{
    struct symlist *nsl;

    while (sl) {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}
/**计算器的核心是eval，它计算语法分析器中构造的抽象语法树。
 * 基于C语言的准则，比较表达式根据比较是否成功来返回1或者0.
 * 我们采用相似的深度优先的树遍历算法来计算表达式的值。
 * 抽象语法树使if/then/else的实现变得直接：计算条件表达式的抽象语法树来决定选择哪个分支，然后计算选择路径上的抽象语法树。
 * 对于do/while循环来说，eval中的一个循环会不断的计算条件表达式，然后执行语句体的抽象语法树，直到条件表达式的抽象语法树不再为真。
 *
 * 当变量被赋值语句修改后，任何使用这个变量的抽象语法树都会基于新值进行计算。
 */

static double callbuiltin(struct fncall *);
static double calluser(struct ufncall *);

double 
eval(struct ast *a)
{
    double v;
    
    if (!a) {
        yyerror("internal error, null eval");
        return 0.0;
    }

    /*
    printf("nodetype:%c\n", a->nodetype);
    if (a->nodetype == 'k') {
        printf("nodetype:%f\n", ((struct numval *)a)->number);
    }
    */
    switch(a->nodetype) {
        case 'K': /*常量*/
            v = ((struct numval *)a)->number;
            break; 
        case 'N':  /*名字引用*/
            v = ((struct symref*)a)->s->value;
            break;
        case '=': /*名字引用*/
            v = ((struct symasgn*)a)->s->value = eval(((struct symasgn*)a)->v);
            break; 
        /**表达式*/
        case '+': 
            v = eval(a->l) + eval(a->r); 
            break;
        case '-': 
            v = eval(a->l) - eval(a->r); 
            break;
        case '*': 
            v = eval(a->l) * eval(a->r);
            break;
        case '/': 
            v = eval(a->l) / eval(a->r);
            break;
        case '|': 
            v = fabs(eval(a->l));
            break;
        case 'M': 
            v = -eval(a->l); 
            break;
        /**比较*/
        case '1':
            v = (eval(a->l) > eval(a->r)) ? 1 : 0;
            break;
        case '2':
            v = (eval(a->l) < eval(a->r)) ? 1 : 0;
            break;
        case '3':
            v = (eval(a->l) != eval(a->r)) ? 1 : 0;
            break;
        case '4':
            v = (eval(a->l) == eval(a->r)) ? 1 : 0;
            break;
        case '5':
            v = (eval(a->l) >= eval(a->r)) ? 1 : 0;
            break;
        case '6':
            v = (eval(a->l) <= eval(a->r)) ? 1 : 0;
            break;
        /**控制流*/
        /**语法中允许空表达式，所以需要检查这种可能性*/
        /**if/then/else*/
        case 'I':
            if (eval(((struct flow*)a)->cond) != 0) {/**检查条件*/
                if (((struct flow*)a)->tl) {/**条件成立分支*/
                    v = eval(((struct flow*)a)->tl);
                } else {
                    v = 0.0; /*默认值*/
                }
            } else { /**条件不成立分支*/
                if (((struct flow*)a)->el) {
                    v = eval(((struct flow*)a)->el);
                } else {
                    v = 0.0; /*默认值*/
                }
            }
            break;
        /**while/do*/
        case 'W':
            v = 0.0; /*默认值*/

            if (((struct flow*)a)->tl) {
                while (eval(((struct flow*)a)->cond) != 0) { /**计算条件*/
                    v = eval(((struct flow *)a)->tl); /*执行目标语句体*/
                }
            }
            break; /**最后一条语句的值就是while/do的值*/
        /**语句列表*/
        case 'L':
            eval(a->l);
            v = eval(a->r);
            break;
        case 'F':
            v = callbuiltin((struct fncall*)a);
            break;
        case 'C':
            v = calluser((struct ufncall*)a);
            break;

        default: printf("internal error: bad node %c\n", a->nodetype);
    }
    return v;
}

/**执行计算器中的函数*/
/*计算器中最特别的部分是处理函数。
 * 内置函数的实现很简单：他们确定具体的函数，然后调用相应的代码来执行这个函数
 */

static double 
callbuiltin(struct fncall *f)
{
    enum bifs functype = f->functype;
    double v = eval(f->l);

    switch (functype) {
        case B_sqrt:
            return sqrt(v);
        case B_exp:
            return exp(v);
        case B_log:
            return log(v);
        case B_print:
            printf("= %4.4g\n", v);
            return v;
        default:
            yyerror("Unknown built-in function %d", functype);
            return 0.0;
    }
}
/**用户自定义函数*/
/**函数定义包含函数名，虚拟参数列表，代表函数体的抽象语法树
 * 当函数被定义时，参数列表和抽象语法树将被简单地保存到符号表中函数名对应的条目中，同时替换了任何可能的旧版本。
 *
 */
void
dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
    if (name->syms) {
        symlistfree(name->syms);
    }
    if (name->func) {
        treefree(name->func);
    }
    name->syms = syms;
    name->func = func;
}
/**
 * 比如你定义一个函数来计算两个参数中的最大值
 * lex max（x,y) = if x >= y then x; else y;;
 * max(4+5, 6+7)
 *
 * 这个函数有两个虚拟参数，x和y，当函数被调用时，求值程序将如下执行：
 * 1. 计算实际参数值，这个例子中就是4+5, 6+7
 * 2. 保存虚拟参数的当前值，然后把实际参数值赋值给它们
 * 3. 执行函数体，在涉及到虚拟参数的地方都使用实际参数值
 * 4. 将虚拟参数恢复原值
 * 5. 返回函数体表达式的值
 * */

static double
calluser(struct ufncall *f)
{
    struct symbol *fn = f->s;       /*函数名*/
    struct symlist *sl;             /*虚拟参数*/
    struct ast *args = f->l;        /*实际参数*/
    double *oldval, *newval;        /*保存的参数值*/
    double v;
    int nargs;
    int i;

    if (!fn->func) {
        yyerror("call to undefined function:", fn->name);
        return 0;
    }

    /**计算参数个数*/
    sl = fn->syms;
    for (nargs = 0; sl; sl = sl->next) {
        nargs++;
    }
    /**为保存参数值做准备*/
    for (i = 0; i < nargs; i++) {
        if (!args) {
            yyerror("too few args in call to %s", fn->name);
            free(oldval);
            free(newval);
            return 0.0;
        }

        if (args->nodetype == 'L') {/*是否是节点列表*/
            newval[i] = eval(args->l);
            args = args->r;
        } else { /*是否是列表末尾*/
            newval[i] = eval(args);
            args = NULL;
        }
    }

    /*保存虚拟参数的旧值，赋予新值*/
    sl = fn->syms;
    for (i = 0; i < nargs; i++) {
        struct symbol *s = sl->sym;
        oldval[i] = s->value;
        s->value = newval[i];
        sl = sl->next;
    }
    free(newval);
    /**计算函数*/
    v = eval(fn->func);
    /*恢复虚拟参数的值*/
    sl = fn->syms;
    for (i = 0; i < nargs; i++) {
        struct symbol *s = sl->sym;
        s->value = oldval[i];
        sl = sl->next;
    }
    free(oldval);
    return v;
}






