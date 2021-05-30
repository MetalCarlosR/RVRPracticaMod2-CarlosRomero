#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char *address, const char *port) : sd(-1)
{
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase
    addrinfo *info;
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int check;

    check = getaddrinfo(address, port, &hints, &info);

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
        return;
    }

    sd = socket(info->ai_family, info->ai_socktype, 0);

    if (sd == -1)
    {
        fprintf(stderr, "Error getting the socket %s\n", gai_strerror(sd));
        return;
    }
    sa = *info->ai_addr;
    sa_len = info->ai_addrlen;
}

int Socket::recv(Serializable &obj, Socket *&sock)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

    if (bytes <= 0)
    {
        return -1;
    }

    if (sock != 0)
    {
        sock = new Socket(&sa, sa_len);
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable &obj, const Socket &sock)
{
    //Serializar el objeto
    obj.to_bin();
    //Enviar el objeto binario a sock usando el socket sd
    int ret = sendto(sd, obj.data(), obj.size(), 0, &sock.sa, sock.sa_len);
    return ret == -1 ? -1 : 0;
}

bool operator==(const Socket &s1, const Socket &s2)
{
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiere
    const sockaddr_in sin1 = (sockaddr_in &)s1.sa;
    const sockaddr_in sin2 = (sockaddr_in &)s1.sa;

    bool family = sin1.sin_family == sin2.sin_family;
    bool addr = sin1.sin_addr.s_addr == sin2.sin_addr.s_addr;
    bool port = sin1.sin_port == sin2.sin_port;

    return family && addr && port;
};

std::ostream &operator<<(std::ostream &os, const Socket &s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr *)&(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};