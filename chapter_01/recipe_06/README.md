# Connecting a socket

Before a TCP socket can be used to communicate with a remote application, it must establish a logical connection with it. According to the TCP protocol, the connection establishment process lies in exchanging of service messages between two applications, which, if succeeds, results in two applications being logically connected and ready for communication with each other.

The client application, when it wants to communicate with the server application, creates and opens an active socket and issues a connect() command on it, specifying a target server application with an endpoint object. This leads to a connection establishment request message being sent to the server application over the network. The server application receives the request and creates an active socket on its side, marking it as connected to a specific client and replies back to the client with the message acknowledging that connection is successfully set up on the server side. Next, the client having received the acknowledgement from the server, marks its socket as connected to the server, and sends one more message to it acknowledging that the connection is successfully set up on the client side. When the server receives the acknowledgement message from the client, the logical connection between two applications is considered established.

The point-to-point communication model is assumed between two connected sockets. This means that if socket A is connected to socket B, both can only communicate with each other and cannot communicate with any other socket C. Before socket A can communicate with socket C, it must close the connection with socket B and establish a new connection with socket C.

The following algorithm descries steps required to perform in the TCP client application to connect an active socket to the server application:

- Obtain the target server application's IP address and a protocol port number.
- Create an object of the asio::ip::tcp::endpoint class from the IP address and the protocol port number obtained in step 1.
- Create and open an active socket.
- Call the socket's connect() method specifying the endpoint object created in step 2 as an argument.
- If the method succeeds, the socket is considered connected and can be used to send and receive data to and from the server.

# How it works

In step 1, we begin with obtaining the target server's IP address and a protocol port number. The process of obtaining these parameters is beyond the scope of this recipe; therefore, here we assume that they have already been obtained and are available at the beginning of our sample.

In step 2, we create an object of the asio::ip::tcp::endpoint class designating the target server application to which we are going to connect.

Then, in step 3 an active socket is instantiated and opened.

In step 4, we call the socket's connect() method, passing an endpoint object designating the target server to it as an argument. This function connects the socket to the server. The connection is performed synchronously, which means that the method blocks the caller thread until either the connection operation is established or an error occurs.

Note that we didn't bind the socket to any local endpoint before connecting it. This doesn't mean that the socket stays unbound. Before performing the connection establishment procedure, the socket's connect() method will bind the socket to the endpoint consisting of an IP address and a protocol port number chosen by the operating system.

Another thing to note in this sample is that we use an overload of the connect() method that throws an exception of the boost::system::system_error type if the operation fails, and so does overload of the asio::ip::address::from_string() static method we use in step 2. Therefore, both calls are enclosed in a try block. Both methods have overloads that don't throw exceptions and accept an object of the boost::system::error_code class, which is used to conduct error information to the caller in case the operation fails. However, in this case, using exceptions to handle errors makes code better structured.

The previous code sample showed how to connect a socket to a specific server application designated by an endpoint when an IP address and a protocol port number are provided to the client application explicitly. However, sometimes the client application is provided with a DNS name that may be mapped to one or more IP addresses. In this case, we first need to resolve the DNS name using the resolve() method provided by the asio::ip::tcp::resolver class. This method resolves a DNS name, creates an object of the asio::ip::tcp::endpoint class from each IP address resulted from resolution, puts all endpoint objects in a collection, and returns an object of the asio::ip::tcp::resolver::iterator class, which is an iterator pointing to the first element in the collection.

When a DNS name resolves to multiple IP addresses, the client application—when deciding to which one to connect—usually has no reasons to prefer one IP address to any other. The common approach in this situation is to iterate through endpoints in the collection and try to connect to each of them one by one until the connection succeeds. Boost.Asio provides auxiliary functionality that implements this approach.

The free function asio::connect() accepts an active socket object and an object of the asio::ip::tcp::resolver::iterator class as input arguments, iterates over a collection of endpoints, and tries to connect the socket to each endpoint. The function stops iteration, and returns when it either successfully connects a socket to one of the endpoints or when it has tried all the endpoints and failed to connect the socket to all of them.

The following algorithm demonstrates steps required to connect a socket to a server application represented by a DNS name and a protocol port number:
 - Obtain the DNS name of a host running the server application and the server's port number and represent them as strings.
 - Resolve a DNS name using the asio::ip::tcp::resolver class.
 - Create an active socket without opening it.
 - Call the asio::connect() function passing a socket object and an iterator object obtained in step 2 to it as arguments.
```
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int main()
{
  // Step1. Assume that the client application has already
  // obtained the DNS name and protocol port number and
  // represented them as strings.
  std::string host = "samplehost.book";
  std::string port_num = "3333";

  // Used by a 'resolver' and a 'socket'.
  asio::io_service ios;

  // Creating a resolver's query.
  asio::ip::tcp::resolver::query resolver_query(host, port_num,
    asio::ip::tcp::resolver::query::numeric_service);

  // Creating a resolver.
  asio::ip::tcp::resolver resolver(ios);

  try {
    // Step 2. Resolving a DNS name.
    asio::ip::tcp::resolver::iterator it =
      resolver.resolve(resolver_query);

    // Step 3. Creating a socket.
    asio::ip::tcp::socket sock(ios);

    // Step 4. asio::connect() method iterates over
    // each endpoint until successfully connects to one
    // of them. It will throw an exception if it fails
    // to connect to all the endpoints or if other
    // error occurs.
    asio::connect(sock, it);
    
    // At this point socket 'sock' is connected to 
    // the server application and can be used
    // to send data to or receive data from it.
  }
  // Overloads of asio::ip::tcp::resolver::resolve and 
  // asio::connect() used here throw
  // exceptions in case of error condition.
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
}
```

Note that in step 3, we don't open the socket when we create it. This is because we don't know the version of IP addresses to which the provided DNS name will resolve. The asio::connect() function opens the socket before connecting it to each endpoint specifying proper protocol object and closes it if the connection fails.

All other steps in the code sample should not be difficult to understand, therefore no explanation is provided.


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
