# Chapter 5: HTTP and SSL/TLS

- [1. Implementing the HTTP client application](recipe_01/README.md)
- [2. Implementing the HTTP server application](recipe_02/README.md)
- [3. Adding SSL/TLS support to client applications](recipe_03/README.md)
- [4. Adding SSL/TLS support to server applications](recipe_04/README.md)

This chapter covers two major topics. The first one is HTTP protocol implementation. The second is the usage of SSL/TLS protocol. Let's briefly examine each of them.

The HTTP protocol is an application layer protocol operating on the top of TCP protocol. It is widely used on the Internet, allowing client applications to request particular resources from the servers, and servers to transmit the requested resources back to the clients. Besides, HTTP allows clients to upload data and send commands to the server.

The HTTP protocol assumes several models or methods of communication, each designed for a specific purpose. The simplest method called GET assumes the following flow of events:

- The HTTP client application (for example, a web browser) generates a request message containing information about a particular resource (residing on the server) to be requested and sends it to the HTTP server application (for example, a web server) using TCP as a transport level protocol.
- The HTTP server application, having received a request from the client, parses it, extracts the requested resource from the storage (for example, from a file system or a database), and sends it back to the client as a part of a HTTP response message.

The format of both the request and response messages is defined by HTTP protocol.

Several other methods are defined by HTTP protocol, allowing client application to actively send data or upload resources to the server, delete resources located on the server, and perform other operations. In the recipes of this chapter, we will consider implementation of the GET method. Because HTTP protocol methods are similar in principle, implementation of one of them gives a good hint about how to implement others.

Another topic covered in this chapter is SSL and TLS protocols. Secure Socket Layer (SSL)and Transport Layer Security (TLS) protocols operate on the top of TCP protocol and are aimed at achieving two main goals as follows:
- Providing a way to authenticate each communication participant using digital certificate
- Securing data being transmitted over the underlying TCP protocol

The SSL and TLS protocols are widespread, especially in the Web. Most web servers to which its potential clients may send sensitive data (passwords, credit card numbers, personal data, and so on) support SSL/TLS-enabled communication. In this case, the so called HTTPS (HTTP over SSL) protocol is used to allow the client to authenticate the server (sometimes servers may want to authenticate the client, though this is rarely the case) and to secure transmitted data by encrypting it, making this data useless for the culprit even if intercepted.

Boost.Asio does not contain the implementation of SSL/TLS protocols. Instead, it relies on the OpenSSL library, Boost.Asio provides a set of classes, functions, and data structures that facilitate the usage of functionality provided by OpenSSL, making the code of the application more uniformed and object-oriented.

The two recipes demonstrate how to build client and server applications that secure their communication using SSL/TLS protocols. To make SSL/TLS-related aspects of the applications more vivid and clear, all other aspects of considered applications were made as simple as possible. Both client and server applications are synchronous and based on recipes found in other chapters of this book. This allows us to compare a basic TCP client or server application with their advanced versions supporting SSL/TLS and to better understand what it takes to add SSL/TLS support to a distributed application.