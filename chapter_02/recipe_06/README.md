# Reading from a TCP socket asynchronously
Asynchronous reading is a flexible and efficient way to receive data from a remote application.

The most basic tool used to asynchronously read data from a TCP socket provided by the Boost.Asio library is the async_read_some() method of the asio::ip::tcp::socket class. Here is one of the method's overloads:
```
template<
    typename MutableBufferSequence,
    typename ReadHandler>
void async_read_some(
    const MutableBufferSequence & buffers,
    ReadHandler handler);
```

This method initiates an asynchronous read operation and returns immediately. It accepts an object that represents a mutable buffer as its first argument to which the data will be read from the socket. The second argument is a callback that is called by Boost.Asio when the operation is completed. This argument can be a function pointer, a functor, or any other object that satisfies the requirements of the ReadHandler concept. The complete list of the requirements can be found in the corresponding section of the Boost.Asio documentation at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/ReadHandler.html.

The callback should have the following signature:
```
void read_handler(
    const boost::system::error_code& ec,
    std::size_t bytes_transferred);
```
Here, ec is an argument that notifies an error code if one occurs, and the bytes_transferred argument indicates how many bytes have been read from the socket during the corresponding asynchronous operation.

As the async_read_some() method's name suggests, it initiates an operation that is intended to read some amount of data from the socket to the buffer. This method guarantees that at least one byte will be read during the corresponding asynchronous operation if an error does not occur. This means that, in a general case, in order to read all the data from the socket, we may need to perform this asynchronous operation several times.

Now that we know how the key method works, let's see how to implement an application that performs asynchronous reading from the socket.

The following algorithm describes the steps required to implement an application, which reads data from a socket asynchronously. Note that this algorithm provides a possible way to implement such an application. Boost.Asio is quite flexible and allows us to organize and structure the application by reading data from a socket asynchronously in different ways:

- Define a data structure that contains a pointer to a socket object, a buffer, a variable that defines the size of the buffer, and a variable used as a counter of bytes read.
- Define a callback function that will be called when an asynchronous reading operation is completed.
- In a client application, allocate and open an active TCP socket, and then, connect it to a remote application. In a server application, obtain a connected active TCP socket by accepting a connection request.
- Allocate a buffer big enough for the expected message to fit in.
- Initiate an asynchronous reading operation by calling the socket's async_read_some() method, specifying a function defined in step 2 as a callback.
- Call the run() method on an object of the asio::io_service class.
- In a callback, increase the counter of bytes read. If the number of bytes read is less than the total amount of bytes to be read (that is, the size of an expected message), initiate a new asynchronous reading operation to read the next portion of data.

The application is run by a single thread; in the context of which the application's main() entry point function is called. Note that Boost.Asio may create additional threads for some internal operations, but it guarantees that no application code is called in the context of those threads.

The main() function begins with allocating, opening, and connecting a socket to a remote application. Then, it calls the readFromSocket() function and passes a pointer to the socket object as an argument. The readFromSocket() function initiates an asynchronous reading operation and returns. We'll consider this function in a moment. The main() function continues with calling the run() method on the object of the asio::io_service class, where Boost.Asio captures the thread of execution and uses it to call the callback functions associated with asynchronous operations when they get completed.

The asio::io_service::run() method blocks as long as there is at least one pending asynchronous operation. When the last callback of the last pending operation is completed, this method returns.

Now, let's come back to the readFromSocket() function and analyze its behavior. It begins with allocating an instance of the Session data structure in the free memory. Then, it allocates a buffer and stores a pointer to it in a previously allocated instance of the Session data structure. A pointer to the socket object and the size of the buffer are stored in the Session data structure as well. Because the socket's async_read_some() method may not read all the data in one go, we may need to initiate another asynchronous reading operation in the callback function. This is why we need the Session data structure and why we allocate it in the free memory and not on a stack. This structure and all the objects that reside in it must live at least until the callback is invoked.

Finally, we initiate the asynchronous operation, calling the socket object's async_read_some() method. The invocation of this method is somewhat complex; therefore, let's take a look at it in more detail:
```
s->sock->async_read_some(
  asio::buffer(s->buf.get(), s->buf_size),
  std::bind(callback,
    std::placeholders::_1,
    std::placeholders::_2,
    s));
```
The first argument is the buffer to which the data will be read. Because the operation is asynchronous, this buffer may be accessed by Boost.Asio at any moment between the operation initiation and when the callback is invoked. This means that the buffer must stay intact and be available until the callback is invoked. We guarantee this by allocating the buffer in the free memory and storing it in the Session data structure, which in turn is allocated in the free memory as well.

