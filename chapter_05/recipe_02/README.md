# Implementing the HTTP server application

Nowadays, there are plenty of HTTP server applications available in the market. However, sometimes there is a need to implement a custom one. This could be a small and simple server, supporting a specific subset of HTTP protocol possibly with custom extensions, or maybe not an HTTP server but a server supporting a communication protocol, which is similar to HTTP or is based on it.

In this recipe, we will consider the implementation of basic HTTP server application using Boost.Asio. Here is the set of requirements that our application must satisfy:

- It should support the HTTP 1.1 protocol
- It should support the GET method
- It should be able to process multiple requests in parallel, that is, it should be an asynchronous parallel server

In fact, we have already considered the implementation of the server application that partially fulfils specified requirements. In Chapter 4, Implementing Server Applications, the recipe named Implementing an asynchronous TCP server demonstrates how to implement an asynchronous parallel TCP server, which communicates with clients according to a dummy application layer protocol. All the communication functionality and protocol details are encapsulated in a single class named Service. All other classes and functions defined in that recipe are infrastructural in their purpose and isolated from the protocol details. Therefore, the current recipe will be based on the one from Chapter 4, Implementing Server Applications, and here we will only consider the implementation of the Service class as all other components stay the same.

Note that, in this recipe, we do not consider the security aspect of the application. Make sure the server is protected before making it available to the public, where though operating correctly and in accordance with HTTP protocol, it could be compromised by the culprits due to security breaches.

We begin our application by including header files containing declarations and definitions of data types and functions that we will use:
```
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <fstream>
#include <atomic>
#include <thread>
#include <iostream>

using namespace boost;
```

Next, we start defining the Service class that provides the implementation of the HTTP protocol. Firstly, we declare a static constant table containing HTTP status codes and status messages. The definition of the table will be given after the Service class' definition:
```
class Service {
  static const std::map<unsigned int, std::string>
http_status_table;
```

The class' constructor accepts a single parameter—shared pointer pointing to an instance of a socket connected to a client. Here's the definition of the constructor:

```
public:
  Service(std::shared_ptr<boost::asio::ip::tcp::socket> sock) :
    m_sock(sock),
    m_request(4096),
    m_response_status_code(200), // Assume success.
    m_resource_size_bytes(0)
  {};
```
Next, we define a single method constituting the Service class' public interface. This method initiates an asynchronous communication session with the client connected to the socket, pointer to which was passed to the Service class' constructor:
```
void start_handling() {
    asio::async_read_until(*m_sock.get(),
      m_request,
      "\r\n",
      [this](
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
    {
      on_request_line_received(ec,
        bytes_transferred);
    });
  }
```
Then, we define a set of private methods that perform receiving and processing of the request sent by the client, parse and execute the request, and send the response back. Firstly, we define a method that processes the HTTP request line:
```
private:
  void on_request_line_received(
    const boost::system::error_code& ec,
    std::size_t bytes_transferred) 
{
    if (ec != 0) {
      std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();

      if (ec == asio::error::not_found) {
        // No delimiter has been found in the
        // request message.

        m_response_status_code = 413;
        send_response();

        return;
      }
      else {
        // In case of any other error –
        // close the socket and clean up.
        on_finish();
        return;
      }
    }

    // Parse the request line.
    std::string request_line;
    std::istream request_stream(&m_request);
    std::getline(request_stream, request_line, '\r');
    // Remove symbol '\n' from the buffer.
    request_stream.get();

    // Parse the request line.
    std::string request_method;
    std::istringstream request_line_stream(request_line);
    request_line_stream >> request_method;

    // We only support GET method.
    if (request_method.compare("GET") != 0) {
      // Unsupported method.
      m_response_status_code = 501;
      send_response();

      return;
    }

    request_line_stream >> m_requested_resource;

    std::string request_http_version;
    request_line_stream >> request_http_version;

    if (request_http_version.compare("HTTP/1.1") != 0) {
      // Unsupported HTTP version or bad request.
      m_response_status_code = 505;
      send_response();

      return;
    }

    // At this point the request line is successfully
    // received and parsed. Now read the request headers.
    asio::async_read_until(*m_sock.get(),
      m_request,
      "\r\n\r\n",
      [this](
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
    {
      on_headers_received(ec,
        bytes_transferred);
    });

    return;
  }
```

