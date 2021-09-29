#include "includes.h"


#define SOCKET_NAME     "my_socket.soc"
#define SIZE            100


int sock;

char* convert_from_10(int num, int p)
{
    char *res = (char *)malloc(70);
    char *elem = "0123456789ABCDEF";
    int k = 0, flag = 1 ? (num < 0) : 0;
    char temp;

    if (num == 0)
        res[k] = elem[0];
        
    while (num)
    {
        res[k] = elem[abs(num) % p];
        num /= p;
        k += 1;
    }

    if (flag)
    {
        res[k] = '-';
        k += 1;
    }

    for (int i = 0; i < k / 2; i++)
    {
        temp = res[i];
        res[i] = res[strlen(res) - i - 1];
        res[strlen(res) - i - 1] = temp;
    }

    return res;
}


void task(char *buf)
{
    int num = atoi(buf);

    printf("10: %d\n\n", num);
    printf(" 2: %s\n\n", convert_from_10(num, 2));
    printf("16: %s\n\n", convert_from_10(num, 16));
    printf(" 8: %s\n\n", convert_from_10(num, 8));
    printf(" 3: %s\n\n", convert_from_10(num, 3));
}


int main()
{
    struct sockaddr_in serv_addr;
    int rsize;
    char buf[SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) 
    {
        perror("ERROR: socket failed");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        close(sock);
        perror("ERROR: bind failed");
        return EXIT_FAILURE;
    }

    printf("Server was created\n");

    while (1)
    {
        rsize = recvfrom(sock, buf, SIZE, 0, NULL, NULL);
        if (rsize < 0)
        {
            printf("ERROR: recvfrom()\n");
            close(sock);
            return EXIT_FAILURE;
        }
        
        buf[rsize] = '\0';
        printf("Server got a number: %s\n\n", buf);

        
        task(buf);
    }

    close(sock);

    printf("Server was closed\n");

    return 0;
}