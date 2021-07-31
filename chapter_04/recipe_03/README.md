# Implementing an asynchronous TCP server

An asynchronous TCP server is a part of a distributed application that satisfies the following criteria:

- Acts as a server in the client-server communication model
- Communicates with client applications over TCP protocol
- Uses the asynchronous I/O and control operations
- May handle multiple clients simultaneously

A typical asynchronous TCP server works according to the following algorithm:
- Allocate an acceptor socket and bind it to a particular TCP port.
- Initiate the asynchronous accept operation.
- Spawn one or more threads of control and add them to the pool of threads that run the Boost.Asio event loop.
- When the asynchronous accept operation completes, initiate a new one to accept the next connection request.
- Initiate the asynchronous reading operation to read the request from the connected client.
- When the asynchronous reading operation completes, process the request and prepare the response message.
- Initiate the asynchronous writing operation to send the response message to the client.
- When the asynchronous writing operation completes, close the connection and deallocate the socket.

Note that the steps starting from the fourth step in the preceding algorithm may be performed in arbitrary order depending on the relative timing of the concrete asynchronous operations in a concrete application. Due to the asynchronous model of the server, sequential order of execution of the steps may not hold even when the server is running on a single-processor computer.

We begin implementing our server application by defining a class responsible for handling a single client by reading the request message, processing it, and then sending back the response message. This class represents a single service provided by the server application. Let's name it Service:
```
#include <boost/asio.hpp>

#include <thread>
#include <atomic>
#include <memory>
#include <iostream>

using namespace boost;

class Service {
public:
   Service(std::shared_ptr<asio::ip::tcp::socket> sock) :
      m_sock(sock)
   {}

   void StartHandling() {

      asio::async_read_until(*m_sock.get(), 
            m_request, 
            '\n', 
            [this](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred) 
                        {                  
                              onRequestReceived(ec,
                               bytes_transferred);
               });
   }

private:
   void onRequestReceived(const boost::system::error_code& ec,
                std::size_t bytes_transferred) {
      if (ec != 0) {
         std::cout << "Error occured! Error code = "
            << ec.value()
            << ". Message: " << ec.message();

         onFinish();
                return;
      }
      
// Process the request.
      m_response = ProcessRequest(m_request);

      // Initiate asynchronous write operation.
      asio::async_write(*m_sock.get(), 
            asio::buffer(m_response),
            [this](
                            const boost::system::error_code& ec,
                            std::size_t bytes_transferred) 
                            {
                  onResponseSent(ec,
                                  bytes_transferred);
               });
   }

   void onResponseSent(const boost::system::error_code& ec,
                      std::size_t bytes_transferred) {
      if (ec != 0) {
         std::cout << "Error occured! Error code = "
            << ec.value()
            << ". Message: " << ec.message();
      }

      onFinish();
   }

   // Here we perform the cleanup.
   void onFinish() {
      delete this;
   }

   std::string ProcessRequest(asio::streambuf& request) {

      // In this method we parse the request, process it
      // and prepare the request.

      // Emulate CPU-consuming operations.
      int i = 0;
      while (i != 1000000)
         i++;

      // Emulate operations that block the thread
// (e.g. synch I/O operations).
         std::this_thread::sleep_for(
                      std::chrono::milliseconds(100));

      // Prepare and return the response message. 
      std::string response = "Response\n";
      return response;
   }

private:
   std::shared_ptr<asio::ip::tcp::socket> m_sock;
   std::string m_response;
   asio::streambuf m_request;
};
```

To keep things simple, in our sample server application, we implement a dummy service which only emulates the execution of certain operations. The request processing emulation consists of performing many increment operations to emulate operations that intensively consume CPU and then putting the thread of control to sleep for some time to emulate I/O operations such as reading a file or communicating with a peripheral device synchronously.

Each instance of the Service class is intended to handle one connected client by reading the request message, processing it, and then sending the response message back.

