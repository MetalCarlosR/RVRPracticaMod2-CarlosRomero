#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <thread>
#include <iostream>

#define MAX_THREADS 5

class ThreadMessage
{
private:
    static bool closeThreads;
    static int newId;
    int id_;
    int socket_;
    std::thread thr;

    void threadHandle()
    {
        while (!closeThreads)
        {
            char buffer[80] = {};

            sockaddr client;
            socklen_t clientlen = sizeof(sockaddr);

            int bytes = recvfrom(socket_, buffer, 80, 0, &client, &clientlen);

            if (bytes < 0)
                continue;

            char hostname[NI_MAXHOST];
            int check = getnameinfo(&client, clientlen, hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if (check != 0)
            {
                fprintf(stderr, "Error using getnameinfo: %s\n", gai_strerror(check));
                continue;
            }

            printf("%d bytes en thread %d de %s\n", bytes, id_, hostname);
            
            sleep(3);

            time_t rawTime;
            tm *timeInfo;

            time(&rawTime);

            timeInfo = localtime(&rawTime);

            char *format;

            switch (buffer[0])
            {
            case 't':
                format = "%r";
                break;
            case 'd':
                format = "%F";
                break;
            default:
                printf("Comando no soportado %s\n", buffer);
                sendto(socket_, "Comando no soportado\n", 21, 0, &client, clientlen);
                continue;
            }

            char send[80];
            int timeBytes = strftime(send, 80, format, timeInfo);
            send[timeBytes] = '\n';

            sendto(socket_, send, timeBytes + 1, 0, &client, clientlen);
        }
    }

    static void HandleNewThread(ThreadMessage* msg){
        msg->threadHandle();
    }

public:
    ThreadMessage() {}

    ThreadMessage(int socket) : socket_(socket), id_(newId) , thr(&HandleNewThread,this)
    {
        newId++;
    }

    ~ThreadMessage()
    {
        thr.detach();
    }

    static void Close()
    {
        closeThreads = true;
    }
};

int ThreadMessage::newId = 0;
bool ThreadMessage::closeThreads = false;

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

    ThreadMessage *pool[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++)
        pool[i] = new ThreadMessage(sd);

    char c;
    while (std::cin >> c)
    {
        if (c == 'q')
            break;
    }

    ThreadMessage::Close();

    for (int i = 0; i < MAX_THREADS; i++)
    {
        delete pool[i];
        pool[i] = nullptr;
    }

    close(sd);

    return 0;
}