# Chapter 3: Implementing Client Applications

- [1. Implementing a synchronous TCP client](recipe_01/README.md)
- [2. Implementing a synchronous UDP client](recipe_02/README.md)
- [3. Implementing an asynchronous TCP client](recipe_03/README.md)

A client is a part of a distributed application that communicates with another part of this application called a server, in order to consume services it provides. The server, on the other hand, is a part of distributed application that passively waits for requests arriving from clients. When a request arrives, the server performs the requested operation and sends a response—the result of the operation—back to the client.

The key characteristic of a client is that it needs a service provided by the server and it initiates the communication session with that server in order to consume the service. The key characteristic of the server is that it serves the requests coming from the clients by providing a requested service.

## The classification of client applications

Client applications can be classified by the transport layer protocol they use for communication with the server. If the client uses a UDP protocol, it is called a UDP client. If it uses a TCP protocol, it is called a TCP client correspondingly. Of course, there are many other transport layer protocols that client applications may use for communication. Moreover, there are multiprotocol clients that can communicate over several protocols. However, they are beyond the scope of this book. In this chapter, we are going to focus on pure UDP and TCP clients as such, which are the most popular and are the most often used in general purpose software today.

The decision as to which transport layer protocol to choose for communication between the parts of a distributed application should be made at the early stages of the application design based on the application specification. Because TCP and UDP protocols are conceptually different, it may be quite difficult to switch from one of them to another at the later stages of the application development process.

Another way to classify client applications is according to whether the client is synchronous or asynchronous. A synchronous client application uses synchronous socket API calls that block the thread of execution until the requested operation is completed, or an error occurs. Thus, a typical synchronous TCP client would use the asio::ip::tcp::socket::write_some() method or the asio::write() free function to send a request to the sever and then use the asio::ip::tcp::socket::read_some() method or the asio::read() free function to receive a response. These methods and functions are blocking, which makes the client synchronous.

An asynchronous client application as opposed to a synchronous one uses asynchronous socket API calls. For example, an asynchronous TCP client may use the asio::ip::tcp::socket::async_write_some() method or the asio::async_write() free function to send a request to the server and then use the asio::ip::tcp::socket::async_read_some() method or the asio::async_read() free function to asynchronously receive a response.

Because the structure of a synchronous client significantly differs from that of an asynchronous one, the decision as to which approach to apply should be made early at the application design stage, and this decision should be based on the careful analysis of the application requirements. Besides, possible application evolution paths and new requirements that may appear in the future should be considered and taken into account.

## Synchronous versus asynchronous
As usually, each approach has its advantages and disadvantages. When a synchronous approach gives better results in one situation, it may be absolutely unacceptable in another. In the latter case, an asynchronous approach should be used. Let's compare two approaches to better understand when it is more beneficial to use each of them.

The main advantage of a synchronous approach is its simplicity. A synchronous client is significantly easier to develop, debug, and support than a functionally equal asynchronous one. Asynchronous clients are more complex due to the fact that asynchronous operations that are used by them complete in other places in code (mainly in callbacks) than they are initiated. Usually, this requires allocating additional data structures in the free memory to keep the context of the request and callback functions, and also involves thread synchronization and other extras that may make the application structure quite complex and error-prone. Most of these extras are not required in synchronous clients. Besides, the asynchronous approach brings in additional computational and memory overhead, which makes it less efficient than a synchronous one in some conditions.

However, the synchronous approach has some functional limitations, which often make this approach unacceptable. These limitations consist of the inability to cancel a synchronous operation after it has started, or to assign it a timeout so that it gets interrupted if it is running longer than a certain amount of time. As opposed to synchronous operations, asynchronous ones can be canceled at any moment after operation initiation and before the moment it completes.

Imagine a typical modern web browser. A request cancellation is a very important feature of a client application of this kind. After issuing a command to load a particular website, the user may change his or her mind and decide to cancel the command before the page gets loaded. From the user's perspective, it would be quite strange not to be able to cancel the command until the page gets fully loaded. Therefore, this is when a synchronous approach is not a good option.

Besides the difference in the complexity and functionality described above, the two approaches differ in efficiency when it comes to running several requests in parallel.

