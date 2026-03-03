/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:28 by lomont            #+#    #+#             */
/*   Updated: 2026/03/03 02:28:57 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderResponse.hpp"

HeaderResponse::HeaderResponse(HeaderRequest& setRequest) : request(setRequest) {
	char*	bufferFileRequested;

	std::cout << request.GetMethod();
	if (request.GetMethod() == POST) {
		std::cout << "tu es ici" << std::endl;
		code = CREATED;
		std::string pathfile = "./html/drive/image1.png";
		getContentLength(pathfile);
	}
	else {
		std::string pathfile = FILE_LOCATION + request.GetRequestTarget();
		getContentLength(pathfile);
		bufferFileRequested = SearchFileRequested(pathfile);
		if (!bufferFileRequested)
			ft_error("[CRASH] buffer empty", 28);
		content = std::string(bufferFileRequested, fileSize);
		delete[] bufferFileRequested;
	}
	return;
}

HeaderResponse::HeaderResponse(void) {
	return;
}

HeaderResponse::HeaderResponse(const HeaderResponse& other) {
	*this = other;
	return ;
}

HeaderResponse& HeaderResponse::operator=(const HeaderResponse& other) {
	if (&other != this) {
		this->content = other.content;
		this->fileSize = other.fileSize;
		this->Map = other.Map;
		this->stringFileSize = other.stringFileSize;
		this->code = other.code;
	}
	return (*this);
}

HeaderResponse::~HeaderResponse(void) {
	return;
}

char* HeaderResponse::SearchFileRequested(std::string& pathfile) {
	int		file;
	int		bread;
	char 	*buffer;

	buffer = NULL;
	file = open(pathfile.c_str(), O_RDONLY);
	if (file == -1)
		ft_error("[CRASH] can't open", 21);
	buffer = new char[fileSize + 1];
	if (!buffer)
		ft_error("Memory allocation failed", 20);
	bread = read(file, buffer, fileSize);
	if (bread <= 0) {
		delete[] buffer;
		ft_error("read failed", 21);
	}
	buffer[bread] = '\0';
	close(file);
	return (buffer);
}

std::string HeaderResponse::getContent(void) {
	return (this->content);
}

std::string HeaderResponse::getCurrentTime( void ) {
	time_t currentTime;
	struct tm* localTime;
	char ptr[1024];
	currentTime = time(NULL);
	localTime = localtime(&currentTime);
	if (strftime(ptr, 1024, "%a, %d %b %Y %X %Z", localTime) == 0) {
		std::cerr << "error time" << std::endl;
		exit(15);
	}
	return (std::string(ptr));
}

void	HeaderResponse::getContentLength(std::string& pathfile) {
	struct stat s;
	std::ostringstream oss;
	std::string size;
	if (stat(pathfile.c_str(), &s) == -1) {
		if (stat(PAGE_404, &s) == -1)
			ft_error("[CRASH] PAGE_404 stat failed", 16);
		code = NOT_FOUND;
		pathfile = PAGE_404;
	}
	if (request.GetMethod() == GET && pathfile != PAGE_404)
		code = OK;
	fileSize = s.st_size;
	oss << s.st_size;
	stringFileSize = oss.str();
}

std::string HeaderResponse::getConnectionStatut(void) {
	std::map<std::string, std::string>& map = Map;
	std::map<std::string, std::string>::iterator it;
	it = Map.find("Connection:");
	if (it != map.end())
		return ("Connection: " + it->second);
	return ("");
}

std::string HeaderResponse::code_200( void ) {
	return ("HTTP/1.1 200 OK\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "
	+ stringFileSize + "\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_404(void) {
	return ("HTTP/1.1 404 Not Found\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "
	+ stringFileSize + "\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_201(void) {
	return ("HTTP/1.1 201 Created\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nLocation: " + getLocation() + "Content-Type: application/json\r\nContent-Length: "
	+ stringFileSize + "\r\n\r\n");
}

std::string HeaderResponse::code_204(void) {
	return ("HTTP/1.1 204 No Content\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::getLocation(void) {
	std::string filepath = UPLOAD_LOCATION + request.GetPathImageCreated();
	if (access(filepath.c_str(), R_OK) == 0)
		return (filepath);
	else
		return ("error"); //better define the error management of access
}

off_t& HeaderResponse::GetFileSize(void) {
	return (fileSize);
}

Code HeaderResponse::GetCode(void) {
	return (code);
}

// HTTP/1.1 201 Created
// Date: Wed, 18 Feb 2026 22:01:00 GMT
// Server: nginx/1.18.0
// Location: /api/users/43
// Content-Type: application/json
// Content-Length: 51

// HTTP/1.1 200 OK
// Date: Wed, 18 Feb 2026 22:00:00 GMT
// Server: Apache/2.4.41 (Ubuntu)
// Content-Type: text/html; charset=UTF-8
// Content-Length: 138
// Connection: keep-alive

// HTTP/1.1 204 No Content
// Date: Wed, 18 Feb 2026 22:02:00 GMT
// Server: nginx/1.18.0
// Connection: keep-alive
