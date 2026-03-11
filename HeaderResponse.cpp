/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:28 by lomont            #+#    #+#             */
/*   Updated: 2026/03/11 02:22:25 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderResponse.hpp"
#include "webserv.hpp"

//Constructors

HeaderResponse::HeaderResponse(HeaderRequest& setRequest, struct config* setConfig) : request(setRequest), config(setConfig), error(false) {
	FindPath(); //find path file
	SetBodySize(); //set size of the file requested
	CheckMethod();
	//if location old page
	//if autoindex
	//if cgi pass
	OpenBodyFile();
	buffer = code_200() + buffer;
	//if (!CheckErrors()) // pas bon
		//pas ok
	return ;
}

HeaderResponse::HeaderResponse(void) : request(HeaderRequest()), config(NULL) {
	return ;
}

//Copy constructor & copy assignement

HeaderResponse::HeaderResponse(const HeaderResponse& other) {
	*this = other;
	return ;
}

HeaderResponse& HeaderResponse::operator=(const HeaderResponse& other) {
	if (&other != this) {
		this->request = other.request;
		this->config = other.config;
		this->buffer = other.buffer;
		this->bodySizePrint = other.bodySizePrint;
		this->bodySize = other.bodySize;
		this->pathfile = other.pathfile;
		this->indexLocationConfig = other.indexLocationConfig;
		this->error = other.error;
	}
	return (*this);
}

//Destructor

HeaderResponse::~HeaderResponse(void) {
	return;
}

void HeaderResponse::OpenBodyFile(void) {
	int		file;
	int		bread;
	char 	*buffer;

	if (error)
		return ;
	buffer = NULL;
	file = open(pathfile.c_str(), O_RDONLY);
	if (file == -1) {
		this->request.SetError(INTERNAL);
		error = true;
		return ;
	}
	buffer = new char[bodySize + 1];
	if (!buffer) {
		this->request.SetError(INTERNAL);
		error = true;
		return ;
	}
	bread = read(file, buffer, bodySize);
	if (bread <= 0) {
		delete[] buffer;
		this->request.SetError(INTERNAL);
		error = true;
		return ;
	}
	buffer[bread] = '\0';
	close(file);
	std::cout << "buffer = [" << buffer << "]" << std::endl;
	this->buffer += buffer;
	return;
}

void HeaderResponse::CheckMethod(void) {
	std::vector<std::string> methods;
	std::string				headerMethod;
	bool					methodAccepted;

	if (error)
		return ;
	methodAccepted = false;
	methods = this->config->locationConfig[indexLocationConfig].methods;
	headerMethod = this->request.getPairs()["Method:"];
	std::cout << "ici avant crash" << std::endl;
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		std::cout << "method = [" << *it << "]" << std::endl;
		if (*it == headerMethod)
			methodAccepted = true;
	}
	if (methodAccepted == false) {
		this->request.SetError(NOT_IMPLEMENTED);
		error = true;
	}
	return;
}

void HeaderResponse::FindPath(void) {
	struct LocationConfig*	ptr;
	std::string str;
	std::string uri;
	std::string root;
	size_t		pos;

	if (error)
		return ;
	pos = 0;
	str = this->request.getPairs()["Request-Target:"];
	ptr = this->config->locationConfig;
	std::cout << "voici str = [" << str << "]" << std::endl;
	for (size_t i = 0; i < this->config->numbersOfLocation; i++) {
		std::cout << "location = [" << ptr[i].location << "]" << std::endl;
		if (str.compare(0, ptr[i].location.size(), ptr[i].location) == 0) {
			if (ptr[i].location.size() > pos) {
				pos = ptr[i].location.size();
				uri = ptr[i].location;
				indexLocationConfig = i;
			}
		}
	}
	std::cout << "uri = [" << uri << "]" << std::endl;
	root = ptr[indexLocationConfig].root;
	std::cout << "root = [" << root << "]" << std::endl;
	this->pathfile = root + str;
	std::cout << "pathile = [" << pathfile << "]" << std::endl;
	return ;
}

std::string HeaderResponse::CheckErrors(void) {
	switch (this->request.GetError())
	{
		case OK:
			return ("");
		case BAD_REQUEST :
			return (code_400());
		case NOT_FOUND :
			return (code_404());
		case NOT_ALLOWED :
			return (code_405());
		case LENGTH :
			return (code_411());
		case INTERNAL :
			return (code_500());
		case NOT_IMPLEMENTED :
			return (code_501());
		case VERSION :
			return (code_505());
	}
	return ("");
}

// std::string HeaderResponse::getContent(void) {
// 	return (this->content);
// }

std::string HeaderResponse::getCurrentTime( void ) {
	time_t currentTime;
	struct tm* localTime;
	char ptr[1024];
	currentTime = time(NULL);
	localTime = gmtime(&currentTime);
	if (strftime(ptr, 1024, "%a, %d %b %Y %X GMT", localTime) == 0) {
		std::cerr << "error time" << std::endl;
		exit(15);
	}
	return (std::string(ptr));
}

void HeaderResponse::SetBodySize(void) {
	struct stat s;
	std::ostringstream oss;
	std::string size;

	pathfile = "." + pathfile;
	if (error)
		return ;
	if (stat(pathfile.c_str(), &s) == -1) {
		if (stat(DEFAULT_ERROR_PAGE, &s) == -1)
			return; //peut etre hard code une réponse d'erreur?
		else {
			this->request.SetError(NOT_FOUND);
			pathfile = DEFAULT_ERROR_PAGE;
			error = true;
		}
	}
	bodySize = s.st_size;
	oss << s.st_size;
	bodySizePrint = oss.str();
	std::cout << "bodySizePrint = [" << bodySizePrint << "]" << std::endl;
	std::cout << "error=" << error << std::endl;
	return ;
}

// void	HeaderResponse::CheckConnectionStatut(void) {
// 	std::map<std::string, std::string>& map = Map;
// 	std::map<std::string, std::string>::iterator it;
// 	it = Map.find("Connection:");
// 	if (it != map.end()) {
// 		if (it->second == "keep-alive")
// 			deleteSocket = false;
// 	}
// 	return ;
// }

std::string HeaderResponse::getConnectionStatut(void) {
	std::map<std::string, std::string>& map = this->request.getPairs();
	std::map<std::string, std::string>::iterator it;
	it = map.find("Connection:");
	if (it != map.end())
		return ("Connection: " + it->second);
	return ("");
}

std::string HeaderResponse::GetMethodAllowed(void) {
	//location doit être une référence trouvé
	std::vector<std::string>	tmp;
	std::string					methods;

	tmp = this->config->locationConfig[this->indexLocationConfig].methods;
	for (size_t i = 0; i < tmp.size(); i++) {
		methods = tmp[i];
		if (i + 1 < tmp.size())
			methods += " ";
	}
	return (methods);
}

std::string HeaderResponse::code_200( void ) {
	return ("HTTP/1.1 200 OK\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_201(void) {
	return ("HTTP/1.1 201 Created\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nLocation: " + "hardcode" + "\r\nContent-Type: text/html; charset=UTF-8" + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n\r\n");
}

std::string HeaderResponse::code_204(void) {
	return ("HTTP/1.1 204 No Content\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_400(void) {
	return ("HTTP/1.1 400 Bad Request\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_404(void) {
	return ("HTTP/1.1 404 Not Found\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n"); //récupérer taille page erreur ?
}

std::string HeaderResponse::code_405(void) {
	return ("HTTP/1.1 405 Method Not Allowed\r\nDate: " + this->getCurrentTime() + "\r\nServer:Webserv\r\nAllow: " + GetMethodAllowed() + "\r\n");
}

std::string HeaderResponse::code_411(void) {
	return ("HTTP/1.1 411 Length Required\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_500(void) {
	return ("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html;\r\nContent-Length: " + bodySizePrint + "\r\n\r\n");
}

std::string HeaderResponse::code_501(void) {
	return ("HTTP/1.1 501 Not Implemented\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n\r\n");
}

std::string HeaderResponse::code_505(void) {
	return ("HTTP/1.1 505 HTTP Version Not Supported\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n\r\n");
}


// off_t& HeaderResponse::GetFileSize(void) {
// 	return (fileSize);
// }

// Code HeaderResponse::GetCode(void) {
// 	return (code);
// }

// bool HeaderResponse::IsEmpty(void) {
// 	if (this->empty)
// 		return (true);
// 	return (false);
// }

// bool HeaderResponse::GetDeleteSocket(void) {
// 	return (this->deleteSocket);
// }

std::string HeaderResponse::GetBuffer(void) {
	return (this->buffer);
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
