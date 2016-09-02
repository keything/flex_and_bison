#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "fb_3_1.h"

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

    a->nodetype = 'k';
    a->number = d;
    return (struct ast *)a;

}

double 
eval(struct ast *a)
{
    double v;

    printf("nodetype:%c\n", a->nodetype);
    if (a->nodetype == 'k') {
        printf("nodetype:%f\n", ((struct numval *)a)->number);
    }
    switch(a->nodetype) {
        case 'k': v = ((struct numval *)a)->number;break;
        case '+': v = eval(a->l) + eval(a->r); break;
        case '-': v = eval(a->l) - eval(a->r); break;
        case '*': v = eval(a->l) * eval(a->r); break;
        case '/': v = eval(a->l) / eval(a->r); break;
        case '|': v = eval(a->l); if (v < 0) v = -v; break;
        case 'M': v = eval(a->l); break;
        default: printf("internal error: bad node %c\n", a->nodetype);
    }
    return v;
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
            treefree(a->r);
       /*一颗子树*/
        case '|':
        case 'M':
            treefree(a->l);
       /*没有子树*/
        case 'k':
            free(a);
            break;
        default: printf("internal error: bad node %c\n", a->nodetype);
    }
}
