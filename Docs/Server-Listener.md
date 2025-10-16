# Server-Listener

At the highest level this project concerns handling client interactions.
The primary objective is to create a program that listens on a PORT, allows a client to connect to the PORT and send a predifined set of commands that the program will handle.

LISTENER

- Creates the socket
- Waits for connections
- Creates new connections when a client sends a request

SERVER

- Calls poll() to wait for events on the sockets
- Asks the listener for a new connection when a client connects
- Ask the connection to deal with the receipt/sending of messages
- Manages the removal/insertion of connections
- Handles the removal of clients from channels

## THE LISTENER CLASS

Everything related to the network (accepting connections, listening on a port) is handled by the class Listener.

The Listener class has 3 attributes

`const char* hostname;` This is "127.0.0.1" but can be modified to be any hostname.
`const char* port;`     The port it will listen on for incoming connections, passed to our program as an argument.
`int         socketFd;` The FD of the socket.

Q: How do we create a connection between our program and other programs?
A: The Listener (and several syscalls) will realise this via the SOCKET.

NETWORKING (SOCKETS)

What's a socket?
A socket is a file, just as everything else is in Unix, which is why we have a socketFd in our Listener class.
The socket is nothing more than a channel which enables our program and other programs to communicate over a network (the inclusion of a network separates a socket from, say, a pipe).

To create the socket the Listener has a method `createSocket()`:

```C++
void    Listener::createSocket(int ai_family, int ai_socktype)
{
    int                 err;
    int                 yes = 1;
    struct addrinfo*    info;
    struct addrinfo*    it;
    struct addrinfo     hints;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = ai_family;
    hints.ai_socktype = ai_socktype;
    hints.ai_flags = AI_PASSIVE;
    err = getaddrinfo(hostname, port, &hints, &info);
    if (err != 0)
        throw std::runtime_error("Error in getting the address info");
    for (it = info; it != NULL; it = it->ai_next)
    {
        socketFd = socket(it->ai_family, it->ai_socktype | SOCK_NONBLOCK, it->ai_protocol);
        if (socketFd == -1)
            continue;
        setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(socketFd, it->ai_addr, it->ai_addrlen) == 0)
            break;
    }
    err = (it == NULL);
    freeaddrinfo(info);
    if (err)
        throw std::runtime_error("Could not bind to any socket");
}
```

The first function of interest is `getaddrinfo`,
`int getaddrinfo(char *node, char *service, struct addrinfo *hints, struct addrinfo **res);`

