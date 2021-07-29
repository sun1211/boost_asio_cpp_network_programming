# Using extensible stream-oriented I/O buffers

Extensible buffers are those buffers that dynamically increase their size when new data is written to them. They are usually used to read data from sockets when the size of the incoming message is unknown.

Some application layer protocols do not define the exact size of the message. Instead, the boundary of the message is represented by a specific sequence of symbols at the end of the message itself or by a transport protocol service message end of file (EOF) issued by the sender after it finishes sending the message.

For example, according to the HTTP protocol, the header section of the request and response messages don't have a fixed length and its boundary is represented by a sequence of four ASCII symbols, <CR><LF><CR><LF>, which is part of the message. In such cases, dynamically extensible buffers and functions that can work with them, which are provided by the Boost.Asio library, are very useful.

Extensible stream-oriented buffers are represented in Boost.Asio with the asio::streambuf class, which is a typedef for asio::basic_streambuf:
```
typedef basic_streambuf<> streambuf;
```

The asio::basic_streambuf<> class is inherited from std::streambuf, which means that it can be used as a stream buffer for STL stream classes. In addition to this, several I/O functions provided by Boost.Asio deal with buffers that are represented as objects of this class.

We can work with an object of the asio::streambuf class just like we would work with any stream buffer class that is inherited from the std::streambuf class. For example, we can assign this object to a stream (for example, std::istream, std::ostream, or std::iostream, depending on our needs), and then, use stream's operator<<() and operator>>() operators to write and read data to and from the stream.

# How it works
The main() application entry point function begins with instantiating an object of the asio::streambuf class named buf. Next, the output stream object of the std::ostream class is instantiated. The buf object is used as a stream buffer for the output stream.

In the next line, the Message1\nMessage2 sample data string is written to the output stream object, which in turn redirects the data to the buf stream buffer.

Usually, in a typical client or server application, the data will be written to the buf stream buffer by the Boost.Asio input function such as asio::read(), which accepts a stream buffer object as an argument and reads data from the socket to that buffer.

Now, we want to read the data back from the stream buffer. To do this, we allocate an input stream and pass the buf object as a stream buffer argument to its constructor. After this, we allocate a string object named message1, and then, use the std::getline function to read part of the string currently stored in the buf stream buffer until the delimiter symbol, \n.

As a result, the string1 object contains the Message1 string and the buf stream buffer contains the rest of the initial string after the delimiter symbol, that is, Message2.

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
