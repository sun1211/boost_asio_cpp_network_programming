# Using fixed length I/O buffers

Fixed length I/O buffers are usually used with I/O operations and play the role of either a data source or destination when the size of the message to be sent or received is known. For example, this can be a constant array of chars allocated on a stack, which contain a string that represents the request to be sent to the server. Or, this can be a writable buffer allocated in the free memory, which is used as a data destination point, when reading data from a socket.

In Boost.Asio, a fixed length buffer is represented by one of the two classes: asio::mutable_buffer or asio::const_buffer. Both these classes represent a contiguous block of memory that is specified by the address of the first byte of the block and its size in bytes. As the names of these classes suggest, asio::mutable_buffer represents a writable buffer, whereas asio::const_buffer represents a read-only one.

However, neither the asio::mutable_buffer nor asio::const_buffer classes are used in Boost.Asio I/O functions and methods directly. Instead, the MutableBufferSequence and ConstBufferSequence concepts are introduced.

The MutableBufferSequence concept specifies an object that represents a collection of the asio::mutable_buffer objects. Correspondingly, the ConstBufferSequence concept specifies an object that represents a collection of the asio::const_buffer objects. Boost.Asio functions and methods that perform I/O operations accept objects that satisfy the requirements of either the MutableBufferSequence or ConstBufferSequence concept as their arguments that represent buffers.

A complete specification of the MutableBufferSequence and ConstBufferSequence concepts are available in the Boost.Asio documentation section, which can be found at the following links:

Refer to http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/MutableBufferSequence.html for MutableBufferSequence
Refer to http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/reference/ConstBufferSequence.html for ConstBufferSequence

Although in most use cases, a single buffer is involved in a single I/O operation, in some specific circumstances (for example, in a memory-constrained environment), a developer may want to use a composite buffer that comprises multiple smaller simple buffers distributed over the process's address space. Boost.Asio I/O functions and methods are designed to work with composite buffers that are represented as a collection of buffers that fulfill the requirements of either the MutableBufferSequence or ConstBufferSequence concept.

For instance, an object of the std::vector<asio::mutable_buffer> class satisfies the requirements of the MutableBufferSequence concept, and therefore, it can be used to represent a composite buffer in I/O-related functions and methods.

So, now we know that if we have a buffer that is represented as an object of the asio::mutable_buffer or asio::const_buffer class, we still can't use it with I/O-related functions or methods provided by Boost.Asio. The buffer must be represented as an object, satisfying the requirements of either the MutableBufferSequence or ConstBufferSequence concept, respectively. To do this, we for example could create a collection of buffer objects consisting of a single buffer by instantiating an object of the std::vector<asio::mutable_buffer> class and placing our buffer object into it. Now that the buffer is part of the collection, satisfying the MutableBufferSequence requirements can be used in I/O operations.

However, although this method is fine to create composite buffers consisting of two or more simple buffers, it looks overly complex when it comes to such simple tasks as representing a single simple buffer so that it can be used with Boost.Asio I/O functions or methods. Fortunately, Boost.Asio provides us with a way to simplify the usage of single buffers with I/O-related functions and methods.

The asio::buffer() free function has 28 overloads that accept a variety of representations of a buffer and return an object of either the asio::mutable_buffers_1 or asio::const_buffers_1 classes. If the buffer argument passed to the asio::buffer() function is a read-only type, the function returns an object of the asio::const_buffers_1 class; otherwise, an object of the asio::mutable_buffers_1 class is returned.

The asio::mutable_buffers_1 and asio::const_buffers_1 classes are adapters of the asio::mutable_buffer and asio::const_buffer classes, respectively. They provide an interface and behavior that satisfy the requirements of the MutableBufferSequence and ConstBufferSequence concepts, which allows us to pass these adapters as arguments to Boost.Asio I/O functions and methods.

Let's consider two algorithms and corresponding code samples that describe how to prepare a memory buffer that can be used with Boost.Asio I/O operations. The first algorithm deals with buffers intended to be used for an output operation and the second one is used for an input operation.

