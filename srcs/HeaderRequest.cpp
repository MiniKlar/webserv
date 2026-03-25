/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:01:30 by lomont            #+#    #+#             */
/*   Updated: 2026/03/25 21:45:49 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderRequest.hpp"
#include "webserv.hpp"

//Constructor

HeaderRequest::HeaderRequest(void) : method(OTHER), body(""), error(OK) {
	return ;
}

HeaderRequest::HeaderRequest(std::string bufferHeader) : method(OTHER), body(""), error(OK), isCGI(false) {
	ParseHeaderRequest(bufferHeader);
	return ;
}

HeaderRequest::HeaderRequest(Error err) : method(OTHER), body(""), error(err) {
	return ;
}

//Copy constructor & assignement operator

HeaderRequest::HeaderRequest(const HeaderRequest& other) {
	*this = other;
	return ;
}

HeaderRequest& HeaderRequest::operator=(const HeaderRequest& other) {
	if (&other != this) {
		this->headerPair = other.headerPair;
		this->method = other.method;
		this->body = other.body;
		this->_delete = other._delete;
		this->error = other.error;
		this->isCGI = other.isCGI;
	}
	return (*this);
}

//Destructor

HeaderRequest::~HeaderRequest(void) {
	//ne pas oublier de détruire les objets pour libérer la mémoire
	return ;
}


void HeaderRequest::ParseHeaderRequest(std::string& bufferHeader) {
	std::pair<std::string, std::string> pair;
	size_t				pos;
	size_t				mid;
	size_t				npos;
	size_t 				lastPositionToCheck;

	ParseFirstLine(bufferHeader);		//parse la première ligne
	if (error != OK)
		return ;
	lastPositionToCheck = bufferHeader.find("\r\n\r\n");
	if (lastPositionToCheck == std::string::npos) {
		error = BAD_REQUEST;
		return ;
	}
	pos = bufferHeader.find("\r\n") + 2;
	while (pos < lastPositionToCheck) {
		mid = bufferHeader.find(" ", pos);
		npos = bufferHeader.find("\r\n", mid);
		if (pos > npos) {
			error = BAD_REQUEST;
			return ;
		}
		pair.first = bufferHeader.substr(pos, mid - pos);
		pair.second = bufferHeader.substr((mid + 1), npos - (mid + 1));
		if (pair.first == "Connection:") {
			if (pair.second == "keep-alive")
				this->_delete = false;
			else
				this->_delete = true;
		}
		this->headerPair.insert(pair);
		pos = npos + 2;
	}
}

void HeaderRequest::ParseFirstLine(std::string& bufferHeader) {
	std::string			key;
	std::string			value;
	size_t				pos;
	size_t				npos;

	pos = 0;
	for (int i = 0; i < 3; i++) {
		if (i == 2) {
			pos = bufferHeader.find("/", pos) + 1;
			npos = bufferHeader.find("\r\n", pos);
			value = bufferHeader.substr(pos, npos - pos);
		}
		else {
			npos = bufferHeader.find(" ", pos);
			value = bufferHeader.substr(pos, npos - pos);
		}
		if (pos == std::string::npos || npos == std::string::npos) {
			error = BAD_REQUEST;
			return ;
		}
		switch (i)
		{
			case 0:
				key = "Method:";
				break;
			case 1:
				key = "Request-Target:";
				if (value.find(".php") != std::string::npos || value.find(".bash") != std::string::npos) {
					logs("c'est un CGI");
					this->isCGI = true;
				}
				break;
			case 2:
				key = "HTTP:";
				break;
		}
		this->headerPair.insert(std::pair<std::string, std::string>(key, value));
		pos = npos + 1;
	}
	SetMethod(headerPair["Method:"]);
}

void HeaderRequest::CleanHeader(void) {
	this->body.clear();
	this->error = OK;
	this->headerPair.clear();
	this->method = OTHER;
	this->_delete = false;
}

//Getters

std::map<std::string, std::string>& HeaderRequest::getPairs(void) {
	return (this->headerPair);
}

std::string HeaderRequest::GetBody(void) {
	return (this->body);
}

Method HeaderRequest::GetMethod(void) {
	return (this->method);
}

Error HeaderRequest::GetError(void) {
	return (this->error);
}

bool HeaderRequest::GetDeleteSocket(void) {
	return (this->_delete);
}

bool HeaderRequest::GetIsCGI(void) {
	return (this->isCGI);
}

//Setters

void HeaderRequest::SetMethod(std::string& method) {
	if (method == "POST")
		this->method = POST;
	else if (method == "GET")
		this->method = GET;
	else if (method == "DELETE")
		this->method = DELETE;
	else
		this->method = OTHER;
}

void HeaderRequest::SetError(Error err) {
	this->error = err;
	return;
}

void HeaderRequest::SetBody(std::string str) {
	this->body = str;
	return;
}
