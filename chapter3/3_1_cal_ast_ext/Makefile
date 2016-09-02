fb_3_2: fb_3_2.l fb_3_2.y fb_3_2.h
	bison -d fb_3_2.y
	flex -o fb_3_2.lex.c fb_3_2.l
	gcc -o $@ fb_3_2.tab.c fb_3_2.lex.c fb_3_2.funcs.c -lm -g
clean:
	rm -rf fb_3_2.lex.c fb_3_2 fb_3_2.tab.c fb_3_2.tab.h
