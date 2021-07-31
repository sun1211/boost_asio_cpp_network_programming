# Implementing the HTTP client application

HTTP clients constitute important class of distributed software and are represented by many applications. Web browsers are prominent representatives of this class. They use HTTP protocols to request web pages from web servers. However, today HTTP protocol is used not only in the web. Many distributed applications use this protocol to exchange custom data of any kind. Often, when designing a distributed application, choosing HTTP as a communication protocol is a much better idea than developing custom one.

In this recipe, we will consider an implementation of HTTP client using Boost.Asio that satisfies the following basic requirements:

- Supports the HTTP GET request method
- Executes requests asynchronously
- Supports request canceling

Because one of the requirements of our client application is to support canceling requests that have been initiated but have not been completed yet, we need to make sure that canceling is enabled on all target platforms. Therefore, we begin our client application by configuring Boost.Asio library so that request canceling is enabled. More details on issues related to asynchronous operation canceling are provided in the Cancelling asynchronous operations recipe in Chapter 2, I/O Operations:
```
#include <boost/predef.h> // Tools to identify the OS.

// We need this to enable cancelling of I/O operations on
// Windows XP, Windows Server 2003 and earlier.
// Refer to "http://www.boost.org/doc/libs/1_58_0/
// doc/html/boost_asio/reference/basic_stream_socket/
// cancel/overload1.html" for details.
#ifdef BOOST_OS_WINDOWS
#define _WIN32_WINNT 0x0501

#if _WIN32_WINNT <= 0x0502 // Windows Server 2003 or earlier.
#define BOOST_ASIO_DISABLE_IOCP
#define BOOST_ASIO_ENABLE_CANCELIO  
#endif
#endif
```
Next, we include Boost.Asio library headers and also headers of some components of standard C++ libraries that we will need to implement our application:
```
#include <boost/asio.hpp>

#include <thread>
#include <mutex>
#include <memory>
#include <iostream>

using namespace boost;
```

Now, before we can jump to implementing classes and functions constituting our client application, we have to make one more preparation related to error representation and handling.

When implementing the HTTP client application, we need to deal with three classes of errors:
 - The first class is represented by numerous errors that may occur when executing Boost.Asio functions and classes' methods. For example, if we call the write_some() method on an object representing a socket that has not been opened, the method will return operating system dependent error code (either by throwing an exception or by the means of an out argument depending on the method overload used), designating the fact that an invalid operation has been executed on a non-opened socket.
 - The second class includes both erroneous and non-erroneous statuses defined by HTTP protocol. For example, the status code 200 returned by the server as a response to particular request made by the client, designates the fact that a client's request has been fulfilled successfully. On the other hand, the status code 500 designates that while performing the requested operation, an error occurred on the server that led to the request not being fulfilled.
 - The third class includes errors related to the HTTP protocol itself. In case a server sends a message, as a response to correct the request made by a client and this message is not a properly structured HTTP response, the client application should have means to represent this fact in terms of error code.
Error code for the first class of errors are defined in the sources of Boost.Asio libraries. Status codes of the second class are defined by HTTP protocol. The third class is not defined anywhere and we should define corresponding error codes by ourselves in our application.

We define a single error code that represents quite a general error designating the fact that the message received from the server is not a correct HTTP response message and therefore, the client cannot parse it. Let's name this error code as invalid_response:
```
namespace http_errors {
  enum http_error_codes
  {
    invalid_response = 1
  };
```
Then, we define a class representing an error category, which includes the invalid_response error code defined above. Let's name this category as http_errors_category:
```
class http_errors_category
    : public boost::system::error_category
  {
  public:
    const char* name() const BOOST_SYSTEM_NOEXCEPT 
    { return "http_errors"; }

    std::string message(int e) const {
      switch (e) {
      case invalid_response:
        return "Server response cannot be parsed.";
        break;
      default:
        return "Unknown error.";
        break;
      }
    }
  };
```
Then, we define a static object of this class, a function returning an instance of the object, and the overload for the make_error_code() function accepting error codes of our custom type http_error_codes:
```
const boost::system::error_category&
get_http_errors_category()
{
    static http_errors_category cat;
    return cat;
  }

  boost::system::error_code
    make_error_code(http_error_codes e)
  {
    return boost::system::error_code(
      static_cast<int>(e), get_http_errors_category());
  }
} // namespace http_errors
```
The last step we need to perform before we can use our new error code in our application is to allow Boost library to know that the members of the http_error_codes enumeration should be treated as error codes. To do this, we include the following structure definition into the boost::system namespace:
```
namespace boost {
  namespace system {
    template<>
struct is_error_code_enum
<http_errors::http_error_codes>
{
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
  } // namespace system
} // namespace boost
```

