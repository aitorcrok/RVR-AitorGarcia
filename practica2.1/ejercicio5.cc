#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <iostream>
#include <stdio.h>
/*
 *  ./echo_client <dir. escucha> <puerto>
 */
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cerr << "Error: Deben introducirse dos parámetros, dirección y puerto\n";
        return -1;
    }
    struct addrinfo hints;
    struct addrinfo* res;

    memset((void*) &hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int rc = getaddrinfo(argv[1], argv[2], &hints, &res);

    if(rc != 0)
    {
        std::cerr << "[getaddrinfo]: " << gai_strerror(rc) << std::endl;
        return -1;
    }

    int sd = socket(res->ai_family, res->ai_socktype, 0);

    if(sd == -1)
    {
        std::cerr << "[socket]: creacion socket\n";
        return -1;
    }


    int con = connect(sd, res->ai_addr, (socklen_t) res->ai_addrlen);
    if(con == -1)
    {
        std::cerr << "[connect]: conexion fallida" << std::endl;
    }

    freeaddrinfo(res);
    
    while(true){
        char buffer[80];

        fgets(buffer, 80, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        send(sd, buffer, 79, 0);

        if(buffer[0] == 'Q' && buffer[1] == '\0'){
            break;
        }

        int bytes = recv(sd, (void *) buffer, 79, 0);
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }
    close(sd);

    return 0;
}