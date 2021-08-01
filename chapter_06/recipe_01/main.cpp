#include <boost/asio.hpp>

using namespace boost;

void CompositeBuffersGatherOutput(){
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
}

void CompositeBuffersScatterInput()
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
}

int main()
{
    CompositeBuffersGatherOutput();

    CompositeBuffersScatterInput();

    return 0;
}
