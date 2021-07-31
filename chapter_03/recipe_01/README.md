# Implementing a synchronous TCP client

A synchronous TCP client is a part of a distributed application that complies with the following statements:

 - Acts as a client in the client-server communication model
 - Communicates with the server application using a TCP protocol
 - Uses I/O and control operations (at least those I/O operations that are related to communication with a server) that block the thread of execution until the corresponding operation completes, or an error occurs
A typical synchronous TCP client works according to the following algorithm:
 - Obtain the IP-address and the protocol port number of the server application.
 - Allocate an active socket.
 - Establish a connection with the server application.
 - Exchange messages with the server.
 - Shut down the connection.
 - Deallocate the socket.

The sample client application consists of two main components—the SyncTCPClient class and the application entry point function main() in which the SyncTCPClient class is used to communicate with the server application.
## The SyncTCPClient class
The SyncTCPClient class is the key component in the sample. It implements and provides access to the communication functionality.

The class has three private members as follows:

- asio::io_service m_ios: This is the object providing access to the operating system's communication services, which are used by the socket object
- asio::ip::tcp::endpoint m_ep: This is an endpoint designating the server application
- asio::ip::tcp::socket m_sock: This is the socket used for communication

Each object of the class is intended to communicate with a single server application; therefore, the class's constructor accepts the server IP-address and the protocol port number as its arguments. These values are used to instantiate the m_ep object in the constructor's initialization list. The socket object m_sock is instantiated and opened in the constructor too.

The three public methods comprise the interface of the SyncTCPClient class. The first method named connect() is quite simple; it performs the connection of the socket to the server. The close() method shuts the connection down and closes the socket, which leads to the operating system's socket and other resources associated with it to be deallocated.

The third interface method is emulateLongComputationOp(unsigned int duration_sec). This method is where the I/O operations are performed. It begins with preparing the request string according to the protocol. Then, the request is passed to the class's private method sendRequest(const std::string& request), which sends it to the server. When the request is sent and the sendRequest() method returns, the receiveResponse() method is called to receive the response from the server. When the response is received, the receiveResponse() method returns the string containing the response. After this, the emulateLongComputationOp() method returns the response to its caller.

Let's look at the sendRequest() and receiveResponse() methods in more detail.

The sendRequest() method has the following prototype:
```
void sendRequest(const std::string& request)
```
Its purpose is to send a string, passed to it as an argument, to the server. In order to send the data to the server, the asio::write() free synchronous function is used. The function returns when the request is sent. That's it about the sendRequest() method. Basically, all it does is, it fully delegates its job to the asio::write() free function.

Having sent the request, now we want to receive the response from the server. This is the purpose of the receiveResponse() method of the SyncTCPClient class. To perform its job, method uses the asio::read_until() free function. According to the application layer protocol, the response message sent by the server may vary in length, but must end with the \n symbol; therefore, we specify this symbol as a delimiter when calling the function:
```
asio::streambuf buf;
asio::read_until(m_sock, buf, '\n');
```
The function blocks the thread of execution until it encounters the \n symbol as a part of the message that arrived from the server. When the function returns, the stream buffer buf contains the response. The data is then copied from the buf buffer to the response string and the latter is returned to the caller. The emulateLongComputationOp() method in turn returns the response to its caller—the main() function.

One thing to note with regard to the SyncTCPClient class is that it contains no error handling-related code. That's because the class uses only those overloads of Boost.Asio functions and objects' methods that throw exceptions in case of failure. It is assumed that the user of the class is responsible for catching and handling the exceptions.

## The main() entry point function
This function acts as a user of the SyncTCPClient class. Having obtained the server IP-address and the protocol port number (this part is omitted from the sample), it instantiates and uses an object of the SyncTCPClient class to communicate with the server in order to consume its service, mainly to emulate an operation on the server that performs dummy calculations for 10 seconds. The code of this function is simple and self-explanatory and thus requires no additional comments.


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
