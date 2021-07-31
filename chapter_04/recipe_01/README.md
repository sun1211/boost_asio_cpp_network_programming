# Implementing a synchronous iterative TCP server

A synchronous iterative TCP server is a part of a distributed application that satisfies the following criteria:

- Acts as a server in the client-server communication model
- Communicates with client applications over TCP protocol
- Uses I/O and control operations that block the thread of execution until the corresponding operation completes, or an error occurs
- Handles clients in a serial, one-by-one fashion

A typical synchronous iterative TCP server works according to the following algorithm:

- Allocate an acceptor socket and bind it to a particular TCP port.
- Run a loop until the server is stopped:
  - Wait for the connection request from a client.
  - Accept the client's connection request when one arrives.
  - Wait for the request message from the client.
  - Read the request message.
  - Process the request.
  - Send the response message to the client.
  - Close the connection with the client and deallocate the socket.

We begin implementing our server application by defining a class responsible for handling a single client by reading the request message, processing it, and then sending back the response message. This class represents a single service provided by the server application and, therefore, we will give it a name Service:
```
#include <boost/asio.hpp>

#include <thread>
#include <atomic>
#include <memory>
#include <iostream>

using namespace boost;

class Service {
public:
  Service(){}

  void HandleClient(asio::ip::tcp::socket& sock) {
    try {
      asio::streambuf request;
      asio::read_until(sock, request, '\n');

      // Emulate request processing.
      inti = 0;
      while (i != 1000000)
        i++;
        std::this_thread::sleep_for(
std::chrono::milliseconds(500));

      // Sending response.
      std::string response = "Response\n";
      asio::write(sock, asio::buffer(response));
}
    catch (system::system_error&e) {
      std::cout  << "Error occured! Error code = " 
<<e.code() << ". Message: "
          <<e.what();
    }
  }
};
```

To keep things simple, in our sample server application, we implement a dummy service, which only emulates the execution of certain operations. The request processing emulation consists of performing many increment operations to emulate operations that intensively consume CPU and then putting the thread of control to sleep for some time to emulate such operations as reading a file or communicating with a peripheral device synchronously.

The Service class is quite simple and contains only one method. However, classes representing services in real-world applications would usually be more complex and richer in functionality, though the main idea would stay the same.

Next, we define another class that represents a high-level acceptor concept (as compared to the low-level concept represented by the asio::ip::tcp::acceptor class). This class is responsible for accepting connection requests arriving from clients and instantiating objects of the Service class, which will provide the service to the connected clients. Let's name this class correspondingly—Acceptor:
```
Next, we define another class that represents a high-level acceptor concept (as compared to the low-level concept represented by the asio::ip::tcp::acceptor class). This class is responsible for accepting connection requests arriving from clients and instantiating objects of the Service class, which will provide the service to the connected clients. Let's name this class correspondingly—Acceptor:
```

This class owns an object of the asio::ip::tcp::acceptor class named m_acceptor, which is used to synchronously accept incoming connection requests.

Also, we define a class that represents the server itself. The class is named correspondingly—Server:
```
class Server {
public:
  Server() : m_stop(false) {}

  void Start(unsigned short port_num) {
    m_thread.reset(new std::thread([this, port_num]() {
      Run(port_num);
    }));
  }

  void Stop() {
    m_stop.store(true);
    m_thread->join();
  }

private:
  void Run(unsigned short port_num) {
    Acceptor acc(m_ios, port_num);

    while (!m_stop.load()) {
      acc.Accept();
    }
  }

  std::unique_ptr<std::thread>m_thread;
  std::atomic<bool>m_stop;
  asio::io_servicem_ios;
};
```

This class provides an interface comprised by two methods—Start() and Stop() that are used to start and stop the server correspondingly. The loop runs in a separate tread spawned by the Start() method. The Start() method is nonblocking, while the Stop() method blocks the caller thread until the server is stopped.

