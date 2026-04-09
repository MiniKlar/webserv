/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 00:26:06 by lomont            #+#    #+#             */
/*   Updated: 2026/04/09 21:30:59 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"
#include "webserv.hpp"

//Constructor

Client::Client(int socket_fd, struct config* setConfig) : fd(socket_fd), timestamp(time(NULL)), headerFound(false), pos(0), bytesSent(0), socklenClient(0), headerBuffer(""), headerBody(""), _request(), _response(), config(setConfig) {
	memset(&this->sockaddrClient, 0, sizeof(this->sockaddrClient));
	ft_logs("A new client has been created");
}

//Destructor

Client::~Client(void) {
	ft_logs("A client has requested to close the connection after his request, deleting client socket");
}

void Client::ReceiveHeader(int& fdQueue, std::map<int, Client*>& map) {
	char			buffer[4096];

	ssize_t received = recv(this->fd, buffer, sizeof(buffer) - 1, 0);
	if (received <= 0) {
		if (received == 0)
			CloseConnection(map, true);
		else
			InternalError(fdQueue);
		return ;
	}
	else {
		if (!headerFound) {
			this->headerBuffer.append(buffer, received);
			size_t xpos = this->headerBuffer.find("\r\n\r\n");
			if (xpos == std::string::npos)
				return ;
			headerFound = true;
			this->headerBody = this->headerBuffer.substr(xpos + 4);
			this->headerBuffer.resize(xpos + 4);
			this->_request = HeaderRequest(this->headerBuffer);
			received -= headerBuffer.size();
			headerBuffer.clear();
			std::string& transferEncoding = this->_request.getPairs()["Transfer-Encoding:"];
			std::string& content = this->_request.getPairs()["Content-Length:"];
			if (transferEncoding == "chunked") {
				if (this->headerBody.find("0\r\n\r\n") != std::string::npos) {
					this->headerBody.clear();
					this->_request.SetBody(this->headerBody);
					ChangeEpollState(fdQueue, true);
					return ;
				}
			}
			else if (content.empty()) {
				if (this->_request.GetMethod() == POST)
					this->_request.SetError(LENGTH);
				ChangeEpollState(fdQueue, true);
				return ;
			}
			else {
				long content_length = strtol(content.c_str(), NULL, 10);
				int	max_body_size = this->config->maxBodySize;
				if (content_length > max_body_size) {
					ft_logs("Body too large!");
					this->_request.SetError(BODY_TOO_LARGE);
					ChangeEpollState(fdQueue, true);
					return ;
				}
				else {
					if (received >= content_length) {
						this->_request.SetBody(this->headerBody);
						ChangeEpollState(fdQueue, true);
						return ;
					}
				}
			}
		}
		else {
			this->headerBody.append(buffer, received);
			std::string& content = this->_request.getPairs()["Content-Length:"];
			std::string& transferEncoding = this->_request.getPairs()["Transfer-Encoding:"];
			if (transferEncoding == "chunked") {
				if (this->headerBody.find("0\r\n\r\n") != std::string::npos) {
					this->headerBody.clear();
					this->_request.SetBody(this->headerBody);
					ChangeEpollState(fdQueue, true);
				}
				return ;
			}
			long content_length = strtol(content.c_str(), NULL, 10);
			if (this->headerBody.size() >= static_cast<unsigned int>(content_length)) {
				this->_request.SetBody(this->headerBody);
				ChangeEpollState(fdQueue, true);
				return ;
			}
		}
	}
}

void	Client::ResponseToClient(std::map<int, Client*>& map, int& fdQueue, struct config* config) {
	if (this->_response.IsParsed() == false)
		this->_response = HeaderResponse(this->_request, config);
	std::string str = _response.GetBuffer();
	bytesSent = send(fd, str.c_str(), str.length(), 0);
	if (bytesSent == -1) {
		CloseConnection(map, true); //on close la connexion si erreur send
		return ;
	}
	if (bytesSent != static_cast<ssize_t>(str.size())) {
		ResizeBuffer(str);
		return ;
	}
	else if (_request.GetDeleteSocket() || this->_request.GetError() == BODY_TOO_LARGE) {
		CloseConnection(map, true);
		return ;
	}
	else if (this->_request.GetError() != BODY_TOO_LARGE)
		CleanClient();
	ChangeEpollState(fdQueue, false);
}

void Client::RefreshTimestamp(void) {
	this->timestamp = time(NULL);
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

void Client::CloseConnection(std::map<int, Client*>& map, bool deleteFromMap) {
	if (deleteFromMap)
		map.erase(fd);
	close(this->fd);
	delete this;
}

void Client::ChangeEpollState(int& fdQueue, bool disableRead) {
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
	this->_request = HeaderRequest(INTERNAL);
	ChangeEpollState(fdQueue, true);
}

//Getters

int Client::GetTime(void) {
	return this->timestamp;
}

//Setters

void Client::SetSockaddrClient(struct sockaddr sockaddr) {
	this->sockaddrClient = sockaddr;
}

void Client::SetSockLenClient(socklen_t socklen) {
	this->socklenClient = socklen;
}
