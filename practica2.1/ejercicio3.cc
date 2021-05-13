#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Need more arguments");
        return 1;
    }

    int sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sd < 0)
    {
        fprintf(stderr, "Error getting the socket %s\n", gai_strerror(sd));
        return -1;
    }

    sockaddr_in servaddr;
    socklen_t hostlen = sizeof(sockaddr_in);
    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    char buffer[80];

    sendto(sd, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&servaddr, hostlen);

    if (argv[3][0] == 'q')
        return 0;

    int n = recvfrom(sd, buffer, 80, 0, (struct sockaddr *)&servaddr, &hostlen);

    buffer[n] = '\n';
    printf("%s", buffer);
    close(sd);
    return 0;
}