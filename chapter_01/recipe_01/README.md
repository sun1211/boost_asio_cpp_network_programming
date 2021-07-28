# Creating an endpoint.

the endpoints serve two goals:

- The client application uses an endpoint to designate a particular server application it wants to communicate with.
- The server application uses an endpoint to specify a local IP address and a port number on which it wants to receive incoming messages from clients. If there is more than one IP address on the host, the server application will want to create a special endpoint representing all IP addresses at once.

Before creating the endpoint, the client application must obtain the raw IP address and the protocol port number designating the server it will communicate with. The server application on the other hand, as it usually listens for incoming messages on all IP addresses, only needs to obtain a port number on which to listen.

The following algorithms and corresponding code samples demonstrate two common scenarios of creating an endpoint.
- The first one demonstrates how the client application can create an endpoint to specify the server it wants to communicate with.
- The second one demonstrates how the server application creates an endpoint to specify on which IP addresses and port it wants to listen for incoming messages from clients.
## CREATING AN ENDPOINT IN THE CLIENT TO DESIGNATE THE SERVER
The following algorithm describes steps required to perform in the client application to create an endpoint designating a server application the client wants to communicate with. Initially, the IP address is represented as a string in the dot-decimal notation if this is an IPv4 address or in hexadecimal notation if this is an IPv6 address:

- Obtain the server application's IP address and port number. The IP address should be specified as a string in the dot-decimal (IPv4) or hexadecimal (IPv6) notation.
- Represent the raw IP address as an object of the asio::ip::address class.
- Instantiate the object of the asio::ip::tcp::endpoint class from the address object created in step 2 and a port number.
- The endpoint is ready to be used to designate the server application in Boost.Asio communication related methods.

## CREATING THE SERVER ENDPOINT
The following algorithm describes steps required to perform in a server application to create an endpoint specifying all IP addresses available on the host and a port number on which the server application wants to listen for incoming messages from the clients:

- Obtain the protocol port number on which the server will listen for incoming requests.
- Create a special instance of the asio::ip::address object representing all IP addresses available on the host running the server.
- Instantiate an object of the asio::ip::tcp::endpoint class from the address object created in step 2 and a port number.
- The endpoint is ready to be used to specify to the operating system that the server wants to listen for incoming messages on all IP addresses and a particular protocol port number.

## How it works
Let's consider the first code sample. The algorithm it implements is applicable in an application playing a role of a client that is an application that actively initiates the communication session with a server. The client application needs to be provided an IP address and a protocol port number of the server. Here we assume that those values have already been obtained and are available at the beginning of the algorithm, which makes step 1 details a given.

Having obtained the raw IP address, the client application must represent it in terms of the Boost.Asio type system. Boost.Asio provides three classes used to represent an IP address:

 - asio::ip::address_v4: This represents an IPv4 address
 - asio::ip::address_v6: This represents an IPv6 address
 - asio::ip::address: This IP-protocol-version-agnostic class can represent both IPv4 and IPv6 addresses

 In our sample, we use the asio::ip::address class, which makes the client application IP-protocol-version-agnostic. This means that it can transparently work with both IPv4 and IPv6 servers.

In step 2, we use the asio::ip::address class's static method, from_string(). This method accepts a raw IP address represented as a string, parses and validates the string, instantiates an object of the asio::ip::address class, and returns it to the caller. This method has four overloads. In our sample we use this one:
```
static asio::ip::address from_string(
    const std::string & str,
    boost::system::error_code & ec);
```

This method is very useful as it checks whether the string passed to it as an argument contains a valid IPv4 or IPv6 address and if it does, instantiates a corresponding object. If the address is invalid, the method will designate an error through the second argument. It means that this function can be used to validate the raw user input.

In step 3, we instantiate an object of the boost::asio::ip::tcp::endpoint class, passing the IP address and a protocol port number to its constructor. Now, the ep object can be used to designate a server application in the Boost.Asio communication related functions.

The second sample has a similar idea, although it somewhat differs from the first one. The server application is usually provided only with the protocol port number on which it should listen for incoming messages. The IP address is not provided because the server application usually wants to listen for the incoming messages on all IP addresses available on the host, not only on a specific one.

To represent the concept of all IP addresses available on the host, the classes asio::ip::address_v4 and asio::ip::address_v6 provide a static method any(), which instantiates a special object of corresponding class representing the concept. In step 2, we use the any() method of the asio::ip::address_v6 class to instantiate such a special object.

Note that the IP-protocol-version-agnostic class asio::ip::address does not provide the any() method. The server application must explicitly specify whether it wants to receive requests either on IPv4 or on IPv6 addresses by using the object returned by the any() method of either the asio::ip::address_v4 or asio::ip::address_v6 class correspondingly. In step 2 of our second sample, we assume that our server communicates over IPv6 protocol and therefore called the any() method of the asio::ip::address_v6 class.

In step 3, we create an endpoint object which represents all IP addresses available on the host and a particular protocol port number.

In both our previous samples we used the endpoint class declared in the scope of the asio::ip::tcp class. If we look at the declaration of the asio::ip::tcp class, we'll see something like this:
```
class tcp
{
public:
  /// The type of a TCP endpoint.
  typedef basic_endpoint<tcp> endpoint;

  //...
}
```
It means that this endpoint class is a specialization of the basic_endpoint<> template class that is intended for use in clients and servers communicating over the TCP protocol.

However, creating endpoints that can be used in clients and servers that communicate over the UDP protocol is just as easy. To represent such an endpoint, we need to use the endpoint class declared in the scope of the asio::ip::udp class. The following code snippet demonstrates how this endpoint class is declared:
```
class udp
{
public:
  /// The type of a UDP endpoint.
  typedef basic_endpoint<udp> endpoint;

  //...
}
```
For example, if we want to create an endpoint in our client application to designate a server with which we want to communicate over the UDP protocol, we would only slightly change the implementation of step 3 in our sample. This is how that step would look like with changes highlighted:
```
// Step 3.
asio::ip::udp::endpoint ep(ip_address, port_num);
```
All other code would not need to be changed as it is transport protocol independent.

The same trivial change in the implementation of step 3 in our second sample is required to switch from a server communicating over TCP to one communicating over UDP.

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
