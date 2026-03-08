/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 00:26:06 by lomont            #+#    #+#             */
/*   Updated: 2026/03/08 04:23:44 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"

Client::Client(int socket_fd) : fd(socket_fd), pos(0), _request(), _response(), headerFound(false) {
	logs("A new client has been created");
	return;
}

struct sockaddr* Client::getsockaddrClient(void) {
	return (&this->sockaddrClient);
}

socklen_t* Client::getSockLenClient(void) {
	return (&this->socklenClient);
}

void Client::SetSockaddrClient(struct sockaddr sockaddr) {
	this->sockaddrClient = sockaddr;
}

void Client::SetSockLenClient(socklen_t socklen) {
	this->socklenClient = socklen;
}

void Client::receive_header(struct kevent* server, int& fdQueue) {
	char	buffer[4096];
	ssize_t	received;
	int		err;
	(void)server;
	while (((received = recv(fd, buffer, sizeof(buffer) - 1, 0)) > 0) && !headerFound) {
		this->headerBuffer.append(buffer, received);
		if (this->headerBuffer.find("\r\n\r\n") != std::string::npos)
			headerFound = true;
		logs("a");
	}
	if (received == -1) {
			logs("here");
		err = errno;
		if (err == EAGAIN || err == EWOULDBLOCK) {
			if (!headerFound)
				return ;
		}
		else
			ft_crash("Recv function error: ", 15);
	}
	else if (received == 0) {
		close(fd);
		return ;
	}
	else
		this->headerBody.append(buffer, received);
	if (headerFound)
		this->_request = HeaderRequest(this->headerBuffer);
	if (receive_body() == 0) {
		struct kevent ptr[2];
		EV_SET(&ptr[0], fd, EVFILT_READ, EV_DISABLE, 0, 0, 0);
		EV_SET(&ptr[1], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0);
		kevent(fdQueue, ptr, 2, NULL, 0, NULL);
	}
	return ;
}

int	Client::receive_body(void) {
	char	buffer[4096];
	ssize_t	received;
	size_t	index;
	long	contentSize;
	int		err;

	std::map<std::string, std::string>& map = this->_request.getPairs();
	if (this->headerBody.empty()) {
		index = this->headerBuffer.find("\r\n\r\n");
		this->headerBody = this->headerBuffer.substr(index + 4, headerBuffer.length() - (index + 4));
	}
	logs("fatigué");
	if (map.find("Content-Length:") != map.end()) {
		contentSize = strtol(map.find("Content-Length:")->second.c_str(), NULL, 10);
		while (((received = recv(fd, buffer, sizeof(buffer) - 1, 0)) > 0) && this->pos <= contentSize) {
			pos += received;
			this->headerBody.append(buffer, received);
		}
		if (received == -1) {
		err = errno;
		if (err == EAGAIN || err == EWOULDBLOCK)
			return (-1);
		else
			ft_crash("Recv function error: ", 15); // ne pas quitter mais exception
		}
	}
	else if (this->_request.GetMethod() == POST) {
		//renvoyer un code d'erreur indiquant qu'il manque un length.
	}
	else
		return (0);
	if (!this->headerBody.empty())
		this->_request.SetBody(this->headerBody);
	return (0);
}

void	Client::ResponseToClient(std::map<int, Client*>& map, struct kevent* server, int& fdQueue) {
	std::string	str;
	std::string	content;
	ssize_t		sent;
	(void)server;
	if (_response.IsEmpty()) {
		_response = HeaderResponse(_request);
		str = this->_response.GetResponseHeader(); //fonction pour renvoyer le bon header response dans str
		logs(str); //debug
		sent = send(fd, str.c_str(), str.length(), 0);
		if (sent == -1)
			ft_crash("send failed", 8); //revoir gestion d'erreur le server doit jamais crash
	}
	content = _response.getContent();
	sent = send(fd, content.c_str(), content.length() - bytesSent, 0);
	if (sent == -1)
		ft_crash("send failed", 8); //revoir gestion d'erreur le server doit jamais crash
	bytesSent += sent;

	if (_response.GetDeleteSocket()) {
		close(fd);
		map.erase(fd); //free memory
		delete this;
	}
	else {
		struct kevent ptr[2];
		EV_SET(&ptr[0], fd, EVFILT_WRITE, EV_DISABLE, 0, 0, 0);
		EV_SET(&ptr[1], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
		kevent(fdQueue, ptr, 2, NULL, 0, NULL);
	}
	return ;
}

Client::~Client(void) {
	logs("A client has requested to close the connection after his request, deleting client socket");
	return ;
}
