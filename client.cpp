/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 00:26:06 by lomont            #+#    #+#             */
/*   Updated: 2026/03/03 02:18:39 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"

Client::Client(int socket_fd) : fd(socket_fd), _request(), _response() {
	logs("A new client has been created");
	return;
}

struct sockaddr* Client::getsockaddrClient(void) {
	return (&this->sockaddrClient);
}

socklen_t* Client::getSockLenClient(void) {
	return (&this->socklenClient);
}

void	Client::receive_header(void) {
	char 		buffer[4096];
	std::string bufferHeader;
	std::string bufferBody;
	ssize_t 	received;
	size_t		pos;
	bool 		HeaderFound;

	HeaderFound = false;
	received = recv(fd, buffer, sizeof(buffer) - 1, 0);
	while(received > 0) {
		buffer[received] = '\0';
		if (!HeaderFound) {
			bufferHeader.append(buffer, received);
			pos = bufferHeader.find("\r\n\r\n");
			bufferBody = bufferHeader.substr(pos, bufferHeader.length());
			bufferHeader.erase(pos, bufferHeader.length());
			HeaderFound = true;
		}
		else
			bufferBody.append(buffer, received);
		received = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (received < 0) {
			int err = errno;
			if (err == EAGAIN || err == EWOULDBLOCK)
				warning();
			else
				ft_error("received error: ", 30);
		}
		else if (received == 0)
			logs("Peer has closed its half side of the connection");
	}
	_request = HeaderRequest(bufferHeader, bufferBody);
	return ;
}

void	Client::ResponseToClient(void) {
	ssize_t		sent;
	std::string str;

	_response = HeaderResponse(_request);
	std::cout << "voici le getcode " << _response.GetCode() << std::endl;
	if (_response.GetCode() == OK)
		str = _response.code_200();
	else if (_response.GetCode() == CREATED)
		str = _response.code_201();
	else {
		str = _response.code_404();
	}
	sent = send(fd, str.c_str(), str.length(), 0);
	if (sent == -1)
		ft_error("send failed", 8);
	if (_response.GetCode() != CREATED) {
		std::string content = _response.getContent();
		sent = send(fd, content.c_str(), content.length(), 0);
		if (sent == -1)
			ft_error("send failed", 9);
	}
	return ;
}

Client::~Client(void) {
	logs("A client has finished his request, deleting client socket");
	return ;
}
