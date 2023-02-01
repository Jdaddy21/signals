#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_INPUT_SIZE 1024

int target_pid;
char *inbox;
char *outbox;

void cleanup()
{
    free(inbox);
    free(outbox);
}

void handle_sigint(int signum)
{
    cleanup();
    kill(target_pid, SIGTERM);
    exit(0);
}

void handle_sigterm(int signum)
{
    cleanup();
    exit(0);
}

void handle_sigusr1(int signum)
{
    printf("Inbox: %s\n", inbox);
    inbox[0] = '\0';
}

int main()
{
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);
    printf("Process ID: %d\n", getpid());
    printf("Enter target PID: ");
    scanf("%d", &target_pid);
    inbox = malloc(MAX_INPUT_SIZE * sizeof(char));
    outbox = malloc(MAX_INPUT_SIZE * sizeof(char));
    atexit(cleanup);
    while (fgets(outbox, MAX_INPUT_SIZE, stdin) != NULL)
    {
        kill(target_pid, SIGUSR1);
        while (outbox[0] != '\0')
        {
            sleep(1);
        }
    }
    kill(target_pid, SIGTERM);
    return 0;
}
