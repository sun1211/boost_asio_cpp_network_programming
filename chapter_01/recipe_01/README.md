# Creating an endpoint.

the endpoints serve two goals:

- The client application uses an endpoint to designate a particular server application it wants to communicate with.
- The server application uses an endpoint to specify a local IP address and a port number on which it wants to receive incoming messages from clients. If there is more than one IP address on the host, the server application will want to create a special endpoint representing all IP addresses at once.

Before creating the endpoint, the client application must obtain the raw IP address and the protocol port number designating the server it will communicate with. The server application on the other hand, as it usually listens for incoming messages on all IP addresses, only needs to obtain a port number on which to listen.

The following algorithms and corresponding code samples demonstrate two common scenarios of creating an endpoint.
- The first one demonstrates how the client application can create an endpoint to specify the server it wants to communicate with.
- The second one demonstrates how the server application creates an endpoint to specify on which IP addresses and port it wants to listen for incoming messages from clients.
## CREATING AN ENDPOINT IN THE CLIENT TO DESIGNATE THE SERVER
The following algorithm describes steps required to perform in the client application to create an endpoint designating a server application the client wants to communicate with. Initially, the IP address is represented as a string in the dot-decimal notation if this is an IPv4 address or in hexadecimal notation if this is an IPv6 address:

- Obtain the server application's IP address and port number. The IP address should be specified as a string in the dot-decimal (IPv4) or hexadecimal (IPv6) notation.
- Represent the raw IP address as an object of the asio::ip::address class.
- Instantiate the object of the asio::ip::tcp::endpoint class from the address object created in step 2 and a port number.
- The endpoint is ready to be used to designate the server application in Boost.Asio communication related methods.

## CREATING THE SERVER ENDPOINT
The following algorithm describes steps required to perform in a server application to create an endpoint specifying all IP addresses available on the host and a port number on which the server application wants to listen for incoming messages from the clients:

- Obtain the protocol port number on which the server will listen for incoming requests.
- Create a special instance of the asio::ip::address object representing all IP addresses available on the host running the server.
- Instantiate an object of the asio::ip::tcp::endpoint class from the address object created in step 2 and a port number.
- The endpoint is ready to be used to specify to the operating system that the server wants to listen for incoming messages on all IP addresses and a particular protocol port number.

## How to build
```
mkdir build
cd build
cmake ..
cmake --build .
```

## How to run
```
./bin/main
```
