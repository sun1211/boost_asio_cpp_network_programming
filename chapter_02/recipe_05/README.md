# Writing to a TCP socket asynchronously

Asynchronous writing is a flexible and efficient way to send data to a remote application. 

The most basic tool used to asynchronously write data to the socket provided by the Boost.Asio library is the async_write_some() method of the asio::ip::tcp::socket class. Let's take a look at one of the method's overloads:
```
template<
    typename ConstBufferSequence,
    typename WriteHandler>
void async_write_some(
    const ConstBufferSequence & buffers,
    WriteHandler handler);
```

This method initiates the write operation and returns immediately. It accepts an object that represents a buffer that contains the data to be written to the socket as its first argument. The second argument is a callback, which will be called by Boost.Asio when an initiated operation is completed. This argument can be a function pointer, functor, or any other object that satisfies the requirements of the WriteHandler concept. The complete list of the requirements can be found in the corresponding section of the Boost.Asio documentation at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/WriteHandler.html.

The callback should have the following signature:
```
void write_handler(
    const boost::system::error_code& ec,
    std::size_t bytes_transferred);
```

Here, ec is an argument that indicates an error code if one occurs, and the bytes_transferred argument indicates how many bytes have been written to the socket during the corresponding asynchronous operation.

As the async_write_some() method's name suggests, it initiates an operation that is intended to write some amount of data from the buffer to the socket. This method guarantees that at least one byte will be written during the corresponding asynchronous operation if an error does not occur. This means that, in a general case, in order to write all the data available in the buffer to the socket, we may need to perform this asynchronous operation several times.

The following algorithm describes the steps required to perform and implement an application, which writes data to a TCP socket asynchronously. Note that this algorithm provides a possible way to implement such an application. Boost.Asio is quite flexible and allows us to organize and structure the application by writing data to a socket asynchronously in many different ways:

- Define a data structure that contains a pointer to a socket object, a buffer, and a variable used as a counter of bytes written.
- Define a callback function that will be called when the asynchronous writing operation is completed.
- In a client application, allocate and open an active TCP socket and connect it to a remote application. In a server application, obtain a connected active TCP socket by accepting a connection request.
- Allocate a buffer and fill it with data that is to be written to the socket.
- Initiate an asynchronous writing operation by calling the socket's async_write_some() method. Specify a function defined in step 2 as a callback.
- Call the run() method on an object of the asio::io_service class.
- In a callback, increase the counter of bytes written. If the number of bytes written is less than the total amount of bytes to be written, initiate a new asynchronous writing operation to write the next portion of the data.

# How it works
The application is run by a single thread, in the context of which the application's main() entry point function is called. Note that Boost.Asio may create additional threads for some internal operations, but it guarantees that no application code is executed in the context of those threads.

The main() function allocates, opens, and synchronously connects a socket to a remote application and then calls the writeToSocket() function by passing a pointer to the socket object. This function initiates an asynchronous write operation and returns. We'll consider this function in a moment. The main() function continues with calling the run() method on the object of the asio::io_service class, where Boost.Asio captures the thread of execution and uses it to call the callback functions associated with asynchronous operations when they get completed.

The asio::os_service::run() method blocks, as long as, at least one pending asynchronous operation. When the last callback of the last pending asynchronous operation is completed, this method returns.

Now, let's come back to the writeToSocket() function and analyze its behavior. It begins with allocating an instance of the Session data structure in the free memory. Then, it allocates and fills the buffer with the data to be written to the socket. After this, a pointer to the socket object and the buffer are stored in the Session object. Because the socket's async_write_some() method may not write all the data to the socket in one go, we may need to initiate another asynchronous write operation in a callback function. That's why we need the Session object and we allocate it in the free memory and not on the stack; it must live until the callback function is called.

Finally, we initiate the asynchronous operation, calling the socket object's async_write_some() method. The invocation of this method is somewhat complex, and, therefore, let's consider this in more detail:
```
s->sock->async_write_some(
  asio::buffer(s->buf),
  std::bind(callback,
     std::placeholders::_1,
std::placeholders::_2, 
s));
```

The first argument is a buffer that contains data to be written to the socket. Because the operation is asynchronous, this buffer may be accessed by Boost.Asio at any moment between operation initiation and when the callback is called. This means that the buffer must stay intact and must be available until the callback is called. We guarantee this by storing the buffer in a Session object, which in turn is stored in the free memory.

The second argument is a callback that is to be invoked when the asynchronous operation is completed. Boost.Asio defines a callback as a concept, which can be a function or a functor, that accepts two arguments. The first argument of the callback specifies an error that occurs while the operation is being executed, if any. The second argument specifies the number of bytes written by the operation.

