#include <iostream>
#include "../include/Boost_Asio/Boost_Asio_client.h"

constexpr boost::asio::ip::port_type serverPortNumber = 5190; // The King is dead. Long live the King!

boost::asio::io_service service;


int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");

    if (argc != 2)
    {
        std::cout << "[FAILED] Usage: " << argv[0] << " ServerIP" << std::endl;
        return 1;
    }

    boost::system::error_code getIPError;
    boost::asio::ip::address _serverIP = boost::asio::ip::address::from_string(argv[1], getIPError);
    if (getIPError.value() != boost::system::errc::success)
    {
        std::cout << "[FAILED] GetIP - " << getIPError.message() << std::endl;
        return 2;
    }

    boost::asio::ip::tcp::endpoint _endpoint(_serverIP, serverPortNumber);
    boost::shared_ptr <Boost_Asio_client> client1(new Boost_Asio_client(&service));
    client1->doStart(&_endpoint);

    service.run();
}