Next, we define another class, which represents a high-level acceptor concept (as compared to the low-level concept represented by the asio::ip::tcp::acceptor class). This class is responsible for accepting the connection requests arriving from clients and instantiating the objects of the Service class, which will provide the service to connected clients. Let's name it Acceptor:
```
class Acceptor {
public:
  Acceptor(asio::io_service&ios, unsigned short port_num) :
    m_ios(ios),
    m_acceptor(m_ios,
      asio::ip::tcp::endpoint(
                  asio::ip::address_v4::any(), 
                  port_num)),
    m_isStopped(false)
  {}

  // Start accepting incoming connection requests.
  void Start() {
    m_acceptor.listen();
    InitAccept();
  }
  
  // Stop accepting incoming connection requests.
  void Stop() {
    m_isStopped.store(true);
  }

private:
  void InitAccept() {
    std::shared_ptr<asio::ip::tcp::socket>
              sock(new asio::ip::tcp::socket(m_ios));

    m_acceptor.async_accept(*sock.get(),
      [this, sock](
                   const boost::system::error_code& error) 
           {
        onAccept(error, sock);
      });
  }

  void onAccept(const boost::system::error_code&ec,
               std::shared_ptr<asio::ip::tcp::socket> sock) 
  {
    if (ec == 0) {
      (new Service(sock))->StartHandling();
    }
    else {
      std::cout<< "Error occured! Error code = "
        <<ec.value()
        << ". Message: " <<ec.message();
    }

    // Init next async accept operation if
    // acceptor has not been stopped yet.
    if (!m_isStopped.load()) {
      InitAccept();
    }
    else {
      // Stop accepting incoming connections
      // and free allocated resources.
      m_acceptor.close();
    }
  }

private:
  asio::io_service&m_ios;
  asio::ip::tcp::acceptor m_acceptor;
  std::atomic<bool>m_isStopped;
}; 
```

This class owns an object of the asio::ip::tcp::acceptor class named m_acceptor, which is used to asynchronously accept the incoming connection requests.

Also, we define a class that represents the server itself. The class is named correspondingly—Server:
```
class Server {
public:
   Server() {
      m_work.reset(new asio::io_service::work(m_ios));
   }

   // Start the server.
   void Start(unsigned short port_num, 
unsigned int thread_pool_size) {
      
      assert(thread_pool_size > 0);

      // Create and start Acceptor.
      acc.reset(new Acceptor(m_ios, port_num));
      acc->Start();

      // Create specified number of threads and 
      // add them to the pool.
      for (unsigned int i = 0; i < thread_pool_size; i++) {
         std::unique_ptr<std::thread> th(
                   new std::thread([this]()
                   {
                          m_ios.run();
                   }));

         m_thread_pool.push_back(std::move(th));
      }
   }

   // Stop the server.
   void Stop() {
      acc->Stop();
      m_ios.stop();

      for (auto& th : m_thread_pool) {
         th->join();
      }
   }

private:
   asio::io_servicem_ios;
   std::unique_ptr<asio::io_service::work>m_work;
   std::unique_ptr<Acceptor>acc;
   std::vector<std::unique_ptr<std::thread>>m_thread_pool;
};
```
This class provides an interface consisting of two methods—Start() and Stop(). The Start() method accepts a protocol port number on which the server should listen for the incoming connection requests and the number of threads to add to the pool as input arguments and starts the server. The Stop() method stops the server. The Start() method is nonblocking, while the Stop() method is. It blocks the caller thread until the server is stopped and all the threads running the event loop exit.

