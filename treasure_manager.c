#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

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

//Crearea unui director pentru joc
int create_hunt_directory(const char* hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "./%s", hunt_id);
    if (mkdir(path, 0755) == -1) {
        perror("Error creating hunt directory");
        return -1;
    }
    return 0;
}


//adaugarea unei comori intr-un fisier
int add_treasure(const char* hunt_id, Treasure* treasure) {
    char filename[256];
    snprintf(filename, sizeof(filename), "./%s/treasures.dat", hunt_id);

    int fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening treasure file");
        return -1;
    }

    if (write(fd, treasure, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("Error writing treasure to file");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

//listarea comorilor dintr-un joc
int list_treasures(const char* hunt_id) {
    char filename[256];
    snprintf(filename, sizeof(filename), "./%s/treasures.dat", hunt_id);

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        return -1;
    }

    Treasure treasure;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        printf("ID: %d, User: %s, Coordinates: (%.2f, %.2f), Clue: %s, Value: %d\n",
               treasure.treasure_id, treasure.username, treasure.latitude,
               treasure.longitude, treasure.clue, treasure.value);
    }

    close(fd);
    return 0;
}

//vizualizarea unei comori
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
            printf("ID: %d, User: %s, Coordinates: (%.2f, %.2f), Clue: %s, Value: %d\n",
                   treasure.treasure_id, treasure.username, treasure.latitude,
                   treasure.longitude, treasure.clue, treasure.value);
            close(fd);
            return 0;
        }
    }

    printf("Treasure not found.\n");
    close(fd);
    return -1;
}

//stergerea unei comori 
int remove_treasure(const char* hunt_id, int treasure_id) {
    char filename[256];
    snprintf(filename, sizeof(filename), "./%s/treasures.dat", hunt_id);

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("Error opening treasure file");
        return -1;
    }

    Treasure treasure;
    int pos = 0;
    int found = 0;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (treasure.treasure_id == treasure_id) {
            found = 1;
            break;
        }
        pos++;
    }

    if (!found) {
        printf("Treasure not found.\n");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}



//logarea operatiunilor
void log_operation(const char* hunt_id, const char* operation) {
    char log_filename[256];
    snprintf(log_filename, sizeof(log_filename), "./%s/logged_hunt", hunt_id);

    FILE *logfile = fopen(log_filename, "a");
    if (logfile == NULL) {
        perror("Error opening log file");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(logfile, "[%d-%02d-%02d %02d:%02d:%02d] %s\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, operation);

    fclose(logfile);
}

//crearea legaturii simbolice pentru loguri
int create_symlink_for_log(const char* hunt_id) {
    char log_filename[256];
    snprintf(log_filename, sizeof(log_filename), "./%s/logged_hunt", hunt_id);

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "./logged_hunt-%s", hunt_id);

    if (symlink(log_filename, symlink_name) == -1) {
        perror("Error creating symlink");
        return -1;
    }

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
        if (argc != 8) {
            printf("Usage: add <hunt_id> <treasure_id> <username> <latitude> <longitude> <clue> <value>\n");
            return -1;
        }

        Treasure treasure;
        treasure.treasure_id = atoi(argv[3]);
        strncpy(treasure.username, argv[4], MAX_NAME_LEN);
        treasure.latitude = atof(argv[5]);
        treasure.longitude = atof(argv[6]);
        strncpy(treasure.clue, argv[7], MAX_CLUE_LEN);
        treasure.value = atoi(argv[8]);

        add_treasure(hunt_id, &treasure);
    }
   

    return 0;
}



