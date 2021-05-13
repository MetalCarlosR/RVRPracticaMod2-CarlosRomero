#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Need more arguments");
        return 1;
    }

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        fprintf(stderr, "Error getting the socket %s\n", gai_strerror(sd));
        return -1;
    }

    sockaddr_in servaddr;
    socklen_t servlen = sizeof(sockaddr_in);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    inet_aton(argv[1], &servaddr.sin_addr);

    if (connect(sd, (sockaddr *)&servaddr, servlen) < 0)
    {
        printf("Error connecting to that address");
        return -1;
    }

    while (true)
    {
        char* buffer = NULL;
        size_t len = 80;
        ssize_t line = 0;

        line = getline(&buffer,&len,stdin);

        if (line == 2 && buffer[0] == 'Q')
            break;
        send(sd, buffer, line - 1, 0);

        int n = recv(sd, buffer, 79, 0);

        buffer[n] = '\n';
        printf("%s", buffer);
        free(buffer);
    }
    close(sd);
    return 0;
}