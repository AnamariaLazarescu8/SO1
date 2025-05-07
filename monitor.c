// monitor.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;
volatile sig_atomic_t command_received = 0;

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        command_received = 1;
    } else if (sig == SIGTERM) {
        running = 0;
    }
}

void read_and_execute_command() {
    FILE *f = fopen("commands.txt", "r");
    if (!f) return;

    char cmd[256];
    fgets(cmd, sizeof(cmd), f);
    fclose(f);

    if (strncmp(cmd, "list_hunts", 10) == 0) {
        printf("[monitor] Hunts: Forest (3), Cave (5), Desert (2)\n");
    } else if (strncmp(cmd, "list_treasures", 14) == 0) {
        printf("[monitor] Treasures in Forest: Sword, Shield, Gold\n");
    } else if (strncmp(cmd, "view_treasure", 13) == 0) {
        printf("[monitor] Treasure info: Sword – damage 50, durability 80\n");
    } else if (strncmp(cmd, "stop", 4) == 0) {
        printf("[monitor] Stopping...\n");
        usleep(2000000);  // simulare delay de 2 secunde
        running = 0;
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("[monitor] Monitor started. Waiting for commands...\n");

    while (running) {
        if (command_received) {
            command_received = 0;
            read_and_execute_command();
        }
        pause(); // așteaptă semnale
    }

    printf("[monitor] Monitor exited.\n");
    return 0;
}






