/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headerResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/05 22:42:38 by lomont            #+#    #+#             */
/*   Updated: 2026/04/06 17:34:33 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerResponse.hpp"
#include "webserv.hpp"

//Constructors

HeaderResponse::HeaderResponse(HeaderRequest& setRequest, struct config* setConfig) : cookie(false), error(false), parsed(false), isCGI(false), bodySize(0), indexLocationConfig(0), header(""), buffer(""), bodySizePrint("0"), pathfile(""), config(setConfig), request(setRequest){
	//Create the HeaderResponse depending the method and/or the file is a CGI
	CreateResponse();
	return ;
}

HeaderResponse::HeaderResponse(void) : cookie(false), error(false), parsed(false), isCGI(false), bodySize(0), indexLocationConfig(0), header(""), buffer(""), bodySizePrint(""), pathfile(""), config(NULL), request(HeaderRequest()) {
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
		this->isCGI = other.isCGI;
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
	if (isCGI)
		HandleCGI();
	else if (this->request.GetMethod() == GET && !isCGI)
		HandleGet();
	else if (this->request.GetMethod() == DELETE)
		HandleDelete();
	else if (this->request.GetMethod() == POST && !isCGI)
		HandlePost();
	if (!this->error && !isCGI)
		GetHeaderResponse();
	if (this->header.empty()) {
		if (pathfile == "ERROR") {
			SearchErrorPage();
			ft_logs(pathfile);
			if (!pathfile.empty()) {
				SetFileSize();
				OpenFile();
			}
			else {
				this->buffer = default_error_page();
				this->bodySize = buffer.size();
				std::ostringstream oss;
				oss << this->bodySize;
				this->bodySizePrint = oss.str();
			}
		}
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
	ft_logs(buffer);
	parsed = true;
}

void HeaderResponse::HandleCGI(void) {
	size_t				pos;
	std::ostringstream	oss;
	std::string			header;
	std::string			buffer;
	std::string			cgi_headers;

	ft_logs("tu handle un CGI");

	FindPathFile();
	buffer = PerformCGI();
	if (buffer.empty()) {
		this->error = true;
		this->request.SetError(INTERNAL);
		pathfile = "ERROR";
		return ;
	}
	pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos) {
		this->error = true;
		this->request.SetError(INTERNAL);
		pathfile = "ERROR";
		return ;
	}
	pos += 4;

	cgi_headers = buffer.substr(0, pos);
	this->buffer = buffer.substr(pos);

	this->bodySize = this->buffer.size();
	oss << bodySize;
	this->bodySizePrint = oss.str();

	this->header = "HTTP/1.1 200 OK\r\n";
    this->header += "Date: " + this->GetCurrentTime() + "\r\n";
    this->header += "Server: Webserv\r\n";
    this->header += "Content-Length: " + this->bodySizePrint + "\r\n";
	this->header += cgi_headers;
	parsed = true;
	return ;
}

void HeaderResponse::HandleGet(void) {
	struct stat 				s;
	std::stringstream			ss;
	std::vector<std::string>	index;
	std::string					path;
	bool						found;

	found = false;
	if (this->request.getPairs()["Request-Target:"] == "/get-cookie") {
		bodySize = 0;
		bodySizePrint = "0";
		cookie = true;
		return ;
	}
	FindPathFile(); //trouver le chemin complet du fichier
	if (this->config->locationConfig) {
		index = this->config->locationConfig[indexLocationConfig].index;
		if (this->pathfile.back() == '/') {
			if (!index.empty()) {
				for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); it++) {
					path = std::string(".") + this->pathfile + *it;
					std::cout << "index = " << path << std::endl;
					if (access(path.c_str(), F_OK) == 0) {
						if (stat(path.c_str(), &s) && !(s.st_mode & S_IFDIR)) {
							pathfile = path;
							found = true;
							break;
						}
					}
				}
			}
			if (!found) {
				if (this->config->locationConfig[indexLocationConfig].autoindex == true) {
					ft_logs("tu es sur une route '/' avec autoindex on");
					HandleAutoIndex();
					this->bodySize = buffer.size();
					ss << bodySize;
					this->bodySizePrint = ss.str();
					ft_logs(this->bodySizePrint);
					return ;
				}
			}
		}
	}
	ft_logs("tu es avant setfilesize");
	SetFileSize(); //Trouver et set la taille du fichier qu'on va renvoyer
	std::cout << this->request.GetError() << std::endl;
	ft_logs("tu es apres setfilesize");
	if (!this->error) //S'il n'y a pas eu d'erreur, alors on peut essayer d'ouvrir le fichier
		OpenFile();
	return ;
}

void HeaderResponse::HandleAutoIndex(void) {
	std::string	header;
	std::string	pathfile;
	std::string buffer;

	pathfile = this->pathfile;
	ft_logs("pathfile for autoindex = " + pathfile);
	header = "<html>\n<head><title>Index of " + pathfile + "</title></head>\n<body>\n<h1>Index of " + pathfile + "</h1><hr>\n<table width=\"100%\">\n<tr style=\"text-align: left;\"><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";
	buffer = PerformListing(pathfile);
	if (buffer.empty()) {
		this->error = true;
		this->request.SetError(INTERNAL);
		this->pathfile = "ERROR";
		return ;
	}
	this->buffer = header + buffer + "</table>\n<hr>\n</body>\n</html>";
	ft_logs("buffer = [" + this->buffer + "]");
}

void HeaderResponse::HandleDelete(void) {
	FindPathFile();
	DeleteFile();
	return ;
}

void HeaderResponse::HandlePost(void) {
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
	return ;
}

void HeaderResponse::DeleteFile(void) {
	std::string path = "." + pathfile;
	std::cout << "pathfile à delete = [" << pathfile  << "]" << std::endl;
	if (remove(path.c_str()) == -1) {
		ft_logs("Error: can't delete file; file not found");
		this->request.SetError(NOT_FOUND);
		pathfile = "ERROR";
		this->error = true;
		return;
	}
	ft_logs("File deleted");
	return ;
}

void HeaderResponse::SearchErrorPage(void) {
	struct stat s;
	int errorCode = this->request.GetError();
	std::map<std::vector<int>, std::string>::iterator it = this->config->errorPage.begin();

	for (; it != this->config->errorPage.end(); it++) {
		const std::vector<int>& errorCodesKeys = it->first;
		if (std::find(errorCodesKeys.begin(), errorCodesKeys.end(), errorCode) != errorCodesKeys.end()) {
			pathfile = "." + it->second;
			std::cout << pathfile << std::endl;
			if (access(pathfile.c_str(), F_OK) == 0) {
				if (stat(pathfile.c_str(), &s) == -1 || s.st_mode & S_IFDIR) {
					pathfile.clear();
					continue;
				}
				pathfile = it->second;
				return ;
			}
			pathfile.clear();
		}
	}
	pathfile.clear();
}

void HeaderResponse::FindFileLocation(void) {
	struct LocationConfig*	ptr;
	std::string 			str;
	size_t					pos;

	pos = 0;
	str = this->request.getPairs()["Request-Target:"];
	ptr = this->config->locationConfig;
	if (!ptr) {
		indexLocationConfig = -1;
		return ;
	}
	for (size_t i = 0; i < this->config->numbersOfLocation; i++) {
		if (str.compare(0, ptr[i].location.size(), ptr[i].location) == 0) {
			if (ptr[i].location.size() > pos) {
				pos = ptr[i].location.size();
				indexLocationConfig = i;
			}
		}
	}
	//checkCGI here
	std::cout << "voici str= " << str << std::endl;
	if (str.find(".php") != std::string::npos || str.find(".bash") != std::string::npos) {
		if (!ptr[indexLocationConfig].pathPHPexecutable.empty())
			isCGI = true;
	};
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

	if (this->config->locationConfig) {
		upload_path = this->config->locationConfig[indexLocationConfig].upload_store;
		if (upload_path.empty()) {
			ft_logs("upload path empty");
			this->request.SetError(FORBIDDEN);
			this->error = true;
			return ;
		}
		//extraire boundary depuis Content-type
		std::string contentType = this->request.getPairs()["Content-Type:"];
		std::string	boundary = contentType.substr(contentType.find("boundary=") + 10, contentType.find("\r\n") - contentType.find("boundary=") + 10);
		CreateImage(this->request.GetBody(), boundary, upload_path);
	}
}

void HeaderResponse::CreateImage(const std::string& bufferBody, std::string& boundary, std::string& uploadPath) {
	if (boundary.length() == 0) {
		this->error = true;
		this->request.SetError(BAD_REQUEST);
		this->pathfile = "ERROR";
		return ;
	}
	if (bufferBody.length() > 4) {
		std::string name;
		std::string filename;
		std::string image_png;
		size_t startpos;
		size_t endpos;

		startpos = bufferBody.find("name=") + 6;
		endpos = bufferBody.find(";", startpos);
		name = bufferBody.substr(startpos, endpos - startpos - 1);
		ft_logs(name);
		startpos = bufferBody.find("filename=") + 10;
		endpos = bufferBody.find("\r\n", startpos);
		filename = bufferBody.substr(startpos, endpos - startpos - 1);
		ft_logs(filename);
		startpos = bufferBody.find("Content-Type:") + 14;
		endpos = bufferBody.find("\r\n", startpos);
		image_png = bufferBody.substr(startpos, endpos - startpos);
		ft_logs(image_png);

		startpos = endpos + 4;
		std::string delimiter = "\r\n" + boundary + "--";
		ft_logs(delimiter);
		endpos = bufferBody.find(delimiter, startpos);
		std::string image = bufferBody.substr(startpos, endpos - startpos);

		std::string img_filename = uploadPath + FindFileName();
		img_filename = "." + img_filename;
		ft_logs(img_filename);
		int image_fd = open(img_filename.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IROTH | S_IRUSR | S_IRGRP);
		if (image_fd == -1) {
			this->error = true;
			this->request.SetError(INTERNAL);
			ft_logs("Error while trying to create the new image");
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
		ft_logs("Method not implemented");
		this->request.SetError(NOT_IMPLEMENTED);
		error = true;
		return ;
	}
	//check si le header est pris en charge par la location
	if (this->config->locationConfig) {
		methods = this->config->locationConfig[indexLocationConfig].methods;
		for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
			if (*it == headerMethod)
				methodAccepted = true;
		}
		if (!methodAccepted) {
			if (!this->config->locationConfig[indexLocationConfig].location.empty()) {
				ft_logs("Location moved permanently");
				this->request.SetError(MOVED_PERMANENTLY);
			}
			else {
				ft_logs("Method not allowed by the route");
				this->request.SetError(NOT_ALLOWED);
			}
			error = true;
		}
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
	if (this->config->locationConfig) {
		uri = this->config->locationConfig[indexLocationConfig].location;
		root = this->config->locationConfig[indexLocationConfig].root;
	}
	this->pathfile = root + str;
	ft_logs("pathfile = [" + pathfile + "]");
	return ;
}

std::string HeaderResponse::CheckErrors(void) {
	switch (this->request.GetError())
	{
		case OK:
			return ("");
		case MOVED_PERMANENTLY :
			return (code_301());
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
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\nLocation: " + pathfile + "\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: 23\r\n\r\nImage perfectly created");
}

std::string HeaderResponse::code_204(void) {
	return ("HTTP/1.1 204 No Content\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_301(void) {
	return ("HTTP/1.1 301 Moved Permanently\r\nLocation: " + GetNewLocation() + "\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Length: 0\r\n" + GetConnectionStatut() + "\r\n\r\n");
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

std::string HeaderResponse::default_error_page(void) {
	return ("<!DOCTYPE html>\n<html lang=\"fr\">\n<body>\n<main class=\"panel\">\n<h1>An error occurred.</h1>\n<p>Sorry, the page you are looking for is currently unavailable.<br>\nPlease try again later.</p>\n<p>If you are the system administrator of this resource then you should check\nthe error log for details.</p>\n<p><em>Faithfully yours, lomont.</em></p>\n</main>\n</body>\n</html>");
}

//Getters

std::string HeaderResponse::GetContentType(void) {
	if (this->request.GetMethod() == POST)
		return (TEXT_TYPE);
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

	if (this->config->locationConfig) {
		tmp = this->config->locationConfig[indexLocationConfig].methods;
		for (size_t i = 0; i < tmp.size(); i++) {
			methods += tmp[i];
			if (i + 1 < tmp.size())
				methods += " ";
		}
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

std::string HeaderResponse::GetNewLocation(void) {
	return (this->config->locationConfig[indexLocationConfig]._return.second);
}

//Setters

void HeaderResponse::SetBuffer(std::string str) {
	this->buffer = str;
	return ;
}

void HeaderResponse::SetFileSize(void) {
	struct stat			s;
	std::ostringstream	oss;
	std::string			size;

	if (pathfile.front() != '.')
		pathfile = "." + pathfile;
	if (stat(pathfile.c_str(), &s) == -1) {
			ft_logs("error stat");
			pathfile = "ERROR";
			error = true;
			this->request.SetError(NOT_FOUND);
	}
	if (s.st_mode & S_IFDIR) {
		pathfile = "ERROR";
		this->request.SetError(NOT_FOUND);
		error = true;
	}
	if (pathfile != "ERROR") {
		bodySize = s.st_size;
		oss << s.st_size;
		bodySizePrint = oss.str();
	}
	return ;
}

std::string HeaderResponse::PerformListing(std::string& path) {
	std::string			pathfile;
	std::string			buffer;
	std::string			href;
	std::stringstream	size;
	DIR*				directory;
	std::stringstream	ss;
	struct stat			s_stat;
	struct dirent*		s_dir;

	pathfile = "." + path;
	directory = opendir(pathfile.c_str());
	if (directory == NULL) {
		ft_logs("directory null");
		this->error = true;
		this->request.SetError(NOT_FOUND);
		return ("");
	}
	while ((s_dir = readdir(directory)) != NULL) {
		pathfile = "." + path + s_dir->d_name;
		ft_logs(s_dir->d_name);
		if (stat(pathfile.c_str(), &s_stat) != -1) {
			if (s_stat.st_mode & S_IFDIR) {
				pathfile = s_dir->d_name + std::string("/");
				size << "-";
				href = s_dir->d_name + std::string("/");
			}
			else if (s_stat.st_mode & S_IFREG) {
				size << s_stat.st_size;
				href = s_dir->d_name;
			}
			ss << "<tr><td style=\"text-align: left;\"><a href= "<< href << ">" << s_dir->d_name << "</a></td><td style=\"text-align: left;\">" << ctime(&s_stat.st_mtime) << "</td><td style=\"text-align: left;\">" << size.str() << "</td></tr>";
			size.str("");
		}
	}
	closedir(directory);
	buffer = ss.str();
	return (buffer);
}

//CGI

std::string HeaderResponse::get_exec(std::string path)
{
	std::string line;

	if (*path.begin() != '.')
		path = "." + path;
	std::ifstream file (path.c_str());
	std::cout << "path avant ouverture = " << path.c_str() << std::endl;
	if (!file.is_open())
	{
		ft_logs("can't open script");
		return ("");
	}

	std::getline(file, line);
	if (line[0] != '#' || line[1] != '!')
		return ("");
	else
	{
		line.erase(0,2);
		size_t pos = line.find_first_not_of(" \r\t");
		if (pos != std::string::npos)
			line = line.substr(pos);
	}
	return (line);
}

std::string HeaderResponse::get_query_string(std::string uri)
{
	size_t pos = uri.find("?");
	if (pos != std::string::npos)
		return (uri.substr(pos + 1));
	return ("");
}

char** HeaderResponse::create_env(HeaderRequest& request, std::string& path)
{
	char**	envp;
	std::ostringstream	oss;
	std::string tmp_env[7] = {"REQUEST_METHOD=", "QUERY_STRING=", "SCRIPT_NAME=", "CONTENT_LENGTH=", "CONTENT_TYPE=", "SCRIPT_FILENAME=", "REDIRECT_STATUS=200"};

	tmp_env[0].append(request.getPairs()["Method:"]);
	tmp_env[1].append(get_query_string(request.getPairs()["Request-Target:"]));
	if (path.find("?") != std::string::npos)
		path.resize(path.find("?"));
	tmp_env[2].append(request.getPairs()["Request-Target:"]);
	oss << request.GetBody().length();
	tmp_env[3].append(oss.str());
	tmp_env[4].append(request.getPairs()["Content-Type:"]);
	if (path.find_last_of("/") != std::string::npos)
		tmp_env[5].append(path.substr(path.find_last_of("/"), path.size() - pathfile.find_last_of("/")));
	else
		tmp_env[5].append(path);

	envp = new char*[7 + 1];
	if (!envp)
		exit(1);
	for (int i = 0; i < 7; i++)
		envp[i] = strdup(tmp_env[i].c_str());
	envp[7] = NULL;
	return (envp);
}

char** HeaderResponse::create_args(char* path)
{
	char **args;
	std::string tmp_args[2];
	std::string script_path(path);

	script_path = "." + script_path.substr(script_path.find("=") + 1, script_path.length() - script_path.find("="));
	tmp_args[0] = get_exec(this->pathfile);
	if (tmp_args[0].empty())
		return (NULL);
	tmp_args[1] = script_path;
	args = new char*[2 + 1];
	for (int i = 0; i < 2; i++)
		args[i] = strdup(tmp_args[i].c_str());
	args[2] = NULL;
	return (args);
}

std::string HeaderResponse::PerformCGI()
{
	int			pipe_in[2];
	int			pipe_out[2];
	pid_t		pid;
	std::string buffer;
	char**		envp;
	char**		args;
	bool		post;

	if (this->request.GetMethod() == POST)
		post = true;
	else
		post = false;
	envp = create_env(this->request, this->pathfile);
	args = create_args(envp[5]);
	if (pipe(pipe_out) == -1)
		return ("");
	this->pathfile = "." + this->pathfile;
	std::cout << "voici le path avant le fork = " << this->pathfile << std::endl;
	if (post) {
		if (pipe(pipe_in) == -1)
			return ("");
	}
    pid = fork();
    if (pid == 0)
    {
		if (post) {
			dup2(pipe_in[0], STDIN_FILENO);
			close(pipe_in[1]);
		}

        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[0]);
		pathfile.resize(pathfile.find_last_of("/"));
        chdir(this->pathfile.c_str());
        if (execve(args[0], args, envp) == -1)
			//free if necessary
            exit(1);
    }
    else
    {
		if (post) {
			write(pipe_in[1], this->request.GetBody().c_str(), this->request.GetBody().size());
			close(pipe_in[0]);
			close(pipe_in[1]);
		}
		close(pipe_out[1]);
		fcntl(pipe_out[0], F_SETFL | O_NONBLOCK);
		buffer = read_cgi_output_with_timeout(pipe_out[0], pid, 20);
    }
	delete[] envp;
	delete[] args;
	close(pipe_out[0]);
	ft_logs(buffer);
	return (buffer);
}

bool HeaderResponse::is_timeout(const timeval& start, int sec_limit)
{
    timeval now;

    gettimeofday(&now, NULL);
    return (now.tv_sec - start.tv_sec > sec_limit);
}

std::string HeaderResponse::read_cgi_output_with_timeout(int fd, pid_t pid, int timeout_sec)
{
    char		buf[4096];
    ssize_t		n;
    timeval		start;
    std::string buffer;

    gettimeofday(&start, NULL);
    while (1)
    {
        n = read(fd, buf, sizeof(buf));
        if (n > 0)
            buffer.append(buf, n);
        else if (n == -1 && errno != EAGAIN)
            return ("");

        int ret = waitpid(pid, NULL, WNOHANG);
        if (ret == pid)
        {
            while ((n = read(fd, buf, sizeof(buf))) > 0)
                buffer.append(buf, n);
            return (buffer);
        }

        if (is_timeout(start, timeout_sec))
        {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            ft_logs("CGI script killed due to timeout");
            return ("");
        }
        usleep(10000);
    }
}

//3. Parsing des headers retournés par le script (dans HandleCGI)
//
//Le serveur coupe brutalement au premier \r\n\r\n. Or, un script CGI renvoie ses propres headers (ex: Content-Type, Status, Set-Cookie).
//Vous devez extraire ces headers pour les intégrer à votre objet de réponse HTTP finale au lieu de les ignorer. Il faut également gérer le cas où le script ne renvoie pas de \r\n\r\n (erreur d'exécution).


