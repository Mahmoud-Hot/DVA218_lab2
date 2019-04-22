/* File: server.c
 * Trying out socket communication between processes using the Internet protocol family.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define PORT 5555
#define MAXMSG 512


int blockedIP;

/* makeSocket
 * Creates and names a socket in the Internet
 * name-space. The socket created exists
 * on the machine from which the function is
 * called. Instead of finding and using the
 * machine's Internet address, the function
 * specifies INADDR_ANY as the host address;
 * the system replaces that with the machine's
 * actual address.
 */



typedef struct LinkedList{
    int sock;
    struct LinkedList *node;
}SockList;


SockList *sockList;

void reply();
int makeSocket(unsigned short int port) {
    int sock;
    struct sockaddr_in name;

    /* Create a socket. */
    sock = socket(PF_INET, SOCK_STREAM, 0); //sock_stream = tcp, sock_dgram = udp
    if(sock < 0) {
        perror("Could not create a socket\n");
        exit(EXIT_FAILURE);
    }
    /* Give the socket a name. */
    /* Socket address format set to AF_INET for Internet use. */
    name.sin_family = AF_INET;
    /* Set port number. The function htons converts from host byte order to network byte order.*/
    name.sin_port = htons(port);
    /* Set the Internet address of the host the function is called from. */
    /* The function htonl converts INADDR_ANY from host byte order to network byte order. */
    /* (htonl does the same thing as htons but the former converts a long integer whereas
     * htons converts a short.)
     */
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Assign an address to the socket by calling bind. */
    if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
        perror("Could not bind a name to the socket\n");
        exit(EXIT_FAILURE);
    }
    return(sock);
}

/* readMessageFromClient
 * Reads and prints data read from the file (socket
 * denoted by the file descriptor 'fileDescriptor'.
 */
int readMessageFromClient(int fileDescriptor) {
    char buffer[MAXMSG];
    int nOfBytes;

    nOfBytes = read(fileDescriptor, buffer, MAXMSG);
    if(nOfBytes < 0) {
        perror("Could not read data from client\n");
        exit(EXIT_FAILURE);
    }
    else
    if(nOfBytes == 0)
        /* End of file */
        return(-1);
    else
        /* Data read */
        printf(">Incoming message: %s\n",  buffer);
    reply(fileDescriptor);
    return(0);
}
void reply(int fileDescriptor){
    write(fileDescriptor, "I hear you dude..", MAXMSG);
}

void addToSocketList(int socket){

    SockList *temp, *current;
    current = sockList; //global socklist

    temp = (SockList*)malloc(sizeof(SockList)); //new socket to add
    temp->sock = socket;

    if(sockList == NULL){ //if head == null  set head to temp
        sockList = temp;
        return;
    }

    while(current->node != NULL){
        current = current->node;
    }
    current->node = temp; //add new node to end of list


    return;
}

void removeFromList(int ID, SockList *head){

    SockList * current = head;
    SockList * temp = NULL;

    if(head == NULL){
        return;
    }

    while(current->sock != ID){
        temp = current;
        current = current->node;
    }

    temp->node = current->node;
    free(current);
    return;
}

int main(int argc, char *argv[]) {
    int sock;
    int clientSocket;
    int i;
    fd_set activeFdSet, readFdSet; /* Used by select */
    struct sockaddr_in clientName;
    socklen_t size;




    //sockList = (SockList*)malloc(sizeof(SockList));

    /* Create a socket and set it up to accept connections */
    sock = makeSocket(PORT);
    /* Listen for connection requests from clients */
    if(listen(sock,1) < 0) {
        perror("Could not listen for connections\n");
        exit(EXIT_FAILURE);
    }
    /* Initialize the set of active sockets */
    FD_ZERO(&activeFdSet);
    FD_SET(sock, &activeFdSet);


    printf("\n[waiting for connections...]\n");

    while(1) {
        /* Block until input arrives on one or more active sockets
           FD_SETSIZE is a constant with value = 1024 */
        readFdSet = activeFdSet;
        if(select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0) {
            perror("Select failed\n");
            exit(EXIT_FAILURE);
        }
        /* Service all the sockets with input pending */
        for(i = 0; i < FD_SETSIZE; ++i)
            if(FD_ISSET(i, &readFdSet)) {
                if(i == sock) {
                    /* Connection request on original socket */
                    size = sizeof(struct sockaddr_in);
                    /* Accept the connection request from a client. */
                    clientSocket = accept(sock, (struct sockaddr *)&clientName, (socklen_t *)&size);

                    if(clientName.sin_addr.s_addr == inet_addr("127.0.0.2")){
                        printf("Connection refused, blocked ip.");
                        write(clientSocket, "Remote host close connection", MAXMSG);
                        close(i);
                        removeFromList(sock, sockList);
                    }

                    if(clientSocket < 0) {
                        perror("Could not accept connection\n");
                        exit(EXIT_FAILURE);
                    }


                    printf("Server: Connect from client %s, port %d\n",
                           inet_ntoa(clientName.sin_addr),
                           ntohs(clientName.sin_port));
                    FD_SET(clientSocket, &activeFdSet);


                    SockList *current = sockList;
                    while(current != NULL){
                        write(current->sock, "New client!", MAXMSG);
                        current=current->node;
                    }
                    addToSocketList(clientSocket);

                    /*int len = sizeof(activeFdSet.__fds_bits) /sizeof(activeFdSet.__fds_bits[0]);
                    for(int i = 0; i<len; i++){
                        write(activeFdSet.__fds_bits[i], "New client connected", MAXMSG);
                    }*/

                }
                else {
                    /* Data arriving on an already connected socket */
                    if(readMessageFromClient(i) < 0) {
                        close(i);
                        removeFromList(i, sockList);
                        FD_CLR(i, &activeFdSet);
                    }
                }
            }
    }
}

