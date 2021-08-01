# Using timers

The Boost.Asio library provides two template classes that implement timers. One of them is asio::basic_deadline_timer<>, which was the only one available before Boost.Asio 1.49 version was released. In version 1.49, the second timer asio::basic_waitable_timer<> class template was introduced.

The asio::basic_deadline_timer<> class template was designed to be compatible with the Boost.Chrono library and internally relies on the functionality it provides. This template class is somewhat outdated and provides a limited functionality. Therefore, we will not consider it in this recipe.

On the contrary, a newer asio::basic_waitable_timer<> class template, which is compatible with the C++11 chrono library is more flexible and provides more functionalities. Boost.Asio includes three typedefs for classes that are generically derived from the asio::basic_waitable_timer<> template class:
```
typedef basic_waitable_timer< std::chrono::system_clock >
   system_timer;
typedef basic_waitable_timer< std::chrono::steady_clock > 
   steady_timer;
typedef basic_waitable_timer< std::chrono::high_resolution_clock >
   high_resolution_timer;
```
The asio::system_timer class is based on the std::chrono::system_clock class, which represents a system-wide real-time clock. This clock (and so is the timer) is influenced by external changes of the current system time. Therefore, the asio::system_timer class is a good choice when we need to set up a timer that will notify us when a certain absolute time point is reached (for instance, 13h:15m:45s), taking into account the system clock shifts made after the timer was set up. However, this timer is not good at measuring time intervals (for instance, 35 seconds from now) because the system clock shifts may result in the timer expiring sooner or later than the actual interval elapses.

The asio::steady_timer class is based on the std::chrono::steady_clock class, which represents a steady clock that is not influenced by the system clock changes. It means that asio::steady_timer is a good choice to measure intervals.

The last timer asio::high_resolution_timer class is based on the std::chrono::high_resolution_clock class, which represents a high-resolution system clock. It can be used in cases when high precision in time measurement is required.

In distributed applications implemented with the Boost.Asio library, timers are usually used to implement timeout periods for asynchronous operations. Just after the asynchronous operation starts (for example, asio::async_read()), the application will start a timer set up to expire after a certain period of time, a timeout period. When the timer expires, the application checks whether the asynchronous operation has completed and if it has not, the operation is considered timed out and is canceled.

Because a steady timer is not influenced by the system clock shifts, it is the best fit to implement the timeout mechanism.


Note that on some platforms, steady clocks are not available and the corresponding class that represents a std::chrono::steady_clock exhibits behavior that is identical to that of std::chrono::stystem_clock, which means that just like the latter, it is influenced by the changes of the system clock. It is advised to refer to the documentation of the platform and corresponding C++ standard library implementation to find out whether the steady clock is actually steady.

Let's consider a somewhat unrealistic but representative sample application that demonstrates how to create, start, and cancel Boost.Asio timers. In our sample, we will create and start two steady timers one by one. When the first timer expires, we will cancel the second one, before it has a chance to expire.

We begin our sample application with including the necessary Boost.Asio headers and the using directive:
```
#include <boost/asio/steady_timer.hpp>
#include <iostream>

using namespace boost;
```
Next, we define the only component in our application: the main() entry point function:
```
int main()
{
```

Like almost any nontrivial Boost.Asio application, we need an instance of the asio::io_service class:
```
asio::io_service ios;
```

Then, we create and start the first t1 timer, which is set up to expire in 2 seconds:
```
 asio::steady_timer t1(ios);
 t1.expires_from_now(std::chrono::seconds(2));
```
Then, we create and start the second t2 timer, which is set up to expire in 5 seconds. It should definitely expire later than the first timer:
```
 asio::steady_timer t2(ios);
t2.expires_from_now(std::chrono::seconds(5));
```
Now, we define and set a callback function that is to be called when the first timer expires:
```
t1.async_wait([&t2](boost::system::error_code ec) {
      if (ec == 0) {
         std::cout << "Timer #2 has expired!" << std::endl;
      }
      else if (ec == asio::error::operation_aborted) {
         std::cout << "Timer #2 has been cancelled!" 
                     << std::endl;
      }
      else {
         std::cout << "Error occured! Error code = "
            << ec.value()
            << ". Message: " << ec.message() 
                      << std::endl;
      }

      t2.cancel();
   });
```
Then, we define and set another callback function that is to be called when the second timer expires:
```
 t2.async_wait([](boost::system::error_code ec) {
      if (ec == 0) {
         std::cout << "Timer #2 has expired!" << std::endl;
      }
      else if (ec == asio::error::operation_aborted) {
         std::cout << "Timer #2 has been cancelled!" 
<< std::endl;
      }
      else {
         std::cout << "Error occured! Error code = "
            << ec.value()
            << ". Message: " << ec.message() 
<< std::endl;
      }
   });
```
In the last step, we call the run() method on the instance of the asio::io_service class:
```
 ios.run();

  return 0;
}
```
# How it works
Now, let's track the application's execution path to better understand how it works.

The main() function begins with creating an instance of the asio::io_service class. We need it because just like sockets, acceptors, resolvers, and other components defined by the Boost.Asio library, which use operating system services, timers require an instance of the asio::io_service class as well.

In the next step, the first timer named t1 is instantiated and then the expires_from_now() method is called on it. This method switches the timer to a non-expired state and starts it. It accepts an argument that represents the duration of the time interval, after which the timer should expire. In our sample, we pass an argument that represents the duration of 2 seconds, which means that in 2 seconds, from the moment when the timer starts, it will expire and all those who are waiting for this timer's expiration event will be notified.

Next, the second timer named t2 is created, which is then started and set up to expire in 5 seconds.

When both the timers are started, we asynchronously wait for the timers' expiration events. In other words, we register callbacks on each of the two timers, which will be invoked when the corresponding timers expire. To do this, we call the timer's async_wait() method and pass the pointer to the callback function as an argument. The async_wait() method expects its argument to be a pointer to the function that has the following signature:

```
void callback(
  const boost::system::error_code& ec);
```

The callback function accepts a single ec argument, which designates the wait completion status. In our sample application, we use lambda functions as expiration callbacks for both the timers.

When both timer expiration callbacks are set, the run() method is called on the ios object. The method blocks until both the timers expire. The thread, in the context of which the method run() is invoked, will be used to invoke the expiration callbacks.

When the first timer expires, the corresponding callback function is invoked. It checks the wait completion status and outputs corresponding messages to the standard output stream. And then it cancels the second timer by calling the cancel() method on the t2 object.

The canceling of the second timer leads to the expiration callback being called with the status code, notifying that the timer was canceled before expiration. The expiration callback of the second timer checks the expiration status and outputs corresponding messages to the standard output stream and returns.

When both callbacks are completed, the run() method returns and the execution of the main() function runs to the end. This is when the execution of the application is completed.

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

Timer #2 has expired!
Timer #2 has been cancelled!
```
