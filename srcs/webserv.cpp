/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 23:17:19 by lomont            #+#    #+#             */
/*   Updated: 2026/04/06 23:13:17 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "client.hpp"

volatile sig_atomic_t g_stop = 0;
static std::string GetFileBuffer(int fd);

//Signal function handler
void handler(int signal) {
	(void)signal;
	g_stop = 1;
}

//Constructor
server::server(const std::string &configurationFile) : f(getprotobyname("TCP")) {
	ParseServerConfiguration(configurationFile);
	if (ConfigureServer() == -1)
		return ;
	signal(SIGINT, &handler);
	WaitForConnection();
}

//Destructor
server::~server(void) {
	ft_logs("Server destructor called");
	for (size_t i = 0; i < serverConfigCount; i++)
			delete[] config[i].locationConfig;
	delete[] config;
	delete[] ServerSocket;
	delete[] sa;
	return;
}

void server::ParseServerConfiguration(const std::string &configurationFile)
{
	int			fd;
	const char*	file;
	std::string fileBuffer;

	file = configurationFile.data();
	fd = open(file, O_RDONLY);
	if (fd == -1)
		ft_crash("Error when trying to open the configuration file, please check the name of the file and the permissions");
	fileBuffer = GetFileBuffer(fd);
	this->serverConfigCount = GetServerConfigCount(fileBuffer);
	ParseServerDeclaration(fileBuffer);
	return;
}

static std::string GetFileBuffer(int fd)
{
	ssize_t 	bread;
	char		buffer[4096];
	std::string stringBuffer;

	bread = read(fd, buffer, 4096);
	while (bread > 0) {
		stringBuffer.append(buffer, bread);
		bread = read(fd, buffer, 4096);
	}
	if (bread == -1) {
		close(fd);
		ft_crash("Read error");
	}
	close(fd);
	return (stringBuffer);
}

void server::CreateNewClient(int newConnexion, struct sockaddr sockaddrClient, socklen_t socklenClient, int ServerSocket)
{
	struct epoll_event e_event;
	int configIndex = FindServerConfig(ServerSocket);
	Client *new_client = new Client(newConnexion, &this->config[configIndex]);
	new_client->SetSockaddrClient(sockaddrClient);
	new_client->SetSockLenClient(socklenClient);
	map.insert(std::pair<int, Client *>(newConnexion, new_client));
	fcntl(newConnexion, F_SETFL, O_NONBLOCK);
	e_event.events = EPOLLIN;
	e_event.data.fd = newConnexion;
	if (epoll_ctl(this->EvenementQueue, EPOLL_CTL_ADD, newConnexion, &e_event) == -1) {
		close(newConnexion);
		map.erase(newConnexion);
		delete new_client;
		return ;
	}
}

Client *server::FindCurrentClient(int fd)
{
	std::map<int, Client *>::iterator it = this->map.find(fd);
	if (it == map.end())
		return (NULL); // remettre en exception ou error d'information
	else
		return (it->second);
}

void server::WaitForConnection(void)
{
	int nbOfEvents;
	int serverSocket;
	int newConnexion;
	Client *current;
	struct sockaddr sockaddrClient;
	socklen_t socklenClient;
	epoll_event* s_event = new epoll_event[SIZE_TEVENT];
	socklenClient = sizeof(sockaddrClient);
	while (!g_stop) // laisser tourner le serveur tout le temps
	{
		CheckTimestamp();
		if ((nbOfEvents = epoll_wait(this->EvenementQueue, s_event, SIZE_TEVENT, 2000)) == -1) // si on recoit -1 de kevent alors crash => on stock dans nbOfEvents
			ft_warning("Triggered event retrieval error");
		for (int i = 0; i < nbOfEvents; i++)
		{ // pour nombres d'events triggered
			ft_logs("An Event or a few Events have been triggered");
			if ((serverSocket = this->findServerSocket(s_event[i].data)) != -1) { // si event est un serveur socket
				if ((newConnexion = accept(serverSocket, &sockaddrClient, &socklenClient)) == -1) // si accept fail -> crash
					ft_warning("Couldn't accept the connection");
				else
					CreateNewClient(newConnexion, sockaddrClient, socklenClient, serverSocket); // sinon on crée un nouveau client qui aura pour fd le accept
			}
			else
			{
				current = FindCurrentClient(s_event[i].data.fd); // on cherche le client associé au fd retourné par kevent => ce qui veut dire qu'un client nous a contacté
				if (!current)													  // si on a rien on continue
					continue;
				current->RefreshTimestamp();
				if (s_event[i].events == EPOLLIN) {
					ft_logs("A client is sending us a request...");
					current->ReceiveHeader(this->EvenementQueue, map);
				}
				else if (s_event[i].events == EPOLLOUT) {
					ft_logs("A client is listening for an answer...");
					current->ResponseToClient(map, this->EvenementQueue, this->config);
				}
			}
		}
	}
	for (std::map<int, Client*>::iterator it = map.begin(); it != map.end(); it++) {
		it->second->CloseConnection(map, false);
	}
	delete[] s_event;
	ft_logs("Exiting the web server program properly...");
	return ;
}

void server::CheckTimestamp(void) {
	std::map<int, Client*>::iterator toDelete;

	time_t	_time = time(NULL);
	for (std::map<int, Client*>::iterator it = this->map.begin(); it != this->map.end();) {
		if (it->second->GetTime() + 30 < _time) {
			toDelete = it;
			it++;
			toDelete->second->CloseConnection(map, true);
			ft_logs("Client timeout");
		}
		else
			it++;
	};
	return ;
}

int server::ConfigureServer(void)
{
	const int opt = 1;
	ft_logs("Web server started");
	if (!this->f) {
		ft_error("getprotobyname failed");
		return (-1);
	}
	ServerSocket = new int[serverConfigCount];
	if (!ServerSocket) {
		ft_error("Memory allocation failed for ServerSocket");
		return (-1);
	}
	EvenementQueue = epoll_create(SIZE_TEVENT);
	if (EvenementQueue == -1) {
		ft_error("epoll fd creation failed");
		return (-1);
	}
	this->sa = new struct sockaddr_in[serverConfigCount];
	if (!this->sa) {
		ft_error("Memory allocation failed for struct sa");
		return (-1);
	}
	for (size_t i = 0; i < serverConfigCount; i++)
	{
		fillSockaddrStruct(i);
		ServerSocket[i] = socket(PF_INET, SOCK_STREAM, f->p_proto);
		if (ServerSocket[i] == -1) {
			ft_error("Server socket creation failed");
			return (-1);
		}
		if (setsockopt(ServerSocket[i], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
			ft_error("Setsockopt function failed");
			return (-1);
		}
		if (bind(ServerSocket[i], (struct sockaddr *)&this->sa[i], sizeof(this->sa[i])) == -1) {
			ft_error("Bind function failed");
			return (-1);
		}
		if (listen(ServerSocket[i], SOMAXCONN) == -1) {
			ft_error("Listen function failed");
			return (-1);
		}
		if (fcntl(ServerSocket[i], F_SETFL, O_NONBLOCK) == -1) {
			ft_error("Fcntl function failed");
			return (-1);
		}
		this->event.data.fd = ServerSocket[i];
		this->event.events = EPOLLIN;
		if (epoll_ctl(EvenementQueue, EPOLL_CTL_ADD, ServerSocket[i], &this->event) == -1) {
			ft_error("epoll event addition failed");
			return (-1);
		}
	}
	return (0);
}

int server::FindServerConfig(int ServerSocket) {
	for (size_t i = 0; i < serverConfigCount; i++) {
		if (ServerSocket == this->ServerSocket[i])
			return (i);
	}
	return (-1);
}

void server::fillSockaddrStruct(int index)
{
	memset(&sa[index], 0, sizeof(sa[index]));
	sa[index].sin_family = AF_INET;
	sa[index].sin_port = htons(config[index].interfacePort.second);
	sa[index].sin_addr.s_addr = htonl(INADDR_ANY);
}

int server::findServerSocket(epoll_data_t data)
{
	for (size_t i = 0; i < serverConfigCount; i++) {
		if (data.fd == ServerSocket[i]) {
			return (ServerSocket[i]);
		}
	}
	return (-1);
}
