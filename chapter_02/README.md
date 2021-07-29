# Chapter 1: Starting to Write Your Application

- [1. Using fixed length I/O buffers](recipe_01/README.md)
- [2. Using extensible stream-oriented I/O buffers](recipe_02/README.md)

I/O operations are the key operations in the networking infrastructure of any distributed application. They are directly involved in the process of data exchange. Input operations are used to receive data from remote applications, whereas output operations allow sending data to them.

## I/O buffers
Network programming is all about organizing inter-process communication over a computer network. Communication in this context implies exchanging data between two or more processes. From the perspective of a process that participates in such communication, the process performs I/O operations, sending data to and receiving it from other participating processes.

Like any other type of I/O, the network I/O involves using memory buffers, which are contiguous blocks of memory allocated in the process's address space used to store the data. When doing any sort of input operation (for example, reading some data from a file, a pipe, or a remote computer over the network), the data arrives at the process and must be stored somewhere in its address space so that it is available for further processing. That is, when the buffer comes in handy. Before performing an input operation, the buffer is allocated and then used as a data destination point during the operation. When the input operation is completed, the buffer contains input data, which can be processed by the application. Likewise, before performing the output operation, the data must be prepared and put into an output buffer, which is then used in the output operation, where it plays the role of the data source.

Apparently, the buffers are essential ingredients of any application that performs any type of I/O, including the network I/O. That's why it is critical for the developer who develops a distributed application to know how to allocate and prepare the I/O buffers to use them in the I/O operations.

## Synchronous and asynchronous I/O operations
Boost.Asio supports two types of I/O operations: **synchronous** and **asynchronous**.
- Synchronous operations block the thread of execution invoking them and unblock only when the operation is finished. Hence, the name of this type of operation: synchronous.

- The second type is an asynchronous operation. When an asynchronous operation is initiated, it is associated with a callback function or functor, which is invoked by the Boost.Asio library when the operation is finished. These types of I/O operations provide great flexibility, but may significantly complicate the code. The initiation of the operation is simple and doesn't block the thread of execution, which allows us to use the thread to run other tasks, while the asynchronous operation is being run in the background.

The Boost.Asio library is implemented as a framework, which exploits an inversion of control approach. After one or more asynchronous operations are initiated, the application hands over one of its threads of execution to the library, and the latter uses this thread to run the event loop and invoke the callbacks provided by the application to notify it about the completion of the previously initiated asynchronous operation. The results of asynchronous operations are passed to the callback as arguments.

## Additional operations
In addition to this, we are going to consider such operations as canceling asynchronous operations, shutting down, and closing a socket.

The ability to cancel a previously initiated asynchronous operation is very important. It allows the application to state that the previously initiated operation is not relevant anymore, which may save the application's resources (both CPU and memory), that otherwise (in case, the operation would continue its execution even after it was known that nobody is interested in it anymore) would be unavoidably wasted.

Shutting down the socket is useful if there is a need for one part of the distributed application to inform the other part that the whole message has been sent, when the application layer protocol does not provide us with other means to indicate the message boundary.

As with any other operating system resource, a socket should be returned back to the operating system when it is not needed anymore by the application. A closing operation allows us to do so.

