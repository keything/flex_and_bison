/**
 * 基于抽象语法树的改进的计算器
 */

/** 与词法分析器的解口*/
extern int yylineno; /*来自于词法分析器*/
void yyerror(char *s, ...);

/** ast中的节点*/
struct ast {
    int nodetype;
    struct ast *l;
    struct ast *r;
};

struct numval {
    int nodetype; /**类型k表明常量*/
    double number;
};

/** 构造ast*/
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newnum(double d);

/** 计算ast*/
double eval (struct ast *);
/**删除和释放ast*/
void treefree(struct ast *);
/**
 * 抽象语法树由多个节点组成，每个节点都有一个节点类型。不同的节点可以有不同的域，但目前我们只有两种类型，一种是执行最多两个子节点的指针，另外一种包含一个数组。eval遍历ast，返回它所代表的表达式的值。
 *
 *
 * */