Because our HTTP client application is going to be asynchronous, the user of the client when initiating a request, will need to provide a pointer to a callback function, which will be invoked when the request completes. We need to define a type representing a pointer to such a callback function.

A callback function when called, would need to be passed arguments that clearly designate three things:
- Which request has completed
- What is the response
- Whether the request completed successfully and if not, the error code designating the error that occurred

Note that, later, we will define the HTTPRequest and HTTPResponse classes representing the HTTP request and HTTP response correspondingly, but now we use forward declarations. Here is how the callback function pointer type declaration looks:
```
class HTTPClient;
class HTTPRequest;
class HTTPResponse;

typedef void(*Callback) (const HTTPRequest& request,
  const HTTPResponse& response,
  const system::error_code& ec);
```

## The HTTPResponse class
Now, we can define a class representing a HTTP response message sent to the client as a response to the request:
```
class HTTPResponse {
  friend class HTTPRequest;
  HTTPResponse() : 
    m_response_stream(&m_response_buf)
  {}
public:

  unsigned int get_status_code() const {
    return m_status_code;
  }

  const std::string& get_status_message() const {
    return m_status_message;
  }

  const std::map<std::string, std::string>& get_headers() {
    return m_headers;
  }
  
  const std::istream& get_response() const {
    return m_response_stream;
  }

private:
  asio::streambuf& get_response_buf() {
    return m_response_buf;
  }

  void set_status_code(unsigned int status_code) {
    m_status_code = status_code;
  }

  void set_status_message(const std::string& status_message) {
    m_status_message = status_message;
  }

  void add_header(const std::string& name, 
  const std::string& value) 
  {
    m_headers[name] = value;
  }

private:
  unsigned int m_status_code; // HTTP status code.
  std::string m_status_message; // HTTP status message.
  
  // Response headers.
  std::map<std::string, std::string> m_headers;
  asio::streambuf m_response_buf;
  std::istream m_response_stream;
};
```

The HTTPResponse class is quite simple. Its private data members represent parts of HTTP response such as the response status code and status message, and response headers and body. Its public interface contains methods that return the values of corresponding data members, while private methods allow setting those values.

The HTTPRequest class representing a HTTP request, which will be defined next, is declared as a friend to HTTPResponse. We will see how the objects of the HTTPRequest class use the private methods of the HTTPResponse class to set values of its data members when a response message arrives.

## The HTTPRequest class
Next, we define a class representing a HTTP request containing functionality that constructs the HTTP request message based on information provided by the class user, sends it to the server, and then receives and parses the HTTP response message.

This class is at the center of our application because it contains most of its functionalities.

Later, we will define the HTTPClient class representing an HTTP client, responsibilities of which will be limited to maintaining a single instance of the asio::io_service class common to all the HTTPRequest objects and acting as a factory of the HTTPRequest objects. Therefore, we declare the HTTPClient class as a friend to the HTTPRequest class and make the HTTPRequest class' constructor private:
```
class HTTPRequest {
  friend class HTTPClient;

  static const unsigned int DEFAULT_PORT = 80;

  HTTPRequest(asio::io_service& ios, unsigned int id) :
    m_port(DEFAULT_PORT),
    m_id(id),
    m_callback(nullptr),
    m_sock(ios),
    m_resolver(ios),
    m_was_cancelled(false),
    m_ios(ios)  
{}
```

The constructor accepts two arguments: a reference to an object of the asio::io_service class and an unsigned integer named id. The latter contains a unique identifier of a request, which is assigned by the user of the class and allows distinguishing request objects one from another.

