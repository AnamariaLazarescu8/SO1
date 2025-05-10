// calculate_scores.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_USERS 100

typedef struct {
    char name[50];
    int score;
} UserScore;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hunt_file>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("Failed to open hunt file");
        return 1;
    }

    UserScore users[MAX_USERS];
    int count = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        char user[50];
        int value;
        if (sscanf(line, "%s %d", user, &value) == 2) {
            int found = 0;
            for (int i = 0; i < count; i++) {
                if (strcmp(users[i].name, user) == 0) {
                    users[i].score += value;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                strcpy(users[count].name, user);
                users[count].score = value;
                count++;
            }
        }
    }
    fclose(f);

    for (int i = 0; i < count; i++) {
        printf("%s: %d\n", users[i].name, users[i].score);
    }

    return 0;
}
