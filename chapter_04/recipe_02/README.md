# Implementing a synchronous parallel TCP server

A synchronous parallel TCP server is a part of a distributed application that satisfies the following criteria:

- Acts as a server in the client-server communication model
- Communicates with client applications over TCP protocol
- Uses I/O and control operations that block the thread of execution until the corresponding operation completes, or an error occurs
- May handle more than one client simultaneously
A typical synchronous parallel TCP server works according to the following algorithm:
- Allocate an acceptor socket and bind it to a particular TCP port.
- Run a loop until the server is stopped:
  - Wait for the incoming connection request from a client
  - Accept the client's connection request
  - Spawn a thread of control and in the context of this thread:
    - Wait for the request message from the client
    - Read the request message
    - Process the request
    - Send a response message to the client
    - Close the connection with the client and deallocate the socket

We begin implementing our server application by defining the class responsible for handling a single client by reading the request message, processing it, and then sending back the response message. This class represents a single service provided by the server application and, therefore, we will name it Service:
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

   void StartHandligClient(
         std::shared_ptr<asio::ip::tcp::socket> sock) {


      std::thread th(([this, sock]() {
         HandleClient(sock);
      }));

      th.detach();
   }

private: 
void HandleClient(std::shared_ptr<asio::ip::tcp::socket> sock) {
      try {
         asio::streambuf request;
         asio::read_until(*sock.get(), request, '\n');

         // Emulate request processing.
         int i = 0;
         while (i != 1000000)
            i++;

            std::this_thread::sleep_for(
std::chrono::milliseconds(500));

         // Sending response.
         std::string response = "Response\n";
         asio::write(*sock.get(), asio::buffer(response));
      } 
      catch (system::system_error &e) {
         std::cout    << "Error occured! Error code = " 
<< e.code() << ". Message: "
               << e.what();
      }

      // Clean-up.
      delete this;
   }
};
```
To keep things simple, in our sample server application, we implement a dummy service, which only emulates the execution of certain operations. The request processing emulation consists of performing many increment operations to emulate operations that intensively consume CPU and then putting the thread of control to sleep for some time to emulate I/O operations such as reading a file or communicating with a peripheral device synchronously.

The Service class is quite simple and contains only one method. However, classes representing services in real-world applications would usually be more complex and richer in functionality, though the main idea would stay the same.

Next, we define another class that represents a high-level acceptor concept (as compared to the low-level concept represented by the asio::ip::tcp::acceptor class). This class is responsible for accepting the connection requests arriving from clients and instantiating the objects of the Service class, which will provide the service to connected clients. Let's name it Acceptor:
```
class Acceptor {
public:
   Acceptor(asio::io_service& ios, unsigned short port_num) :
      m_ios(ios),
      m_acceptor(m_ios,
          asio::ip::tcp::endpoint(
asio::ip::address_v4::any(), 
port_num))
   {
      m_acceptor.listen();
   }

   void Accept() {
      std::shared_ptr<asio::ip::tcp::socket> 
sock(new asio::ip::tcp::socket(m_ios));

      m_acceptor.accept(*sock.get());

      (new Service)->StartHandligClient(sock);
   }

private:
   asio::io_service& m_ios;
   asio::ip::tcp::acceptor m_acceptor;
};
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

This class provides an interface comprised of two methods—Start() and Stop() that are used to start and stop the server correspondingly. The loop runs in a separate thread spawned by the Start() method. The Start() method is nonblocking, while the Stop() method is. It blocks the caller thread until the server is stopped.

