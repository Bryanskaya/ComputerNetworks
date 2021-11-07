#include "includes.hpp"


int init_client()
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("ERROR: socket() failed");
        exit(-1);
    }

    return sock;
}

void connect_client(int client)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(HOST);
    if (!server) {
        close(client);
        perror("ERROR: gethostbyname() failed");
        exit(-1);
    }

    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr = *((struct in_addr*) server->h_addr_list[0]);    
    serv_addr.sin_port = htons(PORT);
    
    if (connect(client, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        close(client);
        perror("ERROR: connect() failed");
        exit(-1);
    }
}

std::string form_request(std::string path)
{
    const std::string vrs = "HTTP/1.1";
    return "GET /" + path + " " + vrs + "\r\n";
}

int do_task(int client)
{
    char buf[SIZE];
    std::string request = form_request(REQUEST);

    if (send(client, request.c_str(), request.length(), 0) < 0)
    {
        close(client);
        perror("ERROR: send() failed");
        return EXIT_FAILURE;
    }

    if (recv(client, buf, sizeof(buf), 0) < 0)
    {
        perror("ERROR: recv() failed");
        return EXIT_FAILURE;
    }

    printf("Client has recieved an answer:\n\n%s", buf);
    close(client);
}

int main()
{    
    int client = init_client();
    connect_client(client);
    do_task(client);

    return 0;
}