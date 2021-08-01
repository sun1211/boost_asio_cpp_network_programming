# Getting and setting socket options

The socket's properties and its behavior can be configured by changing the values of its various options. When the socket object is instantiated, its options have default values. In many cases, the socket configured by default is a perfect fit, whereas in others, it may be needed to fine tune the socket by changing values of its options so that it meets the requirements of the application.

Each socket option, whose value can be set or obtained by means of a functionality provided by Boost.Asio, is represented by a separate class. The complete list of classes that represent setting or getting socket options, which are supported by Boost.Asio, can be found on this Boost.Asio documentation page at http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/socket_base.html.

Note that there are fewer classes that represent socket options listed on this page than the options that can be set or obtained from a native socket (an object of the underlying operating system). This is because Boost.Asio supports only a limited amount of socket options. To set or obtain values of other socket options, developers may need to extend the Boost.Asio library by adding classes that represent the required options. However, the topic on the extension of the Boost.Asio library is beyond the scope of this book. We will focus on how to work with socket options that are supported by the library out of the box.

Let's consider a hypothetical situation where we want to make the size of the socket's receive buffer two times bigger than whatever its size is now. To do this, we first need to get the current size of the buffer, then multiply it by two, and finally, set the value obtained after multiplication as the new receive buffer size.

The following sample demonstrates how to do this in the following code:
```
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int main()
{
  try {
    asio::io_service ios;

    // Create and open a TCP socket.
    asio::ip::tcp::socket sock(ios, asio::ip::tcp::v4());

    // Create an object representing receive buffer
      // size option.
    asio::socket_base::receive_buffer_size cur_buf_size;

    // Get the currently set value of the option. 
    sock.get_option(cur_buf_size);

    std::cout << "Current receive buffer size is "
      << cur_buf_size.value() << " bytes."
      << std::endl;

    // Create an object representing receive buffer
      // size option with new value.
    asio::socket_base::receive_buffer_size
      new_buf_size(cur_buf_size.value() * 2);

    // Set new value of the option.
    sock.set_option(new_buf_size);

    std::cout << "New receive buffer size is "
      << new_buf_size.value() << " bytes."
      << std::endl;
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
}
```

# How it works

Our sample consists of a single component: the main() entry point function. This function begins with creating an instance of the asio::io_service class. This instance is then used to create an object that represents a TCP socket.

Note the usage of the socket class constructor, which creates and opens the socket. Before we can get or set options on a particular socket object, the corresponding socket must be opened. This is because before the Boost.Asio socket object is opened, the underlying native socket object of the corresponding operating system is not yet allocated, and there is nothing to set the options on or get them from.

Next, an instance of the asio::socket_base::receive_buffer_size class is instantiated. This class represents an option that controls the size of the socket's receive buffer. To obtain the current value of the option, the get_option() method is called on the socket object and the reference to the option object is passed to it as an argument.

The get_option() method deduces the option that is requested by the type of the argument passed to it. Then, it stores the corresponding option's value in the option object and returns. The value of the option can be obtained from the object that represents the corresponding option by invoking the object's value() method, which returns the value of the option.

After the current value of receive buffer size option is obtained and output to the standard output stream, in order to set the new value of this option, the main() function proceeds with creating one more instance of the asio::socket_base::receive_buffer_size class named new_buf_size. This instance represents the same option as the first instance, cur_buf_size, but this one contains the new value. The new option value is passed to the option object as an argument of its constructor.

After the option object that contains the new receive buffer size option value is constructed, the reference to it is passed as an argument to the socket's set_option() method. Like get_option(), this method deduces the option to be set by the type of the argument passed to it, and then, sets the corresponding option value, making the new value equal to the one stored in the option object.

In the last step, the new option's value is output to the standard output stream.


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

Current receive buffer size is 65536 bytes.
New receive buferr size is 131072 bytes.
```
