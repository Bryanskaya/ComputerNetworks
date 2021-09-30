#include "includes.h"


int main()
{
    int sock, res = 0, num;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    char buf[100];
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        perror("ERROR: socket() failed");
        return EXIT_FAILURE;
    }

    server = gethostbyname(HOST);
    if (!server) 
    {
        perror("ERROR: gethostbyname() failed");
        close(sock);
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr*) server->h_addr_list[0]);    
    serv_addr.sin_port = htons(PORT);

    printf(">>> Input number: ");
    if (scanf("%d", &num) < 0)
    {
        printf("ERROR: wrong number\n");
        return EXIT_FAILURE;
    }

    sprintf(buf, "%d", num);

    if (sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sock);
        perror("ERROR: send() failed");
        return EXIT_FAILURE;
    }
    
    printf("Client sent: %s\n", buf);
    close(sock);

    return res;
}
