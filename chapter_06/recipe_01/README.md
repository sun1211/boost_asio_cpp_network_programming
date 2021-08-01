# Using composite buffers for scatter/gather operations

A composite buffer is basically a complex buffer that consists of two or more simple buffers (contiguous blocks of memory) distributed over the process' address space. Such buffers become especially handy in two situations.

The first situation is when the application needs a buffer either to store the message before sending it to the remote application or to receive the message sent by the remote application. The problem is that the size of the message is so big that allocating a single contiguous buffer that is sufficient to store it may fail due to the process' address space fragmentation. In this case, allocating multiple smaller buffers, whose sizes when summed would be enough to store the data, and combining them in a single composite buffer is a good solution to the problem.

Another situation is actually the first one inverted. Due to specificity of the design of the application, the message to be sent to the remote application is broken into several parts and stored in different buffers, or if the message to be received from the remote application needs to be broken into several parts, each of which should be stored in a separate buffer for further processing. In both the cases, combining several buffers into one composite buffer and then using scatter send or gather receive operations would be a good approach to the problem.

Let's consider two algorithms and corresponding code samples that describe how to create and prepare a composite buffer that is to be used with Boost.Asio I/O operations. The first algorithm deals with the composite buffer intended for use in gather output operations and the second one for scatter input operations.

## Preparing a composite buffer for gather output operations
The following is the algorithm and corresponding code sample that describe how to prepare the composite buffer that is to be used with the socket's method that performs output operations such as asio::ip::tcp::socket::send() or a free function such as asio::write():
- Allocate as many memory buffers as needed to perform the task at hand. Note that this step does not involve any functionality or data types from Boost.Asio.
- Fill the buffers with data to be output.
- Create an instance of a class that satisfies the ConstBufferSequence or MultipleBufferSequence concept's requirements, representing a composite buffer.
- Add simple buffers to the composite buffer. Each simple buffer should be represented as an instance of the asio::const_buffer or asio::mutable_buffer classes.
- The composite buffer is ready to be used with Boost.Asio output functions.

Let's say we want to send a string Hello my friend! to the remote application, but our message was broken into three parts and each part was stored in a separate buffer. What we can do is represent our three buffers as a composite buffer, and then, use it in the output operation. This is how we will do it in the following code:
```
#include <boost/asio.hpp>

using namespace boost;

int main()
{
    // Steps 1 and 2. Create and fill simple buffers.
    const char *part1 = "Hello ";
    const char *part2 = "my ";
    const char *part3 = "friend!";

    // Step 3. Create an object representing a composite buffer.
    std::vector<asio::const_buffer> composite_buffer;

    // Step 4. Add simple buffers to the composite buffer.
    composite_buffer.push_back(asio::const_buffer(part1, 6));
    composite_buffer.push_back(asio::const_buffer(part2, 3));
    composite_buffer.push_back(asio::const_buffer(part3, 7));

    // Step 5. Now composite_buffer can be used with Boost.Asio
    // output operations as if it was a simple buffer represented
    // by contiguous block of memory.

    return 0;
}

```
## Preparing a composite buffer for an input operation
The following is the algorithm and corresponding code sample that describe how to prepare the composite buffer that is to be used with the socket's method that performs an input operation such as asio::ip::tcp::socket::receive() or a free function such as asio::read():
- Allocate as many memory buffers as required to perform the task at hand. The sum of the sizes of the buffers must be equal to or greater than the size of the expected message to be received in these buffers. Note that this step does not involve any functionalities or data types from Boost.Asio.
- Create an instance of a class that satisfies the MutableBufferSequence concept's requirements that represents a composite buffer.
- Add simple buffers to the composite buffer. Each simple buffer should be represented as an instance of the asio::mutable_buffer class.
- The composite buffer is ready to be used with Boost.Asio input operations.
Let's imagine a hypothetical situation, where we want to receive 16 bytes long messages from the server. However, we do not have a buffer that can fit the entire message. Instead, we have three buffers: 6, 3, and 7 bytes long. To create a buffer in which we can receive 16 bytes of data, we can join our three small buffers into a composite one. This is how we do it in the following code:
```
#include <boost/asio.hpp>

using namespace boost;

int main()
{
  // Step 1. Allocate simple buffers.
  char part1[6];
  char part2[3];
  char part3[7];

  // Step 2. Create an object representing a composite buffer.
  std::vector<asio::mutable_buffer> composite_buffer;

  // Step 3. Add simple buffers to the composite buffer object.
  composite_buffer.push_back(asio::mutable_buffer(part1,
  sizeof(part1)));
  composite_buffer.push_back(asio::mutable_buffer(part2,
  sizeof(part2)));
  composite_buffer.push_back(asio::mutable_buffer(part3,
  sizeof(part3)));

  // Now composite_buffer can be used with Boost.Asio 
  // input operation as if it was a simple buffer 
  // represented by contiguous block of memory.

  return 0;
}
```

# How it works
Let's see how the first sample works. It starts with allocating three read-only buffers that are filled with parts of the message string Hello my friend!.

In the next step, an instance of the std::vector<asio::const_buffer> class is created, which is the embodiment of the composite buffer. The instance is given the corresponding name, composite_buffer. Because the std::vector<asio::const_buffer> class satisfies the requirements of ConstBufferSequence, its objects can be used as composite buffers and can be passed to Boost.Asio gather output functions and methods as arguments that represent the data source.

In step 4, each of our three buffers is represented as an instance of the asio::const_buffer class and added to the composite buffer. Because all Boost.Asio output functions and methods that work with fixed-sized buffers are designed to work with composite buffers as well, our composite_buffer object can be used with them like a simple buffer.

The second sample works quite similar to the first one. The only difference is that because the composite buffer created in this sample is intended to be used as a data destination (rather than a data source as it is in the first sample), the three simple buffers added to it are created as writable ones and are represented as instances of the asio::mutable_buffer class when added to the composite buffer.

Another thing to note about the second sample is that because the composite buffer created in this sample is composed of mutable buffers, it can be used in both gather output and scatter input operations. In this particular sample, the initial buffers (part1, part2, and part3) are not filled with any data and they contain garbage; and therefore, using them in output operations is senseless unless they are filled with meaningful data.

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