Next, we define a method intended to process and store the request headers block, containing the request headers:
```
void on_headers_received(const boost::system::error_code& ec,
    std::size_t bytes_transferred)  
  {
    if (ec != 0) {
      std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();

      if (ec == asio::error::not_found) {
        // No delimiter has been fonud in the
        // request message.

        m_response_status_code = 413;
        send_response();
        return;
      }
      else {
        // In case of any other error - close the
        // socket and clean up.
        on_finish();
        return;
      }
    }

    // Parse and store headers.
    std::istream request_stream(&m_request);
    std::string header_name, header_value;

    while (!request_stream.eof()) {
      std::getline(request_stream, header_name, ':');
      if (!request_stream.eof()) {
        std::getline(request_stream, 
        header_value, 
      '\r');

        // Remove symbol \n from the stream.
        request_stream.get();
        m_request_headers[header_name] =
        header_value;
      }
    }

    // Now we have all we need to process the request.
    process_request();
    send_response();

    return;
  }
```
Besides, we need a method that can perform the actions needed to fulfill the request sent by the client. We define the process_request() method, whose purpose is to read the contents of the requested resource from the file system and store it in the buffer, ready to be sent back to the client:
```
void process_request() {
    // Read file.
    std::string resource_file_path =
    std::string("D:\\http_root") +
    m_requested_resource;

    if (!boost::filesystem::exists(resource_file_path)) {
      // Resource not found.
      m_response_status_code = 404;

      return;
    }

    std::ifstream resource_fstream(
    resource_file_path, 
    std::ifstream::binary);

    if (!resource_fstream.is_open()) {
      // Could not open file. 
      // Something bad has happened.
      m_response_status_code = 500;

      return;
    }

    // Find out file size.
    resource_fstream.seekg(0, std::ifstream::end);
    m_resource_size_bytes =
    static_cast<std::size_t>(
    resource_fstream.tellg());

    m_resource_buffer.reset(
    new char[m_resource_size_bytes]);

    resource_fstream.seekg(std::ifstream::beg);
    resource_fstream.read(m_resource_buffer.get(),
    m_resource_size_bytes);

    m_response_headers += std::string("content-length") +
      ": " +
      std::to_string(m_resource_size_bytes) +
      "\r\n";
  }
```
Finally, we define a method that composes a response message and send it to the client:
```
 void send_response()  {
    m_sock->shutdown(
    asio::ip::tcp::socket::shutdown_receive);

    auto status_line =
      http_status_table.at(m_response_status_code);

    m_response_status_line = std::string("HTTP/1.1 ") +
      status_line +
      "\r\n";

    m_response_headers += "\r\n";

    std::vector<asio::const_buffer> response_buffers;
    response_buffers.push_back(
    asio::buffer(m_response_status_line));
    
    if (m_response_headers.length() > 0) {
      response_buffers.push_back(
      asio::buffer(m_response_headers));
    }

    if (m_resource_size_bytes > 0) {
      response_buffers.push_back(
      asio::buffer(m_resource_buffer.get(),
      m_resource_size_bytes));
    }

    // Initiate asynchronous write operation.
    asio::async_write(*m_sock.get(),
      response_buffers,
      [this](
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
    {
      on_response_sent(ec,
        bytes_transferred);
    });
  }
```

When the response sending is complete, we need to shut down the socket to let the client know that a full response has been sent and no more data will be sent by the server. We define the on_response_sent() method for this purpose:
```
  void on_response_sent(const boost::system::error_code& ec,
    std::size_t bytes_transferred) 
{
    if (ec != 0) {
      std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();
    }

    m_sock->shutdown(asio::ip::tcp::socket::shutdown_both);

    on_finish();
  }
```

The last method we need to define is the one that performs cleanup and deletes an instance of the Service object, when the communication session is finished and the object is not needed anymore is not needed anymore:
```
 // Here we perform the cleanup.
  void on_finish() {
    delete this;
  }
```

Of course, we will need some data members in our class. We declare the following data members:
```
private:
  std::shared_ptr<boost::asio::ip::tcp::socket> m_sock;
  boost::asio::streambuf m_request;
  std::map<std::string, std::string> m_request_headers;
  std::string m_requested_resource;

  std::unique_ptr<char[]> m_resource_buffer;  
  unsigned int m_response_status_code;
  std::size_t m_resource_size_bytes;
  std::string m_response_headers;
  std::string m_response_status_line;
};
```

The last thing we need to do to complete the definition of the class representing a service is to define the http_status_table static member declared before and fill it with data—HTTP status code and corresponding status messages:
```
const std::map<unsigned int, std::string>
  Service::http_status_table = 
{
  { 200, "200 OK" },
  { 404, "404 Not Found" },
  { 413, "413 Request Entity Too Large" },
  { 500, "500 Server Error" },
  { 501, "501 Not Implemented" },
  { 505, "505 HTTP Version Not Supported" }
};
```

Our Service class is now ready.

# How it works
Let's begin with considering the Service class' data members and then switch to its functionality. The Service class contains the following non-static data members:
- std::shared_ptr<boost::asio::ip::tcp::socket> m_sock: This is a shared pointer to a TCP socket object connected to the client
- boost::asio::streambuf m_request: This is a buffer into which the request message is read
- std::map<std::string, std::string> m_request_headers: This is a map where request headers are put when the HTTP request headers block is parsed
- std::string m_requested_resource: This is the URI of the resource requested by the client
- std::unique_ptr<char[]> m_resource_buffer: This is a buffer where the contents of a requested resource is stored before being sent to the client as a part of the response message
- unsigned int m_response_status_code: This is the HTTP response status code
- std::size_t m_resource_size_bytes: This is the size of the contents of the requested resource
- std::string m_response_headers: This is a string containing a properly formatted response headers block
- std::string m_response_status_line: This contains a response status line

