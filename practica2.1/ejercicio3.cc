#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <iostream>
/*
 *  ./time_client <dir.> <puerto> <comando>
 */
int main(int argc, char** argv)
{
    if(argc != 4)
    {
        std::cerr << "Error: Deben introducirse tres parámetros, dirección, puerto y comando\n";
        return -1;
    }

    struct sockaddr* servaddr;
    struct addrinfo hints;
    struct addrinfo* res;

    memset(&servaddr, 0, sizeof(struct sockaddr));
    memset((void*) &hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rc = getaddrinfo(argv[1], argv[2], &hints, &res);
    if(rc != 0)
    {
        std::cerr << "[getaddrinfo]: " << gai_strerror(rc) << std::endl;
        return -1;
    }

    servaddr = res->ai_addr;
       
    int sd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sd == -1)
    {
        std::cerr << "[socket]: creacion socket\n";
        return -1;
    }

    freeaddrinfo(res);

    char buffer[12];
    socklen_t servlen = sizeof(struct sockaddr);

    char* comm = argv[3];
    
    sendto(sd, comm, sizeof(comm), 0, servaddr, servlen);

    if(comm[0] == 't' || comm[0] == 'd')
    {
        int bytes = recvfrom(sd, (void *) &buffer, 12, 0, servaddr, &servlen);
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }

    close(sd);

    return 0;
}