# Shutting down and closing a socket


Shutting down and closing a socket
In some distributed applications that communicate over the TCP protocol, there is a need to transfer messages that do not have a fixed size and specific byte sequence, marking its boundary. This means that the receiving side, while reading the message from the socket, cannot determine where the message ends by analyzing the message itself with either its size or its content.

One approach to solve this problem is to structure each message in such a way that it consists of a logical header section and a logical body section. The header section has a fixed size and structure and specifies the size of the body section. This allows the receiving side to first read and parse the header, find out the size of the message body, and then properly read the rest of the message.

This approach is quite simple and is widely used. However, it brings some redundancy and additional computation overhead, which may be unacceptable in some circumstances.

Another approach can be applied when an application uses a separate socket for each message sent to its peer, which is a quite popular practice. The idea of this approach is to shut down the send part of the socket by the message sender after the message is written to the socket. This results in a special service message being sent to the receiver, informing the receiver that the message is over and the sender will not send anything else using the current connection.

The second approach provides many more benefits than the first one and, because it is part of the TCP protocol software, it is readily available to the developer for usage.

Another operation on a socket, that is, closing may seem similar to shutting down, but it is actually very different from it. Closing a socket assumes returning the socket and all the other resources associated with it back to the operating system. Just like memory, a process or a thread, a file handle or a mutex, a socket is a resource of an operating system. And like any other resource, a socket should be returned back to the operating system after it has been allocated, used, and is not needed by the application anymore. Otherwise, a resource leak may occur, which may eventually lead to the exhaustion of the resource and to the application's fault or instability of the whole operating system.

Serious issues that may occur when sockets are not closed make closing a very important operation.

The main difference between shutting down and closing a TCP socket is that closing interrupts the connection if one is established and, eventually, deallocates the socket and returns it back to the operating system, while shutting down only disables writing, reading, or both the operations on the socket and sends a service message to the peer application notifying about this fact. Shutting down a socket never results in deallocating the socket.

Here, we'll consider a distributed application that consists of two parts: a client and a server to better understand how a socket shut down operation can be used to make an application layer protocol more efficient and clear when the communication between parts of distributed applications is based on binary messages of random sizes.

## The client application
The purpose of the client application is to allocate the socket and connect it to the server application. After the connection is established, the application should prepare and send a request message notifying its boundary by shutting down the socket after writing the message to it.

After the request is sent, the client application should read the response. The size of the response is unknown; therefore, the reading should be performed until the server closes its socket to notify the response boundary.

## The server application
The server application is intended to allocate an acceptor socket and passively wait for a connection request. When the connection request arrives, it should accept it and read the data from the socket connected to the client until the client application shuts down the socket on its side. Having received the request message, the server application should send the response message notifying its boundary by shutting down the socket.

## Closing a socket
In order to close an allocated socket, the close() method should be called on the corresponding object of the asio::ip::tcp::socket class. However, usually, there is no need to do it explicitly because the destructor of the socket object closes the socket if one was not closed explicitly.

# How it works
The server application is first started. In its main() entry point function, an acceptor socket is allocated, opened, bound to port 3333, and starts waiting for the incoming connection request from the client.

Then, the client application is started. In its main() entry point function, an active socket is allocated, opened, and connected to the server. After the connection is established, the communicate() function is called. In this function, all the interesting things take place.

The client application writes a request message to the socket and then calls the socket's shutdown() method, passing an asio::socket_base::shutdown_send constant as an argument. This call shuts down the send part of the socket. At this point, writing to the socket is disabled, and there is no way to restore the socket state to make it writable again:
```
sock.shutdown(asio::socket_base::shutdown_send);
```

Shutting down the socket in the client application is seen in the server application as a protocol service message that arrives to the server, notifying the fact that the peer application has shut down the socket. Boost.Asio delivers this message to the application code by means of an error code returned by the asio::read() function. The Boost.Asio library defines this code as asio::error::eof. The server application uses this error code to find out when the client finishes sending the request message.

When the server application receives a full request message, the server and client exchange their roles. Now, the server writes the data, namely, the response message to the socket on its side, and the client application reads this message on its side. When the server finishes writing the response message to the socket, it shuts down the send part of its socket to imply that the whole message has been sent to its peer.

Meanwhile, the client application is blocked in the asio::read() function and reads the response sent by the server until the function returns with the error code equal to asio::error::eof, which implies that the server has finished sending the response message. When the asio::read() function returns with this error code, the client knows that it has read the whole response message, and it can then start processing it:
```
system::error_code ec;
asio::read(sock, response_buf, ec);

if (ec == asio::error::eof) {
  // Whole response message has been received.
  // Here we can handle it.
}
```
Note that after the client has shut down its socket's send part, it can still read data from the socket because the receive part of the socket stays open independently from the send part.

# How to build
```
mkdir build
cd build
cmake ..
cmake --build .
```

# How to run
```
./bin/SockShutdownClient

./bin/SockShutdownServer
```
