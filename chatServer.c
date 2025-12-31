
#include "clientServer.h"
#include <pthread.h> // For pthread_create()

#define STD_ERROR_MSG "Error doing operation"

const int STD_ERROR_MSG_LEN = sizeof(STD_ERROR_MSG) - 1;

#define STD_BYE_MSG "Good bye!"

const int STD_BYE_MSG_LEN = sizeof(STD_BYE_MSG) - 1;

const int ERROR_FD = -1;

int client_fds[2];

typedef struct
{
    int my_fd;
    int other_fd;
    int threadNum;
} client_info_t;

void *handleClient(void *vPtr)
{
    client_info_t *info = (client_info_t *)vPtr;
    int my_fd = info->my_fd;
    int other_fd = info->other_fd;
    int threadNum = info->threadNum;

    free(info);
    printf("Thread %d starting.\n", threadNum);

    while (1)
    {
        uint32_t netLen;
        int bytesRead = read(my_fd, &netLen, sizeof(netLen));

        if (bytesRead <= 0)
        {
            printf("Thread %d: client disconnected.\n", threadNum);
            break;
        }

        uint32_t len = ntohl(netLen);
        char *buffer = malloc(len + 1);

        if (read(my_fd, buffer, len) <= 0)
        {
            free(buffer);
            break;
        }

        buffer[len] = '\0';
        printf("Thread %d received: %s\n", threadNum, buffer);

        // Forward to other client
        write(other_fd, &netLen, sizeof(netLen));
        write(other_fd, buffer, len);

        free(buffer);
    }

    close(my_fd);
    close(other_fd);
    printf("Thread %d quitting.\n", threadNum);
    return NULL;
}

void doServer(int listenFd)
{
    pthread_t t1, t2;
    pthread_attr_t threadAttr;

    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    printf("Waiting for child 1...\n");
    client_fds[0] = accept(listenFd, NULL, NULL);

    printf("Waiting for child 2...\n");
    client_fds[1] = accept(listenFd, NULL, NULL);

    client_info_t *info1 = malloc(sizeof(client_info_t));
    client_info_t *info2 = malloc(sizeof(client_info_t));

    info1->my_fd = client_fds[0];
    info1->other_fd = client_fds[1];
    info1->threadNum = 1;

    info2->my_fd = client_fds[1];
    info2->other_fd = client_fds[0];
    info2->threadNum = 2;

    pthread_create(&t1, &threadAttr, handleClient, info1);
    pthread_create(&t2, &threadAttr, handleClient, info2);

    pthread_attr_destroy(&threadAttr);

    // Server just waits forever
    pause();
}

int getPortNum(int argc,
               char *argv[])
{

    int portNum;

    if (argc >= 2)
        portNum = strtol(argv[1], NULL, 0);
    else
    {
        char buffer[BUFFER_LEN];

        printf("Port number to monopolize? ");
        fgets(buffer, BUFFER_LEN, stdin);
        portNum = strtol(buffer, NULL, 0);
    }

    return (portNum);
}

int getServerFileDescriptor(int port)
{

    int socketDescriptor = socket(AF_INET,     // AF_INET domain
                                  SOCK_STREAM, // Reliable TCP

                                  0);

    if (socketDescriptor < 0)
    {
        perror("socket()");
        return (ERROR_FD);
    }

    struct sockaddr_in socketInfo;

    memset(&socketInfo, '\0', sizeof(socketInfo));

    socketInfo.sin_family = AF_INET;

    socketInfo.sin_port = htons(port);

    socketInfo.sin_addr.s_addr = INADDR_ANY;

    int status = bind(socketDescriptor, // from socket()
                      (struct sockaddr *)&socketInfo,
                      sizeof(socketInfo));

    if (status < 0)
    {
        perror("bind()");
        return (ERROR_FD);
    }

    listen(socketDescriptor, 5);

    return (socketDescriptor);
}

int main(int argc,
         char *argv[])
{
    int port = getPortNum(argc, argv);
    int listenFd = getServerFileDescriptor(port);
    int status = EXIT_FAILURE;

    if (listenFd >= 0)
    {
        doServer(listenFd);
        close(listenFd);
        status = EXIT_SUCCESS;
    }

    return (status);
}
