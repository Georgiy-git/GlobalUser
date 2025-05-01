#include "main.hpp"
//#define DEBUG

int main() {
	setlocale(LC_ALL, "RU");
	std::cout << "Программа запущена.\n";
	io_context context;
	ip::tcp::socket socket(context);
	socket.connect(ip::tcp::endpoint(ip::make_address_v4(ip_address), port));
	StartSession session(context, socket);
	context.run();
}

StartSession::StartSession(io_context& context, ip::tcp::socket& socket)
	: context{ context }, socket{ socket }, input(&buf)
{
	std::cout << "Подключение к серверу выполнено.\n";
	_process_network();
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
	if		(line.starts_with("/help")) { line = "{2}"; }
	else if (line.starts_with("/hello")) { line = "{1}"; }
	else if (line.starts_with("/server stop")) { line = "{127001}"; }
	else if (line.starts_with("/files")) { line = "{3}"; }
	else if (line.starts_with("/loadfile")) { line = "{4}" + line.substr(line.find(' ') + 1); }
	//Пеобр --------------------------------------------------------------------|
	
	//Предохр
	else { line = "{0}" + line; }

	line += '\f';
	socket.send(buffer(line.data(), line.size()));
	_process_network();
}

void StartSession::_process_buffer(size_t bytes)
{
	std::string line;
	std::getline(input, line, '\f');
	line = line.substr(line.find('{'));

#ifdef DEBUG
	std::cout << "Команда с сервера: " << line << std::endl;
#endif 

	//Функционал --------------------------------------------------------------------|
	if (line.starts_with("{mes}") ) { _recive_mes(line); }
	else if (line.starts_with("{login}")) { _set_login(line); }
	else if (line.starts_with("{loadfile}")) { _load_file(line); }
	else {
		std::cout << "Команда с сервера не распознана.\n";
		_send_command();
	}
	//Функционал --------------------------------------------------------------------|
}

void StartSession::_recive_mes(std::string line)
{
	std::cout << line.substr(line.find('}') + 1) << std::endl;
	_send_command();
}

void StartSession::_set_login(std::string line)
{
	std::cout << line.substr(line.find('}') + 1);
	std::string _line;
	std::getline(std::cin, _line);
	socket.send(buffer(_line + '\f'));
	_process_network();
}

void StartSession::_load_file(std::string line)
{
	std::string file_name = line.substr(line.find('}') + 1);
	file_name = file_name.substr(0, file_name.find('|'));
	std::size_t file_size = std::stoi(line.substr(line.find('|') + 1));
	std::size_t quantity = file_size;

	std::ofstream file(file_name.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Ошибка создания файла.\n";
		return;
	}
	
	mutable_buffer _buf;
	error_code ec;
	std::cout << "Загрузка:   0%";
	while (quantity > 0) {
		quantity -= socket.read_some(_buf, ec);
		if (ec) {
			std::cerr << "Ошибка при загрузке: " << ec.message() << std::endl;
			return;
		}
		std::cout << "\b\b\b\b" << std::setw(3) << std::right <<
			(file_size - quantity) / (file_size / 100) << '%';
		file << &_buf;
	}
	std::cout << std::endl;
	_process_network();
}
