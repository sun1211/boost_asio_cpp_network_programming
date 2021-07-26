#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int creatingEndpointClient()
{
    // Step1. Assume that the client application has already
    // obtained the IP-address and the protocol port number.
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    // Used to store information about error that happens
    // while parsing the raw IP-address.
    boost::system::error_code ec;

    // Step2. Using IP protocol version independent address
    // representation.
    asio::ip::address ip_address =
        asio::ip::address::from_string(raw_ip_address, ec);

    if (ec.value() != 0)
    {
        // Provided IP address is invalid. Breaking execution.
        std::cout
            << "Failed to parse the IP address. Error code = "
            << ec.value() << ". Message: " << ec.message();
        return ec.value();
    }

    // Step 3.
    asio::ip::tcp::endpoint ep(ip_address, port_num);

    // Step 4. The endpoint is ready and can be used to specify a
    // particular server in the network the client wants to
    // communicate with.
}

void createEndpointServer()
{
    // Step 1. Here we assume that the server application has
    //already obtained the protocol port number.
    unsigned short port_num = 3333;

    // Step 2. Create special object of asio::ip::address class
    // that specifies all IP-addresses available on the host. Note
    // that here we assume that server works over IPv6 protocol.
    asio::ip::address ip_address = asio::ip::address_v6::any();

    // Step 3.
    asio::ip::tcp::endpoint ep(ip_address, port_num);

    // Step 4. The endpoint is created and can be used to
    // specify the ip-addresses and a port number on which
    // the server application wants to listen to incoming
    // connections.
}

int main()
{
    creatingEndpointClient();

    createEndpointServer();

    return 0;
}
