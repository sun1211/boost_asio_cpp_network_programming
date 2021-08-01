# Adding SSL/TLS support to server applications

SSL/TLS protocol support is usually added to the server application when the services it provides assumes transmission of sensitive data such as passwords, credit card numbers, personal data, and so on, by the client to the server. In this case, adding SSL/TLS protocol support to the server allows clients to authenticate the server and establish a secure channel to make sure that the sensitive data is protected while being transmitted.

Sometimes, a server application may want to use SSL/TLS protocol to authenticate the client; however, this is rarely the case and usually other methods are used to ensure the authenticity of the client (for example, username and password are specified when logging into a mail server).

Before setting out to this recipe, OpenSSL library must be installed and the project must be linked against it. Procedures related to the installation of the library or linking the project against it are beyond the scope of this book. Refer to the official OpenSSL documentation for more information.

The following code sample demonstrates the possible implementation of a synchronous TCP server application supporting SSL/TLS protocol to allow client applications to authenticate the server and protect the data being transmitted.

We begin our application by including Boost.Asio library headers and headers of some components of standard C++ libraries that we will need to implement in our application:
```
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <thread>
#include <atomic>
#include <iostream>

using namespace boost;
```

The <boost/asio/ssl.hpp> header contains types and functions providing integration with OpenSSL library.

Next, we define a class responsible for handling a single client by reading the request message, processing it, and then sending back the response message. This class represents a single service provided by the server application and is named correspondingly—Service:
```
class Service {
public:
  Service(){}

  void handle_client(
  asio::ssl::stream<asio::ip::tcp::socket>& ssl_stream) 
  {
    try {
      // Blocks until the handshake completes.
      ssl_stream.handshake(
        asio::ssl::stream_base::server);

      asio::streambuf request;
      asio::read_until(ssl_stream, request, '\n');

      // Emulate request processing.
      int i = 0;
      while (i != 1000000)
        i++;
      std::this_thread::sleep_for(
        std::chrono::milliseconds(500));

      // Sending response.
      std::string response = "Response\n";
      asio::write(ssl_stream, asio::buffer(response));
    }
    catch (system::system_error &e) {
      std::cout << "Error occured! Error code = "
        << e.code() << ". Message: "
        << e.what();
    }
  }
};
```

Next, we define another class that represents a high-level acceptor concept (as compared to the low-level acceptor represented by the asio::ip::tcp::acceptor class). This class is responsible for accepting connection requests arriving from clients and instantiating objects of the Service class, which will provide the service to connected clients. This class is called Acceptor:
```
class Acceptor {
public:
  Acceptor(asio::io_service& ios, unsigned short port_num) :
    m_ios(ios),
    m_acceptor(m_ios,
    asio::ip::tcp::endpoint(
    asio::ip::address_v4::any(),
    port_num)),
    m_ssl_context(asio::ssl::context::sslv23_server)
  {
    // Setting up the context.
    m_ssl_context.set_options(
      boost::asio::ssl::context::default_workarounds
      | boost::asio::ssl::context::no_sslv2
      | boost::asio::ssl::context::single_dh_use);

    m_ssl_context.set_password_callback(
      [this](std::size_t max_length,
      asio::ssl::context::password_purpose purpose)
      -> std::string 
        {return get_password(max_length, purpose);}
    );

    m_ssl_context.use_certificate_chain_file("server.crt");
    m_ssl_context.use_private_key_file("server.key",
      boost::asio::ssl::context::pem);
    m_ssl_context.use_tmp_dh_file("dhparams.pem");

    // Start listening for incoming connection requests.
    m_acceptor.listen();
  }

  void accept() {
    asio::ssl::stream<asio::ip::tcp::socket>
    ssl_stream(m_ios, m_ssl_context);

    m_acceptor.accept(ssl_stream.lowest_layer());

    Service svc;
    svc.handle_client(ssl_stream);
  }

private:
  std::string get_password(std::size_t max_length,
    asio::ssl::context::password_purpose purpose) const
  {
    return "pass";
  }

private:
  asio::io_service& m_ios;
  asio::ip::tcp::acceptor m_acceptor;

  asio::ssl::context m_ssl_context;
};
```

Now we define a class that represents the server itself. The class is named correspondingly—Server:
```
class Server {
public:
  Server() : m_stop(false) {}

  void start(unsigned short port_num) {
    m_thread.reset(new std::thread([this, port_num]() {
      run(port_num);
    }));
  }

  void stop() {
    m_stop.store(true);
    m_thread->join();
  }

private:
  void run(unsigned short port_num) {
    Acceptor acc(m_ios, port_num);

    while (!m_stop.load()) {
      acc.accept();
    }
  }

  std::unique_ptr<std::thread> m_thread;
  std::atomic<bool> m_stop;
  asio::io_service m_ios;
};
```

