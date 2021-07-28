# Resolving a DNS name

To enable labeling the devices in a network with human-friendly names, the Domain Name System (DNS) was introduced. In short, DNS is a distributed naming system that allows associating human-friendly names with devices in a computer network. A DNS name or a domain name is a string that represents a name of a device in the computer network.

The following algorithm describes steps required to perform in a client application in order to resolve a DNS name to obtain IP addresses (zero or more) of hosts (zero or more) running the server application that the client application wants to communicate with:

 - Obtain the DNS name and the protocol port number designating the server application and represent them as strings.
 - Create an instance of the asio::io_service class or use the one that has been created earlier.
 - Create an object of the resolver::query class representing a DNS name resolution query.
 - Create an instance of DNS name resolver class suitable for the necessary protocol.
 - Call the resolver's resolve() method, passing a query object created in step 3 to it as an argument.

# How it works
In step 1, we begin by obtaining a DNS name and a protocol port number and representing them as strings. Usually, these parameters are supplied by a user through the client application's UI or as command-line arguments. The process of obtaining and validating these parameters is behind the scope of this recipe; therefore, here we assume that they are available at the beginning of the sample.

Then, in step 2, we create an instance of the asio::io_service class that is used by the resolver to access underlying OS's services during a DNS name resolution process.

In step 3 we create an object of the asio::ip::tcp::resolver::query class. This object represents a query to the DNS. It contains a DNS name to resolve, a port number that will be used to construct an endpoint object after the DNS name resolution and a set of flags controlling some specific aspects of resolution process, represented as a bitmap. All these values are passed to the query class's constructor. Because the service is specified as a protocol port number (in our case, 8080) and not as a service name (for example, HTTP, FTP, and so on), we passed the asio::ip::tcp::resolver::query::numeric_service flag to explicitly inform the query object about that, so that it properly parses the port number value.

In step 4, we create an instance of the asio::ip::tcp::resolver class. This class provides the DNS name resolution functionality. To perform the resolution, it requires services of the underlying operating system and it gets access to them through the object of the asio::io_services class being passed to its constructor as an argument.

The DNS name resolution is performed in step 5 in the resolver object's resolve() method. The method overload we use in our sample accepts objects of the asio::ip::tcp::resolver::query and system::error_code classes. The latter object will contain information describing the error if the method fails.

If successful, the method returns an object of the asio::ip::tcp::resolver::iterator class, which is an iterator pointing to the first element of a collection representing resolution results. The collection contains objects of the asio::ip::basic_resolver_entry<tcp> class. There are as many objects in the collection as the total number of IP addresses that resolution yielded. Each collection element contains an object of the asio::ip::tcp::endpoint class instantiated from one IP address resulting from the resolution process and a port number provided with the corresponding query object. The endpoint object can be accessed through the asio::ip::basic_resolver_entry<tcp>::endopoint() getter method.

The default-constructed object of the asio::ip::tcp::resolver::iterator class represents an end iterator. Consider the following sample demonstrating how we can iterate through the elements of the collection representing the DNS name resolution process results and how to access the resulting endpoint objects:
```
sio::ip::tcp::resolver::iterator it = 
    resolver.resolve(resolver_query, ec);

asio::ip::tcp::resolver::iterator it_end;

for (; it != it_end; ++it) {
  // Here we can access the endpoint like this.
  asio::ip::tcp::endpoint ep = it->endpoint();
}
```
Usually, when a DNS name of the host running the server application is resolved to more than one IP address and correspondingly to more than one endpoint, the client application doesn't know which one of the multiple endpoints to prefer. The common approach in this case is to try to communicate with each endpoint one by one, until the desired response is received.

Note that when the DNS name is mapped to more than one IP address and some of them are IPv4 and others are IPv6 addresses, the DNS name may be resolved either to the IPv4 address or to the IPv6 address or to both. Therefore, the resulting collection may contain endpoints representing both IPv4 and IPv6 addresses.

To resolve a DNS name and obtain a collection of endpoints that can be used in the client that is intended to communicate over the UDP protocol, the code is very similar. The sample is given here with differences highlighted and without explanation:
```
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int main()
{
  // Step 1. Assume that the client application has already
// obtained the DNS name and protocol port number and 
// represented them as strings.
std::string host = "samplehost.book";
  std::string port_num = "3333";

  // Step 2.
  asio::io_service ios;

  // Step 3. Creating a query.
  asio::ip::udp::resolver::query resolver_query(host,
port_num, asio::ip::udp::resolver::query::numeric_service);

  // Step 4. Creating a resolver.
  asio::ip::udp::resolver resolver(ios);

  // Used to store information about error that happens
  // during the resolution process.
  boost::system::error_code ec;

  // Step 5.
  asio::ip::udp::resolver::iterator it =
    resolver.resolve(resolver_query, ec);

  // Handling errors if any.
  if (ec != 0) {
    // Failed to resolve the DNS name. Breaking execution.
    std::cout << "Failed to resolve a DNS name."
<< "Error code = " << ec.value() 
<< ". Message = " << ec.message();

    return ec.value();
  }

asio::ip::udp::resolver::iterator it_end;

for (; it != it_end; ++it) {
    // Here we can access the endpoint like this.
    asio::ip::udp::endpoint ep = it->endpoint();
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