Thorough inspection of the Server class reveals one serious drawback of the implementation of the server—the Stop() method may never return under some circumstances. The discussion of this problem and the ways to resolve it is provided later in this recipe.

Eventually, we implement the application entry point function main() that demonstrates how to use the Server class:
```
int main()
{
  unsigned short port_num = 3333;

  try {
    Server srv;
    srv.Start(port_num);

    std::this_thread::sleep_for(std::chrono::seconds(60));

    srv.Stop();
  }
  catch (system::system_error&e) {
        std::cout  << "Error occured! Error code = " 
                   <<e.code() << ". Message: "
                   <<e.what();
  }

  return 0;
}
```

# How it works
The sample server application consists of four components—the Server, Acceptor, and Service classes and the application entry point function main(). Let's consider how each of these components work.

## The Service class
The Service class is the key functional component in the whole application. While other components are infrastructural in their purpose, this class implements the actual function (or service) provided by the server to the clients.

This class is simple and consists of a single HandleClient() method. This method accepts an object representing a socket connected to the client as its input argument and handles that particular client.

In our sample, such handling is trivial. Firstly, the request message is synchronously read from the socket until a new line ASCII symbol \n is encountered. Then, the request is processed. In our case, we emulate processing by running a dummy loop performing one million increment operations and then putting the thread to sleep for half a second. After this, the response message is prepared and synchronously sent back to the client.

The exceptions that may be thrown by Boost.Asio I/O functions and methods are caught and handled in the HandleClient() method and are not propagated to the method caller so that if the handling of one client fails, the server continues working.

Depending on the needs of a particular application, the Service class can be extended and enriched with a functionality to provide the needed service.

## The Acceptor class
The Acceptor class is a part of the server application infrastructure. When constructed, it instantiates an acceptor socket object m_acceptor and calls its listen() method to start listening for connection requests from clients.

This class exposes a single public method named Accept(). This method when called, instantiates an object of the asio::ip::tcp::socket class named sock, representing an active socket, and tries to accept a connection request. If there are pending connection requests available, the connection request is processed and the active socket sock is connected to the new client. Otherwise, this method blocks until a new connection request arrives.

Then, an instance of the Service object is created and its HandleClient() method is called. The sock object connected to the client is passed to this method. The HandleClient() method blocks until communication with the client and request processing completes, or an error occurs. When the HandleClient() method returns, the Accept() method of the Acceptor class returns too. Now, the acceptor is ready to accept the next connection request.

One execution of the class's Accept()method performs the full handling cycle of one client.

## The Server class
The Server class, as its name suggests, represents a server that can be controlled through class's interface methods Start()and Stop().

The Start() method initiates the start-up of the server. It spawns a new thread, which starts its execution from the Server class's Run() private method and returns. The Run() method accepts a single argument named port_num specifying the number of protocol port on which the acceptor socket should listen for incoming connection requests. When invoked, the method first instantiates an object of the Acceptor class and then starts a loop in which the Accept() method of the Acceptor object is called. The loop terminates when the value of the m_stop atomic variable becomes true, which happens when the Stop() method is invoked on the corresponding instance of the Server class.

The Stop() method synchronously stops the server. It does not return until the loop started in the Run() method is interrupted and the thread spawned by the Start() method finishes its execution. To interrupt the loop, the value of the atomic variable m_stop is set to true. After this, the Stop() method calls the join() method on the m_thread object representing the thread running the loop in the Run() method to wait until it exits the loop and finishes its execution.

The presented implementation has a significant drawback consisting in the fact that the server may not be stopped immediately. More than that, there is a possibility that the server will not be stopped at all and the Stop() method will block its caller forever. The root cause of the problem is that the server has a hard dependency on the behavior of the clients.

