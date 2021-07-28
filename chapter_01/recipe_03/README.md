# Creating a passive socket

A passive socket or acceptor socket is a type of socket that is used to wait for connection establishment requests from remote applications that communicate over the TCP protocol. This definition has two important implications:

 - Passive sockets are used only in server applications or hybrid applications that may play both roles of the client and server.
 - Passive sockets are defined only for the TCP protocol. As the UDP protocol doesn't imply connection establishment, there is no need for a passive socket when communication is performed over UDP.

In Boost.Asio a passive socket is represented by the asio::ip::tcp::acceptor class. The name of the class suggests the key function of the objects of the class—to listen for and accept or handle incoming connection requests.

The following algorithm describes the steps required to perform to create an acceptor socket:

- Create an instance of the asio::io_service class or use the one that has been created earlier.
- Create an object of the asio::ip::tcp class that represents the TCP protocol and the required version of the underlying IP protocol (IPv4 or IPv6).
- Create an object of the asio::ip::tcp::acceptor class representing an acceptor socket, passing the object of the asio::io_service class to its constructor.
- Call the acceptor socket's open() method, passing the object representing the protocol created in step 2 as an argument.

# How it works
Because an acceptor socket is very similar to an active socket, the procedure of creating them is almost identical. Therefore, here we only shortly go through the sample code. For more details about each step and each object involved in the procedure, please refer to the Creating an active socket recipe.

In step 1, we create an instance of the asio::io_service class. This class is needed by all Boost.Asio components that need access to the services of the underlying operating system.

In step 2, we create an object representing a TCP protocol with IPv6 as its underlying protocol.

Then in step 3, we create an instance of the asio::ip::tcp::acceptor class, passing an object of the asio::io_service class as an argument to its constructor. Just as in the case of an active socket, this constructor instantiates an object of Boost.Asio the asio::ip::tcp::acceptor class, but does not allocate the actual socket object of the underlying operating system.

The operating system socket object is allocated in step 4, where we open the acceptor socket object, calling its open() method and passing the protocol object to it as an argument. If the call succeeds, the acceptor socket object is opened and can be used to start listening for incoming connection requests. Otherwise, the ec object of the boost::system::error_code class will contain error information.

## How to build
```
mkdir build
cd build
cmake ..
cmake --build .
```

## How to run
```
./bin/main
```
