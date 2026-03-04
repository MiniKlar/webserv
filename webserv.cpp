/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 23:17:19 by lomont            #+#    #+#             */
/*   Updated: 2026/03/04 02:12:03 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

static std::string GetFileBuffer(int fd);

//Constructor server

server::server(const std::string& configurationFile): f(getprotobyname("TCP")) {
	if (configurationFile.empty())
		ParseServerConfiguration(DEFAULT_CONFIGURATION_FILE);
	else
		ParseServerConfiguration(configurationFile);
	ConfigureServer();
}

//Destruct server
server::~server(void) {
	return ;
}

void server::ParseServerConfiguration(const std::string& configurationFile) {
	const char* file;
	int			fd;
	std::string	fileBuffer;

	// if (configurationFile.empty())
	// 	file = DEFAULT_CONFIGURATION_FILE;
	// else
	file = configurationFile.data();
	fd = open(file, O_RDONLY);
	if (fd == -1)
		ft_error("Error when trying to open the configuration file", 1);
	fileBuffer = GetFileBuffer(fd);
	serverConfigCount = GetServerConfigCount(fileBuffer);
	//parser chaque server config avec la même logique
		//trouver l'acollade fermante
		//tant qu'on est pas arrivé à la fin
			//stocker ce qu'on trouve dans des variables (de quel type?)
	ParseServerDeclaration(fileBuffer);
	close(fd);
	exit(1);
	return ;
}

void server::ConfigureServer(void) {
	const int opt = 1;
	logs("Web server started");
	if (!this->f)
		ft_error("getprotobyname failed", 1);
	fillSockaddrStruct();
	ServerSocket = socket(PF_INET, SOCK_STREAM, f->p_proto);
	if (ServerSocket == -1)
		ft_error("socket failed", 2);
	if (setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		ft_error("setsockopt failed", 3);
	BindServerSocket = bind(ServerSocket, (struct sockaddr*)&sa, sizeof(sa));
	if (BindServerSocket == -1)
		ft_error("bind failed", 4);
	ListenServerSocket = listen(ServerSocket, 1);
	if (ListenServerSocket == -1)
		ft_error("listen creation failed", 5);
	fcntl(ServerSocket, F_SETFL, O_NONBLOCK);
	EvenementQueue = kqueue();
	if (EvenementQueue == -1)
		ft_error("kqueue fd creation failed", 6);
	EV_SET(&event, ServerSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
	if (kevent(EvenementQueue, &event, 1, NULL, 0, NULL) == -1)
		ft_error("kevent event addition failed", 7);
}

struct kevent* server::getTevent(void) {
	return (&this->tevent);
}

struct kevent* server::getevent(void) {
	return (&this->event);
}

int& server::getServerSocket(void) {
	return (this->ServerSocket);
}

int& server::getEvenementQueue(void) {
	return (this->EvenementQueue);
}

void server::fillSockaddrStruct(void) {
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
}

static std::string GetFileBuffer(int fd) {
	std::string	stringBuffer;
	char		buffer[4096];
	ssize_t		bread;

	bread = read(fd, buffer, 4096);
	while(bread > 0) {
		stringBuffer.append(buffer);
		bread = read(fd, buffer, 4096);
	}
	if (bread == -1)
		ft_error("Read error", 2);
	return (stringBuffer);
}

size_t server::GetServerConfigCount(const std::string& buffer) {
	size_t configServerCount;
	size_t pos;

	configServerCount = 0;
	pos = 0;
	while ((pos = buffer.find("server {", pos)) != std::string::npos) {
			pos += 1;
			configServerCount++;
	}
	std::cout << configServerCount << std::endl;
	return (configServerCount);
}

void server::ParseServerDeclaration(const std::string& buffer) {
	// for (int i = 0; i < serverConfigCount; i++) {
		// } boucle pour toutes les configurations serveurs.
	size_t poslastaccolade = 120; //fake value, need to calculate to avoid parsing other server conf
	size_t pos = 0;
	size_t xpos = 0;

	//find listen
	pos = buffer.find("listen", pos) + 7;
	xpos = buffer.find(":", pos);
	interfacePort.first = buffer.substr(pos, xpos - pos);
	pos = buffer.find(";", xpos);
	interfacePort.second = atoi(buffer.substr(xpos + 1, pos - xpos - 1).c_str());
	std::cout << interfacePort.first << " " << interfacePort.second << std::endl;
	//end find listen

	//find errorPage
	std::pair<std::vector<int>, std::string> p;
	std::vector<int> v;
	std::string chemin;
	bool brake = false;
	while (pos < poslastaccolade || !buffer[pos]) {
		pos = buffer.find("error_page", pos) + 11; //on arrive après l'espace
		while (!brake) {
			xpos = pos;
			while (!isspace(buffer[pos])) {
				if (!isnumber(buffer[pos])) {
					brake = true;
					break;
				}
				pos++;
			}
			if (!brake) {
				std::cout << atoi(buffer.substr(xpos, pos - xpos).c_str()) << std::endl;
				v.push_back(atoi(buffer.substr(xpos, pos - xpos).c_str())); // vérifier atoll?
				pos++;
			}
			else {
				xpos = buffer.find(";", pos);
				if (buffer[pos] == ' ')
					pos++;
				chemin = buffer.substr(pos, xpos - pos);
				pos = xpos;
			}
		}
	}
	for (size_t it = 0; it < v.size(); it++) {
		std::cout << v[it] << std::endl;
	}
	std::cout << chemin << std::endl;
	exit(1);
}