Thorough inspection of the Server class reveals one serious drawback of the implementation of the server—the Stop() method may block forever. The discussion of this problem and ways to resolve it is provided below.

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
   catch (system::system_error &e) {
      std::cout    << "Error occured! Error code = " 
<< e.code() << ". Message: "
            << e.what();
   }

   return 0;
}
```

# How it works
The sample server application consists of four components—the Server, Acceptor, and Service classes and the application entry point function main(). Let's consider how each of these components work.

## The Service class
The Service class is the key functional component in the whole application. While other components constitute the infrastructure of the server, this class implements the actual function (or service) provided by the server to the clients.

This class has a single method in its interface called StartHandlingClient(). This method accepts a pointer to an object representing a TCP socket connected to the client as its input argument and starts handling that particular client.

This method spawns a thread of control, which starts its execution from the class's HandleClient() private method, where the actual synchronous handling is performed. Having spawned the thread, the StartHandlingClient() method "lets it go" by detaching the thread from the std::thread object representing it. After this, the StartHandlingClient() method returns.

The HandleClient() private method, as its name suggests, handles the client. In our sample, such handling is trivial. Firstly, the request message is synchronously read from the socket until a new line ASCII symbol \n is encountered. Then, the request is processed. In our case, we emulate processing by running a dummy loop performing one million increment operations and then putting the thread to sleep for half a second. After this, the response message is prepared and sent back to the client.

When the response message is sent, the object of the Service class associated with the HandleClient() method, which is currently running, is deleted by the delete operator. Of course, the design of the class assumes that its instances will be allocated in free memory by a new operator rather than on the stack.

Depending on the needs of a particular application, the Service class can be extended and enriched with the functionality to provide the needed service.

## The Acceptor class
The Acceptor class is a part of the server application infrastructure. When constructed, it instantiates an acceptor socket object m_acceptor and calls its listen() method to start listening for connection requests from clients.

This class exposes a single Accept() public method. This method when called, instantiates an object of the asio::ip::tcp::socket class named sock, representing an active socket, and tries to accept a connection request. If there are pending connection requests available, the connection request is processed and the active socket sock is connected to the new client. Otherwise, this method blocks until a new connection request arrives.

Then, an instance of the Service object is allocated in free memory and its StartHandlingClient() method is called. The sock object is passed to this method as an input argument. The StartHandlingClient() method spawns a thread in the context of which the client will be handled and returns immediately. When the StartHandlingClient() method returns, the Accept() method of the Acceptor class returns too. Now, the acceptor is ready to accept the next connection request.

Note that Acceptor does not take the ownership of the object of the Service class. Instead, the object of the Service class will destroy itself when it completes its job.

## The Server class
The Server class, as its name suggests, represents a server that can be controlled through the class's interface Start()and Stop() methods.

The Start() method initiates the start-up of the server. It spawns a new thread that begins its execution from the Server class's Run() private method and returns. The Run() method accepts a single argument port_num specifying the number of the protocol port on which the acceptor socket should listen for incoming connection requests. When invoked, the method first instantiates an object of the Acceptor class and then starts a loop in which the Accept() method of the Acceptor object is called. The loop terminates when the value of the m_stop atomic variable becomes true, which happens when the Stop() method is invoked on the corresponding instance of the Server class.

The Stop() method synchronously stops the server. It does not return until a loop that started in the Run() method is interrupted and the thread that is spawned by the Start() method finishes its execution. To interrupt the loop, the value of the atomic variable m_stop is set to true. After this, the Stop() method calls the join() method on the m_thread object representing the thread running the loop in the Run() method in order to wait until it finishes its execution.

The presented implementation has a significant drawback consisting of the fact that the server may not be stopped immediately. More than that, there is a possibility that the server will not be stopped at all and the Stop() method will block its caller forever. The root cause of the problem is that the server has a hard dependency on the behavior of the clients.

If the Stop() method is called and sets the value of atomic variable m_stop variable to true just before the loop termination condition in the Run() method is checked, the server is stopped almost immediately and no problem occurs. However, if the Stop() method is called while the server's thread is blocked in the acc.Accept() method waiting for the next connection request from the client—or in one of synchronous I/O operations inside the Service class is waiting for the request message from the connected client or for the client to receive the response message—the server cannot be stopped until these blocking operations complete. Hence, for example, if at the moment when the Stop() method is called, there are no pending connection requests, the server will not be stopped until a new client connects and gets handled, which in general case may never happen and may lead to the server being blocked forever.

Later, in this section, we will consider possible ways to tackle this drawback.

## The main() entry point function
This function demonstrates the usage of the server. It creates an instance of the Server class named srv and calls its method Start() to start the server. Because the server is represented as an active object running in its own thread of control, the Start() method returns immediately and the thread running the main()method continues the execution. To allow the server to run for some time, the main thread is put to sleep for 60 seconds. After the main thread wakes up, it calls the Stop() method on the srv object to stop the server. When the Stop() method returns, the main() function returns too and our sample application exits.

Of course, in the real application, the server would be stopped as a reaction to the user input or any other relevant event, rather than after the dummy 60 seconds after the server's start-up run out.

## Eliminating the drawbacks
The drawbacks inherent in synchronous parallel server application implemented with Boost.Asio library are similar to those of synchronous iterative server application considered in previous recipe. Please refer to the Implementing synchronous iterative TCP server recipe for the discussion of the drawbacks and the ways to eliminate them.


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
