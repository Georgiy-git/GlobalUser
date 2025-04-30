#include "main.hpp"

int main() {
	setlocale(LC_ALL, "RU");
	std::cout << "Программа запущена.\n";
	io_context context;
	ip::tcp::socket socket(context);
	socket.connect(ip::tcp::endpoint(ip::make_address_v4(ip_address), port));
	StartSession session(context, socket);
	session._send_command();
	context.run();
}

StartSession::StartSession(io_context& context, ip::tcp::socket& socket)
	: context{ context }, socket{ socket }, input(&buf)
{
	std::cout << "Подключение к серверу выполнено.\n";
}

StartSession::~StartSession()
{
	if (socket.is_open()) {
		socket.close();
	}
	std::cout << "Деструктор выполнился.\n";
}

void StartSession::_process_network()
{
	async_read_until(socket, buf, '\f', [this](error_code ec, size_t bytes) {
		if (ec) {
			if (ec == error::connection_refused) {
				std::cout << "Ошибка обработки сети: соединение сброшено.\n";
			}
			else if (ec == error::eof) {
				std::cout << "Ошибка обработки сети: соединение разорвано.\n";
			}
			else {
				std::cout << "Ошибка обработки сети: " << ec.message() << std::endl;
			}
			context.stop(); //Гарант
		}
		else {
			//Польза --------------------------------------------------------------------|
			_process_buffer(bytes);
			//Польза --------------------------------------------------------------------|
		}
		});
}

void StartSession::_send_command()
{
	std::cout << "Введите команду: ";
	std::string line;
	std::getline(std::cin, line);
	if (line.starts_with("/exit")) { exit(0); }

	//Пеобр --------------------------------------------------------------------|
	if		(line.starts_with("/help")) { line = "2|" + line; }
	else if (line.starts_with("/hello")) { line = "1|" + line; }
	//Пеобр --------------------------------------------------------------------|
	
	//Предохр
	else { line = "0|" + line; }

	socket.send(buffer(line + '\f'));
	_process_network();
}

void StartSession::_process_buffer(size_t bytes)
{
	std::string line;
	std::getline(input, line, '\f');

	//Функционал --------------------------------------------------------------------|
	if (line.starts_with("mes")) { _recive_mes(line); }
	//else if (line.starts_with("")) {}
	//Функционал --------------------------------------------------------------------|

	_send_command();; //Гарант
}

void StartSession::_recive_mes(std::string line)
{
	std::cout << line.substr(line.find('|') + 1) << std::endl;
}