If the Stop() method is called and the value of the atomic variable m_stop is set to true just before the loop termination condition in the Run() method is checked, the server is stopped almost immediately and no problem appears. However, if the Stop() method is called while the server's thread is blocked in the acc.Accept() method waiting for the next connection request from the client, or in one of the synchronous I/O operations inside the Service class waiting for the request message from the connected client, or for the client to receive the response message, the server cannot be stopped until these blocking operations are completed. Hence, for example, if at the moment, when the Stop() method is called, there are no pending connection requests, the server will not be stopped until a new client connects and gets handled, which in general case may never happen and will lead to the server being blocked forever.

Later, in this section, we will consider the possible ways to tackle this drawback.

## The main() entry point function
This function demonstrates the usage of the server. It creates an instance of the Server class named srv and calls its Start() method to start the server. Because the server is represented as an active object running in its own thread of control, the Start() method returns immediately and the thread running method main() continues execution. To let the server run for some time, the main thread is put to sleep for 60 seconds. After the main thread wakes up, it calls the Stop() method on the srv object to stop the server. When the Stop() method returns, the main() function returns too and our sample application exits.

Of course, in the real application, the server would be stopped as a reaction to a user input or any other relevant event, rather than after dummy 60 seconds, after the server's start-up run out.

## Eliminating the drawbacks
As it has already been mentioned, the presented implementation has two drawbacks that significantly limit its applicability. The first problem is that it may be impossible to stop the server if the Stop() method is called while the server thread is blocked waiting for the incoming connection request, no connection requests arrive. The second problem is that the server can be easily hung by a single malicious (or buggy) client, making it unavailable to other clients. To hang the server, the client application can simply connect to the server and never send any request to it, which will make the server application hang in the blocking input operation forever.

The root cause of both the issues is the usage of blocking operations in the server (which is natural for synchronous servers). A reasonable and simple solution to both these issues would be to assign a timeout to the blocking operations, which would guarantee that the server would unblock periodically to check whether the stop command has been issued and also to forcefully discard clients that do not send requests for a long period of time. However, Boost.Asio does not provide a way to cancel synchronous operations, or to assign timeouts to them. Therefore, we should try to find other ways to make our synchronous server more responsive and stable.

Let's consider ways to tackle each of the two drawbacks.

## Stopping a server in reasonable amount of time
As the only legitimate way to make the accept()synchronous method of an acceptor socket unblock when there are no pending connection requests is to send a dummy connection request to the port on which the acceptor is listening, we can do the following trick to solve our problem.

In the Server class's Stop() method, after setting the value of the m_stop atomic variable to true, we can create a dummy active socket, connect it to this same server, and send some dummy request. This will guarantee that the server thread will leave the accept() method of the acceptor socket and will eventually check the value of the m_stop atomic variable to find out that its value is equal to true, which will lead to termination of the loop and completion of the Acceptor::Accept() method.

In the described method, it is assumed that the server stops itself by sending a message to itself (actually a message is sent from an I/O thread to the worker thread). Another approach would be to have a special client (separate application) that would connect and send a special service message (for example, stop\n) to the server, which will be interpreted by the server as a signal to stop. In this case, the server would be controlled externally (from a different application) and the Server class would not need to have the Stop() method.

## Dealing with the server's vulnerability
Unfortunately, the nature of blocking the I/O operation without the timeout assigned to it is such that it can be used to easily hang the iterative server that uses such operations and make it inaccessible to other clients.

Obviously, to protect the server from this vulnerability, we need to redesign it so that it never gets blocked by I/O operations. One way to achieve this is to use nonblocking sockets (which will turn our server into reactive) or use asynchronous I/O operations. Both the options mean that our server stops being synchronous. We will consider some of these solutions in other recipes of this chapter.

## Analyzing the results
Vulnerabilities that are inherent in the synchronous iterative servers implemented with Boost.Asio described above do not allow using them in public networks, where there is a risk of misuse of the server by a culprit. Usually, synchronous servers would be used in closed and protected environments where clients are carefully designed so that they do not hang the server.

Another limitation of the iterative synchronous server is that they are not scalable and cannot take advantage of a multiprocessor hardware. However, their advantage—simplicity—is the reason why this type of a server is a good choice in many cases.


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
