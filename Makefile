raysweeper: raysweeper.c
	clang -g -fsanitize=address -lraylib -o $@ $<
