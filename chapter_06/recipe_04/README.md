# Performing a stream-based I/O

The concepts of a stream and stream-based I/O are powerful in their expressiveness and elegance when used properly. Sometimes, most of the application's source code consists of stream-based I/O operations. The source code readability and maintainability of such an application would be increased if network communication modules were implemented by means of stream-based operations as well.

The Boost.Asio library contains the asio::ip::tcp::iostream wrapper class that provides an I/O stream-like interface to the TCP socket objects, which allows us to express inter-process communication operations in terms of stream-based operations.

Let's consider a TCP client application, which takes advantage of a stream-based I/O provided by Boost.Asio. When using this approach, the TCP client application becomes as simple as the following code:`
```
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int main() 
{
  asio::ip::tcp::iostream stream("localhost", "3333");
  if (!stream) {
    std::cout << "Error occurred! Error code = " 
      << stream.error().value()
      << ". Message = " << stream.error().message()
      << std::endl;

    return -1;
  }
  
  stream << "Request.";
  stream.flush();

  std::cout << "Response: " << stream.rdbuf();

  return 0;
} 
```

# How it works
The sample TCP client is quite simple and consists of a single component: the main() entry point function. The main() function begins with creating an instance of the asio::ip::tcp::iostream class, which wraps a TCP socket and provides an I/O stream-like interface to it.

The stream object is constructed with a constructor that accepts a server DNS name and a protocol port number and automatically tries to resolve the DNS name and then tries to connect to that server. Note that the port number is represented as a string rather than an integer. This is because both arguments passed to this constructor are directly used to create the resolver query, which requires the port number to be represented as a string (it should be expressed as a service name such as http, ftp, and so on or a port number that is represented as a string such as "80", "8081", "3333", and so on).

Alternatively, we can construct the stream object using the default constructor, which does not perform the DNS name resolution and connection. Then, when the object is constructed, we can call the connect() method on it by specifying the DNS name and protocol port number in order to perform the resolution and connect the socket.

Next, the state of the stream object is tested to find out whether the connection has succeeded. And if the stream object is in a bad or erroneous state, the appropriate message is output to the standard output stream and the application exits. The error() method of the asio::ip::tcp::iostream class returns an instance of the boost::system::error_code class, which provides the information about the last error that occurred in the stream.

However, if the stream has been successfully connected to the server, the output operation is performed on it, which sends the string Request, to the server. After this, the flush() method is called on the stream object to make sure that all the buffered data is pushed to the server.

In the last step, the input operation is performed on the stream to read all the data that was received from the server as a response. The received message is output to the standard output stream. After this, the main() function returns and the application exits.

Not only can we implement the client-side I/O in a stream-oriented fashion using the asio::ip::tcp::iostream class, we can also perform I/O operations on the server side as well. In addition to this, this class allows us to specify timeouts for operations, which makes a stream-based I/O more advantageous than a normal synchronous I/O. Let's take a look at how this is done.

## Implementing a server-side I/O
The following code snippet demonstrates how to implement a simple server that performs a stream-based I/O using the asio::ip::tcp::iostream class:
```
 // ... 
  asio::io_service io_service;

  asio::ip::tcp::acceptor acceptor(io_service,
    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333));
   
  asio::ip::tcp::iostream stream;

acceptor.accept(*stream.rdbuf());
std::cout << "Request: " << stream.rdbuf();
stream << "Response.";
// ...
```

This code snippet demonstrates a piece of source code of a simple server application. It creates instances of acceptors and the asio::ip::tcp::iostream classes. And then, the interesting thing happens.

The accept() method is invoked on the acceptor object. As an argument, this method is passed an object, a pointer to which is returned by the rdbuf() method called on the stream object. The rdbuf() method of the stream object returns a pointer to the stream buffer object. This stream buffer object is an instance of a class, which is inherited from the asio::ip::tcp::socket class, which means that the stream buffer used by objects of the asio::ip::tcp ::iostream class plays two roles: one of a stream buffer and another of a socket. Therefore, this twofold stream buffer/socket object can be used as a normal active socket to connect and communicate with the client application.

When the connection request is accepted and the connection is established, further communication with the client is done in a stream-fashioned style just like it is done in the client application, as demonstrated earlier in this recipe.

## Setting timeout intervals
Because I/O operations are provided by the asio::ip::tcp::stream class block the thread of execution, and they potentially may run for a substantial amount of time, the class provides a way to set a timeout period that, when it runs out, leads to the interruption of the operation that currently blocks the thread, if any.

The timeout interval can be set by the expires_from_now() method of the asio::ip::tcp::stream class. This method accepts the duration of the timeout interval as an input parameter and starts the internal timer. If at the moment, when the timer expires, an I/O operation is still in progress, that operation is considered timed out and is, therefore, forcefully interrupted.

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
