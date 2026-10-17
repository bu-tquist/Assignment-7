#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"


/*
Saves server game data to specified filepath.
Returns 0 on success, -1 on failure.
*/
int savegame(struct gameDB* game, char* filepath) {
    if (game == NULL || filepath == NULL) {
        return -1;
    }

    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        return -1;
    }

    ssize_t bytes_written = write(fd, game, sizeof(struct gameDB));
    close(fd);

    if (bytes_written != (ssize_t)sizeof(struct gameDB)) {
        return -1;
    }

    return 0;
}

/*
Loads a file written by savegame into game.
Returns 0 on success, -1 on failure.
*/
int loadgame(struct gameDB* game, char* filepath) {
    if (game == NULL || filepath == NULL) {
        return -1;
    }

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    ssize_t bytes_read = read(fd, game, sizeof(struct gameDB));
    close(fd);

    if (bytes_read != (ssize_t)sizeof(struct gameDB)) {
        return -1;
    }

    return 0;
}
