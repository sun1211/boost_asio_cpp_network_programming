# Implementing an asynchronous TCP client

As it has already been mentioned in the introduction section of this chapter, the simplest asynchronous client is structurally more complex than equivalent synchronous one. When we add a feature such as request canceling to the asynchronous client, it becomes even more complex.

In this recipe, we'll consider an asynchronous TCP client application supporting the asynchronous execution of the requests and request canceling functionality. Here is the list of requirements the application will fulfill:

- Input from the user should be processed in a separate thread—the user interface thread. This thread should never be blocked for a noticeable amount of time.
- The user should be able to issue multiple requests to different servers.
- The user should be able to issue a new request before the previously issued requests complete.
- The user should be able to cancel the previously issued requests before they complete.

class AsyncTCPClient  is the key component in our sample, providing most of the functionality of the application. This functionality is accessible to the user of the class through its public interface that contains three public methods:
- void emulateLongComputationOp(unsigned int duration_sec, const std::string& raw_ip_address, unsigned short port_num, Callback callback, unsigned int request_id): This method initiates a request to the server
- void cancelRequest(unsigned int request_id): This method cancels the previously initiated request designated by the request_id argument
- void close(): This method blocks the calling thread until all the currently running requests complete and deinitializes the client. When this method returns, the corresponding instance of the AsyncTCPClient class can't be used anymore.
Now, we define a function that will serve as a callback, which we'll pass to the AsyncTCPClient::emulateLongComputationOp() method. In our case, this function is quite simple. It outputs the result of the request execution and the response message to the standard output stream if the request is completed successfully:
```
void handler(unsigned int request_id,
         const std::string& response, 
                const system::error_code& ec) 
{
  if (ec == 0) {
    std::cout << "Request #" << request_id
      << " has completed. Response: "
      << response << std::endl;
  } else if (ec == asio::error::operation_aborted) {
    std::cout << "Request #" << request_id
      << " has been cancelled by the user." 
            << std::endl;
  } else {
    std::cout << "Request #" << request_id
      << " failed! Error code = " << ec.value()
      << ". Error message = " << ec.message() 
             << std::endl;
  }

  return;
}
```

The handler() function's signature corresponds to the function pointer type Callback defined earlier.

Now that we have all the ingredients, we define an entry point of the application—the main() function—which demonstrates how to use the components defined above in order to communicate with the server. In our sample function, main() emulates the behavior of a human user by initiating three requests and canceling one of them

## How it works

Our sample client application uses two threads of execution. The first one—UI thread—is responsible for processing a user input and initiating requests. The responsibility of the second thread—I/O thread—is to run the event loop and call the asynchronous operation's callback routines. Such configuration allows us to make our application's user interface responsive.

### Starting the application – the main() entry point function
The main() function is invoked in the context of the UI thread. This function emulates the behavior of the user who initiates and cancels requests. Firstly, it creates an instance of the AsyncTCPClient class and then calls its emulateLongComputationOp() method three times to initiate three asynchronous requests, each time specifying a different target server. The first request (the one assigned ID 1) is canceled by calling the cancelRequest()method several seconds after the request has been initiated.

### Request completion – the handler() callback function
For all three requests initiated in the main() function handler() is specified as a callback. This function is called when the request is finished regardless of the reason as to why it finished—be it a successful completion or an error. Also, this function is called when the request is canceled by the user. The function accepts three arguments as follows:

- unsigned int request_id: This contains the unique identifier of the request. This is the same identifier that was assigned to the request when it was initiated.
- std::string& response: This contains the response data. This value is considered valid only if the request is completed successfully and is not canceled.
- system::error_code& ec: If an error occurs during a request life cycle, this object contains the error information. If the request was canceled, it contains the asio::error::operation_aborted value.
The handler() function is quite simple in our sample. Based on the values of the parameters passed to it, it outputs information about the finished request.

### The AsyncTCPClient class – initializing
As it has already been mentioned, all the functionality related to communication with the server application is hidden in the AsyncTCPClient class. This class has a nonempty constructor that accepts no arguments and does two things. Firstly, it instantiates an object of the asio::io_service::work class passing an instance of the asio::io_service class named m_ios to its constructor. Then, it spawns a thread that calls the run() method of the m_ios object. The object of the asio::io_service::work class keeps threads running event loop from exiting this loop when there are no pending asynchronous operations. The spawned thread plays the role of I/O thread in our application; in the context of this thread, the callbacks assigned asynchronous operations will be invoked.

