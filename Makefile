PRELOAD_LIB := triplet_challenge_preload.so
BIN := triplet_challenge_stats
CCFLAGS := -O3 -pedantic -Wall -Wextra -Werror

all: $(PRELOAD_LIB) $(BIN)

$(PRELOAD_LIB): triplet_challenge_preload.c triplet_challenge_stats.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $< -ldl -lrt

$(BIN): triplet_challenge_stats.c triplet_challenge_stats.h
	$(CC) $(CFLAGS) -o $@ $< -lrt

clean:
	rm -rf $(PRELOAD_LIB) $(BIN)
