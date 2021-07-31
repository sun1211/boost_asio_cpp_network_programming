# The OSI reference model

The OSI model is used to connect to the open systems—these are the systems that are open and communicate with other systems. By using this model, we do not depend on an operating system anymore, so we are allowed to communicate with any operating system on any computer. This model contains seven layers, where each layer has a specific function and defines the way data is handled on certain different layers. The seven layers that are contained in this model are the **Physical layer**, **Data Link layer**, **Network layer**, **Transport layer**, **Session layer**, **Presentation layer**, and the **Application layer**.

## THE PHYSICAL LAYER
This is the first layer in the OSI model and contains a definition of the network's physical specification, including the physical media (cables and connectors) and basic devices (repeaters and hubs). The layer is responsible for the input raw bits transmission data stream into zeros and for the ones that are on the communication channel. It then places the data onto the physical media. It is concerned with data transmission integrity and makes sure that the bits that are sent from one device are exactly the same as the data that is received by the other device

## THE DATA LINK LAYER
The main role of the Data Link layer is to provide a link for raw data transmission. Before the data is transmitted, it is broken up into data frames, and the Data Link layer transmits them consecutively. The receiver will send back an acknowledge frame for each frame that has been sent if the service is reliable.

This layer consists of two sublayers: **Logical Link Control (LLC**) and **Media Access Control (MAC)**. The LLC sublayer is responsible for transmission error checking and deals with frame transmission, while the MAC sublayer defines how to retrieve data from the physical media or store data in the physical media.

We can also find the MAC address, also called as the **physical address**, in this layer. The MAC address is used to identify every device that connects to the network because it is unique for each device.

The MAC address contains twelve hexadecimal characters, where two digits are paired with each other. The first six digits represent the organizationally unique identifier and the remaining digits represent the manufacturer serial number. If you are really curious to know what this number means, you can go to *www.macvendorlookup.com* and fill the text box with our MAC address to know more about it.

## THE NETWORK LAYER
The Network layer is responsible for defining the best way to route the packets from a source to the destination device. It will generate routing tables using **Internet Protocol (IP)** as the routing protocol, and the IP address is used to make sure that the data gets its route to the required destination. There are two versions of IP nowadays: **IPv4** and **IPv6**. In IPv4, we use 32-bit addresses to address the protocol and we use 128-bit addresses in IPv6. You are going to learn more about Internet Protocol, IPv4, and IPv6 in the next topic.


## THE TRANSPORT LAYER
The Transport layer is responsible for transferring data from a source to destination. It will split up the data into smaller parts, or in this case segments, and then will join all the segments to restore the data to its initial form in the destination.

There are two main protocols that work in this layer: the Transmission Control Protocol (TCP) and the User Datagram Protocol (UDP).
 - TCP supplies the delivery of data by establishing a session. The data will not be transmitted until a session is established. TCP is also known as the connection-oriented protocol, which means that the session has to be established before transmitting the data.
 - UDP is a method of delivering data with the best efforts, but does not give a guaranteed delivery because it does not establish a session. Therefore, UDP is also known as the connection-less protocol. In-depth explanation about TCP and UDP can be found in the next topic.

## THE SESSION LAYER
The Session layer is responsible for the establishment, maintenance, and termination of the session. We can analogize the session like a connection between two devices on the network. For example, if we want to send a file from a computer to another, this layer will establish the connection first before the file can be sent. This layer will then make sure that the connection is still up until the file is sent completely. Finally, this layer will terminate the connection if it is no longer needed. The connection we talk about is the session.

This layer also makes sure that the data from a different application is not interchanged. For example, if we run the Internet browser, chat application, and download manager at the same time, this layer will be responsible for establishing the session for every single application and ensure that they remain separated from other applications.

There are three communication methods that are used by this layer: the simplex, half-duplex, or full-duplex method.
- In the simplex method, data can only be transferred by one party, so the other cannot transfer any data. This method is no longer common in use, since we need applications that can interact with each other.
- In the half-duplex method, any data can be transferred to all the involved devices, but only one device can transfer the data in the time, after it completes the sending process. Then, the others can also send and transfer data.
- The full-duplex method can transfer data to all the devices at the same time. To send and receive data, this method uses different paths.