### The AsyncTCPClient class – initiating a request
The emulateLongComputationOp() method is intended to initiate an asynchronous request. It accepts five arguments. The first one named duration_sec represents the request parameter according to the application layer protocol. The raw_ip_address and port_num specify the server to which the request should be sent. The next argument is a pointer to a callback function, which will be called when the request is complete. We'll turn back to the discussion of the callback later in this section. The last argument request_id is the unique identifier of the request. This identifier is associated with the request and is used to refer to it later, for example, when there is a need to cancel it.

The emulateLongComputationOp() method begins with preparing a request string and allocating an instance of the Session structure that keeps the data associated with the request including a socket object that is used to communicate with the server.

Then, the socket is opened and the pointer to the Session object is added to the m_active_sessions map. This map contains pointers to the Session objects associated with all active requests, that is, those requests that have been initiated but have not finished yet. When the request completes, before the corresponding callback is called, the pointer to the Session object associated with this request is removed from the map.

The request_id argument is used as a key of the corresponding Session object added to the map. We need to cache the Session objects in order to have access to them in case the user decides to cancel the previously initiated request. If we would not need to support canceling of a request, we could avoid using the m_active_sessions map.

We synchronize the access to the m_active_sessions map with a m_active_session_guard mutex. Synchronization is necessary because the map can be accessed from multiple threads. Items are added to it in UI thread, and removed in an I/O thread that calls a callback when the corresponding request is finished.

Now, when the pointer to the corresponding Session object is cached, we need to connect the socket to the server, which we do by calling the socket's async_connect() method:

```
session->m_sock.async_connect(session->m_ep,
  [this, session](const system::error_code& ec)
  { 
         // ...
  });
```
An endpoint object designating the server to which we want to connect and a callback function to be called when the connection is complete or an error occurs, are passed as arguments to this method. In our sample we use lambda function as a callback function. The call to the socket's async_connect() method is the last statement in the emulateLongComputationOp() method. When async_connect() returns, emulateLongComputationOp() returns too, which means that the request has been initiated.

Let's have a closer look at the lambda function that we pass to async_connect() as a callback. Here is its code:
```
[this, session](const system::error_code&ec)
{
  if (ec != 0) {
    session->m_ec = ec;
    onRequestComplete(session);
    return;
  }

  std::unique_lock<std::mutex>
    cancel_lock(session->m_cancel_guard);

  if (session->m_was_cancelled) {
     onRequestComplete(session);
     return;
  }

  asio::async_write(session->m_sock,
  asio::buffer(session->m_request),
        [this, session](const boost::system::error_code& ec,
              std::size_t bytes_transferred)
              {
                    //...
        });
}
```

The callback begins with checking the error code passed to it as the ec argument, the value of which when different from zero means that the corresponding asynchronous operation has failed. In case of failure, we store the ec value in the corresponding Session object, call the class's onRequestComplete() private method passing the Session object to it as an argument, and then return.

If the ec object designates success, we lock the m_cancel_guard mutex (the member of the request descriptor object) and check whether the request has not been canceled yet. More details about the canceling request are provided later in this section, where the cancelRequest() method is considered.

If we see that the request has not been canceled, we initiate the next asynchronous operation calling the Boost.Asio free function async_write() to send the request data to the server. Again, we pass to it a lambda function as a callback. This callback is very similar to the one passed to the anync_connect() method when the asynchronous connection operation was initiated. We first check the error code and then if it indicates success, we check whether or not the request has been canceled. Also, if it has not, we initiate the next asynchronous operation—async_read_until()—in order to receive a response from the server:
```
[this, session](const boost::system::error_code& ec,
         std::size_t bytes_transferred){
  if (ec != 0) {
    session->m_ec = ec;
    onRequestComplete(session);
    return;
  }

  std::unique_lock<std::mutex>
    cancel_lock(session->m_cancel_guard);

  if (session->m_was_cancelled) {
    onRequestComplete(session);
    return;
  }

  asio::async_read_until(session->m_sock,
        session->m_response_buf, '\n', 
     [this, session](const boost::system::error_code& ec,
              std::size_t b'ytes_transferred) 
        {
      // ...
        });
}
```
Again, we pass a lambda function as a callback argument to the async_read_until() free function. This callback function is quite simple:

```
[this, session](const boost::system::error_code& ec,
    std::size_t bytes_transferred) 
{
  if (ec != 0) {
    session->m_ec = ec;
  } else {
    std::istream strm(&session->m_response_buf);
    std::getline(strm, session->m_response);
  }

  onRequestComplete(session);
}
```

It checks the error code and in the case of success, it stores the received response data in the corresponding Session object. Then, the AsyncTCPClient class's private method onRequestComplete() is called and the Session object is passed to it as an argument.

