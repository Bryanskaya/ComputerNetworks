#include "includes.hpp"


int init_client()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
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

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr*) server->h_addr_list[0]);    
    serv_addr.sin_port = htons(PORT);
    
    if (connect(client, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        close(client);
        perror("ERROR: connect() failed");
        exit(-1);
    }
}

int get_lst(int client)
{
    int rsize, err = EXIT_FAILURE;
    char lst_buf[256];

    rsize = recv(client, lst_buf, sizeof(lst_buf), 0);
    if (rsize)
    {
        lst_buf[rsize] = '\0';
        printf("Client got:\n%s\n", lst_buf);
        return EXIT_SUCCESS;
    }
    
    printf("Server was disconnected\n");

    return EXIT_FAILURE;
}

int choose_file(int client, char buf[])
{
    printf(">>> ");
    scanf("%s", buf);

    if (send(client, buf, strlen(buf), 0) < 0)
    {
        close(client);
        perror("ERROR: send() failed");
        return EXIT_FAILURE;
    }
    
    printf("Client sent: %s\n", buf);
    return EXIT_SUCCESS;
}

int get_num_blocks(int client)
{
    int rsize, num;

    rsize = recv(client, &num, sizeof(int), 0);
    if (num < 0)
    {
        printf("File was not found\n");
        return num;
    }

    return num;
}

void get_file(int client, char filename[])
{
    int num, rsize;
    char buf[BLOCK + 1];
    FILE *f;

    if ((num = get_num_blocks(client)) < 0) return;

    f = fopen(filename, "wb");

    for (int i = 0; i < num; i++)
    {
        rsize = recv(client, buf, BLOCK, 0);
        buf[rsize] = '\0';

        fwrite(&buf, 1, rsize, f);            
    }
    
    printf("File was downloaded\n");

    fclose(f);
}

void do_task(int client)
{
    char buf[100];

    if (get_lst(client)) return;
    if (choose_file(client, buf)) return;

    get_file(client, buf);
}

int main()
{    
    int client = init_client();
    connect_client(client);
    do_task(client);

    return 0;
}