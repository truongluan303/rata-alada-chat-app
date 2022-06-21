#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

using namespace std;


#define EXIT_COMMAND    "/exit"
#define CONSOLE_GREEN   "\x1b[32m"
#define CONSOLE_RESET   "\x1b[0m"


int             client_socket_desc;
hostent*        host;
sockaddr_in     send_sock_addr;
size_t          bytes_read          = 0;
size_t          bytes_written       = 0;
struct timeval  start1;
struct timeval  end1;


/******************************************************************************
 * @brief   Handle keyboard intterupt
 * 
 * @param   signum      Signal number
 *****************************************************************************/
void handle_keyboard_interrupt(int signum);


/******************************************************************************
 * @brief   Execute the exit protocols
 *****************************************************************************/
void exit_session();


//===========================================================================//
//                              Guest (Client Side)                          //
//===========================================================================//
int main(int argc, char *argv[])
{
    // we need ip address and port number in that order
    if(argc != 3)
    {
        cerr << "Usage: ip_address port" << endl; exit(0); 
    } //grab the IP address and port number

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    char msg[1500];

    //setup a socket and connection tools 
    host = gethostbyname(server_ip); 
    bzero((char*)&send_sock_addr, sizeof(send_sock_addr)); 
    send_sock_addr.sin_family = AF_INET; 
    send_sock_addr.sin_addr.s_addr = inet_addr(
        inet_ntoa(*(struct in_addr*)*host->h_addr_list)
    );
    send_sock_addr.sin_port = htons(port);
    client_socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    // try to connect...
    int status = connect(
        client_socket_desc, (sockaddr*) &send_sock_addr, sizeof(send_sock_addr)
    );
    if (status < 0)
    {
        cout << "Error while connecting!" << endl;
        exit(1);
    }
    cout << "Connected!" << endl;
    gettimeofday(&start1, NULL);

    // register signal and signal handler
    signal(SIGINT, handle_keyboard_interrupt);
    cout << CONSOLE_GREEN;
    while (true)
    {
        cout << "<?> ";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg));   //clear the buffer
        strcpy(msg, data.c_str());

        if (data == EXIT_COMMAND)
        {
            exit_session();
        }
        bytes_written += send(client_socket_desc, (char*)&msg, strlen(msg), 0);
        cout << "Awaiting server response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        bytes_read += recv(client_socket_desc, (char*)&msg, sizeof(msg), 0);
        
        if (!strcmp(msg, EXIT_COMMAND))
        {
            cout << "Server has quit the session!" << endl;
            break;
        }
        cout << msg << endl;
    }
}


void handle_keyboard_interrupt(int signum)
{
    exit_session();
}


void exit_session()
{
    send(
        client_socket_desc,
        (char*)&EXIT_COMMAND,
        strlen(EXIT_COMMAND),
        0
    );
    gettimeofday(&end1, NULL);
    close(client_socket_desc);

    cout << CONSOLE_RESET << endl << endl;
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytes_written;
    cout << " Bytes read: " << bytes_read << endl;
    cout << "Elapsed time: " << (end1.tv_sec- start1.tv_sec);
    cout << " seconds" << endl;
    cout << "Connection closed" << endl;
    exit(0);
}