The onRequestComplete() method is called whenever the request completes with any result. It is called when the request completes successfully, when the request fails at any stage of its life cycle, or when it is canceled by the user. The purpose of this method is to perform a cleanup and then to call a callback provided by the caller of the emulateLongComputationOp() method, when initiating this request.

The onRequestComplete() method begins with shutting down the socket. Note that here we use the overload of the socket's shutdown() method, which doesn't throw exceptions. We don't care if the shutting down of the connection fails as this is not a critical operation in our case. Then, we remove the corresponding entry from the m_active_sessions map as the request is finished and hence it is not active anymore. Also, as the last step, the user supplied callback is called. After the callback function returns, the request life cycle is finished.

### The AsyncTCPClient class – canceling the request
Now, let's take a look at the cancelRequst() method of the AsyncTCPClient class. This method accepts an identifier of the request to be canceled as an argument. It begins with looking for the Session object corresponding to the specified request in the m_active_sessions map. If one is found, it calls the cancel() method on the socket object stored in this Session object. This leads to the interruption of the currently running asynchronous operation associated with this socket object.

However, there is a chance that the cancelRequest() method will be called at the moment when one asynchronous operation has already been completed and the next one has not been initiated yet. For example, imagine that the I/O thread is now running the callback of the async_connect() operation associated with a particular socket. At this moment, no asynchronous operation associated with this socket is in progress (because the next asynchronous operation async_write() has not been initiated yet); therefore, calling cancel() on this socket will have no effect. That's why we use an additional flag Session::m_was_cancelled designating, as its name suggests, whether the request has been canceled (or to be more precise, whether the cancelRequest() method has been called by the user). In the callback of the asynchronous operation, we look at the value of this flag before initiating the next asynchronous operation. If we see that the flag is set (which means that the request was canceled), we don't initiate the next asynchronous operation, but instead we interrupt the request execution and call the onRequestComplete() method.

We use the Session::m_cancel_guard mutex in the cancelRequest() method and in the callbacks of the asynchronous operations such as async_connect() and async_write() to enforce the following order of operations: request can be canceled either before the value of the Session::m_was_cancelled flag is tested in the callback, or after the next asynchronous operation is initiated. This order guarantees the proper canceling of a request whenever a user calls the cancelRequest() method.

### The AsyncTCPClient class – closing the client
After the client has been used and is not needed anymore, it should be properly closed. The close() method of the AsyncTCPClient class allows us to do that. Firstly, this method destroys the m_work object that allows the I/O thread to exit the event message loop when all the asynchronous operations are completed. Then, it joins the I/O thread to wait until it exits.

After the close() method returns, the corresponding object of the AsyncTCPClient class cannot be used anymore.

The AsyncTCPClient class in the presented sample implements an asynchronous single-threaded TCP client. It uses a single thread that runs the event loop and processes the requests. Usually, when the request rate is low, the size of the response is not large and the request handler does not perform the complex and time-consuming processing of the response (stage 5 of the request life cycle); one thread is enough.

However, when we want the client to make millions of requests and process them as fast as possible, we may want to turn our client into a multithreaded one, where multiple threads may run several requests truly simultaneously. Of course, it assumes that the computer running the client is a multicore or a multiprocessor computer. The application running more threads than the number of cores or processors installed in the computer may slow down the application due to the effect of the thread switching overhead.

## Implementing a multithreaded TCP client application
In order to turn our single-treaded client application into a multithreaded one, we need to make several changes in it. Firstly, we need to replace the m_thread member of the AnyncTCPClient class that represents a single I/O thread, with a list of pointers to the std::thread objects, which will represent a collection of I/O threads:
```
std::list<std::unique_ptr<std::thread>> m_threads;
```

Next, we need to change the class's constructor so that it accepts an argument representing the number of I/O threads to be created. Besides, the constructor should spawn the specified number of I/O threads and add them all to the pool of threads running the event loop:
```
AsyncTCPClient(unsigned char num_of_threads){
  m_work.reset(new boost::asio::io_service::work(m_ios));

  for (unsigned char i = 1; i <= num_of_threads; i++) {
         std::unique_ptr<std::thread> th(
               new std::thread([this](){
        m_ios.run();
      }));

      m_threads.push_back(std::move(th));
    }
  }
```

Like in a single-threaded version of the client, each thread calls the run() method of the m_ios object. As a result, all threads are added to the thread pool, controlled by the m_ios object. All threads from the pool will be used to call the corresponding asynchronous operation completion callbacks. This means that on a multicore or multiprocessor computer, several callbacks may be running truly simultaneously in different threads, each on a separate processor; whereas, in a single-threaded version of the client, they would be executed serially.

