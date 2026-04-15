/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headerRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:01:30 by lomont            #+#    #+#             */
/*   Updated: 2026/04/15 17:17:58 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerRequest.hpp"
#include "webserv.hpp"

//Constructor

HeaderRequest::HeaderRequest(void) : _delete(false), authorized(false), error(OK), method(OTHER), body("") {
}

HeaderRequest::HeaderRequest(std::string bufferHeader) : _delete(false), authorized(false), error(OK), method(OTHER), body("") {
	ParseHeaderRequest(bufferHeader);
}

HeaderRequest::HeaderRequest(Error err) : _delete(false), authorized(false), error(err), method(OTHER), body("") {
}

//Copy constructor & assignement operator

HeaderRequest::HeaderRequest(const HeaderRequest& other) {
	*this = other;
	return ;
}

HeaderRequest& HeaderRequest::operator=(const HeaderRequest& other) {
	if (&other != this) {
		this->_delete = other._delete;
		this->authorized = other.authorized;
		this->error = other.error;
		this->method = other.method;
		this->body = other.body;
		this->headerPair = other.headerPair;
	}
	return *this;
}

//Destructor

HeaderRequest::~HeaderRequest(void) {
}


void HeaderRequest::ParseHeaderRequest(std::string& bufferHeader) {
	ParseFirstLine(bufferHeader);
	if (error != OK)
		return ;
	size_t lastPositionToCheck = bufferHeader.find("\r\n\r\n");
	if (lastPositionToCheck == std::string::npos) {
		error = BAD_REQUEST;
		return ;
	}
	size_t pos = bufferHeader.find("\r\n") + 2;
	std::pair<std::string, std::string>	pair;
	while (pos < lastPositionToCheck) {
		size_t mid = bufferHeader.find(" ", pos);
		size_t npos = bufferHeader.find("\r\n", mid);
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
		else if (pair.first == "Cookie:") {
			if (pair.second == "session_auth=67")
				this->authorized = true;
		}
		this->headerPair.insert(pair);
		pos = npos + 2;
	}
}

void HeaderRequest::ParseFirstLine(std::string& bufferHeader) {
	std::string	key;
	std::string	value;
	size_t 		pos = 0;
	size_t 		npos = 0;
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
				break;
			case 2:
				key = "HTTP:";
				break;
		}
		std::cout << "key = " << key << std::endl;
		std::cout << "value = " << value << std::endl;
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
	return this->headerPair;
}

std::string HeaderRequest::GetBody(void) {
	return this->body;
}

Method HeaderRequest::GetMethod(void) {
	return this->method;
}

Error HeaderRequest::GetError(void) {
	return this->error;
}

bool HeaderRequest::GetDeleteSocket(void) {
	return this->_delete;
}

bool HeaderRequest::GetAuthorized(void) {
	return this->authorized;
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
}

void HeaderRequest::SetBody(std::string str) {
	this->body = str;
}

void HeaderRequest::SetDeleteRequest(bool to_delete) {
	if (to_delete)
		this->_delete = true;
	else
		this->_delete = false;
}
