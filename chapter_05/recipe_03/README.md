# Adding SSL/TLS support to client applications

Client applications usually use SSL/TLS protocol to send sensitive data such as passwords, credit card numbers, personal data. SSL/TLS protocol allows clients to authenticate the server and encrypt the data. The authentication of the server allows the client to make sure that the data will be sent to the expected addressee (and not to a malicious one). Data encryption guarantees that even if the transmitted data is intercepted somewhere on its way to the server, the interceptor will not be able to use it.

Before setting out to this recipe, OpenSSL library must be installed and the project must be linked against it. Procedures related to the installation of the library or linking the project against it are beyond the scope of this book. Refer to the OpenSSL library documentation for more information.

The following code sample demonstrates the possible implementation of a synchronous TCP client application supporting SSL/TLS protocol to authenticate the server and encrypt the data being transmitted.

We begin our application by adding the include and using directives:
```
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>

using namespace boost;
```

The <boost/asio/ssl.hpp> header contains types and functions providing integration with OpenSSL library.

Next, we define a class that plays the role of the synchronous SSL/TLS-enabled TCP client:
```
class SyncSSLClient {
public:
  SyncSSLClient(const std::string& raw_ip_address,
    unsigned short port_num) :
    m_ep(asio::ip::address::from_string(raw_ip_address),
    port_num),
    m_ssl_context(asio::ssl::context::sslv3_client),    
    m_ssl_stream(m_ios, m_ssl_context)
  {
    // Set verification mode and designate that 
    // we want to perform verification.
    m_ssl_stream.set_verify_mode(asio::ssl::verify_peer);

    // Set verification callback. 
    m_ssl_stream.set_verify_callback([this](
      bool preverified,
      asio::ssl::verify_context& context)->bool{
      return on_peer_verify(preverified, context);
    });  
  }

  void connect() {
    // Connect the TCP socket.
    m_ssl_stream.lowest_layer().connect(m_ep);

    // Perform the SSL handshake.
    m_ssl_stream.handshake(asio::ssl::stream_base::client);
  }

  void close() {
    // We ignore any errors that might occur
    // during shutdown as we anyway can't
    // do anything about them.
    boost::system::error_code ec;

    m_ssl_stream.shutdown(ec); // Shutdown SSL.

    // Shut down the socket.
    m_ssl_stream.lowest_layer().shutdown(
      boost::asio::ip::tcp::socket::shutdown_both, ec);

    m_ssl_stream.lowest_layer().close(ec);
  }

  std::string emulate_long_computation_op(
    unsigned int duration_sec) {

    std::string request = "EMULATE_LONG_COMP_OP "
      + std::to_string(duration_sec)
      + "\n";

    send_request(request);
    return receive_response();
  };

private:
  bool on_peer_verify(bool preverified,
    asio::ssl::verify_context& context) 
  {
    // Here the certificate should be verified and the
    // verification result should be returned.
    return true;
  }

  void send_request(const std::string& request) {
    asio::write(m_ssl_stream, asio::buffer(request));
  }

  std::string receive_response() {
    asio::streambuf buf;
    asio::read_until(m_ssl_stream, buf, '\n');

    std::string response;
    std::istream input(&buf);
    std::getline(input, response);

    return response;
  }

private:
  asio::io_service m_ios;
  asio::ip::tcp::endpoint m_ep;

  asio::ssl::context m_ssl_context;
  asio::ssl::stream<asio::ip::tcp::socket>m_ssl_stream;
};
```
Now we implement the main() application entry point function that uses the SyncSSLClient class to authenticate the server and securely communicate with it using SSL/TLS protocol:
```
int main()
{
  const std::string raw_ip_address = "127.0.0.1";
  const unsigned short port_num = 3333;

  try {
    SyncSSLClient client(raw_ip_address, port_num);

    // Sync connect.
    client.connect();

    std::cout << "Sending request to the server... "
      << std::endl;

    std::string response =
      client.emulate_long_computation_op(10);

    std::cout << "Response received: " << response
      << std::endl;

    // Close the connection and free resources.
    client.close();
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
}
```

# How it works
he sample client application consists of two main components: the SyncSSLClient class and a main() application entry point function that uses the SyncSSLClient class to communicate with the server application over SSL/TLS protocol. Let's consider how each component works separately.

