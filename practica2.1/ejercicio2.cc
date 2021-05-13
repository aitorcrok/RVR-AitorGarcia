#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#include <iostream>

/*
 *  ./time_server <dir. escucha> <puerto>
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
    hints.ai_socktype = SOCK_DGRAM;

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

    bind(sd, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);

    bool quit = false;

    while(quit == false)
    {
        char command[80];
        memset(command, 0, 80);
        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];

        struct sockaddr cliente;
        socklen_t       clientelen = sizeof(struct sockaddr);

        int bytes = recvfrom(sd, (void *) &command, 79, 0, &cliente, &clientelen);

        if(bytes == -1){
            return -1;
        }

        command[bytes] = '\0';

        getnameinfo(&cliente, clientelen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        std::cout << bytes << " bytes de " << host << ":" << serv << std::endl;

        if(command[1] != '\0')
        {
            std::cout << "Comando no soportado " << command;
        }
        else if(command[0] == 'q')
        {
            quit = true;
            std::cout << "Saliendo...\n";
        }
        else
        {
            time_t t;
            struct tm* res;

            t = time(NULL);
            res = localtime(&t);
            if(res == NULL)
            {
                std::cerr << "[localtime]\n";
            }
            if(command[0] == 't')
            {
                char buffer[12];
                // %I:%M:%S %p
                // o
                // %r

                strftime(buffer, sizeof(buffer), "%r", res);
                
                sendto(sd, buffer, 12, 0, &cliente, clientelen);
            }
            else if(command[0] == 'd')
            {
                char buffer[11];
                // %F

                strftime(buffer, sizeof(buffer), "%F", res);
                
                sendto(sd, buffer, 11, 0, &cliente, clientelen);
            }
            else
            {
                std::cout << "Comando no soportado " << command;
            }
        }

    }

    close(sd);

    return 0;
}