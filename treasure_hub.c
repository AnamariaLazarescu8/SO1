


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










// treasure_hub.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

pid_t monitor_pid = -1;
int monitor_exiting = 0;

void write_command(const char* cmd) {
    FILE *f = fopen("commands.txt", "w");
    if (f) {
        fprintf(f, "%s\n", cmd);
        fclose(f);
    }
}

void send_command(const char* cmd) {
    if (monitor_pid <= 0 || monitor_exiting) {
        printf("Error: Monitor not running or shutting down.\n");
        return;
    }
    write_command(cmd);
    kill(monitor_pid, SIGUSR1);
}

void sigchld_handler(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("[hub] Monitor exited with status %d\n", WEXITSTATUS(status));
    monitor_pid = -1;
    monitor_exiting = 0;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char input[256];

    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = 0; // elimină newline

        if (strcmp(input, "start_monitor") == 0) {
            if (monitor_pid > 0) {
                printf("Monitor already running.\n");
                continue;
            }
            monitor_pid = fork();
            if (monitor_pid == 0) {
                execl("./monitor", "monitor", NULL);
                perror("execl");
                exit(1);
            }
            printf("Monitor started with PID %d\n", monitor_pid);
        } else if (strcmp(input, "list_hunts") == 0) {
            send_command("list_hunts");
        } else if (strcmp(input, "list_treasures") == 0) {
            send_command("list_treasures");
        } else if (strcmp(input, "view_treasure") == 0) {
            send_command("view_treasure");
        } else if (strcmp(input, "stop_monitor") == 0) {
            if (monitor_pid <= 0) {
                printf("Monitor not running.\n");
            } else {
                monitor_exiting = 1;
                send_command("stop");
            }
        } else if (strcmp(input, "exit") == 0) {
            if (monitor_pid > 0) {
                printf("Error: Monitor still running.\n");
            } else {
                printf("Goodbye!\n");
                break;
            }
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}



all: treasure_hub monitor

treasure_hub: treasure_hub.c
	gcc -o treasure_hub treasure_hub.c

monitor: monitor.c
	gcc -o monitor monitor.c

clean:
	rm -f treasure_hub monitor commands.txt


//Pentru testare
make
./treasure_hub


//apoi voi scrie comenzile
> start_monitor
> list_hunts
> list_treasures
> view_treasure
> stop_monitor
> exit




