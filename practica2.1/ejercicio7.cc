#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <thread>
#include <memory>
#include <signal.h>

#define MAX_THREADS 5

class ThreadMessageTCP
{
private:
    static bool closeThreads;
    static std::unique_ptr<ThreadMessageTCP> threads[MAX_THREADS];

    int id_;
    int socket_;
    std::thread thr;
    bool threadClosed = true;

    void threadHandle()
    {
        while (!closeThreads)
        {
            char buffer[80] = {};

            int bytes = recv(socket_, buffer, 80, 0);

            if (bytes <= 0)
            {
                printf("Conexión terminada en el thread %d\n", id_);
                break;
            }

            printf("Echo %s en thread %d\n", buffer, id_);

            send(socket_, buffer, bytes, 0);
        }
        close(socket_);
        threadClosed = true;
    }

    static void HandleNewThread(ThreadMessageTCP *msg)
    {
        msg->threadHandle();
    }

    void startThread(int sck)
    {
        if (thr.joinable())
            thr.detach();
        threadClosed = false;
        socket_ = sck;
        thr = std::thread(&HandleNewThread, this);
    }

public:
    ThreadMessageTCP() {}

    ThreadMessageTCP(int id) : id_(id) {}

    ~ThreadMessageTCP()
    {
        if (thr.joinable())
            thr.join();
    }

    static void Init()
    {
        for (int i = 0; i < MAX_THREADS; i++)
        {
            threads[i].reset(new ThreadMessageTCP(i));
        }
    }

    static void Close()
    {
        for (int i = 0; i < MAX_THREADS; i++)
        {
            if (!threads[i].get()->threadClosed)
            {
                threads[i].get()->thr.join();
                close(threads[i].get()->socket_);
            }
        }
        closeThreads = true;
    }

    static bool manageNewThread(int socket)
    {
        for (int i = 0; i < MAX_THREADS; i++)
        {
            if (threads[i].get()->threadClosed)
            {
                threads[i].get()->startThread(socket);
                return true;
            }
        }
        return false;
    }
};

bool ThreadMessageTCP::closeThreads = false;

std::unique_ptr<ThreadMessageTCP> ThreadMessageTCP::threads[MAX_THREADS] = {};

void endSignal(int signal)
{
    ThreadMessageTCP::Close();
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Need more arguments");
        return 1;
    }

    signal(SIGINT, endSignal);

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

    int client_sd;
    char hostname[NI_MAXHOST];
    char servname[NI_MAXSERV];

    int i = 0;

    ThreadMessageTCP::Init();
    while (true)
    {
        client_sd = accept(sd, &client, &clientlen);
        check = getnameinfo(&client, clientlen, hostname, NI_MAXHOST, servname, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        if (check != 0)
        {
            fprintf(stderr, "Error using getnameinfo: %s\n", gai_strerror(check));
            return -1;
        }

        printf("Conexion desde %s %s\n", hostname, servname);

        char *mess;
        if (!ThreadMessageTCP::manageNewThread(client_sd))
        {
            mess = "No hay threads disponibles, por favor cierra la conexión e intételo más tarde\n";
            send(client_sd, mess, strlen(mess), 0);
            close(client_sd);
        }
        else
        {
            mess = "Conexión aceptada\n";
            send(client_sd, mess, strlen(mess), 0);
        }
    }

    return 0;
}