Eventually, we implement the main() application entry point function that demonstrates how to use the Server class. This function is identical to the one defined in the recipe from Chapter 4, Implementing Server Applications, that we took as a base for this recipe:
```
int main()
{
  unsigned short port_num = 3333;

  try {
    Server srv;
    srv.start(port_num);

    std::this_thread::sleep_for(std::chrono::seconds(60));

    srv.stop();
  }
  catch (system::system_error &e) {
    std::cout   << "Error occured! Error code = " 
    << e.code() << ". Message: "
        << e.what();
  }

  return 0;
}
```

Note that the last two components of the server application, namely, the Server class and the main() application entry point function are identical to the corresponding components defined in the recipe from Chapter 4, Implementing Server Applications, that we took as a base for this recipe.

# How it works
The sample server application consists of four components: the Service, Acceptor, and Server classes and the main(), application entry point function, which demonstrates how to use the Server class. Because the source code and the purpose of the Server class and the main() entry point function are identical to those of the corresponding components defined in the recipe from Chapter 4, Implementing Server Applications, that we took as a base for this recipe, we will not discuss them here. We will only consider the Service and Acceptor classes that were updated to provide support for SSL/TLS protocol.

## The Service class
The Service class is the key functional component in the application. While other components are infrastructural in their purpose, this class implements the actual function (or service) required by the clients.

The Service class is quite simple and consists of a single method handle_client(). As its input argument, this method accepts a reference to an object representing an SSL stream that wraps a TCP socket connected to a particular client.

The method begins with performing an SSL/TLS handshake by invoking the handshake() method on the ssl_stream object. This method is synchronous and does not return until the handshake completes or an error occurs.

After the handshake has completed, a request message is synchronously read from the SSL stream until a new line ASCII symbol \n is encountered. Then, the request is processed. In our sample application, request processing is trivial and dummy and consists in running a loop performing one million increment operations and then putting the thread to sleep for half a second. After this, the response message is prepared and sent back to the client.

Exceptions that may be thrown by the Boost.Asio functions and methods are caught and handled in the handle_client() method and are not propagated to the method's caller so that, if handling of one client fails, the server continues working.

Note that the handle_client() method is very similar to the corresponding method defined in the recipe Implementing a synchronous iterative TCP server, from Chapter 4, Implementing Server Applications, that we took as a base for this recipe. The difference consists in the fact that in this recipe, the handle_client() method operates on an object representing an SSL stream as opposed to an object representing a TCP socket being operated on in the base implementation of the method. Besides, an additional operation—an SSL/TLS handshake—is performed in the method defined in this recipe.

## The Acceptor class

The Acceptor class is a part of the server application infrastructure. Each object of this class owns an instance of the asio::ssl::context class named m_ssl_context. This member represents an SSL context. Basically, the asio::ssl::contex class is a wrapper around the SSL_CTX data structure defined by OpenSSL library. Objects of this class contain global settings and parameters used by other objects and functions involved in the process of communication using SSL/TLS protocol.

The m_ssl_context object, when instantiated, is passed a asio::ssl::context::sslv23_server value to its constructor to designate that the SSL context will be used by the application playing a role of a server only and that multiple secure protocols should be supported, including multiple versions of SSL and TLS. This value defined by Boost.Asio corresponds to a value representing a connection method returned by the SSLv23_server_method() function defined by OpenSSL library.

The SSL context is configured in the Acceptor class' constructor. The context options, password callback and files containing digital certificates, and private keys and Diffie-Hellman protocol parameters, are specified there.

After SSL context has been configured, the listen() method is called on the acceptor object in the Acceptor class' constructor to start listening for connection requests from the clients.

The Acceptor class exposes a single accept() public method. This method, when called, first instantiates an object of the asio::ssl::stream<asio::ip::tcp::socket> class named ssl_stream, representing an SSL/TLS communication channel with the underlying TCP socket. Then, the accept() method is called on the m_acceptor acceptor object to accept a connection. The TCP socket object owned by ssl_stream, returned by its lowest_layer() method, is passed to the accept() method as an input argument. When a new connection is established, an instance of the Service class is created and its handle_client() method is called, which performs communication with the client and request handling.

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