## THE PRESENTATION LAYER
The Presentation layer role is used to determine the data that has been sent, to translate the data into the appropriate format, and then to present it. For example, we send an MP3 file over the network and the file is split up into several segments. Then, using the header information on the segment, this layer will construct the file by translating the segments.

Moreover, this layer is responsible for data compression and decompression because all the data transmitted over the Internet is compressed to save the bandwidth. This layer is also responsible for data encryption and decryption in order to secure communication between two devices.

## THE APPLICATION LAYER
The Application layer deals with the computer application that is used by a user. Only the application that connects to a network will connect to this layer. This layer contains several protocols that are needed by a user, which are as follows:

 - The Domain Name System (DNS): This protocol is the one that finds the hostname of an IP address. With this system, we do not need to memorize every IP address any longer, just the hostname. We can easily remember a word in the hostname instead of a bunch of numbers in the IP address.
 - The Hypertext Transfer Protocol (HTTP): This protocol is the one that transmits data over the Internet on web pages. We also have the HTTPS format that is used to send encrypted data for security issues.
 - The File Transfer Protocol (FTP): This protocol is the one that is used to transfer files from or to an FTP server.
 - The Trivial FTP (TFTP): This protocol is similar to FTP, which is used to send smaller files.
 - The Dynamic Host Configuration Protocol (DHCP): This protocol is a method that is used to assign the TCP/IP configuration dynamically.
 - The Post Office Protocol (POP3): This protocol is an electronic mail protocol used to get back e-mails from POP3 servers. The server is usually hosted by an Internet Service Provider (ISP).
 - The Simple Mail Transfer Protocol (SMTP): This protocol is in contrast with POP3 and is used to send electronic mails.
 - The Internet Message Access Protocol (IMAP): This protocol is used to receive e-mail messages. With this protocol, users can save their e-mail messages on their folder on a local computer.
 - The Simple Network Management Protocol (SNMP): This protocol is used to manage network devices (routers and switches) and detect problems to report them before they become significant.
 - The Server Message Block (SMB): This protocol is an FTP that is used on Microsoft networks primarily for file and printer sharing.

This layer also decides whether enough network resources are available for network access. For instance, if you want to surf the Internet using an Internet browser, the Application layer decides whether access to the Internet is available using HTTP.

We can divide all the seven layers into two section layers: the Upper Layer and Lower Layer. The upper layer is responsible for interacting with the user and is less concerned about the low-level details, whereas the lower layer is responsible for transferring data over the network, such as formatting and encoding.


# Building Boost libraries
As we discussed previously, most libraries in Boost are header-only, but not all of them. There are some libraries that have to be built separately. They are:

 - Boost.Chrono: This is used to show the variety of clocks, such as current time, the range between two times, or calculating the time passed in the process.
 - Boost.Context: This is used to create higher-level abstractions, such as coroutines and cooperative threads.
 - Boost.Filesystem: This is used to deal with files and directories, such as obtaining the file path or checking whether a file or directory exists.
 - Boost.GraphParallel: This is an extension to the Boost Graph Library (BGL) for parallel and distributed computing.
 - Boost.IOStreams: This is used to write and read data using stream. For instance, it loads the content of a file to memory or writes compressed data in GZIP format.
 - Boost.Locale: This is used to localize the application, in other words, translate the application interface to user's language.
 - Boost.MPI: This is used to develop a program that executes tasks concurrently. MPI itself stands for Message Passing Interface.
 - Boost.ProgramOptions: This is used to parse command-line options. Instead of using the argv variable in the main parameter, it uses double minus (--) to separate each command-line option.
 - Boost.Python: This is used to parse Python language in C++ code.
 - Boost.Regex: This is used to apply regular expression in our code. But if our development supports C++11, we do not depend on the Boost.Regex library anymore since it is available in the regex header file.
 - Boost.Serialization: This is used to convert objects into a series of bytes that can be saved and then restored again into the same object.
 - Boost.Signals: This is used to create signals. The signal will trigger an event to run a function on it.
 - Boost.System: This is used to define errors. It contains four classes: system::error_code, system::error_category, system::error_condition, and system::system_error. All of these classes are inside the boost namespace. It is also supported in the C++11 environment, but because many Boost libraries use Boost.System, it is necessary to keep including Boost.System.
 - Boost.Thread: This is used to apply threading programming. It provides classes to synchronize access on multiple-thread data. In C++11 environments, the Boost.Thread library offers extensions, so we can interrupt thread in Boost.Thread.
 - Boost.Timer: This is used to measure the code performance by using clocks. It measures time passed based on usual clock and CPU time, which states how much time has been spent to execute the code.
 - Boost.Wave: This provides a reusable C preprocessor that we can use in our C++ code.
