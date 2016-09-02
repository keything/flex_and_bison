fb_3_1: fb_3_1.l fb_3_1.y fb_3_1.h
	bison -d fb_3_1.y
	flex -o fb_3_1.lex.c fb_3_1.l
	gcc -o $@ fb_3_1.tab.c fb_3_1.lex.c fb_3_1.funcs.c
clean:
	rm -rf fb_3_1.lex.c fb_3_1 fb_3_1.tab.c fb_3_1.tab.h
