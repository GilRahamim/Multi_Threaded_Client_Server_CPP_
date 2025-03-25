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

// Define an alias for an unordered_map representing the graph.
typedef unordered_map<int, list<int> > Graph;

/**
 * Reads pairs of numbers from a string and constructs a graph.
 * Each number in the pair is inserted into a variable and then used as components in the graph representation.
 * Finally, returns the constructed graph.
 */

Graph BuildGraph(const string& document) {
    Graph newGraph; // Create a new graph object.
    ifstream ifs(document); // Open the file specified by the document string for reading.
    string line; // String to store each line read from the file.

    // Read each line from the file.
    while (getline(ifs, line)) {
        istringstream iss(line); // Create a string stream for the current line.
        int v1, v2;
        // Read numbers from the string stream until no more numbers are available.
        while (iss >> v1 >> v2) {
            newGraph[v1].push_back(v2);
            newGraph[v2].push_back(v1);
        }
    }
    return newGraph;
}

/**
 * Implements the breadth-first search (BFS) algorithm to find the shortest path between two given nodes in the graph.
 */
vector<int> bfsAlgorithm(Graph &newGraph, int vSource, int vDest) {
    unordered_map<int, bool> visited; // Map to track visited nodes during BFS.
    unordered_map<int, int> prev; // Map to store the previous node in the shortest path.
    queue<int> queue; // Queue for BFS traversal.
    vector<int> path; // Vector to store the shortest path.

    // Initialize visited and prev maps for all nodes in the graph.
    for (auto &v: newGraph) {
        visited[v.first] = false;
        prev[v.first] = -1;
    }

    // Add the source node to the queue and mark it as visited.
    queue.push(vSource);
    visited[vSource] = true;

    // Perform BFS traversal until the queue is empty.
    while (!queue.empty()) {
        // Get the front node from the queue.
        int v = queue.front();
        queue.pop();

        // Check if the destination node is reached.
        if (v == vDest) {
            for (int now = vDest; now != -1; now = prev[now]) {
                path.push_back(now);
            }
            reverse(path.begin(), path.end());
            // return the shortest path.
            return path;
        }
        // Iterate over the neighbors of the current node.
        for (int neighbor: newGraph[v]) {
            // If the neighbor is not visited, add it to the queue and mark it as visited.
            if (!visited[neighbor]) {
                queue.push(neighbor);
                visited[neighbor] = true;
                prev[neighbor] = v;
            }
        }
    }
    // return the shortest path.
    return path;
}

// Cache memory to store recently computed paths.
deque<vector<int> > CacheMemory;
/**
     * Checks if the requested path is in the cache memory.
     * If not, computes the path using BFS algorithm and updates the cache memory.
     * Returns the requested path.
     */
vector<int> isInCacheMemory(Graph &newGraph, int vSource, int vDest) {
    // Vector to store the requested path.
    vector<int> path;
    // Check if the requested path is in the cache memory.
    for (auto &pathInMemory: CacheMemory) {
        if (pathInMemory.front() == vSource && pathInMemory.back() == vDest) {
            // if the path exists in the cache, return it.
            return pathInMemory;
        }
    }
    // If the path is not in the cache, compute it using BFS algorithm.
    path = bfsAlgorithm(const_cast<Graph &>(newGraph), vSource, vDest);
    // If the cache memory is full, remove the oldest entry.
    if (CacheMemory.size() >= 10) {
        CacheMemory.pop_front();
        CacheMemory.push_back(path);
    } else {
        CacheMemory.push_back(path);
    }
    // return the requested path.
    return path;
}

/**
 * Handles client requests for finding the shortest path between two nodes in the graph.
 */

void clientRequests(int client_socket, const Graph &newGraph) {
    // Buffer to store incoming message from client.
    char arr[1024];
    memset(arr, 0, sizeof(arr));
    // Read message from client socket.
    ssize_t msg = read(client_socket, arr, sizeof(arr));
    if (msg < 0) {
        perror("Failed to read the message");
    }
    istringstream iss(arr);
    int vSource, vDest;
    iss >> vSource >> vDest;
    // Get the shortest path between the source and destination nodes.
    vector<int> path = isInCacheMemory(const_cast<Graph &>(newGraph), vSource, vDest);

    string response;
    if (path.empty()) {
        response = "path not found";
    } else {
        for (int v: path) {
            response += to_string(v) + " ";
        }
    }
    // Send response message to client.
    write(client_socket, response.c_str(), response.size());
    close(client_socket);
}

/**
 * Main function that initializes the server and handles incoming client connections.
 */
int main(int argc, char *argv[]) {
    // Check if the correct number of command-line arguments are provided.
    if (argc < 3) {
        cerr << "ERROR!: please enter only file name and port number" << endl;
        return 1;
    }
    // Get the graph file name and port number from command-line arguments.
    string document = argv[1];
    int port = atoi(argv[2]);
    // Build the graph from the specified document.
    Graph newGraph = BuildGraph(document);
    // Initialize server socket.
    int serverSocket, newSocket;
    struct sockaddr_in address;
    int addressLength = sizeof(address);
    int option = 1;

    // Create server socket.
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket Failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port.
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
        //will set the socket option to reuse the address and port
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set server address properties.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind server socket to the specified address and port.
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections.
    if (listen(serverSocket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept incoming client connections and handle requests.
    while (true) {
        if ((newSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*) &addressLength)) < 0) {
            //if the client is not accepted so will print the error and continue accepting other clients
            perror("accept");
            continue;
        }

        // Spawn a new thread to handle the client request.
        thread(clientRequests, newSocket, ref(newGraph)).detach();
    }
    return 0;
}
