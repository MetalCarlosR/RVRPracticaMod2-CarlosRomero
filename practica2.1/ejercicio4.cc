#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Need more arguments");
        return 1;
    }

    addrinfo *info;
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int check;

    check = getaddrinfo(argv[1], argv[2], &hints, &info);

    if (check != 0)
    {
        if (check == EAI_SYSTEM)
        {
            perror("getaddrinfo");
        }
        else
        {
            fprintf(stderr, "Error using getaddrinfo: %s\n", gai_strerror(check));
        }
        return -1;
    }

    int sd = socket(info->ai_family, info->ai_socktype, 0);

    if (sd == -1)
    {
        fprintf(stderr, "Error getting the socket %s\n", gai_strerror(sd));
        return -1;
    }

    bind(sd, info->ai_addr, info->ai_addrlen);

    freeaddrinfo(info);

    check = listen(sd, 16);

    if (check < 0)
    {
        fprintf(stderr, "Error using listen: %s\n", gai_strerror(check));
    }

    sockaddr client;
    socklen_t clientlen = sizeof(sockaddr);

    int client_sd = accept(sd, &client, &clientlen);

    char hostname[NI_MAXHOST];
    char servname[NI_MAXSERV];
    check = getnameinfo(&client, clientlen, hostname, NI_MAXHOST, servname, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

    if (check != 0)
    {
        fprintf(stderr, "Error using getnameinfo: %s\n", gai_strerror(check));
        return -1;
    }

    printf("Conexion desde %s %s\n", hostname, servname);

    while (true)
    {
        char buffer[80] = {};

        int bytes = recv(client_sd, buffer, 80, 0);

        if (bytes <= 0)
        {
            printf("Conexión terminada\n");
            break;
        }
        
        printf("Echo %s\n", buffer);

        send(client_sd, buffer, bytes, 0);
    }

    close(sd);

    return 0;
}