Now that we know the purpose of the Service class' data members, let's trace how it works. Here, we will only consider how the Service class works. The description of all other components of the server application and how they work is given in the recipe named Implementing an asynchronous TCP server in Chapter 4, Implementing Server Applications.

When a client sends a TCP connection request and this request is accepted on the server (this happens in the Acceptor class, which is not considered in this recipe), an instance of the Service class is created and its constructor is passed a shared pointer pointing to the TCP socket object, connected to that client. The pointer to the socket is stored in the Service object's data member m_sock.

Besides, during the construction of the Service object, the m_request stream buffer member is initialized with the value of 4096, which sets the maximum size of the buffer in bytes. Limiting the size of the request buffer is a security measure, which helps to protect the server from malicious clients that may try to send very long dummy request messages exhausting all memory at the disposal of the server application. For the correct request, a buffer of 4096 bytes in size is more than enough.

After an instance of the Service class has been constructed, its start_handling() method is called by the Acceptor class. From this method, the sequence of asynchronous method invocations begins, which performs request receiving, processing, and response sending. The start_handling() method immediately initiates an asynchronous reading operation calling the asio::async_read_until() function in order to receive the HTTP request line sent by the client. The on_request_line_received() method is specified as a callback.

When the on_request_line_received() method is invoked, we first check the error code specifying the operation completion status. If the status code is not equal to zero, we consider two options. The first option—when the error code is equal to the asio::error::not_found value—means that more bytes have been received from the client than the size of the buffer and the delimiter of the HTTP request line (the \r\n symbol sequence) has not been encountered. This case is described by the HTTP status code 413. We set the value of the m_response_status_code member variable to 413 and call the send_response() method that initiates the operation that sends a response designating the error back to the client. We will consider the send_response() method later in this section. At this point, the request processing is finished.

If the error code neither designates success nor is equal to asio::error::not_found, it means that some other error has occurred from which we cannot recover, therefore, we just output the information about the error and do not reply to the client at all. The on_finish() method is called to perform the cleanup, and the communication with the client is interrupted.

Finally, if receiving of the HTTP request line succeeds, it is parsed to extract the HTTP request method, the URI identifying the requested resource and the HTTP protocol version. Because our sample server only supports the GET method, if the method specified in the request line is different from GET, further request processing is interrupted and the response containing the error code 501 is sent to the client to inform it that the method specified in the request is not supported by the server.

Likewise, the HTTP protocol version specified by the client in the HTTP request line is checked to be the one supported by the server. Because our server application supports only version 1.1, if the version specified by the client is different, the response with the HTTP status code 505 is sent to the client and the request processing is interrupted.

A URI string extracted from the HTTP request line is stored in the m_requested_resource data member and will be used later.

When the HTTP request line is received and parsed, we continue reading the request message in order to read the request headers block. To do this, the asio::async_read_until() function is called. Because the request headers block ends with the \r\n\r\n symbol sequence, this symbol sequence is passed to the function as a delimiter argument. The on_headers_received() method is specified as an operation completion callback.

The on_headers_received() method performs error checking similar to the one that is performed in the on_request_line_received() method. In case of an error, request processing interrupts. In the case of success, the HTTP request headers block is parsed and broken into separate name-value pairs, which are then stored in the m_request_headers member map. After the headers block has been parsed, the process_request() and send_response() methods are called consequently.

The purpose of the process_request() method is to read the file specified in the request as the URI and put its content to the buffer, from which the contents will be sent to the client as a part of the response message. If the specified file is not found in the server root directory, the HTTP status code 404 (page not found) code is sent to the client as a part of the response message and the request processing interrupts.

However, if the requested file is found, its size is first calculated and then the buffer of the corresponding size is allocated in the free memory and the file contents are read in that buffer.

After this, an HTTP header named content-length specifying the size of the response body is added to the m_response_headers string data member. This data member represents the response headers block and its value will later be used as a part of the response message.

At this point, all ingredients required to construct the HTTP response message are available and we can move on to preparing and sending the response to the client. This is done in the send_response() method.

The send_response() method starts with shutting down the receive side of the socket letting the client know that the server will not read any data from it anymore. Then, it extracts the response status message corresponding to the status code stored in the m_response_status_code member variable from the http_status_table static table.

Next, the HTTP response status line is constructed and the headers block is appended with the delimiting symbol sequence \r\n according to the HTTP protocol. At this point, all the components of the response message—the response status line, response headers block, and response body—are ready to be sent to the client. The components are combined in the form of a vector of buffers, each represented with an instance of the asio::const_buffer class and containing one component of the response message. A vector of buffers embodies a composite buffer consisting of three parts. When this composite buffer is constructed, it is passed to the asio::async_write() function to be sent to the client. The Service class' on_response_sent() method is specified as a callback.

When the response message is sent and the on_response_sent() callback method is invoked, it first checks the error code and outputs the log message if the operation fails; then, it shuts down the socket and calls the on_finish() method. The on_finish() method in its turn deletes the instance of the Service object in the context of which it is called.

At this point, client handling is finished.

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
