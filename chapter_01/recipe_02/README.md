# Creating an active socket

Basically, there are two types of sockets. A socket intended to be used to send and receive data to and from a remote application or to initiate a connection establishment process with it is called an **active socket**, whereas a **passive socket** is the one used to passively wait for incoming connection requests from remote applications. Passive sockets don't take part in user data transmission.

The following algorithm describes the steps required to perform in a client application to create and open an active socket:

- Create an instance of the asio::io_service class or use the one that has been created earlier.
- Create an object of the class that represents the transport layer protocol (TCP or UDP) and the version of the underlying IP protocol (IPv4 or IPv6) over which the socket is intended to communicate.
- Create an object representing a socket corresponding to the required protocol type. Pass the object of asio::io_service class to the socket's constructor.
- Call the socket's open() method, passing the object representing the protocol created in step 2 as an argument.

# How it works
In step 1, we instantiate an object of the asio::io_service class. This class is a central component in the Boost.Asio I/O infrastructure. It provides access to the network I/O services of the underlying operating system. Boost.Asio sockets get access to those services through the object of this class. Therefore, all socket class constructors require an object of asio::io_service as an argument. We'll consider the asio::io_service class in more detail in the following chapters.

In the next step, we create an instance of the asio::ip::tcp class. This class represents a TCP protocol. It provides no functionality, but rather acts like a data structure that contains a set of values that describe the protocol.

The asio::ip::tcp class doesn't have a public constructor. Instead, it provides two static methods, asio::ip::tcp::v4() and asio::ip::tcp::v6(), that return an object of the asio::ip::tcp class representing the TCP protocol with the underlying IPv4 or IPv6 protocol correspondingly.

Besides, the asio::ip::tcp class contains declarations of some basic types intended to be used with the TCP protocol. Among them are asio::tcp::endpoint, asio::tcp::socket, asio::tcp::acceptor, and others. Let's have a look at those declarations found in the boost/asio/ip/tcp.hpp file:
```
namespace boost {
namespace asio {
namespace ip {

  // ...
  class tcp
  {
  public:
    /// The type of a TCP endpoint.
    typedef basic_endpoint<tcp> endpoint;
    
    // ...
  
    /// The TCP socket type.
    typedef basic_stream_socket<tcp> socket;

    /// The TCP acceptor type.
    typedef basic_socket_acceptor<tcp> acceptor;
    
    // ...
```

In step 3, we create an instance of the asio::ip::tcp::socket class, passing the object of the asio::io_service class to its constructor as an argument. Note that this constructor does not allocate the underlying operating system's socket object. The real operating system's socket is allocated in step 4 when we call the open() method and pass an object specifying protocol to it as an argument.

In Boost.Asio, opening a socket means associating it with full set of parameters describing a specific protocol over which the socket is intended to be communicating. When the Boost.Asio socket object is provided with these parameters, it has enough information to allocate a real socket object of the underlying operating system.

The asio::ip::tcp::socket class provides another constructor that accepts a protocol object as an argument. This constructor constructs a socket object and opens it. Note that this constructor throws an exception of the type boost::system::system_error if it fails. Here is a sample demonstrating how we could combine steps 3 and 4 from the previous sample:
```
try {
  // Step 3 + 4 in single call. May throw.
  asio::ip::tcp::socket sock(ios, protocol);
} catch (boost::system::system_error & e) {
  std::cout << "Error occured! Error code = " << e.code()
    << ". Message: "<< e.what();
}
```

The previous sample demonstrates how to create an active socket intended to communicate over the TCP protocol. The process of creating a socket intended for communication over the UDP protocol is almost identical.

The following sample demonstrates how to create an active UDP socket. It is assumed that the socket is going to be used to communicate over the UDP protocol with IPv6 as the underlying protocol. No explanation is provided with the sample because it is very similar to the previous one and therefore should not be difficult to understand:
```
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int main()
{
  // Step 1. An instance of 'io_service' class is required by
  // socket constructor. 
  asio::io_service ios;

  // Step 2. Creating an object of 'udp' class representing
  // a UDP protocol with IPv6 as underlying protocol.
  asio::ip::udp protocol = asio::ip::udp::v6();

  // Step 3. Instantiating an active UDP socket object.
  asio::ip::udp::socket sock(ios);

  // Used to store information about error that happens
  // while opening the socket.
  boost::system::error_code ec;

  // Step 4. Opening the socket.
  sock.open(protocol, ec);

  if (ec.value() != 0) {
    // Failed to open the socket.
    std::cout
      << "Failed to open the socket! Error code = "
      << ec.value() << ". Message: " << ec.message();
    return ec.value();
  }

  return 0;
}
```



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
