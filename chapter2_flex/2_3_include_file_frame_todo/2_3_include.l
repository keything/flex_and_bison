/**
## 起始状态和嵌套输入文件
目的：处理嵌套的包含文件include file，并打印出来
使用flex特性：使用一个很强大的flex特性--起始状态（start state)，它允许我们指定在特定时刻哪些模式可以被用来匹配。
在靠近文件顶端的%x行把IFILE定义为起始状态，它将在我们寻找#include语句中的文件名时被使用。在任何时候，这个词法分析器都处在一个起始状态中，并且只匹配这个状态锁激活的那些模式。实际上，起始状态定义了一个不同的词法分析器，拥有自己的规则。

fb_2_3 包含文件的框架
// todo 没有执行成功
*/

%option noyywrap
%x IFILE

%{
struct bufstack {
    struct bufstack *prev;      /*上一个文件信息*/
    YY_BUFFER_STATE bs;         /*保存的缓冲区*/
    int lineno;                 /*保存的行号*/
    char *filename;             /**文件名*/
    FILE *f;                    /*当前文件*/
} *curbs = 0;

char *curfilename; /*当前输入文件的名称*/

int newfile(char *fn);
int popfile(void);
%}

%%
^"#"[ \t]*include[ \t]*\[\"<] }BEGIN IFILE;}
<IFILE>[^ \t\n\">]+ {
    {int c;
    while ((c = input()) && c != '\n');
    }
    yylineno++;
    if (! newfile(yytext))
        yyterminate(); /*no such file*/
    BEGIN INITIAL;
}
<IFILE>.|\n {
    fprintf(stderr, "%4d bad include line\n", yylineno);
    yyterminate();
}

<<EOF>> {if (!popfile()) yyterminate();}

^.      {fprintf(yyout, "%4d %s", yylineno, yytext);}
^\n     {fprintf(yyout, "%4d %s", yylineno++, yytext);}
\n      {ECHO; yylineno++;}
.       {ECHO;}
%%

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "need filename\n");
        return 1;
    }

    if (newfile(argv[1])) {
        yylex();
    }
}

int newfile(char *fn)
{
    FILE *f = fopen(fn, "r");
    struct bufstack *bs = malloc(sizeof(struct bufstack));

    /**如果文件打开失败或者没有足够空间时就退出*/
    if (!f) { perror(fn); return 0;}
    if (!bs) {perror("malloc"); exit(1);}

    /**记住当前状态*/
    if (curbs)curbs->lineno = yylineno;
    bs->prev = curbs;

    /**建立当前文件信息*/
    bs->bs = yy_create_buffer(f, YY_BUF_SIZE);
    bs->f  = f;
    bs->filename = fn;
    yy_switch_to_buffer(bs->bs);
    curbs = bs;
    yylineno = 1;
    curfilename = fn;
    return 1;
}

int popfile(void)
{
    struct bufstack *bs = curbs;
    struct bufstack *prevbs;

    if (!bs) return 0;

    /**删除当前文件信息*/
    fclose(bs->f);
    yy_delete_buffer(bs->bs);

    /*切换回上一个文件*/
    prevbs = bs->prev;
    free(bs);

    if (!prevbs) return 0;

    yy_switch_to_buffer(prevbs->bs);
    curbs = prevbs;
    yylineno = curbs->lineno;
    curfilename = curbs->filename;
    return 1;
}
