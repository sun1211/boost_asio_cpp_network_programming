# Chapter 6: Other Topics

- [1. Using composite buffers for scatter/gather operations](recipe_01/README.md)
- [2. Using timers](recipe_02/README.md)
- [3. Getting and setting socket options](recipe_03/README.md)
- [4. Performing a stream-based I/O](recipe_04/README.md)

his final chapter includes four recipes that stand somewhat apart from those in previous chapters that demonstrate the core Boost.Asio concepts, covering the majority of typical use cases. However, it does not mean that recipes demonstrated in this chapter are less important. On the contrary, they are very important and even critical to specific cases. However, they will be required less often in typical distributed applications.

Though most applications will not require scatter/gather I/O operations and composite buffers, for some, which keep different parts of messages in separate buffers, such facilities may turn out to be very usable and convenient.

The Boost.Asio timer is a powerful instrument that allows measuring time intervals. Often, this is used to set deadlines for the operations that may last unpredictably long and to interrupt those operations if they do not complete after running for a certain period of time. For many distributed applications, such an instrument is critical, taking into account the fact that Boost.Asio does not provide a way to specify a timeout for potentially long-running operations. In addition to this, timers provided by Boost.Asio can be used to solve other tasks that are not related to network communication.

Tools that allow getting and setting socket options are quite important as well. When developing a simple network application, the developer may be fully satisfied with the socket equipped with default values of the options that are automatically set during instantiation of the socket object. However, in more sophisticated cases, it may be absolutely necessary to reconfigure the socket by customizing the values of its options.

Boost.Asio classes that wrap the socket and provide a stream-like interface to it allow us to create simple and elegant distributed applications. And simplicity is known to be one of the key characteristics of a good software.