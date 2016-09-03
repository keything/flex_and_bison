fb_1_5: fb_1_5.l fb_1_5.y
	bison -d fb_1_5.y
	flex fb_1_5.l
	gcc -o $@ fb_1_5.tab.c lex.yy.c
clean:
	rm -rf fb_1_5.tab.c lex.yy.c fb_1_5 fb_1_5.tab.h
