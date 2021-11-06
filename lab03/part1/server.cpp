#include "includes.hpp"


int clients_arr[NUM];


static void list_file(const char *filename, char buf[]) 
{ 
    struct dirent   *dirp;      
    DIR             *dp;         
    int             i = 1, count = 0;

    if ((dp = opendir(filename)) == NULL)
    {
        printf("ERROR: opendir %s\n", filename);
        return;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))  
            continue; 
        count += snprintf(buf + count, sizeof(dirp->d_name), "%s\n", dirp->d_name);
    }

    if (closedir(dp) < 0) 
        printf("ERROR: close catalog %s\n", filename);
}

void send_file(char filename[], int sock)
{
    FILE *f;
    struct stat f_info;
    int size, num = -1, temp;
    char buff[BLOCK];
    

    if ((f = fopen(filename, "rb")) == NULL) 
    {
        printf("Cannot open file\n");
        send(sock, &num, sizeof(int), 0);
        return;
    }

    fstat(fileno(f), &f_info);
    size = f_info.st_size;
    num = size / BLOCK + 1;
    send(sock, &num, sizeof(int), 0);

    for(int i = 0; i < num; i++)
    {
        temp = fread(buff, 1, BLOCK, f);
        send(sock, buff, temp, 0);
    }
}

void close_socket(int sock)
{
    for (int i = 0; i < NUM; i++)
        if (clients_arr[i])
            close(clients_arr[i]);
    
    close(sock);
}

void create_connection(int sock, char buf[])
{
    struct sockaddr_in client_addr;
    int addrSize = sizeof(client_addr), flag = 1;

    int new_sock = accept(sock, NULL, NULL);
    if (new_sock < 0) 
    {
        perror("ERROR: accept failed");
        exit(-1);
    }

    printf("New connection added. Client fd = %d\t ip = %s:%d\n",
        new_sock, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    if (send(new_sock, buf, strlen(buf), 0) < 0)
    {
        close(new_sock);
        perror("ERROR: send() failed");
        exit(-1);
    }

    for (int i = 0; i < NUM && flag; i++)
    {
        if (!clients_arr[i]) 
        {
            clients_arr[i] = new_sock;
            flag = 0;
        }
    }
}

int process_customers(int sock, char lst_buf[])
{
    fd_set set;
    char buf[SIZE];
    int max_fd, err, rsize, fd;

    while (1)
    {
        struct timeval interval = {30, 0};

        FD_ZERO(&set);
        FD_SET(sock, &set);
        

        max_fd = sock;

        for (int i = 0; i < NUM; i++)
        {
            if (clients_arr[i] > 0)
                FD_SET(clients_arr[i], &set);

            if (clients_arr[i] > max_fd)
                max_fd = clients_arr[i];
        }

        err = select(max_fd + 1, &set, NULL, NULL, &interval);
        if (err < 0)
        {
            close_socket(sock);
            perror("ERROR: select failed");
            return EXIT_FAILURE;
        }
        else if (!err)
        {
            close_socket(sock);
            printf("Time of waiting is over\n");
            return 0;
        }
        
        if (FD_ISSET(sock, &set))
            create_connection(sock, lst_buf);

        for (int i = 0; i < NUM; i++)
        {
            fd = clients_arr[i];
            if ((fd > 0) && FD_ISSET(fd, &set))
            {
                rsize = recv(fd, buf, sizeof(buf), 0);
                if (!rsize)
                {
                    printf("Client was disconnected\n");
                    clients_arr[i] = 0;
                }
                else
                {
                    buf[rsize] = '\0';
                    printf("Server got: %s\n", buf);

                    send_file(buf, fd);
                }
            }
        }
    }
}

int init_server()
{
    struct sockaddr_in serv_addr;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
    {
        perror("ERROR: socket failed");
        return EXIT_FAILURE;
    }
    fcntl(sock, F_SETFL, O_NONBLOCK);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        close(sock);
        perror("ERROR: bind failed");
        return EXIT_FAILURE;
    }

    if (listen(sock, NUM) < 0)
    {
        close(sock);
        perror("ERROR: listen failed");
        return EXIT_FAILURE;
    }

    return sock;
}

int main()
{    
    int     err;
    int     server = init_server();
    char    buf[256];

    printf("Server was created\n");

    list_file("./", buf);

    err = process_customers(server, buf);

    close_socket(server);

    printf("Server was closed\n");

    return 0;
}