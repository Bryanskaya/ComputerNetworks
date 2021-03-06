#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <iostream>
#include <map>

#include "threads.hpp"


#define HOST                "127.0.0.1"
#define PORT                8001
#define NUM                 5
#define SIZE                100
#define BASE_PATH           "./"
#define HISTORY_REQUESTS    "./history.txt"
