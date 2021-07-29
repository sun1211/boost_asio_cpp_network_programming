# Reading from a TCP socket synchronously

Reading from a TCP socket is an input operation that is used to receive data sent by the remote application connected to this socket. Synchronous reading is the simplest way to receive the data using a socket provided by Boost.Asio. The methods and functions that perform synchronous reading from the socket blocks the thread of execution and doesn't return until the data (at least some amount of data) is read from the socket or an error occurs.

The most basic way to read data from the socket provided by the Boost.Asio library is the read_some() method of the asio::ip::tcp::socket class. Let's take a look at one of the method's overloads:
```
template<
typename MutableBufferSequence>
std::size_t read_some(
    const MutableBufferSequence & buffers);
```

This method accepts an object that represents a writable buffer (single or composite) as an argument, and as its name suggests, reads some amount of data from the socket to the buffer. If the method succeeds, the return value indicates the number of bytes read. It's important to note that there is no way to control how many bytes the method will read. The method only guarantees that at least one byte will be read if an error does not occur. This means that, in a general case, in order to read a certain amount of data from the socket, we may need to call the method several times.

The following algorithm describes the steps required to synchronously read data from a TCP socket in a distributed application:

 - In a client application, allocate, open, and connect an active TCP socket. In a server application, obtain a connected active TCP socket by accepting a connection request using an acceptor socket.
 - Allocate the buffer of a sufficient size to fit in the expected message to be read.
 - In a loop, call the socket's read_some() method as many times as it is needed to read the message.

Although in the presented code sample, reading from a socket is performed in the context of an application that acts as a client, the same approach can be used to read data from the socket in a server application.

The main()application entry point function is quite simple. First, it allocates a TCP socket, opens, and synchronously connects it to a remote application. Then, the readFromSocket() function is called and the socket object is passed to it as an argument. In addition to this, the main() function contains a try-catch block intended to catch and handle exceptions that may be thrown by Boost.Asio methods and functions.

The interesting part in the sample is the readFromSocket() function that performs synchronous reading from the socket. It accepts a reference to the socket object as an input argument. Its precondition is that the socket passed to it as an argument must be connected; otherwise, the function fails.

The function begins with allocating a buffer named buf. The size of the buffer is chosen to be MESSAGE_SIZE bytes. This is because in our sample, we expect to receive exactly a MESSAGE_SIZE bytes long message from a remote application.

Then, a variable named total_bytes_read is defined and its value is set to 0. This variable is used as a counter that keeps the count of the total number of bytes read from the socket.

Next, the loop is run in which the socket's read_some() method is called. Let's take a closer look at the loop:
```
while (total_bytes_read != MESSAGE_SIZE) {
    total_bytes_read += sock.read_some(
      asio::buffer(buf + total_bytes_read,
      MESSAGE_SIZE - total_bytes_read));
  }
```
The termination condition evaluates to true when the value of the total_bytes_read variable is equal to the size of the expected message, that is, when the whole message has been read from the socket. In each iteration of the loop, the value of the total_bytes_read variable is increased by the value returned by the read_some() method, which is equal to the number of bytes read during this method call.

Each time the read_some() method is called, the input buffer passed to it is adjusted. The start byte of the buffer is shifted by the value of total_bytes_read as compared to the original buffer (because the preceding part of the buffer has already been filled with data read from the socket during preceding calls to the read_some() method) and the size of the buffer is decreased by the same value, correspondingly.

After the loop terminates, all the data expected to be read from the socket is now in the buffer.

The readFromSocket() function ends with instantiating an object of the std::string class from the received buffer and returning it to the caller.

It's worth noting that the amount of bytes read from the socket during a single call to the read_some() method depends on several factors. In a general case, it is not known to the developer; and, therefore, it should not be accounted for. The proposed solution is independent of this value and calls the read_some() method as many times as needed to read all the data from the socket.

## Alternative – the receive() method
The asio::ip::tcp::socket class contains another method to read data from the socket synchronously called receive(). There are three overloads of this method. One of them is equivalent to the read_some() method, as described earlier. It has exactly the same signature and provides exactly the same functionality. These methods are synonyms in a sense.

The second overload accepts one additional argument as compared to the read_some() method. Let's take a look at it:
```
template<
    typename MutableBufferSequence>
std::size_t receive(
    const MutableBufferSequence & buffers,
    socket_base::message_flags flags);
```
This additional argument is named flags. It can be used to specify a bit mask, representing flags that control the operation. Because these flags are rarely used, we won't consider them in this book. Refer to the Boost.Asio documentation to find out more about this topic.

The third overload is equivalent to the second one, but it doesn't throw exceptions in case of a failure. Instead, the error information is returned by means of an additional output argument of the boost::system::error_code type.

Reading from a socket using the socket's read_some() method seems very complex for such a simple operation. This approach requires us to use a loop, a variable to keep track of how many bytes have already been read, and properly construct a buffer for each iteration of the loop. This approach is error-prone and makes the code more difficult to understand and maintain.

