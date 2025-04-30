#pragma once

#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

using namespace boost::asio;
using error_code = boost::system::error_code;

const int port = 53888;
const std::string ip_address = "127.0.0.1";

class StartSession
{
public:
	StartSession(io_context& context, ip::tcp::socket& socket);
	~StartSession();

	void _process_network();
	void _send_command();
	void _process_buffer(size_t bytes);

	//Функционал --------------------------------------------------------------------|
	void _recive_mes(std::string);
	void _set_login(std::string);
	//Функционал --------------------------------------------------------------------|

private:
	streambuf buf;
	io_context& context;
	ip::tcp::socket& socket;
	std::istream input;
};