Because we want to pass an additional argument to our callback function, a pointer to the corresponding Session object, which acts as a context for the operation, we use the std::bind() function to construct a function object to which we attach a pointer to the Session object as the third argument. The function object is then passed as a callback argument to the socket object's async_write_some() method.

Because it is asynchronous, the async_write_some() method doesn't block the thread of execution. It initiates the writing operation and returns.

The actual writing operation is executed behind the scenes by the Boost.Asio library and underlying operating system, and when the operation is complete or an error occurs, the callback is invoked.

When invoked, the callback function named, literally, callback in our sample application begins with checking whether the operation succeeded or an error occurred. In the latter case, the error information is output to the standard output stream and the function returns. Otherwise, the counter of the total written bytes is increased by the number of bytes written as a result of an operation. Then, we check whether the total number of bytes written to the socket is equal to the size of the buffer. If these values are equal, this means that all the data has been written to the socket and there is no more work to do. The callback function returns. However, if there is still data in the buffer that is to be written, a new asynchronous write operation is initiated:
```
s->sock->async_write_some(
asio::buffer(
s->buf.c_str() + 
s->total_bytes_written, 
s->buf.length() – 
s->total_bytes_written),
std::bind(callback, std::placeholders::_1,
std::placeholders::_2, s));
```

Note how the beginning of the buffer is shifted by the number of bytes already written, and how the size of the buffer is decreased by the same value, correspondingly.

As a callback, we specify the same callback() function using the std::bind() function to attach an additional argument—the Session object, just like we did when we initiated the first asynchronous operation.

The cycles of initiation of an asynchronous writing operation and consequent callback invocation repeat until all the data from the buffer is written to the socket or an error occurs.

When the callback function returns without initiating a new asynchronous operation, the asio::io_service::run() method, called in the main() function, unblocks the thread of execution and returns. The main() function returns as well. This is when the application exits.

Although the async_write_some() method described in the previous sample allows asynchronously writing data to the socket, the solution based on it is somewhat complex and error-prone. Fortunately, Boost.Asio provides a more convenient way to asynchronously write data to a socket using the free function asio::async_write(). Let's consider one of its overloads:
```
template<
    typename AsyncWriteStream,
    typename ConstBufferSequence,
    typename WriteHandler>
void async_write(
    AsyncWriteStream & s,
    const ConstBufferSequence & buffers,
    WriteHandler handler);
```

This function is very similar to the socket's async_write_some() method. Its first argument is an object that satisfies the requirements of the AsyncWriteStream concept. For the complete list of the requirements, refer to the corresponding Boost.Asio documentation section at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/AsyncWriteStream.html. The object of the asio::ip::tcp::socket class satisfies these requirements and, therefore, can be used with this function.

The second and the third arguments of the asio::async_write() function are similar to the first and second arguments of the async_write_some() method of a TCP socket object described in the previous sample. These arguments are buffers that contain data that is to be written and functions or objects that represent a callback, which will be called when the operation is completed.

In contrast to the socket's async_write_some() method, which initiates the operation that writes some amount of data from the buffer to the socket, the asio::async_write() function initiates the operation, which writes all the data available in the buffer. In this case, the callback is called only when all the data available in the buffer is written to the socket or when an error occurs. This simplifies writing to the socket and makes the code shorter and cleaner.

If we change our previous sample so that it uses the asio::async_write() function instead of the socket object's async_write_some() method to write data to the socket asynchronously, our application becomes significantly simpler.

Firstly, we don't need to keep track of the number of bytes written to the socket, so therefore, the Session structure becomes smaller:
```
struct Session {
  std::shared_ptr<asio::ip::tcp::socket> sock;
  std::string buf;
}; 
```

Secondly, we know that when the callback function is invoked, it means that either all the data from the buffer has been written to the socket or an error has occurred. This makes the callback function much simpler:
```
void callback(const boost::system::error_code& ec,
  std::size_t bytes_transferred,
  std::shared_ptr<Session> s)
{
  if (ec.value() != 0) {
    std::cout << "Error occured! Error code = "
      << ec.value()
      << ". Message: " << ec.message();

    return;
  }

  // Here we know that all the data has
  // been written to the socket.
}
```

The asio::async_write() function is implemented by means of zero or more calls to the socket object's async_write_some() method. This is similar to how the writeToSocket() function in our initial sample is implemented.

Note that the asio::async_write() function has three more overloads, providing additional functionalities. Some of them may be very useful in specific circumstances. Refer to the Boost.Asio documentation to find out more about this function at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/async_write.html.

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