Imagine that we are developing a web crawler, an application that traverses the pages of websites and processes them in order to extract some interesting information. Given a file with a long list of websites (say several millions), the application should traverse all the pages of each of the sites listed in the file and then process each page. Naturally, one of the key requirements of the application is to perform the task as fast as possible. Provided with these requirements, which approach should we choose, synchronous or asynchronous?

Before we answer this question, let's consider the stages of a request life cycle and their timings from the client application's perspective. Conceptually, the request life cycle consists of five stages as follows:

 - Preparing the request: This stage involves any operations required to prepare a request message. The duration of this step depends on the particular problem the application solves. In our example, this could be reading the next website address from the input file and constructing a string representing a request in accordance with an HTTP protocol.
 - Transmitting a request from the client to the server: This stage assumes the transmission of the request data from the client to the server over the network. The duration of this step does not depend on a client application. It depends on the properties and the current state of the network.
 - Processing the request by the server: The duration of this step depends on the server's properties and its current load. In our example, the server application is a web server and the request processing lies in constructing a requested web page, which may involve I/O operations such as reading files and loading data from a database.
 - Transmitting a response from the server to the client: Like stage 2, this stage also assumes the transmission of the data over the network; however, this time it is in the opposite direction—from the server to the client. The duration of this stage does not depend on the client or the server. It only depends on the properties and the state of the network.
 - Processing the response by the client: The duration of this stage depends on a particular task that the client application is intended to perform. In our example, this could be scanning the web page, extracting interesting information and storing it into a database.

Note that, for the sake of simplicity, we omitted low-level substages such as connection establishment and connection shutdown, which are important when using TCP protocol but don't add a substantial value in our conceptual model of a request life cycle.

As we can see, only in stages 1 and 5 does the client perform some effective job related to the request. Having initiated the transmission of the request data at the end of stage 1, the client has to wait during the next three stages (2, 3, and 4) of the request life cycle before it can receive the response and process it.

Now, with the stages of the request life cycle in mind, let's see what happens when we apply synchronous and asynchronous approaches to implement our sample web crawler.

If we apply a synchronous approach, the thread of execution processing a single request synchronously will be sleeping during stages 2-4 of the request life cycle, and only during stages 1 and 5, will it perform an effective job (for simplicity, we assume that stages 1 and 5 don't include instructions that block the thread). This means that the resource of an operating system, namely a thread, is used inefficiently, because there are number of times when it is simply doing nothing while there is still a lot of work available—millions of other pages to request and process. In this situation, an asynchronous approach seems to be more efficient. With an asynchronous approach, instead of a thread being blocked during stages 2-4 of a request life cycle, it can be effectively used to perform stages 1 or 5 of another request.

Thus, we direct a single thread to process the different stages of different requests (this is called overlapping), which results in the more efficient usage of a thread and consequently increases the overall performance of the application.

However, an asynchronous approach is not always more efficient than a synchronous one. As it has been mentioned, asynchronous operations imply additional computational overheads, which means that the overall duration of an asynchronous operation (from initiation till completion) is somewhat bigger than the equivalent synchronous one. This means that, if the average total duration of stages 2-4 is less than the overhead of the timing asynchronous approach per single request, then a synchronous approach turns out to be more efficient, and therefore may be considered to be the right way to go.

Assessing the total duration of stages 2-4 of the request life cycle and the overhead of the asynchronous approach is usually done experimentally. The duration may significantly vary, and it depends on the properties and the state of the network through which the requests and responses are transmitted and also on the properties and the load level of the server application that serves the request.

## The sample protocol
we are going to consider three recipes, each of which demonstrates how to implement a particular type of a client application: the synchronous UDP client, synchronous TCP client, and asynchronous TCP client. In all the recipes, it is assumed that the client application communicates with the server application using the following simple application-level protocol.

The server application accepts a request represented as an ASCII string. The string has the following format:
```
EMULATE_LONG_COMP_OP [s]<LF>
```

Where [s] is a positive integer value and <LF> is ASCII a new-line symbol.

The server interprets this string as a request to perform a dummy operation that lasts for [s] seconds. For example, a request string may look as follows:
```
"EMULATE_LONG_COMP_OP 10\n"
```

This means that the client sending this request wants the server to perform the dummy operation for 10 seconds and then send a response to it.

Like the request, the response returned by the server is represented by an ASCII string. It may either be OK<LF> if the operation completes successfully or ERROR<LF> if the operation fails.