- `node`:       name of the host (can be an IP address like "127.0.0.1" or a name like "www.ourserv.com")
- `service`:    the port where the program is listening (for example "8080")
- `hints`:      a struct that contains information about what kind of socket we would like to obtain as an output from this function. In our case we are specifing:
  - `ai_family`   = `IPv4` (specifies if IPv4 or IPv6)
  - `ai_socktype` = `SOCK_STREAM` (the option for TCP connections (SOCK_DGRAM for UDP connections (faster but less consistent, we don't use it))). We also add SOCK_NONBLOCK mask so that if there's nothing to read when we try, our program will not wait
  - `ai_flags`    = `AI_PASSIVE` (flag used to create a socket that will be used to accept incoming connections)
- `res`:        the result of the function will be stored in this parameter as a linked list of structs that hold our socket option, it will be the info we pass to the syscall socket() to create our listening socket

## NETWORKING (BINDING)

Once we have our result from getaddrinfo we will traverse the linked list and try to create a new socket with the syscall socket() and the structs that getaddrinfo returned to us.
Once the syscall returns success and we have our socket we will set the socket option with setsocktopt. The use of this functions is to avoid the error "Address already in use" when calling later the function bind.
So we have our file descriptor returned from socket, if we want to associate our socket to a specific port we need to call bind().
Bind will literally connect the socket to the port. We call bind like this

`bind(socketFd, it->ai_addr, it->ai_addrlen)`

We pass:
    - the socket fd
    - the address ip + port
    - the length of the address (this is necessary because ipv4 and ipv6 have different sizes)

We now have a socket bound to a port, the next step will be start listening for incoming connections.
We do that by calling listen() and passing the socket file descriptor and a number that specifies the queue of connections we are allowing on this socket.

To recap:

- Call `getaddrinfo()`
- Call `socket()` with the options provided by getaddrinfo until the function gives us a valid file descriptor
- Call bind on the file descriptor that was returned from socket

## NETWORKING (NEW CONNECTIONS)

The final Listener class method to look at is `acceptConnection()`.

This is called when a new client is trying to connect to our program:

```C++
Connection      Listener::acceptConnection() 
{
    int                     newFd;
    struct sockaddr_storage s;
    socklen_t               addrlen;

    addrlen = sizeof(s);
    newFd = accept(socketFd, (struct sockaddr*)&s, &addrlen);
    if (newFd == -1) // cannot accept more connections from this socket, stop listening
    {
        close(socketFd);
        socketFd = -1;
        throw(std::runtime_error("Reached max socket capacity"));
    }
    fcntl(newFd, F_SETFL, O_NONBLOCK);
    return (Connection(newFd, &s));
}
```

The method calls accept(), a function that will take the socket file descriptor, the
struct that will hold the informations about the new connection (ip etc.) and the len of the address.

If this function returns an error it means that we reached max capacity on the socket and we will not handle anymore incoming connections by closing the socket file descriptor.

Otherwise accept will return us a new file descriptor that will refer to a new socket.
The server has two types of sockets, one that is the listener socket and is only used for granting access to clients to connect to the server.

Once a client tries to connect the listener socket will receive a packet informing that someone wants to create a new connection, This connection is then created on a NEW socket, that will be the one responsible for handling the communication between our server and that client.

We can think of the listener socket as some kind of doorbell, our program is the hotel and if someone wants to gets in it will have to request by ringing the doorbell (send a packet to our listener socket).

This alerts our program that a new guest has arrived, we can open the door and let them in.`accept()` is like the receptionist at the hotel that will check in the new guest. It will take its information and store it in the struct we pass to the function

It returns the room (the socket file descriptor) where the person can stay in our hotel (the server).

## THE SERVER CLASS

The server class polices the program. It performs checks and reacts to events according to some rules.

### ATTRIBUTES

These are the attributes of the server:

```C++
private:
    Listener                                        listener;
    std::string                                     password;
    std::vector<pollfd>                             polls;
    std::list<Connection>                           connections;
    std::map<const std::string, Channel>            channels;
    std::map<int, Connection*>                      fdToConnection;
    std::map<const std::string, Connection*>        nickToConnection;
    connectionID                                    currentId;
```

- The server has one Listener, as previously discussed.
- The password is what is given to the program with the PASS command in order to get into the server.
- A vector of pollfd
- A list of Connection classes, a linked list of all the clients connected to the server
- `channels`, `fdToConneciton` and `nickToConnection` are three maps used to access a specific channel/connection efficiently.
- `channels` stores all channels on the server and can be retrieved from its name.
    The maps are :
- `fdToConnection` apparently you cannot have references so that's why pointers are used.
- `nickToConnection` maps nicknames to Connection pointers of clients. Useful for commands like send a private message to person "x".
- `currentId`, everytime a new connection is created it will be assigned a unique ID that will never change, The type is connectionID which expands to

`typedef long connectionID; in  the Connection.hpp`

People can change their nickname on the server and so a nickname is not a good way to identify a single connection.

## POOL EVENTS

The most important thing to know about the Server class is how it does input/output operations. How does the server knows when it received a message from a client?

It uses the syscall, poll() which is utilised in the method pollEvents().

```c++
void Server::pollEvents()
{
    int ret;

    ret = poll(polls.data(), polls.size(), EVENT_TIMEOUT_TIME);
    if (ret == -1)
        throw(std::runtime_error("Poll error"));
    for (size_t i = 0; i < polls.size(); i++)
    {
        if (polls.at(i).revents != 0) // io event happened on fd
        {
            if (i == 0)
                createConnection();
            else
                handleClientInteraction(polls.at(i));
        }
    }
    checkForTimeouts();
}
```

We pass to poll() the array of pollfd structs, the number of pollfds we have and the number of milliseconds we want to wait for an event to happen.
If no event happens in the time specified, poll returns 0.

`std::vector<pollfd> polls;`

pollfd is a struct that we use to pass information to the function poll:

```C++
struct pollfd {
               int   fd;
               short events;
               short revents;
           };
```

- `fd`: the file descriptor of the connection's socket, the one returned from the call accept.
- `events`: Which kind of event we want poll to check for, in our case it will always start as `POLLIN` which means we want poll to notify us when there's data that was sent FROM the client
to the socket. So we are polling IN our server new data. The other option we can specify is `POLLOUT` which means that we are asking poll to notify us when we can SEND to the client a new message. We will only add `POLLOUT` when we have a message to send to the client. We don't set it by default otherwise every time we will have a notification from poll saying that the client can receive a message.
- `revents` This field will be set by the poll function when there's a new event from the ones specified in `events`. That's what this line is for

`polls.at(i).revents != 0`

In this method we call poll() and wait for it to return.
poll() watches on each of the FDs passed to it (IO MULTIPLEXING) and once something happens will return to us.
We can then traverse the vector of polls we have and check what happened.

Our first pollfd refers to the listener socket, hence:

`if (i === 0) createConnection()`

The only thing that can trigger our listener is a new incoming connection so we will act accordingly.

Otherwise we handle the client interaction, most client interaction is handled by the Connection class.

The server checks if the event received is:

- POLLHUP The client hung up the connection and we will close it)
- POLLOUT The connection class will handle sending the message to the client. The server must only check if there was an error in which case it will close the connection. If the message sent was a message of closed connection. In IRC, before closing a connection, you need to notify the client that you are closing the connection and only after that can you close it.
- POLLIN  The client sent us a message, we check if there are any errors and if not we make sure we received the endline. If that's the case we pass to handling the command.

## HANLDING COMMANDS

The server merely has to split each command by line. Each line has a different command.
Then split the line into a vector of words, where the first word is the command, and the rest are the arguments. This job is done by the method:

`handleClientCommand()`

After we have our vector of words we will pass it to `handleSimpleCommand()` that will match the command received with the ones that are available on this server, if the command is not recognized the server, it will silently fail, meaning no error message is sent to the client.

## HANDLING CONNECTIONS

Connections are stored in a list. At the same time we also have a map of strings to connection pointers and also each connection has its own file descriptor that is stored in the vector of pollfd.

When a connection is created:

- Create the connection (done by our server with the method `createConnection()`) by adding it to the list of connections.
- Assign a poll struct to the new Connection created since each connection stores internally a pointer to its own pollfd.

To instead remove a connection from the server:

- Remove the client from the channel(s) they are in
- Remove the connection from the map of nicknames to connections
- Remove the connection from the map of fds to connections
- Remove the connection by erasing it from the list of connections
- Set the pollfd file descriptor to -1 so that it will be ignored from the next calls to pol (we could also remove it directly from the pollfd vector but this is more efficient)
