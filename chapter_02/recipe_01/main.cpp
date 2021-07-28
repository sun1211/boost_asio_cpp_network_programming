#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

//we want to send a string Hello to the remote application.
// Before we send the data using Boost.Asio, we need to properly represent the buffer
void FixedLengthBufferOutput()
{
    std::string buf = "hello"; // 'buf' is the raw buffer.

    // Step 3. Creating buffer representation that satisfies
    // ConstBufferSequence concept requirements.
    asio::mutable_buffers_1 output_buf = asio::buffer(buf);

    // Step 4. 'output_buf' is the representation of the
    // buffer 'buf' that can be used in Boost.Asio output
    // operations.
}

#include <boost/asio.hpp>
#include <iostream>
#include <memory> // For std::unique_ptr<>

//receive a block of data from the server.
//To do this, we first need to prepare a buffer where the data will be stored
using namespace boost;
void FixedLengthBufferInput()
{
    // We expect to receive a block of data no more than 20 bytes
    // long.
    const size_t BUF_SIZE_BYTES = 20;

    // Step 1. Allocating the buffer.
    std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);

    // Step 2. Creating buffer representation that satisfies
    // MutableBufferSequence concept requirements.
    asio::mutable_buffers_1 input_buf =
        asio::buffer(static_cast<void *>(buf.get()),
                     BUF_SIZE_BYTES);

    // Step 3. 'input_buf' is the representation of the buffer
    // 'buf' that can be used in Boost.Asio input operations.
}

int main()
{
    FixedLengthBufferOutput();

    FixedLengthBufferInput();

    return 0;
}