Fortunately, Boost.Asio provides a family of free functions that simplify synchronous reading of data from a socket in different contexts. There are three such functions, each having several overloads, that provide a rich functionality that facilitates reading data from a socket.

**The asio::read() function**
The asio::read() function is the simplest one out of the three. Let's take a look at the declaration of one of its overloads:
```
template<
    typename SyncReadStream,
    typename MutableBufferSequence>
std::size_t read(
    SyncReadStream & s,
    const MutableBufferSequence & buffers);
```
This function accepts two arguments. The first of them named s is a reference to an object that satisfies the requirements of the SyncReadStream concept. For a complete list of the requirements, refer to the corresponding Boost.Asio documentation section available at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/SyncReadStream.html. The object of the asio::ip::tcp::socket class that represents a TCP socket satisfies these requirements and, therefore, can be used as the first argument of the function. The second argument named buffers represents a buffer (simple or composite) to which the data will be read from the socket.

In contrast to the socket's read_some() method, which reads some amount of data from the socket to the buffer, the asio::read() function, during a single call, reads data from the socket until the buffer passed to it as an argument is filled or an error occurs. This simplifies reading from the socket and makes the code shorter and cleaner.

This is how our readFromSocket() function from the previous sample would look like if we used the asio::read() function instead of the socket object's read_some() method to read data from the socket:
```
std::string readFromSocketEnhanced(asio::ip::tcp::socket& sock) {
  const unsigned char MESSAGE_SIZE = 7;
  char buf[MESSAGE_SIZE];

  asio::read(sock, asio::buffer(buf, MESSAGE_SIZE));

  return std::string(buf, MESSAGE_SIZE);
}
```

In the preceding sample, a call to the asio::read() function will block the thread of execution until exactly 7 bytes are read or an error occurs. The benefits of this approach over the socket's read_some() method are obvious.

**The asio::read_until() function**
The asio::read_until() function provides a way to read data from a socket until a specified pattern is encountered in the data. There are eight overloads of this function. Let's consider one of them:
```
template<
    typename SyncReadStream,
    typename Allocator>
std::size_t read_until(
    SyncReadStream & s,
    boost::asio::basic_streambuf< Allocator > & b,
    char delim);
```
This function accepts three arguments. The first of them named s is a reference to an object that satisfies the requirements of the SyncReadStream concept. For a complete list of the requirements, refer to the corresponding Boost.Asio documentation section at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/SyncReadStream.html. The object of the asio::ip::tcp::socket class that represents a TCP socket satisfies these requirements and, therefore, can be used as the first argument of the function.

The second argument named b represents a stream-oriented extensible buffer in which the data will be read. The last argument named delim specifies a delimiter character.

The asio::read_until() function will read data from the s socket to the buffer b until it encounters a character specified by the delim argument in the read portion of the data. When the specified character is encountered, the function returns.

It's important to note that the asio::read_until() function is implemented so that it reads the data from the socket by blocks of variable sizes (internally it uses the socket's read_some() method to read the data). When the function returns, the buffer b may contain some symbols after the delimiter symbol. This may happen if the remote application sends some more data after the delimiter symbol (for example, it may send two messages in a row, each having a delimiter symbol in the end). In other words, when the asio::read_until() function returns successfully, it is guaranteed that the buffer b contains at least one delimiter symbol but may contain more. It is the developer's responsibility to parse the data in the buffer and handle the situation when it contains data after the delimiter symbol.

This is how we will implement our readFromSocket() function if we want to read all the data from a socket until a specific symbol is encountered. Let's assume the message delimiter to be a new line ASCII symbol, \n:
```
std::string readFromSocketDelim(asio::ip::tcp::socket& sock) {
  asio::streambuf buf;

  // Synchronously read data from the socket until
  // '\n' symbol is encountered.  
  asio::read_until(sock, buf, '\n');

  std::string message;

  // Because buffer 'buf' may contain some other data
  // after '\n' symbol, we have to parse the buffer and
  // extract only symbols before the delimiter. 
  
  std::istream input_stream(&buf);
  std::getline(input_stream, message);
  return message;
}
```
This example is quite simple and straightforward. Because buf may contain more symbols after the delimiter symbol, we use the std::getline() function to extract the messages of interest before the delimiter symbol and put them into the message string object, which is then returned to the caller.

**The asio::read_at() function**
The asio::read_at() function provides a way to read data from a socket, starting at a particular offset. Because this function is rarely used, it is beyond the scope of this book. Refer to the corresponding Boost.Asio documentation section for more details about this function and its overloads at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/read_at.html.

The asio::read(), asio::read_until(), and asio::read_at() functions are implemented in a similar way to how the original readFromSocket() function in our sample is implemented by means of several calls to the socket object's read_some() method in a loop until the termination condition is satisfied or an error occurs.

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