Then, we define methods constituting the public interface of the class:
```
public:
  void set_host(const std::string& host) {
    m_host = host;
  }

  void set_port(unsigned int port) {
    m_port = port;
  }

  void set_uri(const std::string& uri) {
    m_uri = uri;
  }

  void set_callback(Callback callback) {
    m_callback = callback;
  }

  std::string get_host() const {
    return m_host;
  }

  unsigned int get_port() const {
    return m_port;
  }

  const std::string& get_uri() const {
    return m_uri;
  }

  unsigned int get_id() const {
    return m_id;
  }

  void execute() {
    // Ensure that precorditions hold.
    assert(m_port > 0);
    assert(m_host.length() > 0);
    assert(m_uri.length() > 0);
    assert(m_callback != nullptr);

    // Prepare the resolving query.
    asio::ip::tcp::resolver::query resolver_query(m_host,
      std::to_string(m_port), 
      asio::ip::tcp::resolver::query::numeric_service);

    std::unique_lock<std::mutex>
      cancel_lock(m_cancel_mux);

    if (m_was_cancelled) {
      cancel_lock.unlock();
      on_finish(boost::system::error_code(
      asio::error::operation_aborted));
      return;
    }
    
    // Resolve the host name.
    m_resolver.async_resolve(resolver_query,
      [this](const boost::system::error_code& ec,
      asio::ip::tcp::resolver::iterator iterator)
    {
      on_host_name_resolved(ec, iterator);
    });
  }

  void cancel() {
    std::unique_lock<std::mutex>
      cancel_lock(m_cancel_mux);
    
    m_was_cancelled = true;
    
    m_resolver.cancel();
    
    if (m_sock.is_open()) {
      m_sock.cancel();
    }  
}
```
The public interface includes methods that allow the class' user to set and get HTTP request parameters such as the DNS name of the host running the server, protocol port number, and URI of the requested resource. Besides, there is a method that allows setting a pointer to a callback function that will be called when the request completes.

The execute() method initiates the execution of the request. Also, the cancel() method allows canceling the initiated request before it completes. We will consider how these methods work in the next section of the recipe.

Now, we define a set of private methods that contain most of the implementation details. Firstly, we define a method that is used as a callback for an asynchronous DNS name resolution operation:
```
private:
  void on_host_name_resolved(
    const boost::system::error_code& ec,
    asio::ip::tcp::resolver::iterator iterator) 
{
    if (ec != 0) {
      on_finish(ec);
      return;
    }

    std::unique_lock<std::mutex>
      cancel_lock(m_cancel_mux);

    if (m_was_cancelled) {
      cancel_lock.unlock();
      on_finish(boost::system::error_code(
      asio::error::operation_aborted));
      return;
    }

    // Connect to the host.
    asio::async_connect(m_sock,
      iterator,
      [this](const boost::system::error_code& ec,
      asio::ip::tcp::resolver::iterator iterator)
    {
      on_connection_established(ec, iterator);
    });

  }
```

Then, we define a method used as a callback for an asynchronous connection operation, which is initiated in the on_host_name_resolved() method just defined:

```
void on_connection_established(
    const boost::system::error_code& ec,
    asio::ip::tcp::resolver::iterator iterator) 
{
    if (ec != 0) {
      on_finish(ec);
      return;
    }

    // Compose the request message.
    m_request_buf += "GET " + m_uri + " HTTP/1.1\r\n";

    // Add mandatory header.
    m_request_buf += "Host: " + m_host + "\r\n";

    m_request_buf += "\r\n";

    std::unique_lock<std::mutex>
      cancel_lock(m_cancel_mux);

    if (m_was_cancelled) {
      cancel_lock.unlock();
      on_finish(boost::system::error_code(
      asio::error::operation_aborted));
      return;
    }

    // Send the request message.
    asio::async_write(m_sock,
      asio::buffer(m_request_buf),
      [this](const boost::system::error_code& ec,
      std::size_t bytes_transferred)
    {
      on_request_sent(ec, bytes_transferred);
    });
  }
```

