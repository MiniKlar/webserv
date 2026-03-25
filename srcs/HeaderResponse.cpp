/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:28 by lomont            #+#    #+#             */
/*   Updated: 2026/03/26 00:13:29 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderResponse.hpp"
#include "webserv.hpp"

//Constructors

HeaderResponse::HeaderResponse(HeaderRequest& setRequest, struct config* setConfig) : request(setRequest), config(setConfig), header(""), buffer(""), bodySizePrint("0"), pathfile(""), indexLocationConfig(0), bodySize(0), error(false), parsed(false), cookie(false) {
	//Create the HeaderResponse depending the method and/or the file is a CGI
	CreateResponse();
	return ;
}

HeaderResponse::HeaderResponse(void) : request(HeaderRequest()), config(NULL), header(""), buffer(""), bodySizePrint(""), pathfile(""), indexLocationConfig(0), bodySize(0), error(false), parsed(false) {
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
		this->pathfile = other.pathfile;
		this->indexLocationConfig = other.indexLocationConfig;
		this->bodySize = other.bodySize;
		this->error = other.error;
		this->parsed = other.parsed;
		this->cookie = other.cookie;
	}
	return (*this);
}

//Destructor

HeaderResponse::~HeaderResponse(void) {
	return;
}

//Member functions

void HeaderResponse::CreateResponse(void) {
	//On cherche la location du fichier demandé grâce à son path et aux routes décrites dans le fichier de configuration webserv
	FindFileLocation();
	//On check que la méthode est bien implémenté et accepté par la route demandé
	CheckMethod();
	//Selon la route on va choisir les fonctions à utiliser
	if (this->request.GetIsCGI() == true)
		//TODO si erreur dans le script alors que faire?
		//TODO revoir comment fonctionne un cgi
		HandleCGI();
	else {
		if (this->request.GetMethod() == GET)
			HandleGet();
		else if (this->request.GetMethod() == DELETE)
			HandleDelete();
		else if (this->request.GetMethod() == POST)
			HandlePost();
		if (!this->error)
			GetHeaderResponse();
		if (this->header.empty()) {
			SearchErrorPage();
			SetFileSize();
			OpenFile();
			this->header = CheckErrors();
		}
		if (cookie) {
			size_t pos;
			pos = header.find("\r\n\r\n");
			header.erase(pos, 2);
			header.append("Set-Cookie: session_auth=67; Path=/; Max-Age=3600");
			header.append("\r\n\r\n");
		}
		buffer = header + buffer;
		parsed = true;
	}
	//if location old page
	//if autoindex
}

void HeaderResponse::HandleCGI() {
	std::ostringstream	oss;
	std::string			buffer;

	buffer = PerformCGI();
	size_t pos = buffer.find("\r\n") + 3;
	this->bodySize = pos - buffer.size();
	oss << bodySize;
	this->bodySizePrint = oss.str();
	parsed = true;
	return ;
}

void HeaderResponse::HandleGet() {
	if (this->request.getPairs()["Request-Target:"] == "/get-cookie") {
		bodySize = 0;
		bodySizePrint = "0";
		cookie = true;
		return ;
	}
	FindPathFile(); //trouver le chemin complet du fichier
	SetFileSize(); //Trouver et set la taille du fichier qu'on va renvoyer
	if (!this->error) //S'il n'y a pas eu d'erreur, alors on peut essayer d'ouvrir le fichier
		OpenFile();
}
void HeaderResponse::HandleDelete() {
	FindPathFile();
	DeleteFile();
}

void HeaderResponse::HandlePost() {
	if (this->request.GetAuthorized() == false) {
		this->error = true;
		this->request.SetError(FORBIDDEN);
		return ;
	}
	else if (this->request.GetError() != OK) {
		this->error = true;
		return ;
	}
	ParseBody();
}

void HeaderResponse::DeleteFile(void) {
	std::string path = "." + pathfile;
	std::cout << "pathfile à delete = [" << pathfile  << "]" << std::endl;
	if (remove(path.c_str()) == -1) {
		logs("Error: can't delete file; file not found");
		this->request.SetError(NOT_FOUND);
		this->error = true;
		return;
	}
	logs("File deleted");
	return ;
}

void HeaderResponse::SearchErrorPage(void) {
	int errorCode = this->request.GetError();
	std::map<std::vector<int>, std::string>::iterator it = this->config->errorPage.begin();

	for (; it != this->config->errorPage.end(); it++) {
		const std::vector<int>& errorCodesKeys = it->first;
		if (std::find(errorCodesKeys.begin(), errorCodesKeys.end(), errorCode) != errorCodesKeys.end()) {
			pathfile = it->second;
			return ;
		}
	}
}

void HeaderResponse::FindFileLocation(void) {
	struct LocationConfig*	ptr;
	std::string 			str;
	size_t					pos;

	pos = 0;
	str = this->request.getPairs()["Request-Target:"];
	ptr = this->config->locationConfig;
	logs(str);
	for (size_t i = 0; i < this->config->numbersOfLocation; i++) {
		logs(ptr[i].location);
		if (str.compare(0, ptr[i].location.size(), ptr[i].location) == 0) {
			if (ptr[i].location.size() > pos) {
				pos = ptr[i].location.size();
				indexLocationConfig = i;
			}
		}
	}
	//TODO faire attention si il n'y a pas de routes => à check
}

void HeaderResponse::GetHeaderResponse(void) {
	Method method = request.GetMethod();
	if (method == GET)
		this->header = code_200();
	else if (method == POST)
		this->header = code_201();
	else if (method == DELETE)
		this->header = code_204();
	return ;
}

void HeaderResponse::OpenFile(void) {
	int		file;
	int		bread;
	char 	*buffer;

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
		close(file);
		return ;
	}
	bread = read(file, buffer, bodySize);
	if (bread <= 0) {
		delete[] buffer;
		this->request.SetError(INTERNAL);
		error = true;
		close(file);
		return ;
	}
	buffer[bread] = '\0';
	close(file);
	this->buffer.append(buffer, bread);
	return;
}

//This function searches if a pathname is available, or it finds the oldest file to overwrite it
std::string HeaderResponse::FindFileName(void) {
	std::string 		oldestFile;
	std::string 		filename;
	std::ostringstream 	oss;
	struct stat			s_stat;
	timespec			t_time;

	t_time.tv_sec = 0;
	t_time.tv_nsec = 0;
	for (int i = 0; i < 3; i++) {
		oss << "image" << i << ".png";
		filename = std::string(".") + UPLOAD_FOLDER + oss.str();
		if (access(filename.c_str(), F_OK) != 0)
			return (oss.str());
		if (stat(filename.c_str(), &s_stat) == 0) {
				if (s_stat.st_mtimespec.tv_sec < t_time.tv_sec
						|| (s_stat.st_mtimespec.tv_sec == t_time.tv_sec && s_stat.st_mtimespec.tv_nsec < t_time.tv_nsec)
							|| t_time.tv_sec == 0) {
						oldestFile = oss.str();
						t_time.tv_sec = s_stat.st_mtimespec.tv_sec;
						t_time.tv_nsec = s_stat.st_mtimespec.tv_nsec;
				}
		}
		oss.str("");
	}
	return (oldestFile);
}

void HeaderResponse::ParseBody() {
	std::string	upload_path;

	upload_path = this->config->locationConfig[indexLocationConfig].upload_store;
	if (upload_path.empty()) {
		logs("upload path empty");
		this->request.SetError(FORBIDDEN);
		this->error = true;
		return ;
	}
	//extraire boundary depuis Content-type
	std::string contentType = this->request.getPairs()["Content-Type:"];
	std::string	boundary = contentType.substr(contentType.find("boundary=") + 10, contentType.find("\r\n") - contentType.find("boundary=") + 10);
	CreateImage(this->request.GetBody(), boundary, upload_path);
}

void HeaderResponse::CreateImage(const std::string& bufferBody, std::string& boundary, std::string& uploadPath) {
	if (boundary.length() == 0)
		return ; //TODO revoir gestion error
	if (bufferBody.length() > 4) {
		std::string name;
		std::string filename;
		std::string image_png;
		size_t startpos;
		size_t endpos;

		startpos = bufferBody.find("name=") + 6;
		endpos = bufferBody.find(";", startpos);
		name = bufferBody.substr(startpos, endpos - startpos - 1);
		logs(name);
		startpos = bufferBody.find("filename=") + 10;
		endpos = bufferBody.find("\r\n", startpos);
		filename = bufferBody.substr(startpos, endpos - startpos - 1);
		logs(filename);
		startpos = bufferBody.find("Content-Type:") + 14;
		endpos = bufferBody.find("\r\n", startpos);
		image_png = bufferBody.substr(startpos, endpos - startpos);
		logs(image_png);

		startpos = endpos + 4;
		std::string delimiter = "\r\n" + boundary + "--";
		logs(delimiter);
		endpos = bufferBody.find(delimiter, startpos);
		std::string image = bufferBody.substr(startpos, endpos - startpos);

		std::string img_filename = uploadPath + FindFileName();
		img_filename = "." + img_filename;
		logs(img_filename);
		int image_fd = open(img_filename.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IROTH | S_IRUSR | S_IRGRP);
		if (image_fd == -1) {
			this->error = true;
			this->request.SetError(INTERNAL);
			logs("Error while trying to create the new image");
			return ;
		}
		write(image_fd, image.data(), image.size());
		close(image_fd);
		pathfile = img_filename;
	};
}

void HeaderResponse::CheckMethod(void) {
	std::vector<std::string>	methods;
	std::string					headerMethod;
	bool						methodAccepted;

	methodAccepted = false;
	headerMethod = this->request.getPairs()["Method:"];
	//check si la méthode fait parti des méthodes prises en charge par webserv
	if (headerMethod != "GET" && headerMethod != "POST" && headerMethod != "DELETE") {
		logs("Method not implemented");
		this->request.SetError(NOT_IMPLEMENTED);
		error = true;
		return ;
	}
	//check si le header est pris en charge par la location
	methods = this->config->locationConfig[indexLocationConfig].methods;
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		if (*it == headerMethod)
			methodAccepted = true;
	}
	if (!methodAccepted) {
		logs("Method not allowed by the route");
		this->request.SetError(NOT_ALLOWED);
		error = true;
	}
	return;
}

void HeaderResponse::CleanHeader(void) {
	this->config = NULL;
	this->header.clear();
	this->buffer.clear();
	this->bodySizePrint.clear();
	this->pathfile.clear();
	this->indexLocationConfig = -1;
	this->bodySize = 0;
	this->error = false;
	this->parsed = false;
}

void HeaderResponse::FindPathFile(void) {
	std::string str;
	std::string uri;
	std::string root;

	if (error)
		return ;
	str = this->request.getPairs()["Request-Target:"];
	uri = this->config->locationConfig[indexLocationConfig].location;
	root = this->config->locationConfig[indexLocationConfig].root;
	this->pathfile = root + str;
	logs("pathfile = [" + pathfile + "]");
	return ;
}

std::string HeaderResponse::CheckErrors(void) {
	switch (this->request.GetError())
	{
		case OK:
			return ("");
		case BAD_REQUEST :
			return (code_400());
		case FORBIDDEN :
			return (code_403());
		case NOT_FOUND :
			return (code_404());
		case NOT_ALLOWED :
			return (code_405());
		case LENGTH :
			return (code_411());
		case BODY_TOO_LARGE:
			return (code_413());
		case INTERNAL :
			return (code_500());
		case NOT_IMPLEMENTED :
			return (code_501());
		case VERSION :
			return (code_505());
	}
	return ("");
}

//Response code

std::string HeaderResponse::code_200( void ) {
	return ("HTTP/1.1 200 OK\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_201(void) {
	return ("HTTP/1.1 201 Created\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\nLocation: " + pathfile + "\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: 0" + "\r\n\r\n");
}

std::string HeaderResponse::code_204(void) {
	return ("HTTP/1.1 204 No Content\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_400(void) {
	return ("HTTP/1.1 400 Bad Request\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_403(void) {
	return ("HTTP/1.1 403 Forbidden\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_404(void) {
	return ("HTTP/1.1 404 Not Found\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\n"+ GetContentType() +"\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_405(void) {
	return ("HTTP/1.1 405 Method Not Allowed\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Length: 0\r\nAllow: " + GetMethodAllowed() + "\r\n\r\n");
}

std::string HeaderResponse::code_411(void) {
	return ("HTTP/1.1 411 Length Required\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_413(void) {
	return ("HTTP/1.1 413 Content Too Large\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
		+ bodySizePrint + "\r\nContent-Max-Size: " + GetMaxBodySize() + "\r\nConnection: Close" + "\r\n\r\n");
}

std::string HeaderResponse::code_500(void) {
	return ("HTTP/1.1 500 Internal Server Error\r\nContent-Type:" + GetContentType() + "\r\nContent-Length: " + bodySizePrint + "\r\n\r\n");
}

std::string HeaderResponse::code_501(void) {
	return ("HTTP/1.1 501 Not Implemented\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\n\r\n");
}

std::string HeaderResponse::code_505(void) {
	return ("HTTP/1.1 505 HTTP Version Not Supported\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\n\r\n");
}

//Getters

std::string HeaderResponse::GetContentType(void) {
	if (this->request.GetMethod() == POST)
		return (IMG_TYPE);
	else
		return (HTML_TYPE);
}

std::string HeaderResponse::GetBuffer(void) {
	return (this->buffer);
}

std::string HeaderResponse::GetConnectionStatut(void) {
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

	tmp = this->config->locationConfig[indexLocationConfig].methods;
	for (size_t i = 0; i < tmp.size(); i++) {
		methods += tmp[i];
		if (i + 1 < tmp.size())
			methods += " ";
	}
	return (methods);
}

std::string HeaderResponse::GetMaxBodySize(void) {
	std::ostringstream oss;

	oss << this->config->maxBodySize;
	return (oss.str());
}

bool HeaderResponse::IsParsed(void) {
	return (this->parsed);
}

std::string HeaderResponse::GetCurrentTime( void ) {
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

//Setters

void HeaderResponse::SetBuffer(std::string str) {
	this->buffer = str;
	return ;
}

void HeaderResponse::SetFileSize(void) {
	struct stat s;
	std::ostringstream oss;
	std::string size;

	pathfile = "." + pathfile;
	if (stat(pathfile.c_str(), &s) == -1) {
		if (stat(DEFAULT_ERROR_PAGE, &s) == -1) {
			logs("error stat");
			return; //peut etre hard code une réponse d'erreur?
		}
		else {
			this->request.SetError(NOT_FOUND);
			pathfile = DEFAULT_ERROR_PAGE;
			error = true;
		}
	}
	if (s.st_mode & S_IFDIR) {
		if (stat(DEFAULT_ERROR_PAGE, &s) == -1)
			return; //peut etre hard code une réponse d'erreur?
		this->request.SetError(NOT_FOUND);
		pathfile = DEFAULT_ERROR_PAGE;
		error = true;
	}
	bodySize = s.st_size;
	oss << s.st_size;
	bodySizePrint = oss.str();
	return ;
}

//CGI

std::string HeaderResponse::PerformCGI(void)
{
	pid_t	pid;
	char 	buf[4096];
	char	*envp[] = {(char*)"REQUEST_METHOD=GET", (char*)"SCRIPT_NAME=test.bash", (char*)"CONTENT_LENGTH=0"};
    int		pipe_out[2];

	//Create envp


	//
    if (pipe(pipe_out) == -1)
        return ("");
    pid = fork();
    if (pid == 0)
    {
        dup2(pipe_out[1], STDOUT_FILENO);

        close(pipe_out[0]);
        close(pipe_out[1]);

        //chdir(path);
        char *args[] = {(char*)"/bin/bash", (char*)"test.bash", NULL};
        if (execve(args[0], args, envp) == -1)
			//free if necessary
            exit(1);
    }
    else
    {
        close(pipe_out[1]);
        size_t n;
        std::string answer;
        while ((n = read(pipe_out[0], buf, sizeof(buf))) > 0)
            answer.append(buf, n);
        waitpid(pid, NULL, 0);
		close(pipe_out[0]);
        return (answer);
    }
	return ("");
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
