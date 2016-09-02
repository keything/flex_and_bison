#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "fb_3_1.h"

struct ast *
newast(int nodetype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (!a) {
        yyerror("out of space");
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
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = 'K';
    a->number = d;
    return (struct ast *)a;

}

double 
eval(struct ast *a)
{
    double v;

    switch(a->nodetype) {
    }
}
