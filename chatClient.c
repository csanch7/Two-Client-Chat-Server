#include "clientServer.h"
#include <pthread.h>

#define DEFAULT_HOSTNAME "localhost"

void *sendThread(void *arg)
{
    int socketFd = *(int *)arg;
    char buffer[BUFFER_LEN];

    while (fgets(buffer, BUFFER_LEN, stdin) != NULL)
    {
        uint32_t len = strlen(buffer);
        uint32_t netLen = htonl(len);

        if (write(socketFd, &netLen, sizeof(netLen)) <= 0)
            break;
        if (write(socketFd, buffer, len) <= 0)
            break;
    }

    shutdown(socketFd, SHUT_WR);  // Signal no more outgoing data
    return NULL;
}

void *recvThread(void *arg)
{
    int socketFd = *(int *)arg;

    while (1)
    {
        uint32_t netLen;
        int bytesRead = read(socketFd, &netLen, sizeof(netLen));

        if (bytesRead <= 0)
            break;

        uint32_t len = ntohl(netLen);
        char *buffer = malloc(len + 1);
        if (!buffer)
            break;

        int totalRead = 0;
        while (totalRead < len)
        {
            int n = read(socketFd, buffer + totalRead, len - totalRead);
            if (n <= 0)
            {
                free(buffer);
                return NULL;
            }
            totalRead += n;
        }

        buffer[len] = '\0';
        printf("Other child: %s", buffer);
        fflush(stdout);

        free(buffer);
    }

    return NULL;
}

void obtainUrlAndPort(int urlLen, char *url, int *portPtr)
{
    printf("Machine name [%s]? ", DEFAULT_HOSTNAME);
    fgets(url, urlLen, stdin);

    char *cPtr = strchr(url, '\n');
    if (cPtr) *cPtr = '\0';
    if (url[0] == '\0')
        strncpy(url, DEFAULT_HOSTNAME, urlLen);

    char buffer[BUFFER_LEN];
    printf("Port number? ");
    fgets(buffer, BUFFER_LEN, stdin);
    *portPtr = strtol(buffer, NULL, 10);
}

int attemptToConnectToServer(const char *url, int port)
{
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
        return -1;

    struct addrinfo *hostPtr;
    if (getaddrinfo(url, NULL, NULL, &hostPtr) != 0)
        return -1;

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr =
        ((struct sockaddr_in *)hostPtr->ai_addr)->sin_addr.s_addr;

    if (connect(socketFd, (struct sockaddr *)&server, sizeof(server)) < 0)
        return -1;

    freeaddrinfo(hostPtr);
    return socketFd;
}

int main()
{
    char url[BUFFER_LEN];
    int port;
    int socketFd;

    obtainUrlAndPort(BUFFER_LEN, url, &port);
    socketFd = attemptToConnectToServer(url, port);

    if (socketFd < 0)
    {
        fprintf(stderr, "Could not connect to server.\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected! Start chatting.\n");

    pthread_t sender, receiver;
    pthread_create(&sender, NULL, sendThread, &socketFd);
    pthread_create(&receiver, NULL, recvThread, &socketFd);

    pthread_join(sender, NULL);
    pthread_join(receiver, NULL);

    close(socketFd);
    printf("\nDisconnected.\n");
    return EXIT_SUCCESS;
}
