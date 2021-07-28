# Binding a socket to an endpoint

Before an active socket can communicate with a remote application or a passive socket can accept incoming connection requests, they must be associated with a particular local IP address (or multiple addresses) and a protocol port number, that is, an endpoint. The process of associating a socket with a particular endpoint is called binding. When a socket is bound to an endpoint, all network packets coming into the host from the network with that endpoint as their target address will be redirected to that particular socket by the operating system. Likewise, all the data coming out from a socket bound to a particular endpoint will be output from the host to the network through a network interface associated with the corresponding IP address specified in that endpoint.

The following algorithm describes steps required to create an acceptor socket and to bind it to an endpoint designating all IP addresses available on the host and a particular protocol port number in the IPv4 TCP server application:

- Obtain the protocol port number on which the server should listen for incoming connection requests.
- Create an endpoint that represents all IP addresses available on the host and the protocol port number obtained in the step 1.
- Create and open an acceptor socket.
- Call the acceptor socket's bind() method, passing the endpoint object as an argument to it.

# How it works

We begin by obtaining a protocol port number in step 1. The process of obtaining this parameter is beyond the scope of this recipe; therefore, here we assume that the port number has already been obtained and is available at the beginning of the sample.

In step 2 we create an endpoint representing all IP addresses available on the host and the specified port number.

In step 3 we instantiate and open the acceptor socket. The endpoint we created in step 2 contains information about the transport protocol and the version of the underlying IP protocol (IPv4). Therefore, we don't need to create another object representing the protocol to pass it to the acceptor socket's constructor. Instead, we use the endpoint's protocol() method, which returns an object of the asio::ip::tcp class representing the corresponding protocols.

The binding is performed in step 4. This is quite a simple operation. We call the acceptor socket's bind() method, passing an object representing an endpoint to which the acceptor socket should be bound as an argument of the method. If the call succeeds, the acceptor socket is bound to the corresponding endpoint and is ready to start listening for incoming connection requests on that endpoint.

UDP servers don't establish connections and use active sockets to wait for incoming requests. The process of binding an active socket is very similar to binding an acceptor socket. Here, we present a sample code demonstrating how to bind a UDP active socket to an endpoint designating all IP addresses available on the host and a particular protocol port number. The code is provided without explanation:
```
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int main()
{
  // Step 1. Here we assume that the server application has
  // already obtained the protocol port number.
  unsigned short port_num = 3333;

  // Step 2. Creating an endpoint.
  asio::ip::udp::endpoint ep(asio::ip::address_v4::any(),
    port_num);

  // Used by 'socket' class constructor.
  asio::io_service ios;

  // Step 3. Creating and opening a socket.
  asio::ip::udp::socket sock(ios, ep.protocol());

  boost::system::error_code ec;

  // Step 4. Binding the socket to an endpoint.
  sock.bind(ep, ec);

  // Handling errors if any.
  if (ec != 0) {
    // Failed to bind the socket. Breaking execution.
    std::cout << "Failed to bind the socket."
      << "Error code = " << ec.value() << ". Message: "
      << ec.message();

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
