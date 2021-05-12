#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
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
    memset(&hints,0,sizeof(addrinfo));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

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

    while (true)
    {
        char buffer[80];

        sockaddr client;
        socklen_t clientlen = sizeof(sockaddr);

        int bytes = recvfrom(sd, buffer, 80, 0, &client, &clientlen);

        char hostname[NI_MAXHOST];
        check = getnameinfo(&client, clientlen, hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if (check != 0)
        {
            fprintf(stderr, "Error using getnameinfo: %s\n", gai_strerror(check));
            continue;
        }

        printf("%d bytes de %s\n", bytes, hostname);

        time_t rawTime;
        tm *timeInfo;

        time(&rawTime);

        timeInfo = localtime(&rawTime);

        char *format;
        if (buffer[0] == 't')
        {
            format = "%r";
        }
        else if (buffer[0] == 'd')
        {
            format = "%F";
        }
        else if (buffer[0] == 'q')
        {
            printf("Saliendo...\n");
            break;
        }
        else
        {
            printf("Comando no soportado %s\n", buffer);
            sendto(sd, "Comando no soportado", 21, 0, &client, clientlen);
            continue;
        }
        char send[100];
        int timeBytes = strftime(send, 80, format, timeInfo);

        sendto(sd, send, timeBytes, 0, &client, clientlen);
    }

    return 0;
}