There are also a few libraries that have optional, separately compiled binaries. They are as follows:

 - Boost.DateTime: It is used to process time data; for instance, calendar dates and time. It has a binary component that is only needed if we use to_string, from_string, or serialization features. It is also needed if we target our application in Visual C++ 6.x or Borland.
 - Boost.Graph: It is used to create two-dimensional graphics. It has a binary component that is only needed if we intend to parse GraphViz files.
 - Boost.Math: It is used to deal with mathematical formulas. It has binary components for cmath functions.
 - Boost.Random: It is used to generate random numbers. It has a binary component, which is only needed if we want to use random_device.
 - Boost.Test: It is used to write and organize test programs and their runtime execution. It can be used in header-only or separately compiled mode, but separate compilation is recommended for serious use.
 - Boost.Exception: It is used to add data to an exception after it has been thrown. It provides non-intrusive implementation of exception_ptr for 32-bit _MSC_VER==1310 and _MSC_VER==1400, which requires a separately compiled binary. This is enabled by #define BOOST_ENABLE_NON_INTRUSIVE_EXCEPTION_PTR.

# Setup environment
**1. Install CMake**
```
cd ~
wget https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5.tar.gz
tar xf cmake-3.14.5.tar.gz
cd cmake-3.14.5
./bootstrap --parallel=10
make -j4
sudo make -j4 install
```
**2. Install Boost**
```
cd ~
wget https://boostorg.jfrog.io/artifactory/main/release/1.69.0/source/boost_1_69_0.tar.gz
tar xf boost_1_69_0.tar.gz
cd boost_1_69_0
./bootstrap.sh
./b2 ... cxxflags="-std=c++0x -stdlib=libc++" linkflags="-stdlib=libc++" ...
sudo ./b2 toolset=gcc -j4 install
```

# Table of contents

## [Chapter 1: The Basics](chapter_01/README.md)

- [1. Creating an endpoint](chapter_01/recipe_01/README.md)
- [2. Creating an active socket](chapter_01/recipe_02/README.md)
- [3. Creating a passive socket](chapter_01/recipe_03/README.md)
- [4. Resolving a DNS name](chapter_01/recipe_04/README.md)
- [5. Binding a socket to an endpoint](chapter_01/recipe_05/README.md)
- [6. Connecting a socket](chapter_01/recipe_06/README.md)
- [7. Accepting connections](chapter_01/recipe_07/README.md)

## [Chapter 2: I/O Operations](chapter_02/README.md)

- [1. Using fixed length I/O buffers](chapter_02/recipe_01/README.md)
- [2. Using extensible stream-oriented I/O buffers](chapter_02/recipe_02/README.md)
- [3. Writing to a TCP socket synchronously](chapter_02/recipe_03/README.md)
- [4. Reading from a TCP socket synchronously](chapter_02/recipe_04/README.md)
- [5. Writing to a TCP socket asynchronously](chapter_02/recipe_05/README.md)
- [6. Reading from a TCP socket asynchronously](chapter_02/recipe_06/README.md)
- [7. Canceling asynchronous operations](chapter_02/recipe_07/README.md)
- [8. Shutting down and closing a socket](chapter_02/recipe_08/README.md)


## [Chapter 3: Implementing Client Applications](chapter_03/README.md)

- [1. Implementing a synchronous TCP client](chapter_03/recipe_01/README.md)
- [2. Implementing a synchronous UDP client](chapter_03/recipe_02/README.md)
- [2. Implementing an asynchronous TCP client](chapter_03/recipe_03/README.md)

## [Chapter 4: Implementing Server Applications](chapter_04/README.md)

- [1. Implementing a synchronous iterative TCP server](chapter_04/recipe_01/README.md)
- [2. Implementing a synchronous parallel TCP server](chapter_04/recipe_02/README.md)
- [3. Implementing an asynchronous TCP server](chapter_04/recipe_03/README.md)