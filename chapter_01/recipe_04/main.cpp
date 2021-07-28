#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

int main()
{
    // Step 1. Assume that the client application has already
    // obtained the DNS name and protocol port number and
    // represented them as strings.
    std::string host = "google.com";
    std::string port_num = "8080";

    // Step 2.
    asio::io_service ios;

    // Step 3. Creating a query.
    asio::ip::tcp::resolver::query resolver_query(host,
                                                  port_num, asio::ip::tcp::resolver::query::numeric_service);

    // Step 4. Creating a resolver.
    asio::ip::tcp::resolver resolver(ios);

    // Used to store information about error that happens
    // during the resolution process.
    boost::system::error_code ec;

    // Step 5.
    asio::ip::tcp::resolver::iterator it =
        resolver.resolve(resolver_query, ec);

    // Handling errors if any.
    if (ec != 0)
    {
        // Failed to resolve the DNS name. Breaking execution.
        std::cout << "Failed to resolve a DNS name."
                  << "Error code = " << ec.value()
                  << ". Message = " << ec.message();

        return ec.value();
    }

    return 0;
}