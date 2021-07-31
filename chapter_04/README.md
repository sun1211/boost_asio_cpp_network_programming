# Chapter 4: Implementing Server Applications

- [1. Implementing a synchronous iterative TCP server](recipe_01/README.md)
- [2. Implementing a synchronous parallel TCP server](recipe_02/README.md)

A server is a part of a distributed application that provides a service or services that are consumed by other parts of this application—clients. Clients communicate with the server in order to consume services provided by it.

Usually, a server application plays a passive role in the client-server communication process. During start-up, a server application attaches to a particular well-known port (meaning, it is known to the potential clients or at least it can be obtained by the clients at runtime from some well-known registry) on the host machine. After this, it passively waits for incoming requests arriving to that port from the clients. When the request arrives, the server processes it (serves) by performing actions according to the specification of the service it provides.

Depending on the services that particular server provides, the request processing may mean a different thing. An HTTP server, for example, would usually read the content of a file specified in the request message and send it back to the client. A proxy server would simply redirect a client's request to a different server for the actual processing (or maybe for another round of redirection). Other more specific servers may provide services that perform complex computations on the data provided by the client in the request and return results of such computations back to the client.

Not all servers play a passive role. Some server applications may send messages to the clients without waiting for the clients to first send a request. Usually, such servers act as notifiers, and they notify clients of some interesting events. In this case, clients may not need to send any data to the server at all. Instead, they passively wait for notifications from the server and having received one, they react accordingly. Such a communication model is called push-style communication. This model is gaining popularity in modern web applications, providing additional flexibility.

So, the first way to classify a server application is by the function (or functions) they perform or a service (or services) they provide to their clients.

Another obvious classification dimension is the transport layer protocol used by the server to communicate with the clients.

TCP protocol is very popular today and many general purpose server applications use it for communication. Other, more specific servers may use UDP protocol. Hybrid server applications that provide their services through both TCP and UDP protocols at the same time fall under the third category and are called multiprotocol servers. In this chapter, we will consider several types of TCP servers.

One more characteristic of a server is a manner in which it serves clients. An iterative server serves clients in one-by-one fashion, meaning that it does not start serving the next client before it completes serving the one it is currently serving. A parallel server can serve multiple clients in parallel. On a single-processor computer, a parallel server interleaves different stages of communication with several clients running them on a single processor. For example, having connected to one client and while waiting for the request message from it, the server can switch to connecting the second client, or read the request from the third one; after this, it can switch back to the first client to continue serving it. Such parallelism is called pseudo parallelism, as a processor is merely switching between several clients, but does not serve them truly simultaneously, which is impossible with a single processor.

On multiprocessor computers, the true parallelism is possible, when a server serves more than one client at the same time using different hardware threads for each client.

Iterative servers are relatively simple to implement and can be used when the request rate is low enough so that the server has time to finish processing one request before the next one arrives. It is clear that iterative servers are not scalable; adding more processors to the computer running such a server will not increase the server's throughput. Parallel servers, on the other hand, can handle higher request rates; if properly implemented, they are scalable. A truly parallel server running on a multiprocessor computer can handle higher request rates than the same server running on a single processor computer.

Another way to classify server applications, from an implementation's point of view, is according to whether the server is synchronous or asynchronous. A synchronous server uses synchronous socket API calls that block the thread of execution until the requested operation is completed, or else an error occurs. Thus, a typical synchronous TCP server would use methods such as asio::ip::tcp::acceptor::accept() to accept the client connection request, asio::ip::tcp::socket::read_some() to receive the request message from the client, and then asio::ip::tcp::socket::write_some() to send the response message back to the client. All three methods are blocking. They block the thread of execution until the requested operation is completed, or an error occurs, which makes the server using these operations synchronous.

An asynchronous server application, as opposed to the synchronous one, uses asynchronous socket API calls. For example, an asynchronous TCP server may use the asio::ip::tcp::acceptor::async_accept() method to asynchronously accept the client connection request, the asio::ip::tcp::socket::async_read_some() method or the asio::async_read() free function to asynchronously receive the request message from the client, and then the asio::ip::tcp::socket::async_write_some() method or the asio::async_write() free function to asynchronously send a response message back to the client.

Because the structure of a synchronous server application significantly differs from that of an asynchronous one, the decision as to which approach to apply should be made early at the server application design stage, and this decision should be based on the careful analysis of the application requirements. Besides, the possible application evolution paths and new requirements that may appear in the future should be considered and taken into account.

As usually, each approach has its advantages and disadvantages. When a synchronous approach yields better results in one situation, it may be absolutely unacceptable in another; in this case, an asynchronous approach might be the right choice. Let's compare two approaches to better understand the strengths and weaknesses of each of them.

The main advantage of a synchronous approach as compared to an asynchronous one is its simplicity. A synchronous server is significantly easier to implement, debug, and support than a functionally equal asynchronous one. Asynchronous servers are more complex due to the fact that asynchronous operations used by them complete in other places in code than they are initiated. Usually, this requires allocating additional data structures in the free memory to keep the context of the request, implementing callback functions, thread synchronization, and other extras that may make the application structure quite complex and error-prone. Most of these extras are not required in synchronous servers. Besides, an asynchronous approach brings in additional computational and memory overheads, which may make it less efficient than a synchronous one in some situations.

However, a synchronous approach has some functional limitations, which often makes it unacceptable. These limitations consist of the inability to cancel a synchronous operation after it has started, or to assign it a timeout so that it gets interrupted if it is running for too long. As opposed to synchronous operations, asynchronous ones can be canceled at any moment after the operation has been initiated.

The fact that synchronous operations cannot be canceled significantly limits the area of the application of synchronous servers. Publicly available servers that use synchronous operations are vulnerable to the attacks of a culprit. If such a server is single-threaded, a single malicious client is enough to block the server, not allowing other clients to communicate with it. Malicious client used by a culprit connects to the server and does not send any data to it, while the latter is blocked in one of the synchronous reading functions or methods, which does not allow it to serve other clients.

Such servers would usually be used in safe and protected environments in private networks, or as an internal part of an application running on a single computer using such a server for interprocess communication. Another possible application area of synchronous servers is, of course, the implementation of throwaway prototypes.

Besides the difference in the structural complexity and functionality described above, the two approaches differ in the efficiency and scalability when it comes to serving large numbers of clients sending requests at high rates. Servers using asynchronous operations are more efficient and scalable than synchronous servers especially when they run on multiprocessor computers with operating systems natively supporting an asynchronous network I/O.

## The sample protocol
we are going to consider three recipes describing how to implement the synchronous iterative TCP server, synchronous parallel TCP server, and asynchronous TCP server. In all the recipes, it is assumed that the server communicates with clients using the following intentionally trivialized (for the sake of clarity) application layer protocol.

A server application accepts request messages represented as ASCII strings containing a sequence of symbols ending with a new-line ASCII symbol. All the symbols coming after the new-line symbol are ignored by the server.

Having received a request, the server performs some dummy operations and replies with a constant message as follows:
```
"Response\n"
```
Such a trivial protocol allows us to concentrate on the implementation of the server and not the service provided by it.