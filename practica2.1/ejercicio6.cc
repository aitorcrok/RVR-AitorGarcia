#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <thread>
#include <stdio.h>

#define MAX_THREAD 5
/*
 *  ./time_server_mt <dir. escucha> <puerto>
 */

class Thread
{
public:
    Thread(int sd):_sd(sd){};
    int _sd;

    void mensaje()
    {
        char command[80];
        memset(command, 0, 80);
        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];
        struct sockaddr cliente;
        socklen_t       clientelen = sizeof(struct sockaddr);

        while(true){
            int bytes = recvfrom(_sd, (void *) &command, 79, 0, &cliente, &clientelen);

            if(bytes == -1){
                return;
            }

            command[bytes] = '\0';

            getnameinfo(&cliente, clientelen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

            std::cout << "[Thread " << std::this_thread::get_id() << "]" << bytes << " bytes de " << host << ":" << serv << std::endl;

            if(command[1] != '\0')
            {
                std::cout << "[Thread " << std::this_thread::get_id() << "] Comando no soportado " << command << std::endl;
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
                    
                    sendto(_sd, buffer, 12, 0, &cliente, clientelen);
                }
                else if(command[0] == 'd')
                {
                    char buffer[11];
                    // %F

                    strftime(buffer, sizeof(buffer), "%F", res);
                    
                    sendto(_sd, buffer, 11, 0, &cliente, clientelen);
                }
                else
                {
                    std::cout << "[Thread " << std::this_thread::get_id() << "] Comando no soportado " << command << std::endl;
                }
            }
            sleep(3);
        }
    }
};

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

    std::thread pool[MAX_THREAD];

    for(int i = 0; i < MAX_THREAD; i++)
    {
        pool[i] = std::thread([&](){
            Thread t(sd);
            while(true){
               t.mensaje();
            }
        });
    }

    for(int i = 0; i < MAX_THREAD; i++)
    {
        pool[i].detach();
    }

    while(true)
    {
        char buffer[80];
        fgets(buffer, 80, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if(buffer[0] == 'q' && buffer[1] == '\0'){
            break;
        }
    }
    
    std::cout << "Cerrando todos los threads...\n";

    for(int i = 0; i < MAX_THREAD; i++)
    {
        pool[i].~thread();
    }
    close(sd);

    return 0;
}