The next method we define—on_request_sent()—is a callback, which is called after the request message is sent to the server:
```
void on_request_sent(const boost::system::error_code& ec,
    std::size_t bytes_transferred) 
{
    if (ec != 0) {
      on_finish(ec);
      return;
    }

    m_sock.shutdown(asio::ip::tcp::socket::shutdown_send);

    std::unique_lock<std::mutex>
      cancel_lock(m_cancel_mux);

    if (m_was_cancelled) {
      cancel_lock.unlock();
      on_finish(boost::system::error_code(
      asio::error::operation_aborted));
      return;
    }

    // Read the status line.
    asio::async_read_until(m_sock,
      m_response.get_response_buf(),
      "\r\n",
      [this](const boost::system::error_code& ec,
      std::size_t bytes_transferred)
    {
      on_status_line_received(ec, bytes_transferred);
    });
  }
```
Then, we need another callback method, which is called when the first portion of the response message, namely, status line, is received from the server:
```
void on_status_line_received(
    const boost::system::error_code& ec,
    std::size_t bytes_transferred)
  {
    if (ec != 0) {
      on_finish(ec);
      return;
    }

    // Parse the status line.
    std::string http_version;
    std::string str_status_code;
    std::string status_message;

    std::istream response_stream(
    &m_response.get_response_buf());
    response_stream >> http_version;

    if (http_version != "HTTP/1.1"){
      // Response is incorrect.
      on_finish(http_errors::invalid_response);
      return;
    }

    response_stream >> str_status_code;

    // Convert status code to integer.
    unsigned int status_code = 200;

    try {
      status_code = std::stoul(str_status_code);
    }
    catch (std::logic_error&) {
      // Response is incorrect.
      on_finish(http_errors::invalid_response);
      return;
    }

    std::getline(response_stream, status_message, '\r');
    // Remove symbol '\n' from the buffer.
    response_stream.get();

    m_response.set_status_code(status_code);
    m_response.set_status_message(status_message);

    std::unique_lock<std::mutex>
      cancel_lock(m_cancel_mux);

    if (m_was_cancelled) {
      cancel_lock.unlock();
      on_finish(boost::system::error_code(
      asio::error::operation_aborted));
      return;
    }

    // At this point the status line is successfully
    // received and parsed.
    // Now read the response headers.
    asio::async_read_until(m_sock,
      m_response.get_response_buf(),
      "\r\n\r\n",
      [this](
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
    {
      on_headers_received(ec,
        bytes_transferred);
    });
  }
```

Now, we define a method that serves as a callback, which is called when the next portion of the response message—the response headers block—arrives from the server. We will name it as on_headers_received():
```
void on_headers_received(const boost::system::error_code& ec,
    std::size_t bytes_transferred) 
{
    if (ec != 0) {
      on_finish(ec);
      return;
    }

    // Parse and store headers.
    std::string header, header_name, header_value;
    std::istream response_stream(
    &m_response.get_response_buf());

    while (true) {
      std::getline(response_stream, header, '\r');

      // Remove \n symbol from the stream.
      response_stream.get();

      if (header == "")
        break;

      size_t separator_pos = header.find(':');
      if (separator_pos != std::string::npos) {
        header_name = header.substr(0,
        separator_pos);

        if (separator_pos < header.length() - 1)
          header_value =
          header.substr(separator_pos + 1);
        else
          header_value = "";

        m_response.add_header(header_name,
        header_value);
      }
    }

    std::unique_lock<std::mutex>
      cancel_lock(m_cancel_mux);

    if (m_was_cancelled) {
      cancel_lock.unlock();
      on_finish(boost::system::error_code(
      asio::error::operation_aborted));
      return;
    }

    // Now we want to read the response body.
    asio::async_read(m_sock,
      m_response.get_response_buf(),
      [this](
      const boost::system::error_code& ec,
      std::size_t bytes_transferred)
    {
      on_response_body_received(ec,
        bytes_transferred);
    });

    return;
  }
```
Besides, we need a method that will handle the last part of the response—the response body. The following method is used as a callback, which is called after the response body arrives from the server:
```
void on_response_body_received(
const boost::system::error_code& ec,
    std::size_t bytes_transferred) 
{
    if (ec == asio::error::eof)
      on_finish(boost::system::error_code());
    else
      on_finish(ec);  
}
```

