# Implementing a synchronous UDP client

A synchronous UDP client is a part of a distributed application that complies with the following statements:

- Acts as a client in the client-server communication model
- Communicates with the server application using UDP protocol
- Uses I/O and control operations (at least those I/O operations that are related to communication with the server) that block the thread of execution until the corresponding operation completes, or an error occurs
A typical synchronous UDP client works according to the following algorithm:

- Obtain an IP-address and a protocol port number of each server the client application is intended to communicate with.
- Allocate a UDP socket.
- Exchange messages with the servers.
- Deallocate the socket.

The sample consists of two main components—the SyncUDPClient class and the application entry point function main() that uses the SyncUDPClient class to communicate with two server applications.

## The SyncUDPClient class
The SyncUDPClient class is the key component in the sample. It implements the server communication functionality and provides access to it for the user.

The class has two private members as follows:

 - asio::io_service m_ios: This is the object providing access to the operating system's communication services, which are used by the socket object
 - asio::ip::udp::socket m_sock: This is the UDP socket used for communication
The socket object m_sock is instantiated and opened in the class's constructor. Because the client is intended to use IPv4 protocol, we pass the object returned by the asio::ip::udp::v4() static method to the socket's open() method to designate the socket to use IPv4 protocol.

Because the SyncUDPClient class implements communication over UDP protocol, which is a connectionless protocol, a single object of this class can be used to communicate with multiple servers. The interface of the class consists of a single method—emulateLongComputationOp(). This method can be used to communicate with the server just after the object of the SyncUDPClient class is instantiated. The following is the prototype of the method:
```
std::string emulateLongComputationOp(
         unsigned int duration_sec,
         const std::string& raw_ip_address,
         unsigned short port_num)
```
Besides the duration_sec argument that represents a request parameter, the method accepts the server IP-address and the protocol port number. This method may be called multiple times to communicate with different servers.

The method begins with preparing a request string according to the application layer protocol and creating an endpoint object designating the target server application. Then, the request string and the endpoint object are passed to the class's private method sendRequest(), which sends the request message to the specified server. When the request is sent and the sendRequest() method returns, the receiveResponse() method is called to receive a response from the server.

When the response is received, the receiveResponse() method returns the string containing the response. In turn, the emulateLongComputationOp() method returns the response to its caller. The sendRequest() method uses the socket object's send_to() method to send the request message to a particular server. Let's have a look at the declaration of this method:
```
 template <typename ConstBufferSequence>
  std::size_t send_to(const ConstBufferSequence& buffers,
      const endpoint_type& destination)
```
The method accepts a buffer containing the request and an endpoint designating the server to which the content of the buffer should be sent as arguments and blocks until the whole buffer is sent, or an error occurs. Note that, if the method returns without an error, it only means that the request has been sent and does not mean that the request has been received by the server. UDP protocol doesn't guarantee message delivery and it provides no means to check whether the datagram has been successfully received on the server-side or got lost somewhere on its way to the server.

Having sent the request, now we want to receive the response from the server. This is the purpose of the receiveResponse() method of the SyncUDPClient class. This method begins with allocating a buffer that will hold the response message. We choose the size of the buffer such that it can fit the largest message that the server may send according to the application layer protocol. This message is an ERROR\n string that consists of six ASCII symbols, which is therefore 6 bytes long; hence is the size of our buffer - 6 bytes. Because the buffer is small enough, we allocate it on the stack.

To read the response data arriving from the server, we use the socket object's receive_from() method. Here is the prototype of the method:

```
template <typename MutableBufferSequence>
  std::size_t receive_from(const MutableBufferSequence& buffers,
      endpoint_type& sender_endpoint)
```

This method copies a datagram that came from the server designated by the sender_endpoint object to the buffer specified by the buffers argument.

There are two things to note about socket object's receive_from() method. The first thing is that this method is synchronous and it blocks the thread of execution until the datagram arrives from the specified server. If the datagram never arrives (for example, gets lost somewhere on its way to the client), the method will never unblock and the whole application will hang. The second thing is that if the size of the datagram that arrives from the server is larger than the size of the supplied buffer, the method will fail.

After the response is received, the std::string object is created, initialized with a response string, and returned to the caller—the emulateLongComputationOp() method. This in turn returns the response to its caller—the main() function.

The SyncUDPClient class does not contain error handling-related code. That's is because it uses only those overloads of Boost.Asio functions and objects' methods that throw exceptions in case of failure. It is assumed that the user of the class is responsible for catching and handling the exceptions.

## The main() entry point function
In this function, we use the SyncUDPClient class in order to communicate with two server applications. Firstly, we obtain the IP-addresses and the port numbers of the target server applications. Then, we instantiate the object of the SyncUDPClient class and call the object's emulateLongComputationOp() method twice to synchronously consume the same service from two different servers.

# How to build
```
mkdir build
cd build
cmake ..
cmake --build .
```

# How to run
```
./bin/main
```