## PREPARING A BUFFER FOR AN OUTPUT OPERATION
The following algorithm and corresponding code sample describes how to prepare a buffer that can be used with the Boost.Asio socket's method that performs an output operation such as asio::ip::tcp::socket::send() or the asio::write()free function:

- Allocate a buffer. Note that this step does not involve any functionality or data types from Boost.Asio.
- Fill the buffer with the data that is to be used as the output.
- Represent the buffer as an object that satisfies the ConstBufferSequence concept's requirements.
- The buffer is ready to be used with Boost.Asio output methods and functions.

Let's consider the first code sample that demonstrates how to prepare a buffer that can be used with Boost.Asio output methods and functions. The main()entry point function starts with instantiating the object of the std::string class. Because we want to send a string of text, std::string is a good candidate to store this kind of data. In the next line, the string object is assigned a value of Hello. This is where the buffer is allocated and filled with data. This line implements steps 1 and 2 of the algorithm.

Next, before the buffer can be used with Boost.Asio I/O methods and functions, it must be properly represented. To better understand why this is needed, let's take a look at one of the Boost.Asio output functions. Here is the declaration of the send()method of the Boost.Asio class that represents a TCP socket:
```
template<typename ConstBufferSequence>
std::size_t send(const ConstBufferSequence & buffers);
```

As we can see, this is a template method, and it accepts an object that satisfies the requirements of the ConstBufferSeqenece concept as its argument that represents the buffer. A suitable object is a composite object that represents a collection of objects of the asio::const_buffer class and provides a typical collection interface that supports an iteration over its elements. For example, an object of the std::vector<asio::const_buffer> class is suitable for being used as the argument of the send() method, but objects of the std::string or asio::const_bufer class are not.

In order to use our std::string object with the send()method of the class that represents a TCP socket, we can do something like this:

```
asio::const_buffer asio_buf(buf.c_str(), buf.length());
std::vector<asio::const_buffer> buffers_sequence;
buffers_sequence.push_back(asio_buf);
```

The object named buffer_sequence in the preceding snippet satisfies the ConstBufferSequence concept's requirements, and therefore, it can be used as an argument for the send() method of the socket object. However, this approach is very complex. Instead, we use the asio::buffer()function provided by Boost.Asio to obtain adaptor objects, which we can directly use in I/O operations:
```
asio::mutable_buffers_1 output_buf = asio::buffer(buf);
```

After the adaptor object is instantiated, it can be used with Boost.Asio output operations to represent the output buffer.

## PREPARING A BUFFER FOR AN INPUT OPERATION
The following algorithm and corresponding code sample describes how to prepare the buffer that can be used with the Boost.Asios socket's method that performs an input operation such as asio::ip::tcp::socket::receive() or the asio::read()free function:

- Allocate a buffer. The size of the buffer must be big enough to fit the block of data to be received. Note that this step does not involve any functionalities or data types from Boost.Asio.
- Represent the buffer using an object that satisfies the MutableBufferSequence concept's requirements.
- The buffer is ready to be used with Boost.Asio input methods and functions.

The second code sample is very similar to the first one. The main difference is that the buffer is allocated but is not filled with data because its purpose is different. This time, the buffer is intended to receive the data from a remote application during the input operation.

With an output buffer, an input buffer must be properly represented so that it can be used with Boost.Asio I/O methods and functions. However, in this case, the buffer must be represented as an object that meets the requirements of the MutableBufferSequence concept. Contrary to ConstBufferSequence, this concept represents the collection of mutable buffers, that is, those that can be written to. Here, we use the buffer() function, which helps us create the required representation of the buffer. The object of the mutable_buffers_1 adaptor class represents a single mutable buffer and meets the MutableBufferSequence concept's requirements.

In the first step, the buffer is allocated. In this case, the buffer is an array of chars allocated in the free memory. In the next step, the adaptor object is instantiated that can be used with both the input and output operations.

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
