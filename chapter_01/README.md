# Chapter 1: Starting to Write Your Application

- [1. Creating an endpoint](recipe_01/README.md)
- [2. Creating an active socket](recipe_02/README.md)
- [3. Creating a passive socket](recipe_03/README.md)
- [4. Resolving a DNS name](recipe_04/README.md)
- [5. Binding a socket to an endpoint](recipe_05/README.md)
- [6. Connecting a socket](recipe_06/README.md)
- [7. Accepting connections](recipe_07/README.md)

Computer networks and communication protocols significantly increase capabilities of modern software, allowing different applications or separate parts of the same application to communicate with each other to achieve a common goal. Some applications have communication as their main function, for example, instant messengers, e-mail servers and clients, file download software, and so on. Others have the network communication layer as a fundamental component, on top of which the main functionality is built. Some of the examples of such applications are web browsers, network file systems, distributed database management systems, media streaming software, online games, offline games with multiplayer over the network option support, and many others. Besides, nowadays almost any application in addition to its main functionality provides supplementary functions, involving network communication. The most prominent examples of such functions are online registration and automatic software update. In the latter case, the update package is downloaded from the application developer's remote server and installed on the user's computer or mobile device.

The application that consists of two or more parts, each of which runs on a separate computing device, and communicates with other parts over a computer network is called a distributed application. For example, a web server and a web browser together can be considered as one complex distributed application. The browser running on a user's computer communicates with the web server running on a different remote computer in order to achieve a common goal—to transmit and display a web page requested by the user.

Distributed applications provide significant benefits as compared to traditional applications running on a single computer. The most valuable of them are the following:

 - Ability to transmit data between two or more remote computing devices. This is absolutely obvious and the most valuable benefit of distributed software.
 - Ability to connect computers in a network and install special software on them, creating powerful computing systems that can perform tasks that can't otherwise be performed on a single computer in an adequate amount of time.
 - Ability to effectively store and share data in a network. In a computer network, a single device can be used as data storage to store big amounts of data and other devices can easily request some portions of that data when necessary without the need to keep the copy of all data on each device. As an example, consider large datacenters hosting hundreds of millions of websites. The end user can request the web page they need anytime by sending the request to the server over the network (usually, the Internet). There is no need to keep the copy of the website on the user's device. There is a single storage of the data (a website) and millions of users can request the data from that storage if and when this information is needed.

For two applications running on different computing devices to communicate with each other, they need to agree on a communication protocol. Of course, the developer of the distributed application is free to implement his or her own protocol. However, this would be rarely the case at least for two reasons. First, developing such a protocol is an enormously complex and time-consuming task. Secondly, such protocols are already defined, standardized, and even implemented in all popular operating systems including Windows, Mac OS X, and majority of the distributions of Linux.

These protocols are defined by the TCP/IP standard. Don't be fooled by the standard's name; it defines not only TCP and IP but many more other protocols, comprising a TCP/IP protocol stack with one or more protocols on each level of the stack. Distributed software developers usually deal with transport level protocols such as TCP or UDP. Lower layer protocols are usually hidden from the developer and are handled by the operating system and network devices.

In this book, we only touch upon TCP and UDP protocols that satisfy the needs of most developers of distributed software. If the reader is not familiar with the TCP/IP protocol stack, the OSI model, or TCP and UDP protocols, it's highly advised to read some theory on these topics. Though this book provides some brief information about them, it is mostly focused on practical aspects of using TCP and UDP protocols in distributed software development.

The TCP protocol is a transport layer protocol with the following characteristics:

- It's reliable, which means that this protocol guarantees delivery of the messages in proper order or a notification that the message has not been delivered. The protocol includes error handling mechanisms, which frees the developer from the need to implement them in the application.
- It assumes logical connection establishment. Before one application can communicate with another over the TCP protocol, it must establish a logical connection by exchanging service messages according to the standard.
- It assumes the point-to-point communication model. That is, only two applications can communicate over a single connection. No multicast messaging is supported.
- It is stream-oriented. This means that the data being sent by one application to another is interpreted by the protocol as a stream of bytes. In practice, it means that if a sender application sends a particular block of data, there is no guarantee that it will be delivered to the receiver application as the same block of data in a single turn, that is, the sent message may be broken into as many parts as the protocol wants and each of them will be delivered separately, though in correct order.

The UDP protocol is a transport layer protocol having different (in some sense opposite) characteristics from those of the TCP protocol. The following are its characteristics:

- It's unreliable, which means that if a sender sends a message over a UDP protocol, there is no guarantee that the message will be delivered. The protocol won't try to detect or fix any errors. The developer is responsible for all error handling.
- It's connectionless, meaning that no connection establishment is needed before the applications can communicate.
- It supports both one-to-one and one-to-many communication models. Multicast messages are supported by the protocol.
- It's datagram oriented. This means that the protocol interprets data as messages of a particular size and will try to deliver them as a whole. The message (datagram) either will be delivered as a whole, or if the protocol fails to do that won't be delivered at all.

Because the UDP protocol is unreliable, it is usually used in reliable local networks. To use it for communication over the Internet (which is an unreliable network), the developer must implement error handling mechanisms in its application.

As it has already been mentioned, both TCP and UDP protocols and the underlying protocols required by them are implemented by most popular operating systems. A developer of a distributed application is provided an API through which it can use protocols implementation. The TCP/IP standard does not standardize the protocol API implementation; therefore, several API implementations exist. However, the one based on Berkeley Sockets API is the most widely used.

Berkeley Sockets API is the name of one of the many possible implementations of TCP and UDP protocols' API. This API was developed at the Berkeley University of California, USA (hence the name) in the early 1980s. It is built around a concept of an abstract object called a socket. Such a name was given to this object in order to draw the analogy with a usual electrical socket. However, this idea seems to have somewhat failed due to the fact that Berkeley Sockets turned out to be a significantly more complex concept.

Now Windows, Mac OS X, and Linux operating systems all have this API implemented (though with some minor variations) and software developers can use it to consume TCP and UDP protocols' functionality when developing distributed applications.

Though very popular and widely used, Sockets API has several flaws. First, because it was designed as a very generic API that should support many different protocols, it is quite complex and somewhat difficult to use. The second flaw is that this is a C-style functional API with a poor type system, which makes it error prone and even more difficult to use. For example, Sockets API doesn't provide a separate type representing a socket. Instead, the built-in type int is used, which means that by mistake any value of the int type can be passed as an argument to the function expecting a socket, and the compiler won't detect the mistake. This may lead to run-time crashes, the root cause of which is hard to find.

Network programming is inherently complex and doing it with a low-level C-style socket API makes it even more complex and error prone. Boost.Asio is an O-O C++ library that is, just like raw Sockets API, built around the concept of a socket. Roughly speaking, Boost.Asio wraps raw Sockets API and provides the developer with O-O interface to it. It is intended to simplify network programming in several ways as follows:

- It hides the raw C-style API and providing a user with an object-oriented API
- It provides a rich-type system, which makes code more readable and allows it to catch many errors at compilation time
- As Boost.Asio is a cross-platform library, it simplifies development of cross-platform distributed applications
- It provides auxiliary functionality such as scatter-gather I/O operations, stream-based I/O, exception-based error handling, and others
- The library is designed so that it can be relatively easily extended to add new custom functionality