The second argument is a callback that is to be invoked when the asynchronous operation is completed. Boost.Asio defines a callback as a concept, which can be a function or a functor, that accepts two arguments. The first argument of the callback specifies an error that occurs while the operation is being executed, if any. The second argument specifies the number of bytes read by the operation.

Because we want to pass an additional argument to our callback function, a pointer to the corresponding Session object, which serves as a context for the operation—we use the std::bind() function to construct a function object to which we attach a pointer to the Session object as the third argument. The function object is then passed as a callback argument to the socket object's async_write_some() method.

Because it is asynchronous, the async_write_some() method doesn't block the thread of execution. It initiates the reading operation and returns.

The actual reading operation is executed behind the scenes by the Boost.Asio library and underlying operating system, and when the operation is completed or an error occurs, the callback is invoked.

When invoked, the callback function named, literally, callback in our sample application begins with checking whether the operation succeeded or an error occurred. In the latter case, the error information is output to the standard output stream and the function returns. Otherwise, the counter of the total read bytes is increased by the number of bytes read as a result of the operation. Then, we check whether the total number of bytes read from the socket is equal to the size of the buffer. If these values are equal, it means that the buffer is full and there is no more work to do. The callback function returns. However, if there is still some space in the buffer, we need to continue with reading; therefore, we initiate a new asynchronous reading operation:

```
s->sock->async_read_some(
    asio::buffer(s->buf.get(), s->buf_size),
    std::bind(callback,
      std::placeholders::_1,
      std::placeholders::_2,
      s));
```
Note that the beginning of the buffer is shifted by the number of bytes already read and the size of the buffer is decreased by the same value, respectively.

As a callback, we specify the same callback function using the std::bind() function to attach an additional argument—the Session object.

The cycles of initiation of an asynchronous reading operation and consequent callback invocation repeat until the buffer is full or an error occurs.

When the callback function returns without initiating a new asynchronous operation, the asio::io_service::run() method, called in the main() function, unblocks the thread of execution and returns. The main() function returns as well. This is when the application exits.

Although the async_read_some() method, as described in the previous sample, allows asynchronously reading data from the socket, the solution based on it is somewhat complex and error-prone. Fortunately, Boost.Asio provides a more convenient way to asynchronously read data from a socket: the free function asio::async_read(). Let's consider one of its overloads:
```
template<
    typename AsyncReadStream,
    typename MutableBufferSequence,
    typename ReadHandler>
void async_read(
    AsyncReadStream & s,
    const MutableBufferSequence & buffers,
    ReadHandler handler);
```
This function is very similar to the socket's async_read_some() method. Its first argument is an object that satisfies the requirements of the AsyncReadStream concept. For the complete list of the requirements, refer to the corresponding Boost.Asio documentation section at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/AsyncReadStream.html. The object of the asio::ip::tcp::socket class satisfies these requirements and, therefore, can be used with this function.

The second and third arguments of the asio::async_read() function are similar to the first and second arguments of the async_read_some() method of a TCP socket object described in the previous sample. These arguments are buffers used as data destination points and functions or objects that represent a callback, which will be called when the operation is completed.

In contrast to the socket's async_read_some() method, which initiates the operation, that reads some amount of data from the socket to the buffer, the asio::async_read() function initiates the operation that reads the data from the socket until the buffer passed to it as an argument is full. In this case, the callback is called when the amount of data read is equal to the size of the provided buffer or when an error occurs. This simplifies reading from the socket and makes the code shorter and cleaner.

If we change our previous sample so that it uses the asio::async_read() function instead of the socket object's async_read_some() method to read data from the socket asynchronously, our application becomes significantly simpler.

Firstly, we don't need to keep track of the number of bytes read from the socket; therefore, the Session structure becomes smaller:
```
struct Session {
  std::shared_ptr<asio::ip::tcp::socket> sock;
  std::unique_ptr<char[]> buf;
  unsigned int buf_size;
}; 
```

Secondly, we know that when the callback function is invoked, it means that either an expected amount of data has been read from the socket or an error has occurred. This makes the callback function much simpler:
```
void callback(const boost::system::error_code& ec,
  std::size_t bytes_transferred,
  std::shared_ptr<Session> s)
{
  if (ec != 0) {
    std::cout << "Error occured! Error code = "
      << ec.value()
      << ". Message: " << ec.message();

    return;
  }

  // Here we know that the reading has completed
  // successfully and the buffer is full with
  // data read from the socket.
}
```
The asio::async_read() function is implemented by means of zero or more calls to the socket object's async_read_some() method. This is similar to how the readFromSocket() function in our initial sample is implemented.

Note that the asio::async_read() function has three more overloads, providing additional functionalities. Some of them may be very useful in specific circumstances. Refer to the Boost.Asio documentation to find out about this at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/async_read.html.

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
