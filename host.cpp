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


int             server_socket_desc;
int             new_socket_desc;
int             bytes_read          = 0;
int             bytes_written       = 0;
struct timeval  start1;
struct timeval  end1;
sockaddr_in     server_addr;


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
//                              Host (Server Side)                           //
//===========================================================================//
int main(int argc, char *argv[])
{
    // as the host will create a new "room,"
    // we only need to specify a port number
    if(argc != 2)
    {
        cerr << "Usage: port" << endl;
        exit(0);
    }
    int port = atoi(argv[1]);   // port number
    char msg[1500];             // buffer to send and receive messages with
     
    //setup a socket and connection tools
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
 
    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    server_socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_desc < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    int bind_status = bind(
        server_socket_desc, (struct sockaddr*) &server_addr, sizeof(server_addr)
    );
    if(bind_status < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    cout << "Waiting for a client to connect..." << endl;
    //listen for up to 5 requests at a time
    listen(server_socket_desc, 5);
    //receive a request from client using accept
    //we need a new address to connect with the client
    sockaddr_in new_socket_addr;
    socklen_t new_socket_addr_size = sizeof(new_socket_addr);
    //accept, create a new socket descriptor to 
    //handle the new connection with client
    new_socket_desc = accept(
        server_socket_desc, (sockaddr *)&new_socket_addr, &new_socket_addr_size
    );
    if (new_socket_desc < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(1);
    }
    cout << "Connected with client!" << endl;

    gettimeofday(&start1, NULL);

    // register signal and signal handler
    signal(SIGINT, handle_keyboard_interrupt);

    while (true)
    {
        // receive a message from the client (listen)
        cout << "Awaiting client response..." << endl;
        memset(&msg, 0, sizeof(msg));   //clear the buffer
        bytes_read += recv(new_socket_desc, (char*)&msg, sizeof(msg), 0);
        if(!strcmp(msg, EXIT_COMMAND))
        {
            exit_session();
        }
        cout << "Client: " << msg << endl;
        cout << "<?> ";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg)); //clear the buffer
        strcpy(msg, data.c_str());
        if (data == EXIT_COMMAND)
        {
            exit_session();
        }
        //send the message to client
        bytes_written += send(new_socket_desc, (char*)&msg, strlen(msg), 0);
    }
}


void handle_keyboard_interrupt(int signum)
{
    exit_session();
}


void exit_session()
{
    send(
        server_socket_desc,
        (char*)&EXIT_COMMAND,
        strlen(EXIT_COMMAND),
        0
    );
    gettimeofday(&end1, NULL);
    close(server_socket_desc);

    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytes_written;
    cout << " Bytes read: " << bytes_read << endl;
    cout << "Elapsed time: " << (end1.tv_sec- start1.tv_sec);
    cout << " seconds" << endl;
    cout << "Connection closed" << endl;
    exit(0);
}