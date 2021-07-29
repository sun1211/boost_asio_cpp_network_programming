# Writing to a TCP socket synchronously

Writing to a TCP socket is an output operation that is used to send data to the remote application connected to this socket. Synchronous writing is the simplest way to send the data using a socket provided by Boost.Asio. The methods and functions that perform synchronous writing to the socket block the thread of execution and do not return until the data (at least some amount of data) is written to the socket or an error occurs.

The most basic way to write to the socket provided by the Boost.Asio library is to use the write_some() method of the asio::ip::tcp::socket class. Here is the declaration of one of the method's overloads:
```
template<
typename ConstBufferSequence>
std::size_t write_some(
const ConstBufferSequence & buffers);
```

This method accepts an object that represents a composite buffer as an argument, and as its name suggests, writes some amount of data from the buffer to the socket. If the method succeeds, the return value indicates the number of bytes written. The point to emphasize here is that the method may not send all the data provided to it through the buffers argument. The method only guarantees that at least one byte will be written if an error does not occur. This means that, in a general case, in order to write all the data from the buffer to the socket, we may need to call this method several times.

The following algorithm describes the steps required to synchronously write data to a TCP socket in a distributed application:

- In a client application, allocate, open, and connect an active TCP socket. In a server application, obtain a connected active TCP socket by accepting a connection request using an acceptor socket.
- Allocate the buffer and fill it with data that is to be written to the socket.
- In a loop, call the socket's write_some() method as many times as it is needed to send all the data available in the buffer.

# How it works
The main() application entry point function begins with instantiating an object of the asio::streambuf class named buf. Next, the output stream object of the std::ostream class is instantiated. The buf object is used as a stream buffer for the output stream.

In the next line, the Message1\nMessage2 sample data string is written to the output stream object, which in turn redirects the data to the buf stream buffer.

Usually, in a typical client or server application, the data will be written to the buf stream buffer by the Boost.Asio input function such as asio::read(), which accepts a stream buffer object as an argument and reads data from the socket to that buffer.

Now, we want to read the data back from the stream buffer. To do this, we allocate an input stream and pass the buf object as a stream buffer argument to its constructor. After this, we allocate a string object named message1, and then, use the std::getline function to read part of the string currently stored in the buf stream buffer until the delimiter symbol, \n.

As a result, the string1 object contains the Message1 string and the buf stream buffer contains the rest of the initial string after the delimiter symbol, that is, Message2.

The main()application entry point function is quite simple. It allocates a socket, opens, and synchronously connects it to a remote application. Then, the writeToSocket() function is called and the socket object is passed to it as an argument. In addition to this, the main()function contains a try-catch block intended to catch and handle exceptions that may be thrown by Boost.Asio methods and functions.

The interesting part in the sample is the writeToSocket()function that performs synchronous writing to the socket. It accepts a reference to the socket object as an argument. Its precondition is that the socket passed to it is already connected; otherwise, the function fails.

The function begins with allocating and filling the buffer. In this sample, we use an ASCII string as data that is to be written to the socket, and, therefore, we allocate an object of the std::string class and assign it a value of Hello, which we will use as a dummy message that will be written to the socket.

Then, the variable named total_bytes_written is defined and its value is set to 0. This variable is used as a counter that stores the count of bytes already written to the socket.

Next, the loop is run in which the socket's write_some() method is called. Except for the degenerate case when the buffer is empty (that is, the buf.length() method returns a value of 0), at least one iteration of the loop is executed and the write_some() method is called at least once. Let's take a closer look at the loop:
```
 while (total_bytes_written != buf.length()) {
    total_bytes_written += sock.write_some(
      asio::buffer(buf.c_str() +
      total_bytes_written,
      buf.length() - total_bytes_written));
  }
```

The termination condition evaluates to true when the value of the total_bytes_written variable is equal to the size of the buffer, that is, when all the bytes available in the buffer have been written to the socket. In each iteration of the loop, the value of the total_bytes_written variable is increased by the value returned by the write_some() method, which is equal to the number of bytes written during this method call.

Each time the write_some() method is called, the argument passed to it is adjusted. The start byte of the buffer is shifted by the value of total_bytes_written as compared to the original buffer (because the previous bytes have already been sent by preceding calls to the write_some() method) and the size of the buffer is decreased by the same value, correspondingly.

After the loop terminates, all the data from the buffer is written to the socket and the writeToSocket() function returns.

It's worth noting that the amount of bytes written to the socket during a single call to the write_some() method depends on several factors. In the general case, it is not known to the developer; and therefore, it should not be accounted for. A demonstrated solution is independent of this value and calls the write_some() method as many times as needed to write all the data available in the buffer to the socket.

## Alternative – the send() method
The asio::ip::tcp::socket class contains another method to synchronously write data to the socket named send(). There are three overloads of this method. One of them is equivalent to the write_some() method, as described earlier. It has exactly the same signature and provides exactly the same functionality. These methods are synonyms in a sense.

The second overload accepts one additional argument as compared to the write_some() method. Let's take a look at it:
```
template<
typename ConstBufferSequence>
std::size_t send(
    const ConstBufferSequence & buffers,
    socket_base::message_flags flags);
```
This additional argument is named flags. It can be used to specify a bit mask, representing flags that control the operation. Because these flags are used quite rarely, we won't consider them in this book. Refer to the Boost.Asio documentation to find out more information on this topic.

The third overload is equivalent to the second one, but it doesn't throw exceptions in case of a failure. Instead, the error information is returned by means of an additional method's output argument of the boost::system::error_code type.

Writing to a socket using the socket's write_some() method seems very complex for such a simple operation. Even if we want to send a small message that consists of several bytes, we must use a loop, a variable to keep track of how many bytes have already been written, and properly construct a buffer for each iteration of the loop. This approach is error-prone and makes the code more difficult to understand.

Fortunately, Boost.Asio provides a free function, which simplifies writing to a socket. This function is called asio::write(). Let's take a look at one of its overloads:
```
template<
    typename SyncWriteStream,
    typename ConstBufferSequence>
std::size_t write(
    SyncWriteStream & s,
    const ConstBufferSequence & buffers);
```

This function accepts two arguments. The first of them named s is a reference to an object that satisfies the requirements of the SyncWriteStream concept. For a complete list of the requirements, refer to the corresponding Boost.Asio documentation section at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/SyncWriteStream.html. The object of the asio::ip::tcp::socket class that represents a TCP socket satisfies these requirements and, therefore, can be used as the first argument of the function. The second argument named buffers represents the buffer (simple or composite) and contains data that is to be written to the socket.

In contrast to the socket object's write_some() method, which writes some amount of data from the buffer to the socket, the asio::write() function writes all the data available in the buffer. This simplifies writing to the socket and makes the code shorter and cleaner.

This is how our writeToSocket() function from a previous sample would look like if we used the asio::write() function instead of the socket object's write_some() method to write data to the socket:
```
void writeToSocketEnhanced(asio::ip::tcp::socket& sock) {
  // Allocating and filling the buffer.
  std::string buf = "Hello";

  // Write whole buffer to the socket.
  asio::write(sock, asio::buffer(buf));
}
```
The asio::write() function is implemented in a similar way as the original writeToSocket() function is implemented by means of several calls to the socket object's write_some() method in a loop.

Note that the asio::write() function has seven more overloads on the top of the one we just considered. Some of them may be very useful in specific cases. Refer to the Boost.Asio documentation to find out more about this function at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/write.html.


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
