#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#define MAX_NAME_LEN 100
#define MAX_CLUE_LEN 255

typedef struct {
    int treasure_id;
    char username[MAX_NAME_LEN];
    float latitude;
    float longitude;
    char clue[MAX_CLUE_LEN];
    int value;
} Treasure;

int create_hunt_directory(const char* hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "./%s", hunt_id);
    if (mkdir(path, 0755) == -1 && errno != EEXIST) {
        perror("Error creating hunt directory");
        return -1;
    }
    return 0;
}

void log_operation(const char* hunt_id, const char* operation) {
    char log_filename[256];
    snprintf(log_filename, sizeof(log_filename), "./%s/logged_hunt", hunt_id);

    FILE *logfile = fopen(log_filename, "a");
    if (!logfile) {
        perror("Error opening log file");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(logfile, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, operation);
    fclose(logfile);
}

int create_symlink_for_log(const char* hunt_id) {
    char log_filename[256], symlink_name[256];
    snprintf(log_filename, sizeof(log_filename), "./%s/logged_hunt", hunt_id);
    snprintf(symlink_name, sizeof(symlink_name), "./logged_hunt-%s", hunt_id);

    unlink(symlink_name); // remove old symlink if exists
    if (symlink(log_filename, symlink_name) == -1) {
        perror("Error creating symlink");
        return -1;
    }
    return 0;
}

int add_treasure(const char* hunt_id, Treasure* treasure) {
    char filename[256];
    snprintf(filename, sizeof(filename), "./%s/treasures.dat", hunt_id);
    int fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening treasure file");
        return -1;
    }

    if (write(fd, treasure, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("Error writing treasure");
        close(fd);
        return -1;
    }

    close(fd);

    log_operation(hunt_id, "add treasure");
    create_symlink_for_log(hunt_id);
    return 0;
}

int list_treasures(const char* hunt_id) {
    char filename[256];
    snprintf(filename, sizeof(filename), "./%s/treasures.dat", hunt_id);
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("Error stating treasure file");
        return -1;
    }

    printf("Hunt: %s\nFile size: %ld bytes\nLast modified: %s",
           hunt_id, st.st_size, ctime(&st.st_mtime));

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return -1;
    }

    Treasure treasure;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        printf("ID: %d, User: %s, Coords: (%.2f, %.2f), Clue: %s, Value: %d\n",
               treasure.treasure_id, treasure.username,
               treasure.latitude, treasure.longitude,
               treasure.clue, treasure.value);
    }

    close(fd);
    log_operation(hunt_id, "list treasures");
    return 0;
}

int view_treasure(const char* hunt_id, int treasure_id) {
    char filename[256];
    snprintf(filename, sizeof(filename), "./%s/treasures.dat", hunt_id);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return -1;
    }

    Treasure treasure;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (treasure.treasure_id == treasure_id) {
            printf("ID: %d, User: %s, Coords: (%.2f, %.2f), Clue: %s, Value: %d\n",
                   treasure.treasure_id, treasure.username,
                   treasure.latitude, treasure.longitude,
                   treasure.clue, treasure.value);
            close(fd);
            log_operation(hunt_id, "view treasure");
            return 0;
        }
    }

    printf("Treasure not found.\n");
    close(fd);
    return -1;
}

int remove_treasure(const char* hunt_id, int treasure_id) {
    char filename[256];
    snprintf(filename, sizeof(filename), "./%s/treasures.dat", hunt_id);

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return -1;
    }

    Treasure treasure;
    Treasure* all = NULL;
    int count = 0;

    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (treasure.treasure_id != treasure_id) {
            all = realloc(all, sizeof(Treasure) * (count + 1));
            all[count++] = treasure;
        }
    }
    close(fd);

    fd = open(filename, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("Error truncating file");
        free(all);
        return -1;
    }

    if (write(fd, all, sizeof(Treasure) * count) == -1) {
        perror("Error writing back data");
    }

    close(fd);
    free(all);

    log_operation(hunt_id, "remove treasure");
    return 0;
}

int remove_hunt(const char* hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "./%s", hunt_id);

    DIR *dir = opendir(path);
    if (!dir) {
        perror("Cannot open hunt directory");
        return -1;
    }

    struct dirent *entry;
    char filepath[512];
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        if (unlink(filepath) == -1) perror("Error deleting file");
    }
    closedir(dir);

    if (rmdir(path) == -1) {
        perror("Failed to delete hunt directory");
        return -1;
    }

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "./logged_hunt-%s", hunt_id);
    unlink(symlink_name);

    printf("Hunt %s deleted.\n", hunt_id);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: treasure_manager <command> <hunt_id> [args...]\n");
        return -1;
    }

    const char *command = argv[1];
    const char *hunt_id = argv[2];

    if (strcmp(command, "add") == 0) {
        if (argc != 9) {
            printf("Usage: add <hunt_id> <treasure_id> <username> <latitude> <longitude> <clue> <value>\n");
            return -1;
        }
        Treasure t;
        t.treasure_id = atoi(argv[3]);
        strncpy(t.username, argv[4], MAX_NAME_LEN);
        t.latitude = atof(argv[5]);
        t.longitude = atof(argv[6]);
        strncpy(t.clue, argv[7], MAX_CLUE_LEN);
        t.value = atoi(argv[8]);

        create_hunt_directory(hunt_id);
        return add_treasure(hunt_id, &t);

    } else if (strcmp(command, "list") == 0) {
        return list_treasures(hunt_id);

    } else if (strcmp(command, "view") == 0) {
        if (argc != 4) {
            printf("Usage: view <hunt_id> <treasure_id>\n");
            return -1;
        }
        return view_treasure(hunt_id, atoi(argv[3]));

    } else if (strcmp(command, "remove_treasure") == 0) {
        if (argc != 4) {
            printf("Usage: remove_treasure <hunt_id> <treasure_id>\n");
            return -1;
        }
        return remove_treasure(hunt_id, atoi(argv[3]));

    } else if (strcmp(command, "remove_hunt") == 0) {
        return remove_hunt(hunt_id);

    } else {
        printf("Unknown command.\n");
        return -1;
    }
}

/*   
  gcc -o treasure_manager treasure_manager.c
./treasure_manager add hunt1 101 user1 45.12 21.43 "Near the oak tree" 100
./treasure_manager list hunt1
./treasure_manager view hunt1 101
./treasure_manager remove_treasure hunt1 101
./treasure_manager remove_hunt hunt1




*/