Finally, we define the on_finish() method that serves as a final point of all execution paths (including erroneous) that start in the execute() method. This method is called when the request completes (either successfully or not) and its purpose is to call the callback provided by the HTTPRequest class' user to notify it about the completion of the request:
```
void on_finish(const boost::system::error_code& ec) 
{
    if (ec != 0) {
      std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();
    }

    m_callback(*this, m_response, ec);

    return;
  }
```

We will need some data fields associated with each instance of the HTTPRequest class. Here, we declare the class' corresponding data members:
```
private:
  // Request parameters. 
  std::string m_host;
  unsigned int m_port;
  std::string m_uri;

  // Object unique identifier. 
  unsigned int m_id;

  // Callback to be called when request completes. 
  Callback m_callback;

  // Buffer containing the request line.
  std::string m_request_buf;

  asio::ip::tcp::socket m_sock;  
  asio::ip::tcp::resolver m_resolver;

  HTTPResponse m_response;

  bool m_was_cancelled;
  std::mutex m_cancel_mux;

  asio::io_service& m_ios;
```

## The HTTPClient class
The last class that we need in our application is the one that would be responsible for the following three functions:

- To establish a threading policy
- To spawn and destroy threads in a pool of threads running the Boost.Asio event loop and delivering asynchronous operations' completion events
- To act as a factory of the HTTPRequest objects
We will name this class as HTTPClient:
```
class HTTPClient {
public:
  HTTPClient(){
    m_work.reset(new boost::asio::io_service::work(m_ios));

    m_thread.reset(new std::thread([this](){
      m_ios.run();
    }));
  }

  std::shared_ptr<HTTPRequest>
  create_request(unsigned int id) 
  {
    return std::shared_ptr<HTTPRequest>(
    new HTTPRequest(m_ios, id));
  }

  void close() {
    // Destroy the work object. 
    m_work.reset(NULL);

    // Waiting for the I/O thread to exit.
    m_thread->join();
  }

private:
  asio::io_service m_ios;
  std::unique_ptr<boost::asio::io_service::work> m_work;
  std::unique_ptr<std::thread> m_thread;
};
```
## The callback and the main() entry point function
At this point, we have the basic HTTP client that comprises three classes and several supplementary data types. Now we will define two functions that are not parts of the client, but demonstrate how to use it to communicate with the server using the HTTP protocol. The first function will be used as a callback, which will be called when the request completes. Its signature must correspond to the function pointer type Callback defined earlier. Let's name our callback function as handler():
```
void handler(const HTTPRequest& request,
  const HTTPResponse& response,
  const system::error_code& ec)
{
  if (ec == 0) {
    std::cout << "Request #" << request.get_id()
      << " has completed. Response: "
      << response.get_response().rdbuf();
  }
  else if (ec == asio::error::operation_aborted) {
    std::cout << "Request #" << request.get_id()
      << " has been cancelled by the user." 
      << std::endl;
  }
  else {
    std::cout << "Request #" << request.get_id()
      << " failed! Error code = " << ec.value()
      << ". Error message = " << ec.message() 
    << std::endl;
  }

  return;
}
```
The second and the last function we need to define is the main() application entry point function that uses the HTTP client to send HTTP requests to the server:
```
int main()
{
  try {
    HTTPClient client;

    std::shared_ptr<HTTPRequest> request_one =
      client.create_request(1);

    request_one->set_host("localhost");
    request_one->set_uri("/index.html");
    request_one->set_port(3333);
    request_one->set_callback(handler);

    request_one->execute();

    std::shared_ptr<HTTPRequest> request_two =
      client.create_request(1);

    request_two->set_host("localhost");
    request_two->set_uri("/example.html");
    request_two->set_port(3333);
    request_two->set_callback(handler);

    request_two->execute();

    request_two->cancel();

    // Do nothing for 15 seconds, letting the
    // request complete.
    std::this_thread::sleep_for(std::chrono::seconds(15));

    // Closing the client and exiting the application.
    client.close();
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
};
```
# How it works
Now let's consider how our HTTP client works. The application consists of five components, among which are the three classes such as HTTPClient, HTTPRequest, and HTTPResponse, and two functions such as the handler() callback function and the main() application entry point function. Let's consider how each component works separately.