Eventually, we implement the application entry point function main() that demonstrates how to use an object of the Server class:
```
const unsigned intDEFAULT_THREAD_POOL_SIZE = 2;

int main()
{
  unsigned short port_num = 3333;

  try {
    Server srv;

    unsigned intthread_pool_size =
      std::thread::hardware_concurrency() * 2;
    
      if (thread_pool_size == 0)
      thread_pool_size = DEFAULT_THREAD_POOL_SIZE;

    srv.Start(port_num, thread_pool_size);

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
The sample server application consists of four components—the Service, Acceptor, and Service classes and an application entry point function main(). Let's consider how each of these components work.

## The Service class
The Service class is the key functional component in the application. While other components constitute an infrastructure of the server, this class implements the actual function (or service) provided by the server to the clients.

One instance of this class is intended to handle a single connected client by reading the request, processing it, and then sending back the response message.

The class's constructor accepts a shared pointer to an object representing a socket connected to a particular client as an argument and caches this pointer. This socket will be used later to communicate with the client application.

The public interface of the Service class consists of a single method StartHandling(). This method starts handling the client by initiating the asynchronous reading operation to read the request message from the client specifying the onRequestReceived() method as a callback. Having initiated the asynchronous reading operation, the StartHandling() method returns.

When the request reading completes, or an error occurs, the callback method onRequestReceived() is called. This method first checks whether the reading succeeded by testing the ec argument that contains the operation completion status code. In case the reading finished with an error, the corresponding message is output to the standard output stream and then the onFinish() method is called. After this, the onRequestReceieved() method returns, which leads to client-handling process interruption.

If the request message has been read successfully, the ProcessRequest() method is called to perform the requested operations and prepare the response message. When the ProcessRequest() method completes and returns the string containing the response message, the asynchronous writing operation is initiated to send this response message back to the client. The onResponseSent() method is specified as a callback.

When the writing operation completes (or an error occurs), the onResponseSent()method is called. This method first checks whether the operation succeeded. If the operation failed, the corresponding message is output to the standard output stream. Next, the onFinish()method is called to perform the cleanup. When the onFinish()method returns, the full cycle of client handling is considered completed.

The ProcessRequest() method is the heart of the class because it implements the service. In our server application, we have a dummy service that runs a dummy loop performing one million increment operations and then puts the thread to sleep for 100 milliseconds. After this, the dummy response message is generated and returned to the caller.

Depending on the needs of a particular application, the Service class and particularly its ProcessRequest() method can be extended and enriched with a functionality to provide the needed service.

The Service class is designed so that its objects delete themselves when their job is completed. Deletion is performed in the class's onFinish() private method, which is called in the end of the client handling cycle whether it is successful or erroneous:
```
void onFinish() {
  delete this;
}
```

## The Acceptor class
The Acceptor class is a part of the server application's infrastructure. Its constructor accepts a port number on which it will listen for the incoming connection requests as its input argument. The object of this class contains an instance of the asio::ip::tcp::acceptor class as its member named m_acceptor, which is constructed in the Acceptor class's constructor.

The Acceptor class exposes two public methods—Start() and Stop(). The Start() method is intended to instruct an object of the Acceptor class to start listening and accepting incoming connection requests. It puts the m_acceptor acceptor socket into listening mode and then calls the class's InitAccept() private method. The InitAccept() method, in turn, constructs an active socket object and initiates the asynchronous accept operation, calling the async_accept() method on the acceptor socket object and passing the object representing an active socket to it as an argument. The onAccept() method of the Acceptor class is specified as a callback.

When the connection request is accepted or an error occurs, the callback method onAccept() is called. This method first checks whether any error occurred while the asynchronous operation was executed by checking the value of its input argument ec. If the operation completed successfully, an instance of the Service class is created and its StartHandling() method is called, which starts handling the connected client. Otherwise, in case of error, the corresponding message is output to the standard output stream.

Next, the value of the m_isStopped atomic variable is checked to see whether the stop command has been issued on the Acceptor object. If it has (which means that the Stop() method has been called on the Acceptor object), a new asynchronous accept operation is not initiated and the low-level acceptor object is closed. At this point, Acceptor stops listening and accepting incoming connection requests from clients. Otherwise, the InitAccept() method is called to initiate a new asynchronous accept operation to accept the next incoming connection request.

As it has already been mentioned, the Stop() method instructs the Acceptor object not to initiate the next asynchronous accept operation when the currently running one completes. However, the currently running accept operation is not canceled by this method.

## The Server class
The Server class, as its name suggests, represents a server itself. The class's public interface consists of two methods: Start() and Stop().

The Start() method starts the server up. It accepts two arguments. The first argument named port_num specifies the number of the protocol port on which the server should listen for incoming connections. The second argument named thread_pool_size specifies the number of threads to add to the pool of threads running the even loop and deliver asynchronous operation completion events. This argument is very important and should be chosen with care as it directly influences the performance of the server.

The Start() method begins by instantiating an object of the Acceptor class that will be used to accept incoming connections and then starting it up by calling its Start() method. After this, it spawns a set of worker threads, each of which is added to the pool, by calling the run() method of the asio::io_service object. Besides, all the std::thread objects are cached in the m_thread_pool member vector so that the corresponding threads can be joined later when the server is stopped.

The Stop()method first stops the Acceptor object acc, calling its Stop()method. Then, it calls the stop() method on the asio::io_service object m_ios, which makes all the threads that previously called m_ios.run() to join the pool to exit as soon as possible, discarding all pending asynchronous operations. After this, the Stop() method waits for all threads in the pool to exit by iterating through all the std::thread objects cached in the m_thread_pool vector and joining each of them.

When all threads exit, the Stop() method returns.

## The main() entry point function
This function demonstrates the usage of the server. Firstly, it instantiates an object of the Server class named srv. Because the Start() method of the Server class requires a number of threads constituting a pool to be passed to it, before starting the server, the optimal size of the pool is calculated. The general formula often used in parallel applications to find the optimal number of threads is the number of processors the computer has multiplied by 2. We use the std::thread::hardware_concurrency() static method to obtain the number of processors. However, because this method may fail to do its job returning 0, we fall back to default value represented by the constant DEFAULT_THREAD_POOL_SIZE, which is equal to 2 in our case.

When the thread pool size is calculated, the Start() method is called to start the server. The Start() method does not block. When it returns, the thread running the main() method continues the execution. To allow the server to run for some time, the main thread is put to sleep for 60 seconds. When the main thread wakes up, it calls the Stop() method on the srv object to stop the server. When the Stop() method returns, the main() function returns too and our application exits.

Of course, in the real application, the server would be stopped as a reaction to some relevant event such as the user input, rather than when some dummy period of time elapses.

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
