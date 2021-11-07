#include "includes.hpp"


int clients_arr[NUM];
std::string addr_arr[NUM];


void save_info(const std::string user, const std::string &path)
{
    std::ofstream f(HISTORY_REQUESTS, std::ios::app);
    f << user << "\t" << path << "\n";
}

void get_params(const std::string buf, std::string &kind, std::string &path, 
                std::string &vrs, std::string &ext)
{
    int from = 0, to = buf.find(" ");

    kind = buf.substr(from, to - from);
    
    from = to + 1; 
    to = buf.find(" ", from);

    path = buf.substr(from, to - from);
    path = path.substr(1);

    from = path.find(".");
    ext = path.substr(from + 1, path.length());

    from = to + 1;
    to = buf.find("\r", from);

    vrs = buf.substr(from, to - from);
}

void form_response(const std::string buf, int fd, std::string ip)
{
    std::string kind, path, vrs, ext;

    get_params(buf, kind, path, vrs, ext);

    if (kind != "GET")
    {
        perror("ERROR: only GET can be processed");
        return;
    }

    if (vrs != "HTTP/1.1")
    {
        perror("ERROR: only HTTP/1.1 can be processed");
        return;
    }

    std::ifstream file(BASE_PATH + std::string(path));

    std::string response = "";
    std::string content_type, body = "", status, status_code;
    std::string temp;

    std::map<std::string, std::string> type_arr = {
        { "html", "text/html" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "png", "image/png" },
        { "txt", "text/html" },
    };

    content_type = type_arr[ext];
    if (content_type == "")
        content_type = "text/html;";
    
    if (file.is_open())
    {
        while (std::getline(file, temp))
            body += temp + "\n";

        status_code = "200";
        status = "OK";
    }
    else
    {
        status_code = "404";
        status = "Not Found";
        body = "<html>\n\r<body>\n\r<h1>404 Not Found</h1>\n\r</body>\n\r</html>";
    }

    response += vrs + " " + status_code + " " + status + "\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "Content-Type: " + content_type  + "\r\n";
    response += "Connection: closed\r\n\r\n";
    response += body;

    save_info(ip, path);
    send(fd, response.c_str(), response.size(), 0);
}


void close_socket(int sock)
{
    for (int i = 0; i < NUM; i++)
        if (clients_arr[i])
            close(clients_arr[i]);
    
    close(sock);
}

void create_connection(int sock)
{
    struct sockaddr_in client_addr;
    socklen_t size_client = sizeof(client_addr);
    int flag = 1;

    int new_sock = accept(sock, (struct sockaddr*) &client_addr, &size_client);
    if (new_sock < 0) 
    {
        perror("ERROR: accept failed");
        exit(-1);
    }

    printf("New connection added. Client fd = %d\t ip = %s:%d\n",
        new_sock, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    for (int i = 0; i < NUM && flag; i++)
    {
        if (!clients_arr[i]) 
        {
            clients_arr[i] = new_sock;
            addr_arr[i] = inet_ntoa(client_addr.sin_addr) + std::string(":") + std::to_string(ntohs(client_addr.sin_port));
            flag = 0;
        }
    }
}

int process_customers(int sock, ThreadPool &pool)
{
    fd_set set;
    std::string buf;
    char buf_arr[SIZE];
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
            create_connection(sock);

        for (int i = 0; i < NUM; i++)
        {
            fd = clients_arr[i];
            if ((fd > 0) && FD_ISSET(fd, &set))
            {
                rsize = recv(fd, buf_arr, sizeof(buf_arr), 0);
                if (!rsize)
                    printf("Client was disconnected\n");
                else if (rsize == -1)
                {
                    perror("ERROR: recv failed");
                    return EXIT_FAILURE;
                }
                else
                {
                    buf_arr[rsize] = '\0';
                    buf = buf_arr;

                    
                    auto res = pool.add(form_response, buf, fd, addr_arr[i]);
                    res.get();
                    //form_response(buf, fd);
                    close(fd);
                }

                clients_arr[i] = 0;
                addr_arr[i] = "";
            }
        }
    }

    return EXIT_SUCCESS;
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
    ThreadPool pool(NUM);

    printf("Server was created\n");

    err = process_customers(server, pool);

    close_socket(server);

    printf("Server was closed\n");

    return err;
}