## The HTTPClient class
A class' constructor begins with creating an instance of the asio::io_service::work object in order to make sure that threads running the event loop do not exit this loop when there are no pending asynchronous operations. Then, a thread of control is spawned and added to the pool by calling the run() method on the m_ios object. This is where the HTTPClient class performs its first and part of the second functions, namely, establishing threading policy and adding threads to the pool.

The third function of the HTTPClient class—to act as a factory of the object representing HTTP requests—is performed in its create_request() public method. This method creates an instance of the HTTPRequest class in the free memory and returns a shared pointer object pointing to it. As its input argument, the method accepts an integer value that represents the unique identifier to be assigned to the newly created request object. This identifier is used to distinguish between different request objects.

The close() method from the class' public interface destroys the asio::io_service::work object, allowing threads to exit the event loop just as soon as all pending operations complete. The method blocks until all threads exit.

## The HTTPRequest class
Let's begin considering the HTTPRequest class' behavior by inspecting its data members and their purpose. The HTTPRequest class contains 12 data members, among which are the following:

- Request parameters
```
  std::string m_host;
  unsigned int m_port;
  std::string m_uri;
```
- A unique identifier of the request
```
 unsigned int m_id;
```
- A pointer to the callback function provided by the class' user to be called when a request completes
```
Callback m_callback;
```
- A string buffer used to store the HTTP request message
```
std::string m_request_buf;
```
- A socket object used to communicate with the server
```
 asio::ip::tcp::socket m_sock;
```
- A resolver object used to resolve the DNS name of the server host provided by the user
```
 asio::ip::tcp::resolver m_resolver;
```
- An instance of the HTTPResponse class that represents the response received from the server
```
HTTPResponse m_response;
```
- A boolean flag and a mutex object supporting the request canceling functionality (which will be explained later)
```
bool m_was_cancelled;
std::mutex m_cancel_mux;
```
- Also, a reference to an instance of the asio::io_service class required by resolver and socket objects. The single instance of the asio::io_service class is maintained by an object of the HTTPClient class
```
 asio::io_service& m_ios;
```

An instance of the HTTPRequest object represents a single HTTP GET request. The class is designed so that in order to send a request, two steps need to be performed. Firstly, the parameters of the request and the callback function to be called when the request completes are set by calling the corresponding setter methods on the object. Then, as a second step, the execute() method is invoked to initiate the request execution. When the request completes, the callback function is called.

The set_host(), set_port(), set_uri(), and set_callback() setter methods allow setting a server host DNS name and port number, URI of the requested resource, and a callback function to be called when the request completes. Each of these methods accepts one argument and stores its value in the corresponding HTTPRequest object's data member.

The get_host(), get_port(), and get_uri() getter methods return values set by corresponding setter methods. The get_id() getter method returns a request object's unique identifier, which is passed to the object's constructor on instantiation.

The execute() method begins the execution of a request by initiating a sequence of asynchronous operations. Each asynchronous operation performs one step of request execution procedure.

Because a server host in the request object is represented with a DNS name (rather than with an IP address), before sending the request message to the server, the specified DNS name must be resolved and transformed into an IP address. Therefore, the first step in the request execution is DNS name resolution. The execute() method begins with preparing the resolving query and then calls the resolver object's async_resolve() method, specifying the HTTPRequest class' on_host_name_resolve() private method as an operation completion callback.

When the server host DNS name is resolved, the on_host_name_resolved() method is called. This method is passed two arguments: the first of which is an error code, designating the status of the operation, and the second one is the iterator that can be used to iterate through a list of endpoints resulting from a resolution process.

The on_host_name_resolved() method initiates the next asynchronous operation in a sequence, namely socket connection, by calling asio::async_connect() free function passing socket object m_sock and iterator parameter to it so that it connects the socket to the first valid endpoint. The on_connection_established() method is specified as an asynchronous connection operation completion callback.

When an asynchronous connection operation completes, the on_connection_established() method is invoked. The first argument passed to it is named ec that designates the operation completion status. If its value is equal to zero, it means that the socket was successfully connected to one of the endpoints. The on_connection_established() method constructs the HTTP GET request message using request parameters stored in the corresponding data members of the HTTPRequest object. Then, the asio::async_write() free function is called to asynchronously send a constructed HTTP request message to the server. The class' private method on_request_sent() is specified as a callback to be called when the asio::async_write() operation completes.

