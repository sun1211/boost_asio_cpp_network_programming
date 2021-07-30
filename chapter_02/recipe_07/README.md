# Canceling asynchronous operations

Sometimes, after an asynchronous operation has been initiated and has not yet completed, the conditions in the application may change so that the initiated operation becomes irrelevant or outdated and nobody is interested in the completion of the operation.

In addition to this, if an initiated asynchronous operation is a reaction to a user command, the user may change their mind while the operation is being executed. The user may want to discard the previous issued command and may want to issue a different one or decide to exit from the application.

Consider a situation where a user types a website address in a typical web browser's address bar and hits the Enter key. The browser immediately initiates a DNS name resolution operation. When the DNS name is resolved and the corresponding IP address is obtained, it initiates the connection operation to connect to the corresponding web server. When a connection is established, the browser initiates an asynchronous write operation to send a request to the server. Finally, when the request is sent, the browser starts waiting for the response message. Depending on the responsiveness of the server application, the volume of the data transmitted over the network, the state of the network, and other factors, all these operations may take a substantial amount of time. And the user while waiting for the requested web page to be loaded, may change their mind, and before the page gets loaded, the user may type another website address in the address bar and hit Enter.

Another (extreme) situation is where a client application sends a request to the server application and starts waiting for the response message, but the server application while processing the client's request, gets into a deadlock due to bugs in it. In this case, the user would have to wait forever for the response message and would never get it.

In both the cases, the user of the client application would benefit from having the ability to cancel the operation they initiated before it completes. In general, it is a good practice to provide the user with the ability to cancel an operation that may take a noticeable amount of time. Because the network communication operations fall into a class of operations that may last for unpredictably long periods of time, it is important to support the cancelation of operations in distributed applications that communicate over the network.

The following algorithm provides the steps required to initiate and cancel asynchronous operations with Boost.Asio:

- If the application is intended to run on Windows XP or Windows Server 2003, define flags that enable asynchronous operation canceling on these versions of Windows.
- Allocate and open a TCP or UDP socket. It may be an active or passive (acceptor) socket in the client or server application.
- Define a callback function or functor for an asynchronous operation. If needed, in this callback, implement a branch of code that handles the situation when the operation has been canceled.
- Initiate one or more asynchronous operations and specify a function or an object defined in step 4 as a callback.
- Spawn an additional thread and use it to run the Boost.Asio event loop.
- Call the cancel() method on the socket object to cancel all the outstanding asynchronous operations associated with this socket.

According to step 1, to compile and run our code on Windows XP or Windows Server 2003, we need to define some flags that control the behavior of the Boost.Asio library with regard to which mechanisms of the underlying OS to exploit.

By default, when it is compiled for Windows, Boost.Asio uses the I/O completion port framework to run operations asynchronously. On Windows XP and Windows Server 2003, this framework has some issues and limitations with regard to the cancelation of an operation. Therefore, Boost.Asio requires developers to explicitly notify that they want to enable the asynchronous operation canceling functionality despite of the known issues, when targeting the application in versions of Windows in question. To do this, the BOOST_ASIO_ENABLE_CANCELIO macro must be defined before Boost.Asio headers are included. Otherwise, if this macro is not defined, while the source code of the application contains calls to asynchronous operations, cancelation methods and functions, the compilation will always fail.

In other words, it is mandatory to define the BOOST_ASIO_ENABLE_CANCELIO macro, when targeting Windows XP or Windows Server 2003, and the application needs to cancel asynchronous operations.

To get rid of issues and limitations imposed by the usage of the I/O completion port framework on Windows XP and Windows Server 2003, we can prevent Boost.Asio from using this framework by defining another macro named BOOST_ASIO_DISABLE_IOCP before including Boost.Asio headers. With this macro defined, Boost.Asio doesn't use the I/O completion port framework on Windows; and therefore, problems related to asynchronous operations canceling disappear. However, the benefits of scalability and efficiency of the I/O completion ports framework disappear too.

