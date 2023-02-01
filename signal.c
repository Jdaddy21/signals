#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#define _XOPEN_SOURCE 600;

pid_t user_pid;
int size = 4096;
char *inbox;
char *outbox;
char inbox_name[512];
char outbox_name[512];

void cleanup()
{
    munmap(inbox, size);
    munmap(outbox, size);
    shm_unlink(inbox_name);
}

static void signal_handler(int signum)
{
    if (signum == SIGTERM)
    {
        cleanup();
    }

    if (signum == SIGINT)
    {
        cleanup();
        kill(user_pid, SIGTERM);
    }

    if (signum == SIGUSR1)
    {
        fputs(inbox, stdout);
        fflush(stdout);
        inbox[0] = '\0';
    }
}

int main(int argc, char *argv[])
{

    int pid = getpid();

    printf("Your PID is: %d\n", pid);

    char temp[32];
    printf("Enter another PID: ");

    fgets(temp, 32, stdin);
    user_pid = atoi(temp);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    snprintf(inbox_name, sizeof inbox_name, "/%d-mmchat", getpid());
    snprintf(outbox_name, sizeof outbox_name, "/%d-mmchat", user_pid);

    int inbox_fd = shm_open(inbox_name, O_CREAT | O_RDWR, 0666);
    int outbox_fd = shm_open(outbox_name, O_CREAT | O_RDWR, 0666);

    ftruncate(inbox_fd, 4096);
    ftruncate(outbox_fd, 4096);

    inbox = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, inbox_fd, 0);
    outbox = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, outbox_fd, 0);

    while (!feof(stdin))
    {
        fgets(outbox, 4096, stdin);
        kill(user_pid, SIGUSR1);
        while (outbox[0])
        {
            usleep(10000);
        }
    }

    kill(user_pid, SIGTERM);
    close(inbox_fd);
    close(outbox_fd);
    cleanup();
}

// #include <signal.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>

// #define MAX_INPUT_SIZE 1024

// int target_pid;
// char *inbox;
// char *outbox;

// void cleanup()
// {
//     free(inbox);
//     free(outbox);
// }

// void handle_sigint(int signum)
// {
//     cleanup();
//     kill(target_pid, SIGTERM);
//     exit(0);
// }

// void handle_sigterm(int signum)
// {
//     cleanup();
//     exit(0);
// }

// void handle_sigusr1(int signum)
// {
//     printf("Inbox: %s\n", inbox);
//     inbox[0] = '\0';
// }

// int main()
// {
//     signal(SIGINT, handle_sigint);
//     signal(SIGTERM, handle_sigterm);
//     signal(SIGUSR1, handle_sigusr1);
//     printf("Process ID: %d\n", getpid());
//     printf("Enter target PID: ");
//     scanf("%d", &target_pid);
//     inbox = malloc(MAX_INPUT_SIZE * sizeof(char));
//     outbox = malloc(MAX_INPUT_SIZE * sizeof(char));
//     atexit(cleanup);
//     while (fgets(outbox, MAX_INPUT_SIZE, stdin) != NULL)
//     {
//         kill(target_pid, SIGUSR1);
//         while (outbox[0] != '\0')
//         {
//             sleep(1);
//         }
//     }
//     kill(target_pid, SIGTERM);
//     return 0;
// }