After each thread is created, the pointer to it is put to the m_threads list so that we have the access to the thread objects later.

Also, the last change is in the close() method. Here, we need to join each thread in the list. This is how the method looks after the change:
```

Implementing an asynchronous TCP client
As it has already been mentioned in the introduction section of this chapter, the simplest asynchronous client is structurally more complex than equivalent synchronous one. When we add a feature such as request canceling to the asynchronous client, it becomes even more complex.

In this recipe, we'll consider an asynchronous TCP client application supporting the asynchronous execution of the requests and request canceling functionality. Here is the list of requirements the application will fulfill:

Input from the user should be processed in a separate thread—the user interface thread. This thread should never be blocked for a noticeable amount of time.
The user should be able to issue multiple requests to different servers.
The user should be able to issue a new request before the previously issued requests complete.
The user should be able to cancel the previously issued requests before they complete.
How to do it…
As our application needs to support request canceling, we begin with specifying settings that enable request canceling on Windows:

#include <boost/predef.h> // Tools to identify the OS.

// We need this to enable cancelling of I/O operations on
// Windows XP, Windows Server 2003 and earlier.
// Refer to "http://www.boost.org/doc/libs/1_58_0/
// doc/html/boost_asio/reference/basic_stream_socket/
// cancel/overload1.html" for details.
#ifdef BOOST_OS_WINDOWS
#define _WIN32_WINNT 0x0501

#if _WIN32_WINNT <= 0x0502 // Windows Server 2003 or earlier.
  #define BOOST_ASIO_DISABLE_IOCP
  #define BOOST_ASIO_ENABLE_CANCELIO  
#endif
#endif
Then, we include the necessary headers and specify the using directive for our convenience:

#include <boost/asio.hpp>

#include <thread>
#include <mutex>
#include <memory>
#include <iostream>

using namespace boost;
We continue with defining a data type representing a pointer to a callback function. Because our client application is going to be asynchronous, we need a notion of callback as a request completion notification mechanism. Later, it will become clear as to why we need it and how it is used:

// Function pointer type that points to the callback
// function which is called when a request is complete.
typedef void(*Callback) (unsigned int request_id,
  const std::string& response,
  const system::error_code& ec);
Next, we define a data structure whose purpose is to keep the data related to a particular request while it is being executed. Let's name it Session:

// Structure represents a context of a single request.
struct Session {
  Session(asio::io_service& ios,
  const std::string& raw_ip_address,
  unsigned short port_num,
  const std::string& request,
  unsigned int id,
  Callback callback) :
  m_sock(ios),
  m_ep(asio::ip::address::from_string(raw_ip_address),
  port_num),
  m_request(request),
  m_id(id),
  m_callback(callback),
  m_was_cancelled(false) {}

  asio::ip::tcp::socket m_sock; // Socket used for communication
  asio::ip::tcp::endpoint m_ep; // Remote endpoint.
  std::string m_request;        // Request string.

  // streambuf where the response will be stored.
  asio::streambuf m_response_buf;
  std::string m_response; // Response represented as a string.

  // Contains the description of an error if one occurs during
  // the request life cycle.
  system::error_code m_ec;

  unsigned int m_id; // Unique ID assigned to the request.

  // Pointer to the function to be called when the request
  // completes.
  Callback m_callback;

  bool m_was_cancelled;
  std::mutex m_cancel_guard;
};
The purpose of all the fields that the Session data structure contains will become clear later as we go.

Next, we define a class that provides the asynchronous communication functionality. Let's name it AsyncTCPClient:

class AsyncTCPClient : public boost::noncopyable {
class AsyncTCPClient : public boost::noncopyable {
public:
   AsyncTCPClient(){
      m_work.reset(new boost::asio::io_service::work(m_ios));

      m_thread.reset(new std::thread([this](){
         m_ios.run();
      }));
   }

   void emulateLongComputationOp(
      unsigned int duration_sec,
      const std::string& raw_ip_address,
      unsigned short port_num,
      Callback callback,
      unsigned int request_id) {

      // Preparing the request string.
      std::string request = "EMULATE_LONG_CALC_OP "
         + std::to_string(duration_sec)
         + "\n";

      std::shared_ptr<Session> session =
         std::shared_ptr<Session>(new Session(m_ios,
         raw_ip_address,
         port_num,
         request,
         request_id,
         callback));

      session->m_sock.open(session->m_ep.protocol());

      // Add new session to the list of active sessions so
      // that we can access it if the user decides to cancel
      // the corresponding request before it completes.
      // Because active sessions list can be accessed from 
      // multiple threads, we guard it with a mutex to avoid 
      // data corruption.
      std::unique_lock<std::mutex>
         lock(m_active_sessions_guard);
      m_active_sessions[request_id] = session;
      lock.unlock();

      session->m_sock.async_connect(session->m_ep, 
         [this, session](const system::error_code& ec) 
         {
         if (ec != 0) {
            session->m_ec = ec;
            onRequestComplete(session);
            return;
         }

         std::unique_lock<std::mutex>
            cancel_lock(session->m_cancel_guard);

         if (session->m_was_cancelled) {
            onRequestComplete(session);
            return;
         }

                asio::async_write(session->m_sock, 
                             asio::buffer(session->m_request),
         [this, session](const boost::system::error_code& ec,
                            std::size_t bytes_transferred) 
         {
         if (ec != 0) {
            session->m_ec = ec;
            onRequestComplete(session);
            return;
         }

         std::unique_lock<std::mutex>
            cancel_lock(session->m_cancel_guard);

         if (session->m_was_cancelled) {
            onRequestComplete(session);
            return;
         }

                asio::async_read_until(session->m_sock,
                                  session->m_response_buf, 
                                  '\n', 
         [this, session](const boost::system::error_code& ec,
              std::size_t bytes_transferred) 
         {
         if (ec != 0) {
            session->m_ec = ec;
         } else {
            std::istream strm(&session->m_response_buf);
            std::getline(strm, session->m_response);
         }

         onRequestComplete(session);
      });});});
   };

   // Cancels the request.  
   void cancelRequest(unsigned int request_id) {
      std::unique_lock<std::mutex>
         lock(m_active_sessions_guard);

      auto it = m_active_sessions.find(request_id);
      if (it != m_active_sessions.end()) {
         std::unique_lock<std::mutex>
            cancel_lock(it->second->m_cancel_guard);

         it->second->m_was_cancelled = true;
         it->second->m_sock.cancel();
      }
   }

   void close() {
      // Destroy work object. This allows the I/O thread to
      // exits the event loop when there are no more pending
      // asynchronous operations. 
      m_work.reset(NULL);

      // Wait for the I/O thread to exit.
      m_thread->join();
   }

private:
   void onRequestComplete(std::shared_ptr<Session> session) {
      // Shutting down the connection. This method may
      // fail in case socket is not connected. We don’t care 
      // about the error code if this function fails.
      boost::system::error_code ignored_ec;

      session->m_sock.shutdown(
         asio::ip::tcp::socket::shutdown_both,
         ignored_ec);

      // Remove session form the map of active sessions.
      std::unique_lock<std::mutex>
         lock(m_active_sessions_guard);

      auto it = m_active_sessions.find(session->m_id);
      if (it != m_active_sessions.end())
         m_active_sessions.erase(it);

      lock.unlock();

      boost::system::error_code ec;

      if (session->m_ec == 0 && session->m_was_cancelled)
         ec = asio::error::operation_aborted;
      else
         ec = session->m_ec;

      // Call the callback provided by the user.
      session->m_callback(session->m_id, 
         session->m_response, ec);
   };

private:
   asio::io_service m_ios;
   std::map<int, std::shared_ptr<Session>> m_active_sessions;
   std::mutex m_active_sessions_guard;
   std::unique_ptr<boost::asio::io_service::work> m_work;
   std::unique_ptr<std::thread> m_thread;
};
This class is the key component in our sample, providing most of the functionality of the application. This functionality is accessible to the user of the class through its public interface that contains three public methods:

void emulateLongComputationOp(unsigned int duration_sec, const std::string& raw_ip_address, unsigned short port_num, Callback callback, unsigned int request_id): This method initiates a request to the server
void cancelRequest(unsigned int request_id): This method cancels the previously initiated request designated by the request_id argument
void close(): This method blocks the calling thread until all the currently running requests complete and deinitializes the client. When this method returns, the corresponding instance of the AsyncTCPClient class can't be used anymore.
Now, we define a function that will serve as a callback, which we'll pass to the AsyncTCPClient::emulateLongComputationOp() method. In our case, this function is quite simple. It outputs the result of the request execution and the response message to the standard output stream if the request is completed successfully:

void handler(unsigned int request_id,
         const std::string& response, 
                const system::error_code& ec) 
{
  if (ec == 0) {
    std::cout << "Request #" << request_id
      << " has completed. Response: "
      << response << std::endl;
  } else if (ec == asio::error::operation_aborted) {
    std::cout << "Request #" << request_id
      << " has been cancelled by the user." 
            << std::endl;
  } else {
    std::cout << "Request #" << request_id
      << " failed! Error code = " << ec.value()
      << ". Error message = " << ec.message() 
             << std::endl;
  }

  return;
}
The handler() function's signature corresponds to the function pointer type Callback defined earlier.

Now that we have all the ingredients, we define an entry point of the application—the main() function—which demonstrates how to use the components defined above in order to communicate with the server. In our sample function, main() emulates the behavior of a human user by initiating three requests and canceling one of them:

int main()
{
  try {
    AsyncTCPClient client;

    // Here we emulate the user's behavior.

    // User initiates a request with id 1.
    client.emulateLongComputationOp(10, "127.0.0.1", 3333,
      handler, 1);
    // Then does nothing for 5 seconds.
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // Then initiates another request with id 2.
    client.emulateLongComputationOp(11, "127.0.0.1", 3334,
      handler, 2);
    // Then decides to cancel the request with id 1.
    client.cancelRequest(1);
    // Does nothing for another 6 seconds.
    std::this_thread::sleep_for(std::chrono::seconds(6));
    // Initiates one more request assigning ID3 to it.
    client.emulateLongComputationOp(12, "127.0.0.1", 3335,
      handler, 3);
    // Does nothing for another 15 seconds.
    std::this_thread::sleep_for(std::chrono::seconds(15));
    // Decides to exit the application.
    client.close();
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
};
How it works…
Our sample client application uses two threads of execution. The first one—UI thread—is responsible for processing a user input and initiating requests. The responsibility of the second thread—I/O thread—is to run the event loop and call the asynchronous operation's callback routines. Such configuration allows us to make our application's user interface responsive.

Starting the application – the main() entry point function
The main() function is invoked in the context of the UI thread. This function emulates the behavior of the user who initiates and cancels requests. Firstly, it creates an instance of the AsyncTCPClient class and then calls its emulateLongComputationOp() method three times to initiate three asynchronous requests, each time specifying a different target server. The first request (the one assigned ID 1) is canceled by calling the cancelRequest()method several seconds after the request has been initiated.

Request completion – the handler() callback function
For all three requests initiated in the main() function handler() is specified as a callback. This function is called when the request is finished regardless of the reason as to why it finished—be it a successful completion or an error. Also, this function is called when the request is canceled by the user. The function accepts three arguments as follows:

unsigned int request_id: This contains the unique identifier of the request. This is the same identifier that was assigned to the request when it was initiated.
std::string& response: This contains the response data. This value is considered valid only if the request is completed successfully and is not canceled.
system::error_code& ec: If an error occurs during a request life cycle, this object contains the error information. If the request was canceled, it contains the asio::error::operation_aborted value.
The handler() function is quite simple in our sample. Based on the values of the parameters passed to it, it outputs information about the finished request.

The AsyncTCPClient class – initializing
As it has already been mentioned, all the functionality related to communication with the server application is hidden in the AsyncTCPClient class. This class has a nonempty constructor that accepts no arguments and does two things. Firstly, it instantiates an object of the asio::io_service::work class passing an instance of the asio::io_service class named m_ios to its constructor. Then, it spawns a thread that calls the run() method of the m_ios object. The object of the asio::io_service::work class keeps threads running event loop from exiting this loop when there are no pending asynchronous operations. The spawned thread plays the role of I/O thread in our application; in the context of this thread, the callbacks assigned asynchronous operations will be invoked.

The AsyncTCPClient class – initiating a request
The emulateLongComputationOp() method is intended to initiate an asynchronous request. It accepts five arguments. The first one named duration_sec represents the request parameter according to the application layer protocol. The raw_ip_address and port_num specify the server to which the request should be sent. The next argument is a pointer to a callback function, which will be called when the request is complete. We'll turn back to the discussion of the callback later in this section. The last argument request_id is the unique identifier of the request. This identifier is associated with the request and is used to refer to it later, for example, when there is a need to cancel it.

The emulateLongComputationOp() method begins with preparing a request string and allocating an instance of the Session structure that keeps the data associated with the request including a socket object that is used to communicate with the server.

Then, the socket is opened and the pointer to the Session object is added to the m_active_sessions map. This map contains pointers to the Session objects associated with all active requests, that is, those requests that have been initiated but have not finished yet. When the request completes, before the corresponding callback is called, the pointer to the Session object associated with this request is removed from the map.

The request_id argument is used as a key of the corresponding Session object added to the map. We need to cache the Session objects in order to have access to them in case the user decides to cancel the previously initiated request. If we would not need to support canceling of a request, we could avoid using the m_active_sessions map.

We synchronize the access to the m_active_sessions map with a m_active_session_guard mutex. Synchronization is necessary because the map can be accessed from multiple threads. Items are added to it in UI thread, and removed in an I/O thread that calls a callback when the corresponding request is finished.

Now, when the pointer to the corresponding Session object is cached, we need to connect the socket to the server, which we do by calling the socket's async_connect() method:

session->m_sock.async_connect(session->m_ep,
  [this, session](const system::error_code& ec)
  { 
         // ...
  });
An endpoint object designating the server to which we want to connect and a callback function to be called when the connection is complete or an error occurs, are passed as arguments to this method. In our sample we use lambda function as a callback function. The call to the socket's async_connect() method is the last statement in the emulateLongComputationOp() method. When async_connect() returns, emulateLongComputationOp() returns too, which means that the request has been initiated.

Let's have a closer look at the lambda function that we pass to async_connect() as a callback. Here is its code:

[this, session](const system::error_code&ec)
{
  if (ec != 0) {
    session->m_ec = ec;
    onRequestComplete(session);
    return;
  }

  std::unique_lock<std::mutex>
    cancel_lock(session->m_cancel_guard);

  if (session->m_was_cancelled) {
     onRequestComplete(session);
     return;
  }

  asio::async_write(session->m_sock,
  asio::buffer(session->m_request),
        [this, session](const boost::system::error_code& ec,
              std::size_t bytes_transferred)
              {
                    //...
        });
}
The callback begins with checking the error code passed to it as the ec argument, the value of which when different from zero means that the corresponding asynchronous operation has failed. In case of failure, we store the ec value in the corresponding Session object, call the class's onRequestComplete() private method passing the Session object to it as an argument, and then return.

If the ec object designates success, we lock the m_cancel_guard mutex (the member of the request descriptor object) and check whether the request has not been canceled yet. More details about the canceling request are provided later in this section, where the cancelRequest() method is considered.

If we see that the request has not been canceled, we initiate the next asynchronous operation calling the Boost.Asio free function async_write() to send the request data to the server. Again, we pass to it a lambda function as a callback. This callback is very similar to the one passed to the anync_connect() method when the asynchronous connection operation was initiated. We first check the error code and then if it indicates success, we check whether or not the request has been canceled. Also, if it has not, we initiate the next asynchronous operation—async_read_until()—in order to receive a response from the server:

[this, session](const boost::system::error_code& ec,
         std::size_t bytes_transferred){
  if (ec != 0) {
    session->m_ec = ec;
    onRequestComplete(session);
    return;
  }

  std::unique_lock<std::mutex>
    cancel_lock(session->m_cancel_guard);

  if (session->m_was_cancelled) {
    onRequestComplete(session);
    return;
  }

  asio::async_read_until(session->m_sock,
        session->m_response_buf, '\n', 
     [this, session](const boost::system::error_code& ec,
              std::size_t b'ytes_transferred) 
        {
      // ...
        });
}
Again, we pass a lambda function as a callback argument to the async_read_until() free function. This callback function is quite simple:

[this, session](const boost::system::error_code& ec,
    std::size_t bytes_transferred) 
{
  if (ec != 0) {
    session->m_ec = ec;
  } else {
    std::istream strm(&session->m_response_buf);
    std::getline(strm, session->m_response);
  }

  onRequestComplete(session);
}
It checks the error code and in the case of success, it stores the received response data in the corresponding Session object. Then, the AsyncTCPClient class's private method onRequestComplete() is called and the Session object is passed to it as an argument.

The onRequestComplete() method is called whenever the request completes with any result. It is called when the request completes successfully, when the request fails at any stage of its life cycle, or when it is canceled by the user. The purpose of this method is to perform a cleanup and then to call a callback provided by the caller of the emulateLongComputationOp() method, when initiating this request.

The onRequestComplete() method begins with shutting down the socket. Note that here we use the overload of the socket's shutdown() method, which doesn't throw exceptions. We don't care if the shutting down of the connection fails as this is not a critical operation in our case. Then, we remove the corresponding entry from the m_active_sessions map as the request is finished and hence it is not active anymore. Also, as the last step, the user supplied callback is called. After the callback function returns, the request life cycle is finished.

The AsyncTCPClient class – canceling the request
Now, let's take a look at the cancelRequst() method of the AsyncTCPClient class. This method accepts an identifier of the request to be canceled as an argument. It begins with looking for the Session object corresponding to the specified request in the m_active_sessions map. If one is found, it calls the cancel() method on the socket object stored in this Session object. This leads to the interruption of the currently running asynchronous operation associated with this socket object.

However, there is a chance that the cancelRequest() method will be called at the moment when one asynchronous operation has already been completed and the next one has not been initiated yet. For example, imagine that the I/O thread is now running the callback of the async_connect() operation associated with a particular socket. At this moment, no asynchronous operation associated with this socket is in progress (because the next asynchronous operation async_write() has not been initiated yet); therefore, calling cancel() on this socket will have no effect. That's why we use an additional flag Session::m_was_cancelled designating, as its name suggests, whether the request has been canceled (or to be more precise, whether the cancelRequest() method has been called by the user). In the callback of the asynchronous operation, we look at the value of this flag before initiating the next asynchronous operation. If we see that the flag is set (which means that the request was canceled), we don't initiate the next asynchronous operation, but instead we interrupt the request execution and call the onRequestComplete() method.

We use the Session::m_cancel_guard mutex in the cancelRequest() method and in the callbacks of the asynchronous operations such as async_connect() and async_write() to enforce the following order of operations: request can be canceled either before the value of the Session::m_was_cancelled flag is tested in the callback, or after the next asynchronous operation is initiated. This order guarantees the proper canceling of a request whenever a user calls the cancelRequest() method.

The AsyncTCPClient class – closing the client
After the client has been used and is not needed anymore, it should be properly closed. The close() method of the AsyncTCPClient class allows us to do that. Firstly, this method destroys the m_work object that allows the I/O thread to exit the event message loop when all the asynchronous operations are completed. Then, it joins the I/O thread to wait until it exits.

After the close() method returns, the corresponding object of the AsyncTCPClient class cannot be used anymore.

There's more…
The AsyncTCPClient class in the presented sample implements an asynchronous single-threaded TCP client. It uses a single thread that runs the event loop and processes the requests. Usually, when the request rate is low, the size of the response is not large and the request handler does not perform the complex and time-consuming processing of the response (stage 5 of the request life cycle); one thread is enough.

However, when we want the client to make millions of requests and process them as fast as possible, we may want to turn our client into a multithreaded one, where multiple threads may run several requests truly simultaneously. Of course, it assumes that the computer running the client is a multicore or a multiprocessor computer. The application running more threads than the number of cores or processors installed in the computer may slow down the application due to the effect of the thread switching overhead.

Implementing a multithreaded TCP client application
In order to turn our single-treaded client application into a multithreaded one, we need to make several changes in it. Firstly, we need to replace the m_thread member of the AnyncTCPClient class that represents a single I/O thread, with a list of pointers to the std::thread objects, which will represent a collection of I/O threads:

std::list<std::unique_ptr<std::thread>> m_threads;
Next, we need to change the class's constructor so that it accepts an argument representing the number of I/O threads to be created. Besides, the constructor should spawn the specified number of I/O threads and add them all to the pool of threads running the event loop:

AsyncTCPClient(unsigned char num_of_threads){
  m_work.reset(new boost::asio::io_service::work(m_ios));

  for (unsigned char i = 1; i <= num_of_threads; i++) {
         std::unique_ptr<std::thread> th(
               new std::thread([this](){
        m_ios.run();
      }));

      m_threads.push_back(std::move(th));
    }
  }
Like in a single-threaded version of the client, each thread calls the run() method of the m_ios object. As a result, all threads are added to the thread pool, controlled by the m_ios object. All threads from the pool will be used to call the corresponding asynchronous operation completion callbacks. This means that on a multicore or multiprocessor computer, several callbacks may be running truly simultaneously in different threads, each on a separate processor; whereas, in a single-threaded version of the client, they would be executed serially.

After each thread is created, the pointer to it is put to the m_threads list so that we have the access to the thread objects later.

Also, the last change is in the close() method. Here, we need to join each thread in the list. This is how the method looks after the change:

void close() {
  // Destroy work object. This allows the I/O threads to
  // exit the event loop when there are no more pending
  // asynchronous operations. 
  m_work.reset(NULL);

  // Waiting for the I/O threads to exit.
  for (auto& thread : m_threads) {
    thread->join();
  }
}
```
Having destroyed the work object, we iterate through the list of I/O threads and join each of them to make sure they all have exited.

The multithreaded TCP client application is ready. Now, when we create an object of multithreaded AsyncTCPClient class, the number specifying how many threads should be used to process the requests should be passed to the constructor of the class. All other aspects of usage of the class are identical to those of a single-threaded one.

# How to build
```
mkdir build
cd build
cmake ..
cmake --build .
```

# How to run
```
./bin/asyncTCPClient
./bin/asyncTCPClientMT
```