## The SyncSSLClient class
The SyncSSLClient class is the key component in our application. It implements the communication functionality.

The class has four private data members as follows:
- asio::io_service m_ios: This is an object providing access to the operating system's communication services that are used by the socket object.
- asio::ip::tcp::endpoint m_ep: This is an endpoint designating the server application.
- asio::ssl::context m_ssl_context: This is an object representing SSL context; basically, this is a wrapper around the SSL_CTX data structure defined by OpenSSL library. This object contains global settings and parameters used by other objects and functions involved in the process of communication using SSL/TLS protocol.
- asio::ssl::stream<asio::ip::tcp::socket> m_ssl_stream: This represents a stream that wraps a TCP socket object and implements all SSL/TLS communication operations.

Each object of the class is intended to communicate with a single server. Therefore, the class' constructor accepts an IP address and a protocol port number designating the server application as its input arguments. These values are used to instantiate the m_ep data member in the constructor's initialization list.

Next, the m_ssl_context and m_ssl_stream members of the SyncSSLClient class are instantiated. We pass the asio::ssl::context::sslv23_client value to the m_ssl_context object's constructor to designate that the context will be used by the application playing a role of a client only and that we want to support multiple secure protocols including multiple versions of SSL and TLS. This value defined by Boost.Asio corresponds to a value representing a connection method returned by the SSLv23_client_method() function defined by OpenSSL library.

The SSL stream object m_ssl_stream is set up in the SyncSSLClient class' constructor. Firstly, the peer verification mode is set to asio::ssl::verify_peer, which means that we want to perform peer verification during a handshake. Then, we set a verification callback method that will be called when certificates arrive from the server. The callback is invoked once for each certificate in the certificates chain sent by the server.

The class' on_peer_verify() method that is set as a peer verification callback is a dummy in our application. The certificate verification process lies beyond the scope of this book. Therefore, the function simply always returns the true constant, meaning that the certificate verification succeeded without performing the actual verification.

The three public methods comprise the interface of the SyncSSLClient class. The method named connect() performs two operations. Firstly, the TCP socket is connected to the server. The socket underlying the SSL stream is returned by the method of the SSL stream object lowest_layer(). Then, the connect() method is called on the socket with m_ep being passed as an argument designating the endpoint to be connected to:
```
// Connect the TCP socket.
m_ssl_stream.lowest_layer().connect(m_ep);
```

After the TCP connection is established, the handshake() method is called on the SSL stream object, which leads to the initiation of the handshake process. This method is synchronous and does not return until the handshake completes or an error occurs:
```
// Perform the SSL handshake.
m_ssl_stream.handshake(asio::ssl::stream_base::client);
```

After the handshake() method returns, both TCP and SSL (or TLS, depending on which protocol was agreed upon during the handshake process) connections are established and the effective communication can be performed.

The close() method shuts down the SSL connection by calling the shutdown() method on the SSL stream object. The shutdown() method is synchronous and blocks until the SSL connection is shut down or an error occurs. After this method returns, the corresponding SSL stream object cannot be used to transmit the data anymore.

The third interface method is emulate_long_computation_op(unsigned int duration_sec). This method is where the I/O operations are performed. It begins with preparing the request string according to the application layer protocol. Then, the request is passed to the class' send_request(const std::string& request) private method, which sends it to the server. When the request is sent and the send_request() method returns, the receive_response() method is called to receive the response from the server. When the response is received, the receive_response() method returns the string containing the response. After this, the emulate_long_computation_op() method returns the response message to its caller.

Note that the emulate_long_computation_op(), send_request(), and receive_response() methods are almost identical to the corresponding methods defined in the SyncTCPClient class, which is a part of the synchronous TCP client application demonstrated in Chapter 3, Implementing Client Applications, which we used as a base for SyncSSLClient class. The only difference is that in SyncSSLClient, an SSL stream object is passed to the corresponding Boost.Asio I/O functions, while in the SyncTCPClient class, a socket object is passed to those functions. Other aspects of the mentioned methods are identical.

## The main() entry point function
This function acts as a user of the SyncSSLClient class. Having obtained the server IP address and protocol port number, it instantiates and uses the object of the SyncSSLClient class to authenticate and securely communicate with the server in order to consume its service, namely, to emulate an operation on the server by performing dummy calculations for 10 seconds. The code of this function is simple and self-explanatory; thus, requires no additional comments.


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
