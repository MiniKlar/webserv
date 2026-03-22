/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 00:26:06 by lomont            #+#    #+#             */
/*   Updated: 2026/03/22 12:20:59 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"

//Constructor

Client::Client(int socket_fd) : fd(socket_fd), pos(0), socklenClient(0), headerBuffer(""), headerBody(""), _request(), _response(), bytesSent(0), headerFound(false) {
	memset(&this->sockaddrClient, 0, sizeof(this->sockaddrClient));
	logs("A new client has been created");
	return;
}

//Destructor

Client::~Client(void) {
	close(fd);
	logs("A client has requested to close the connection after his request, deleting client socket");
	return ;
}

void Client::ReceiveHeader(int& fdQueue, std::map<int, Client*> map) {
	char	buffer[4096];
	ssize_t	received;
	size_t	index;
	long	contentSize;
	int		err;

	received = recv(this->fd, buffer, sizeof(buffer) - 1, 0);
	if (received == -1) {
		err = errno;
		if (err == EAGAIN || err == EWOULDBLOCK) {
			if (!headerFound)
				return ;
		}
		else
			InternalError(fdQueue);
		}
	else if (received == 0) {
		CloseConnection(map);
		return ;
	}
	else if (received > 0) {
		if (!headerFound) {
			this->headerBuffer.append(buffer, received);
			if (this->headerBuffer.find("\r\n\r\n") != std::string::npos) {
				headerFound = true;
				this->_request = HeaderRequest(this->headerBuffer);
			}
			if (received == static_cast<ssize_t>(this->headerBuffer.find("\r\n\r\n")) + 4)
				ChangeKeventState(fdQueue, true);
		}
		else {
			std::map<std::string, std::string>& map = this->_request.getPairs();
			if (this->headerBody.empty()) {
				index = this->headerBuffer.find("\r\n\r\n");
				this->headerBody = this->headerBuffer.substr(index + 4, headerBuffer.length() - (index + 4));
				pos += headerBody.size();
				// std::cout << "pos = " << pos << std::endl;
				this->headerBuffer.erase(index + 4, headerBuffer.length() - (index + 4));
			}
			if (map.find("Content-Length:") != map.end()) {
				contentSize = strtol(map.find("Content-Length:")->second.c_str(), NULL, 10);
				// std::cout << "contentSize = " << contentSize << std::endl;
				// std::cout << "pos = " << pos << std::endl;
				pos += received;
				this->headerBody.append(buffer, received);
				if (this->pos == contentSize) {
					this->_request.SetBody(this->headerBody);
					ChangeKeventState(fdQueue, true);
				}
			}
			else if (this->_request.GetMethod() == POST)
				this->_request.SetError(LENGTH);	//renvoyer un code d'erreur indiquant qu'il manque un length.
		}
	}
	return ;
}

void	Client::ResponseToClient(std::map<int, Client*>& map, int& fdQueue, struct config* config) {
	std::string	str;
	int			err;

	std::cout << this->_response.IsParsed() << std::endl;
	if (this->_response.IsParsed() == false)
		this->_response = HeaderResponse(this->_request, config);
	str = _response.GetBuffer();
	bytesSent = send(fd, str.c_str(), str.length(), 0);
	std::cout << "bytesSent= [" << bytesSent << "]" << std::endl;
	std::cout << "size str = [" << str.size() << "]" << std::endl;
	if (bytesSent == -1) {
		err = errno;
		if (err == EAGAIN || err == EWOULDBLOCK)
			return ;
		else
			CloseConnection(map); //on close la connexion si erreur send
	}
	if (bytesSent != static_cast<ssize_t>(str.size())) {
		ResizeBuffer(str);
		return ;
	}
	else if (_request.GetDeleteSocket())
		CloseConnection(map);
	else {
		ChangeKeventState(fdQueue, false);
		CleanClient();
	}
}

void Client::CleanClient(void) {
	this->_request.CleanHeader();
	this->_response.CleanHeader();
	this->bytesSent = 0;
	this->headerBody.clear();
	this->headerBuffer.clear();
	this->headerFound = false;
	this->pos = 0;
}

void Client::ResizeBuffer(std::string& str) {
	str.erase(0, this->bytesSent);
	this->_response.SetBuffer(str);
	bytesSent = 0;
}

void Client::CloseConnection(std::map<int, Client*>& map) {
	map.erase(fd);
	std::cout << "deleting connection" << std::endl;
	delete this;
	return ;
}

void Client::ChangeKeventState(int& fdQueue, bool disableRead) {
	struct kevent ptr[2];
	if (disableRead) {
		EV_SET(&ptr[0], fd, EVFILT_READ, EV_DISABLE, 0, 0, 0);
		EV_SET(&ptr[1], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0);
	}
	else {
		EV_SET(&ptr[0], fd, EVFILT_WRITE, EV_DISABLE, 0, 0, 0);
		EV_SET(&ptr[1], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
	}
	kevent(fdQueue, ptr, 2, NULL, 0, NULL);
}

void Client::InternalError(int& fdQueue) {
	this->_request = HeaderRequest(INTERNAL);
	ChangeKeventState(fdQueue, true);
}

//Getters
struct sockaddr* Client::getsockaddrClient(void) {
	return (&this->sockaddrClient);
}

socklen_t* Client::getSockLenClient(void) {
	return (&this->socklenClient);
}

//Setters

void Client::SetSockaddrClient(struct sockaddr sockaddr) {
	this->sockaddrClient = sockaddr;
}

void Client::SetSockLenClient(socklen_t socklen) {
	this->socklenClient = socklen;
}
