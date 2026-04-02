/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 00:26:06 by lomont            #+#    #+#             */
/*   Updated: 2026/04/02 17:30:09 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"
#include "webserv.hpp"

//Constructor

Client::Client(int socket_fd, struct config* setConfig) : fd(socket_fd), config(setConfig), pos(0), socklenClient(0), headerBuffer(""), headerBody(""), _request(), _response(), bytesSent(0), headerFound(false), timestamp(time(NULL)) {
	memset(&this->sockaddrClient, 0, sizeof(this->sockaddrClient));
	logs("A new client has been created");
	return;
}

//Destructor

Client::~Client(void) {
	logs("A client has requested to close the connection after his request, deleting client socket");
	return ;
}

void Client::ReceiveHeader(int& fdQueue, std::map<int, Client*>& map) {
	char			buffer[4096];
	ssize_t			received;
	size_t			index;
	unsigned long	contentSize;
	int				err;

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
		CloseConnection(map, true);
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
			std::map<std::string, std::string>& pairs = this->_request.getPairs();
			if (this->_request.GetError() == BODY_TOO_LARGE)
				return ;
			if (this->headerBody.empty()) {
				index = this->headerBuffer.find("\r\n\r\n");
				this->headerBody = this->headerBuffer.substr(index + 4, headerBuffer.length() - (index + 4));
				pos += headerBody.size();
				this->headerBuffer.erase(index + 4, headerBuffer.length() - (index + 4));
			}
			if (pairs.find("Content-Length:") != pairs.end()) {
				contentSize = strtol(pairs.find("Content-Length:")->second.c_str(), NULL, 10);
				if (contentSize > this->config->maxBodySize) {
					this->headerBody.clear();
					this->_request.SetError(BODY_TOO_LARGE);
					ChangeKeventState(fdQueue, true);
					return ;
				}
				pos += received;
				this->headerBody.append(buffer, received);
				if (static_cast<unsigned long>(this->pos) == contentSize) {
					this->_request.SetBody(this->headerBody);
					ChangeKeventState(fdQueue, true);
				}
			}
			else if (this->_request.GetMethod() == POST)
				this->_request.SetError(LENGTH);
		}
	}
	return ;
}

void	Client::ResponseToClient(std::map<int, Client*>& map, int& fdQueue, struct config* config) {
	std::string	str;
	int			err;

	if (this->_response.IsParsed() == false)
		this->_response = HeaderResponse(this->_request, config);
	str = _response.GetBuffer();
	bytesSent = send(fd, str.c_str(), str.length(), 0);
	if (bytesSent == -1) {
		err = errno;
		if (err == EAGAIN || err == EWOULDBLOCK)
			return ;
		else
			CloseConnection(map, true); //on close la connexion si erreur send
	}
	if (bytesSent != static_cast<ssize_t>(str.size())) {
		ResizeBuffer(str);
		return ;
	}
	else if (_request.GetDeleteSocket()) {
		CloseConnection(map, true);
		return ;
	}
	else if (this->_request.GetError() != BODY_TOO_LARGE)
		CleanClient();
	ChangeKeventState(fdQueue, false);
}

void Client::RefreshTimestamp(void) {
	this->timestamp = time(NULL);
	return ;
}

int Client::GetTime(void) {
	return (this->timestamp);
}

void Client::CleanClient(void) {
	this->_request.CleanHeader();
	this->_response.CleanHeader();
	this->bytesSent = 0;
	this->headerBody.clear();
	this->headerBuffer.clear();
	this->headerFound = false;
	this->pos = 0;
	return ;
}

void Client::ResizeBuffer(std::string& str) {
	str.erase(0, this->bytesSent);
	this->_response.SetBuffer(str);
	bytesSent = 0;
	return ;
}

void Client::CloseConnection(std::map<int, Client*>& map, bool deleteFromMap) {
	if (deleteFromMap)
		map.erase(fd);
	close(this->fd);
	delete this;
	return ;
}

void Client::ChangeKeventState(int& fdQueue, bool disableRead) {
	struct epoll_event ptr;
	ptr.data.fd = this->fd;
	if (disableRead) {
		ptr.events = EPOLLOUT;
		epoll_ctl(fdQueue, EPOLL_CTL_MOD, this->fd, &ptr);
	}
	else {
		ptr.events = EPOLLIN;
		epoll_ctl(fdQueue, EPOLL_CTL_MOD, this->fd, &ptr);
	}
	
}

void Client::InternalError(int& fdQueue) {
	logs("here");
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
