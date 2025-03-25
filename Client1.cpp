#include "std_lib_facilities.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//Ron Yehuda ID:206291973
//Gil Rahamim ID:207598137


int main(int argc, char* argv[]) {
    // Check if the correct number of command-line arguments are provided.
    if(argc != 5){
        perror("Input error");
    }
    const char* Serverip = argv[1];// Get the server IP address from command-line argument.
    int port = atoi(argv[2]); // Get the port number from command-line argument.
    int from = atoi(argv[3]); // Get the source node from command-line argument.
    int to = atoi(argv[4]); // Get the destination node from command-line argument.
    int client_socket; // Create a variable to store the client socket.
    struct sockaddr_in serverAddress; // Structure to hold server address information.

    // Create a socket for the client and check for errors.
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    // Set server address family and port.
    serverAddress.sin_family = AF_INET ;
    serverAddress.sin_port = htons(port);

    // Convert IP address from text to binary form and check for errors.
    if (inet_pton(AF_INET, Serverip, &serverAddress.sin_addr) <= 0) {
        perror("Address error ");
        exit(EXIT_FAILURE);
    }

    // Connect to the server and check for errors.
    if (connect(client_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress) ) < 0) {
        perror("Error!: Could not cinnect to server");
        exit(EXIT_FAILURE);
    }

    // Construct the request to send to the server, including source and destination nodes.
     string request = to_string(from) + " " + to_string(to);

    write(client_socket, request.c_str(), request.size() );
    char arr[1024] = {0};
    read(client_socket, arr, sizeof(arr));
    cout << "Answer: " << arr << endl;
    close(client_socket);
    return 0;

}
