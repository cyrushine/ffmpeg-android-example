#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

int SDL_main(int argc, char *argv[]) {
    for(;;) {
        sleep(1000 * 10);
    }
    return 0;
}