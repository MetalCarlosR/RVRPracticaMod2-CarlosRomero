#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



int main(int argc, char* argv[]){
    if(argc < 2)
    {
        printf("Please introduce an address. %s domain.xx / 100.0.0.0",argv[0]);
        return 1;
    }
    
   addrinfo* info;
   int check;

   check = getaddrinfo(argv[1],argv[2],NULL,&info);
    
    if(check != 0){
        if(check == EAI_SYSTEM){
            perror("getaddrinfo");
        }
        else
        {
            fprintf(stderr , "Error using getaddrinfo: %s\n",gai_strerror(check));
        }
        return -1;
    }

    addrinfo* it;
    
    for(it = info; it != NULL; it = it->ai_next){
        char hostname[NI_MAXHOST];
        check = getnameinfo(it->ai_addr, it->ai_addrlen, hostname, NI_MAXHOST, NULL, 0 , NI_NUMERICHOST);
        if(check != 0){
            fprintf(stderr, "Error using getnameinfo: %s\n", gai_strerror(check));
            continue;
        }
        if(hostname != "\0"){
            printf("Host: %s     Family: %d      SocketType: %d\n", hostname,it->ai_family,it->ai_socktype);
        }
    }
    freeaddrinfo(info);

    return 0;
}