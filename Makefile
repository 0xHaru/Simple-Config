CC=gcc
CFLAGS=-Wall -Wextra -g
# CFLAGS=-Wall -Wextra -DNDEBUG -O2

CFG_SRC_HDR=config.c config.h
TST_SRC=test/*.c
TST_HDR=test/*.h
FZZ_SRC=fuzz/fuzz.c

.PHONY: all report clean

all: example

example: example.c $(CFG_SRC_HDR)
	$(CC) example.c config.c -o $@ $(CFLAGS) -Wpedantic

fzz: $(FZZ_SRC) $(CFG_SRC_HDR)
	clang $(FZZ_SRC) config.c -o $@ -g -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all -O1

tst: $(TST_SRC) $(TST_HDR) $(CFG_SRC_HDR)
	$(CC) $(TST_SRC) config.c -o $@ $(CFLAGS)

tst-cov: $(TST_SRC) $(TST_HDR) $(CFG_SRC_HDR)
	$(CC) $(TST_SRC) config.c -o $@ $(CFLAGS) -fprofile-arcs -ftest-coverage -DNDEBUG

report: clean tst-cov
	./tst-cov
	lcov --rc branch_coverage=1 --capture --directory . --output-file coverage.info
	@ mkdir -p report
	genhtml coverage.info --output-directory report --branch-coverage

clean:
	rm -rf example fzz tst tst-cov \
	       tst-cov-*.gcda tst-cov-*.gcno coverage.info \
		   log.txt report/ crash-*

	find ./fuzz/corpus -type f ! -name '*.cfg' -delete