Note that the mentioned issues and limitations related to asynchronous operation canceling do not exist on Windows Vista and Windows Server 2008 and later. Therefore, when targeting these versions of Windows, canceling works fine, and there is no need to disable the I/O completion port framework usage unless there is another reason to do so. Refer to the asio::ip::tcp::cancel() method's documentation section for more details on this issue at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/basic_stream_socket/cancel/overload1.html.

In our sample, we will consider how to construct a cross-platform application that, when targeted at Windows during compilation, can be run on any Windows version, starting from Windows XP or Windows Server 2003. Therefore, we define both the BOOST_ASIO_DISABLE_IOCP and BOOST_ASIO_ENABLE_CANCELIO macros.

To determine the target operating system at compile time, we use the Boost.Predef library. This library provides us with macro definitions that allow us to identify parameters of the environment in which the code is compiled as the target operating system family and its version, processor architecture, compiler, and many others. Refer to the Boost.Asio documentation section for more details on this library at http://www.boost.org/doc/libs/1_58_0/libs/predef/doc/html/index.html.

Then, we check whether the code is being compiled for Windows XP or Windows Server 2003, and if it is, we define the BOOST_ASIO_DISABLE_IOCP and BOOST_ASIO_ENABLE_CANCELIO macros:
```
#ifdef BOOST_OS_WINDOWS
#define _WIN32_WINNT 0x0501

#if _WIN32_WINNT <= 0x0502 // Windows Server 2003 or earlier.
#define BOOST_ASIO_DISABLE_IOCP
#define BOOST_ASIO_ENABLE_CANCELIO  
#endif
#endif
```

Next, we include the common Boost.Asio header and standard library <thread> header. We will need the latter because we'll spawn additional threads in our application. In addition to this, we specify a using directive to make the names of Boost.Asio classes and functions shorter and more convenient to use:
```
#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using namespace boost;
```

Then, we define the application's main() entry point function, which contains all the functionalities of the application

# How it works
Our sample client application consists of a single function, which is the application's main() entry point function. This function begins with allocating and opening a TCP socket according to step 2 of the algorithm.

Next, the asynchronous connection operation is initiated on the socket. The callback provided to the method is implemented as a lambda function. This corresponds to steps 3 and 4 of the algorithm. Note how the fact that the operation was canceled is determined in the callback function. When an asynchronous operation is canceled, the callback is invoked and its argument that specifies the error code contains an OS dependent error code defined in Boost.Asio as asio::error::operation_aborted.

Then, we spawn a thread named worker_thread, which will be used to run the Boost.Asio event loop. In the context of this thread, the callback function will be invoked by the library. The entry point function of the worker_thread thread is quite simple. It contains a try-catch block and a call to the asio::io_service object's run() method. This corresponds to step 5 of the algorithm.

After the worker thread is spawned, the main thread is put to sleep for 2 seconds. This is to allow the connection operation to progress a bit and emulate what could be a delay between the two commands issued by the user in the real application; for example, a web browser.

According to the last step 6 of the algorithm, we call the socket object's cancel() method to cancel the initiated connection operation. At this point, if the operation has not yet finished, it will be canceled and the corresponding callback will be invoked with an argument that specifies the error code containing the asio::error::operation_aborted value to notify that the operation was canceled. However, if the operation has already finished, calling the cancel() method has no effect.

When the callback function returns, the worker thread exits the event loop because there are no more outstanding asynchronous operations to be executed. As a result, the thread exits its entry point function. This leads to the main thread running to its completion as well. Eventually, the application exits.

In the previous sample, we considered the canceling of an asynchronous connection operation associated with an active TCP socket. However, any operation associated with both the TCP and UDP sockets can be canceled in a similar way. The cancel() method should be called on the corresponding socket object after the operation has been initiated.

In addition to this, the async_resolve() method of the asio::ip::tcp::resolver or asio::ip::udp::resolver class used to asynchronously resolve a DNS name can be canceled by calling the resolver object's cancel() method.

All asynchronous operations initiated by the corresponding free functions provided by Boost.Asio can be canceled as well by calling the cancel() method on an object that was passed to the free function as the first argument. This object can represent either a socket (active or passive) or a resolver.

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
