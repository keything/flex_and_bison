/**
 * 基于抽象语法树的改进的计算器
 * 例3-5
 */

/** 与词法分析器的解口*/
extern int yylineno; /*来自于词法分析器*/
void yyerror(char *s, ...);

/**符号表*/
struct symbol {/**变量名*/
    char *name;
    double value;
    struct ast *func; /*函数体*/
    struct symlist *syms; /**虚拟参数列表*/
};
/**固定大小的简单的符号表*/
#define NHASH 9997
struct symbol symtab[NHASH];

struct symbol *lookup(char*);
/**符号列表，作为参数列表*/
struct symlist {
    struct symbol *sym;
    struct symlist *next;
};
struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);
/**在计算器中，每个符号都有一个变量和一个用户自定义函数。value域保存符号的值，func域指向用ast表述的该函数的用户代码，而sym域执行任意多个虚拟参数的链表。在前面的例子中avg是参数，a和b是虚拟参数*/

/*节点类型
 * + - * / |
 * 0-7 比较操作符 位编码 04 等于 02小于 01大于
 * M 单目负号
 * L 表达式或者语句列表
 * I IF语句
 * W WHILE 语句
 * N 符号引用
 * = 赋值
 * S 符号列表
 * F 内置函数调用
 * C 用户函数调用
 */
enum bifs {/*内置函数*/
    B_sqrt = 1,
    B_exp,
    B_log,
    B_print
};

/** ast中的节点*/
/**所有节点都有公共的初始nodetype*/
struct ast {
    int nodetype;
    struct ast *l;
    struct ast *r;
};

struct fncall {/*内置函数*/
    int nodetype; /*类型F*/
    struct ast *l;
    enum bifs functype;
};

struct ufncall {/*用户自定义函数*/
    int nodetype; /*类型C*/
    struct ast *l; /*参数列表*/
    struct symbol *s;
};

struct flow {
    int nodetype; /*类型I或者W*/
    struct ast *cond; /*条件*/
    struct ast *tl; /*then分支或者do语句*/
    struct ast *el; /*可选else分支*/
};

struct numval {
    int nodetype; /**类型K表明常量*/
    double number;
};

struct symref {
    int nodetype; /*类型N*/
    struct symbol *s;
};

struct symasgn {
    int nodetype; /*类型 =*/
    struct symbol *s;
    struct ast *v; /*值*/
};

/** 构造ast*/
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(double d);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

/*定义函数*/
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/** 计算ast*/
double eval (struct ast *);
/**删除和释放ast*/
void treefree(struct ast *);
/**
 * 抽象语法树由多个节点组成，每个节点都有一个节点类型。不同的节点可以有不同的域，但目前我们只有两种类型，一种是执行最多两个子节点的指针，另外一种包含一个数组。eval遍历ast，返回它所代表的表达式的值。
 * */