After a request is sent, and if it is sent successfully, the client application has to let the server know that the full request is sent and the client is not going to send anything else by shutting down the send part of the socket. Then, the client has to wait for the response message from the server. And this is what the on_request_sent() method does. Firstly, it calls the socket object's shutdown() method, specifying that the send part should be closed by the passing value asio::ip::tcp::socket::shutdown_send to the method as an argument. Then, it calls the asio::async_read_until() free function to receive a response from the server.

Because the response may be potentially very big and we do not know its size beforehand, we do not want to read it all at once. We first want to read the HTTP response status line only; then, having analyzed it, either continue reading the rest of the response (if we think we need it) or discard it. Therefore, we pass the \r\n symbols sequence, designating the end of the HTTP response status line as a delimiter argument to the asio::async_read_until() method. The on_status_line_received() method is specified as an operation completion callback.

When the status line is received, the on_status_line_received() method is invoked. This method performs parsing of the status line, extracting values designating the HTTP protocol version, response status code, and response status message from it. Each value is analyzed for correctness. We expect the HTTP version to be 1.1, otherwise the response is considered incorrect and the request execution is interrupted. The status code should be an integer value. If the string-to-integer conversion fails, the response is considered incorrect and its further processing is interrupted too. If the response status line is correct, the request execution continues. The extracted status code and status message are stored in the m_response member object, and the next asynchronous operation in the request execution operation sequence is initiated. Now, we want to read the response headers block.

According to the HTTP protocol, the response headers block ends with the \r\n\r\n symbols sequence. Therefore, in order to read it, we call the asio::async_read_until() free function one more time, specifying the string \r\n\r\n as a delimiter. The on_headers_received() method is specified as a callback.

When the response headers block is received, the on_headers_received() method is invoked. In this method, the response headers block is parsed and broken into separate name-value pairs and stored in the m_response member object as a part of the response.

Having received and parsed the headers, we want to read the last part of the response—the response body. To do this, an asynchronous reading operation is initiated by calling the asio::async_read() free function. The on_response_body_received() method is specified as a callback.

Eventually, the on_response_body_received() method is invoked notifying us of the fact that the whole response message has been received. Because the HTTP server may shutdown the send part of its socket just after it sends the last part of the response message, on the client side, the last reading operation may complete with an error code equal to the asio::error::eof value. This should not be treated as an actual error, but rather as a normal event. Therefore, if the on_response_body_received() method is called with the ec argument equal to asio::error::eof, we pass the default constructed object of the boost::system::error_code class to the on_finish() method in order to designate that the request execution is completed successfully. Otherwise, the on_finish() method is called with an argument representing the original error code. The on_finish() method in its turn calls the callback provided by the client of the HTTPRequest class object.

When the callback returns, request processing is considered finished.

## The HTTPResponse class
The HTTPResponse class does not provide much functionality. It is more like a plain data structure containing data members representing different parts of a response, with getter and setter methods defined, allowing getting and setting corresponding data member values.

All setter methods are private and only the objects of the HTTPRequest class has access to them (recall that the HTTPRequest class is declared as the HTTPResponse class' friend). Each object of the HTTPRequest class has a data member that is an instance of the HTTPResponse class. The object of the HTTPRequest class sets values of its member object of HTTPResponse class as it receives and parses the response received from a HTTP server.

## Callback and the main() entry point functions
These functions demonstrate how to use the HTTPClient and HTTPRequest classes in order to send the GET HTTP requests to the HTTP server and then how to use the HTTPResponse class to obtain the response.

The main() function first creates an instance of the HTTPClient class and then uses it to create two instances of the HTTPRequest class, each representing a separate GET HTTP request. Both request objects are provided with request parameters and then executed. However, just after the second request has been executed, the first one is canceled by invoking its cancel() method.

The handler() function, which is used as a completion callback for both request objects created in the main() function, is invoked when each request completes regardless of whether it succeeded, failed, or was canceled. The handler() function analyses the error code and the request and response objects passed to it as arguments and output corresponding messages to the standard output stream.

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
