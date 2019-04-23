/*
 * Client file that sends message to a server, uses separate thread to listen for any replys.
 *
 * Run instructions:
 * compile: gcc client.c -o client -lpthread
 * execute: ./client 127.0.0.1
 *
 *
 * Created by:
 * Casper Wahl (cwl17001)
 * Hawkar Karim (ham17002)
 * Viktor Lindgren (vln16005)
 *
 * For DVA228 laboration 2 at MDH
 *
 * */




/* File: client.c
 * Trying out socket communication between processes using the Internet protocol family.
 * Usage: client [host name], that is, if a server is running on 'lab1-6.idt.mdh.se'
 * then type 'client lab1-6.idt.mdh.se' and follow the on-screen instructions.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>


#define PORT 5555
#define hostNameLength 50
#define messageLength  256
#define MAXMSG 512

/* initSocketAddress
 * Initialises a sockaddr_in struct given a host name and a port.
 */
void initSocketAddress(struct sockaddr_in *name, char *hostName, unsigned short int port) {
    struct hostent *hostInfo; /* Contains info about the host */
    /* Socket address format set to AF_INET for Internet use. */
    name->sin_family = AF_INET;
    /* Set port number. The function htons converts from host byte order to network byte order.*/
    name->sin_port = htons(port);
    /* Get info about host. */
    hostInfo = gethostbyname(hostName);
    if(hostInfo == NULL) {
        fprintf(stderr, "initSocketAddress - Unknown host %s\n",hostName);
        exit(EXIT_FAILURE);
    }
    /* Fill in the host name into the sockaddr_in struct. */
    name->sin_addr = *(struct in_addr *)hostInfo->h_addr;
}



/*
 * Loops and checks if the server has sent a new message,
 * ex if a new client has connected.
 * This func is ran on a separate thread.
 */
int readMessageFromServer(int fileDescriptor) {
    while(1){
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
            printf(">Server: %s\n",  buffer);

        sleep(1);
    }

    return(0);
}
void writeMessage(int fileDescriptor, char *message) {
    int nOfBytes;

    nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
    if(nOfBytes < 0) {
        perror("writeMessage - Could not write data\n");
        exit(EXIT_FAILURE);
    }


}


int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serverName;
    char hostName[hostNameLength];
    char messageString[messageLength];

    /* Check arguments */
    if(argv[1] == NULL) {
        perror("Usage: client [host name]\n");
        exit(EXIT_FAILURE);
    }
    else {
        strncpy(hostName, argv[1], hostNameLength);
        hostName[hostNameLength - 1] = '\0';
    }
    /* Create the socket */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Could not create a socket\n");
        exit(EXIT_FAILURE);
    }
    /* Initialize the socket address */
    initSocketAddress(&serverName, hostName, PORT);
    /* Connect to the server */
    if(connect(sock, (struct sockaddr *)&serverName, sizeof(serverName)) < 0) {
        perror("Could not connect to server\n");
        exit(EXIT_FAILURE);
    }
    /* Send data to the server */
    printf("\nType something and press [RETURN] to send it to the server.\n");
    printf("Type 'quit' to nuke this program.\n");
    fflush(stdin);


    //thread to run the listener function
    //passes the clients socket to the function as argument
    pthread_t readMessage;
    pthread_create(&readMessage, NULL, readMessageFromServer, (int*)sock);

    while(1) {
        printf("\n>");
        fgets(messageString, messageLength, stdin);
        messageString[messageLength - 1] = '\0';
        if (strncmp(messageString, "quit\n", messageLength) != 0) {
            writeMessage(sock, messageString);
            //readMessageFromServer(sock);
        } else {
            close(sock);
            exit(EXIT_SUCCESS);
        }
    }



}

