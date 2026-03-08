/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 23:17:19 by lomont            #+#    #+#             */
/*   Updated: 2026/03/08 04:07:23 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

static std::string GetFileBuffer(int fd);

//Constructor server

server::server(const std::string& configurationFile): f(getprotobyname("TCP")) {
	ParseServerConfiguration(configurationFile);
	ConfigureServer();
	WaitForConnection();
}

//Destruct server
server::~server(void) {
	return ;
}

void server::CreateNewClient(int& newConnexion, struct sockaddr sockaddrClient, socklen_t socklenClient) {
	Client* new_client = new Client(newConnexion); //free client
	new_client->SetSockaddrClient(sockaddrClient);
	new_client->SetSockLenClient(socklenClient);
	this->map.insert(std::pair<int, Client*>(newConnexion, new_client));
	fcntl(newConnexion, F_SETFL, O_NONBLOCK);
	EV_SET(this->getevent(), newConnexion, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
	if (kevent(this->getEvenementQueue(), this->getevent(), 1, NULL, 0, NULL) == -1)
		ft_crash("Event addition failed", 10);
}

Client* server::FindCurrentClient(int fd) {
	std::map<int, Client*>::iterator it = this->map.find(fd);
	if (it == map.end())
		return (NULL); //remettre en exception ou error d'information
	else
		return (it->second);
}

void server::WaitForConnection(void) {
	int					nbOfEvents;
	int					serverSocket;
	int					newConnexion;
	Client*				current;
	struct sockaddr 	sockaddrClient;
	socklen_t 			socklenClient;

	while (true) {
		if ((nbOfEvents = kevent(this->getEvenementQueue(), NULL, 0, this->getTevent(0), SIZE_TEVENT, NULL)) == -1)
			ft_crash("Triggered event retrieval error", 8);
		for (int i = 0; i < nbOfEvents; i++) {
			if ((serverSocket = this->findServerSocket(this->getTevent(i)->ident)) != -1) {
					if ((newConnexion = accept(serverSocket, &sockaddrClient, &socklenClient)) == -1)
						ft_crash("Couldn't accept the connection", 5);
					else
						CreateNewClient(newConnexion, sockaddrClient, socklenClient);
					continue;
			}
			logs("ici");
			current = FindCurrentClient(static_cast<int>(this->getTevent(i)->ident));
			if (!current)
				continue;
			if (this->getTevent(i)->filter == EVFILT_READ)
				current->receive_header(this->getTevent(i), this->getEvenementQueue());
			else
				current->ResponseToClient(map, this->getTevent(i), this->getEvenementQueue());
		}
	}
}

void server::ParseServerConfiguration(const std::string& configurationFile) {
	const char* file;
	int			fd;
	std::string	fileBuffer;

	file = configurationFile.data();
	fd = open(file, O_RDONLY);
	if (fd == -1)
		ft_crash("Error when trying to open the configuration file, please check the name of the file and the permissions", 2);
	fileBuffer = GetFileBuffer(fd);
	this->serverConfigCount = GetServerConfigCount(fileBuffer);
	ParseServerDeclaration(fileBuffer);
	close(fd);
	return ;
}

void server::ConfigureServer(void) {
	const int opt = 1;
	logs("Web server started");
	if (!this->f)
		ft_crash("getprotobyname failed", 5);
	ServerSocket = new int[serverConfigCount];
	if (!ServerSocket)
		ft_crash("Memory allocation issue for ServerSocket", 6);
	EvenementQueue = kqueue();
	if (EvenementQueue == -1)
		ft_crash("kqueue fd creation failed", 7);
	this->sa = new struct sockaddr_in[serverConfigCount];
	if (!this->sa)
		ft_crash("Memory allocation issue for struct sa", 8);
	for (size_t i = 0; i < serverConfigCount; i++) {
		fillSockaddrStruct(i);
		ServerSocket[i] = socket(PF_INET, SOCK_STREAM, f->p_proto);
		if (ServerSocket[i] == -1)
			ft_crash("Server socket creation failed", 8);
		if (setsockopt(ServerSocket[i], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
			ft_crash("Setsockopt function failed", 9);
		if (bind(ServerSocket[i], (struct sockaddr*)&this->sa[i], sizeof(this->sa[i])) == -1)
			ft_crash("Bind function failed", 10);
		if (listen(ServerSocket[i], SOMAXCONN) == -1)
			ft_crash("Listen function failed", 11);
		if (fcntl(ServerSocket[i], F_SETFL, O_NONBLOCK) == -1)
			ft_crash("Fcntl function failed", 12);
		EV_SET(&this->event, ServerSocket[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		if (kevent(EvenementQueue, &this->event, 1, NULL, 0, NULL) == -1)
			ft_crash("kevent event addition failed", 12);
	}
}

struct kevent* server::getTevent(int i) {
	return (&this->tevent[i]);
}

struct kevent* server::getevent(void) {
	return (&this->event);
}

int	server::findServerSocket(uintptr_t &ident) {
	for (size_t i = 0; i < serverConfigCount; i++) {
		if (static_cast<int>(ident) == ServerSocket[i]) {
			std::cout << ServerSocket[i];
			return (ServerSocket[i]);
		}
	}
	return (-1);
}

int& server::getEvenementQueue(void) {
	return (this->EvenementQueue);
}

void server::fillSockaddrStruct(int index) {
	memset(&sa[index], 0, sizeof(sa[index]));
	sa[index].sin_family = AF_INET;
	sa[index].sin_port = htons(config[index].interfacePort.second);
	sa[index].sin_addr.s_addr = htonl(INADDR_ANY);
}

static std::string GetFileBuffer(int fd) {
	std::string	stringBuffer;
	char		buffer[4096];
	ssize_t		bread;

	bread = read(fd, buffer, 4096);
	while(bread > 0) {
		stringBuffer.append(buffer, bread);
		bread = read(fd, buffer, 4096);
	}
	if (bread == -1) {
		close(fd);
		ft_crash("Read error", 3);
	}
	return (stringBuffer);
}

size_t server::GetServerConfigCount(const std::string& buffer) {
	size_t pos;
	size_t configServerCount;

	pos = 0;
	configServerCount = 0;
	while ((pos = buffer.find("server {", pos)) != std::string::npos) {
			pos += 9;
			configServerCount++;
	}
	return (configServerCount);
}

void server::FindOneConfiguration(const std::string& buffer, size_t pos, struct config *conf) {
	size_t xpos = 0;

	pos = buffer.find("listen", pos) + 7;
	xpos = buffer.find(":", pos);
	conf->interfacePort.first = buffer.substr(pos, xpos - pos);
	pos = buffer.find(";", xpos);
	conf->interfacePort.second = atoi(buffer.substr(xpos + 1, pos - xpos - 1).c_str());
	std::cout << "IP: " << conf->interfacePort.first << " Port: " << conf->interfacePort.second << std::endl;
}

size_t server::SearchLastAccolade(const std::string& buffer, size_t i) {
	size_t accoladeOpened;

	accoladeOpened = 0;
	while (buffer[i]) {
		if (buffer[i] == '{')
			accoladeOpened++;
		else if (buffer[i] == '}')
			accoladeOpened--;
		if (accoladeOpened == 0 && buffer[i] == '}')
			break;
		i++;
	}
	return (i);
}

void server::FindErrorPages(const std::string& buffer, size_t& positionLastAccolade, size_t pos, struct config* conf) {
	std::pair<std::vector<int>, std::string>	p;
	std::vector<int> 							v;
	std::string 								chemin;
	size_t										xpos;
	bool										brake;

	brake = false;
	while (pos < positionLastAccolade || !buffer[pos]) {
		pos = buffer.find("error_page", pos); //on arrive après l'espace
		if (pos == std::string::npos)
			break;
		pos += 11;
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
				v.push_back(atoi(buffer.substr(xpos, pos - xpos).c_str())); // vérifier atoll?
				pos++;
			}
			else {
				xpos = buffer.find(";", pos);
				if (buffer[pos] == ' ')
					pos++;
				chemin = buffer.substr(pos, xpos - pos);
				pos = xpos + 1;
			}
		}
		brake = false;
		p.first = v;
		p.second = chemin;
		conf->errorPage.insert(p);
		for (size_t it = 0; it < v.size(); it++)
			std::cout << v[it] << std::endl;
		std::cout << chemin << std::endl;
		v.clear();
		chemin.clear();
	}
}

void server::FindMaxBody(const std::string& buffer, struct config* conf) {
		size_t	pos;
		size_t	xpos;

		pos = buffer.find("client", 0);
		xpos = buffer.find(";", pos);
		pos += 21;
		std::string sizeMax = buffer.substr(pos, xpos - pos);
		std::cout << sizeMax << std::endl;
		char c = sizeMax[sizeMax.length() - 1];
		conf->maxBodySize = strtol(sizeMax.c_str(), NULL, 10);
		int sizeMemory;
		switch (c)
		{
		case 'K':
			sizeMemory = 1024;
			break;
		case 'M':
			sizeMemory = 1024 * 1024;
			break;
		case 'G' :
			sizeMemory = 1024 * 1024 * 1024;
			break;
		default:
			ft_crash("Size memory error, please choose a memory size between 'K', 'M, 'G''", 4);
			break;
		}
		conf->maxBodySize *= sizeMemory;
		std::cout << conf->maxBodySize << std::endl;
}

size_t FindNumbersOfLocation(const std::string& buffer, size_t positionLastAccolade, size_t pos) {
	size_t i;

	i = 0;
	while ((pos = buffer.find("location", pos)) <= positionLastAccolade) {
		pos += 9;
		i++;
	};
	return (i);
}

size_t server::FindMethods(const std::string& buffer, size_t pos, struct LocationConfig* conf, size_t& border) {
	size_t tmp = pos;
	size_t	xpos;
	size_t	ypos;

	if ((pos = buffer.find("methods", pos)) > border)
		return tmp;
	pos += 8;
	xpos = buffer.find(";", pos);
	while (pos <= xpos) {
		ypos = buffer.find(" ", pos);
		if (ypos > border)
			ypos = xpos;
		if (pos < border|| xpos < border)
			conf->methods.push_back(buffer.substr(pos, ypos - pos));
		pos = ypos + 1;
	}
	return pos;
}

size_t server::FindRoot(const std::string& buffer, size_t pos, struct LocationConfig* conf, size_t& border) {
	size_t tmp = pos;
	size_t	xpos;

	if ((pos = buffer.find("root", pos)) > border)
		return tmp;
	pos += 5;
	xpos = buffer.find(";", pos);
	conf->root = buffer.substr(pos, xpos - pos);
	return pos;
}

size_t server::FindIndex(const std::string& buffer, size_t pos, struct LocationConfig* conf, size_t& border) {
	size_t tmp = pos;
	size_t	xpos;
	size_t	ypos;

	if ((pos = buffer.find(" index", pos)) > border)
		return tmp;
	pos += 6;
	xpos = buffer.find(";", pos);
	while (pos <= xpos) {
		ypos = buffer.find(" ", pos);
		if (ypos > border)
			ypos = xpos;
		if (pos < border|| xpos < border)
			conf->index.push_back(buffer.substr(pos, ypos - pos));
		pos = ypos + 1;
	}
	return pos;
}

size_t server::FindAutoIndex(const std::string& buffer, size_t pos, struct LocationConfig* conf, size_t& border) {
	size_t tmp = pos;
	size_t	xpos;

	if ((pos = buffer.find("autoindex", pos)) > border)
		return tmp;
	pos = pos + 9;
	xpos = buffer.find(";", pos);
	if (buffer.find("on", xpos - pos) < border)
		conf->autoindex = true;
	else
		conf->autoindex = false;
	return pos;
}

size_t server::FindReturn(const std::string& buffer, size_t pos, struct LocationConfig* conf, size_t& border) {
	size_t tmp = pos;
	size_t	xpos;

	if ((pos = buffer.find("return", pos)) > border)
		return tmp;
	pos += 7;
	xpos = buffer.find(" ", pos);
	conf->_return.first = buffer.substr(pos, xpos - pos);
	pos = xpos + 1;
	xpos = buffer.find(";", pos);
	conf->_return.second = buffer.substr(pos, xpos - pos);
	return pos;
}

size_t server::FindUpload(const std::string& buffer, size_t pos, struct LocationConfig* conf, size_t& border) {
	size_t tmp = pos;
	size_t	xpos;

	if ((pos = buffer.find("upload_store", pos)) > border)
		return tmp;
	pos += 13;
	xpos = buffer.find(";", pos);
	conf->upload_store = buffer.substr(pos, xpos - pos);
	return pos;
}

size_t server::FindCGIPass(const std::string& buffer, size_t pos, struct LocationConfig* conf, size_t& border) {
	size_t tmp = pos;
	size_t	xpos;

	if ((pos = buffer.find("cgi_pass", pos)) > border)
		return tmp;
	if ((pos = buffer.find(".php", pos)) > border)
		return tmp;
	pos += 5;
	xpos = buffer.find(";", pos);
	conf->pathPHPexecutable = buffer.substr(pos, xpos - pos);
	return pos;
}

void server::printConfig(struct LocationConfig* conf) {
	std::cout << "location: \"" << conf->location << "\"" << std::endl;
	std::cout << "methods: " << std::endl;
	for (std::vector<std::string>::iterator it = conf->methods.begin(); it != conf->methods.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;
	std::cout << "root: " << conf->root << std::endl;
	std::cout << "upload_store: " << conf->upload_store << std::endl;
	std::cout << "index: ";
	for (std::vector<std::string>::iterator it = conf->index.begin(); it != conf->index.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;
	std::cout << "autoindex: " << conf->autoindex;
	std::cout << std::endl;
	std::cout << "return: " << conf->_return.first << " " << conf->_return.second << std::endl;
	std::cout << "path PHP executable: " << conf->pathPHPexecutable << std::endl;
	std::cout << "------------------------" << std::endl;
}

void server::FindLocation(const std::string& buffer, size_t& positionLastAccolade, size_t& index, size_t pos) {
	size_t	positionFirstAccolade;
	size_t	positionLastAccoladeLocation;
	size_t i = 0;
	struct LocationConfig* ptr;

	std::cout << pos << std::endl;
	if ((pos = buffer.find("location", pos)) == std::string::npos)
		ft_crash("parsing config file [no location found]", 5);
	config[index].numbersOfLocation = FindNumbersOfLocation(buffer, positionLastAccolade, pos);
	config[index].locationConfig = new LocationConfig[config[index].numbersOfLocation];
	while (pos <= positionLastAccolade) {
		ptr = &config[index].locationConfig[i];
		//parse la location
		positionFirstAccolade = buffer.find("{", pos);
		positionLastAccoladeLocation = buffer.find("}", positionFirstAccolade);
		pos += 9;
		ptr->location = buffer.substr(pos, positionFirstAccolade - pos - 1);
		//parse les méthods
		FindMethods(buffer, pos, ptr, positionLastAccoladeLocation);
		//parse le root
		FindRoot(buffer, pos, ptr, positionLastAccoladeLocation);
		//parse l'index
		FindIndex(buffer, pos, ptr, positionLastAccoladeLocation);
		//parse autoindex
		FindAutoIndex(buffer, pos, ptr, positionLastAccoladeLocation);
		//parse upload store
		FindUpload(buffer, pos, ptr, positionLastAccoladeLocation);
		//parse return
		FindReturn(buffer, pos, ptr, positionLastAccoladeLocation);
		//parse cgi_pass
		FindCGIPass(buffer, pos, ptr, positionLastAccoladeLocation);
		//on cherche le prochain "location"
		printConfig(ptr);
		if ((pos = buffer.find("location", pos)) == std::string::npos)
			break;
		i++;
	}
}

void server::ParseServerDeclaration(const std::string& buffer) {
	size_t	positionLastAccolade;
	size_t	pos;

	positionLastAccolade = 0;
	config = new struct config[serverConfigCount];
	for (size_t i = 0; i < serverConfigCount; i++) {
		pos = positionLastAccolade;
		positionLastAccolade = SearchLastAccolade(buffer, positionLastAccolade);
		FindOneConfiguration(buffer, pos, &config[i]);
		FindErrorPages(buffer, positionLastAccolade, pos, &config[i]);
		FindMaxBody(buffer, &config[i]);
		FindLocation(buffer, positionLastAccolade, i, pos);
		positionLastAccolade += 1;
		//std::cout << "End of one server configuration" << std::endl;
	}
}
