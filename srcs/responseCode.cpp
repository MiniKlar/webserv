/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responseCode.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 00:52:37 by lomont            #+#    #+#             */
/*   Updated: 2026/04/08 00:51:07 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerResponse.hpp"

std::string HeaderResponse::code_200( void ) {
	return ("HTTP/1.1 200 OK\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n");
}

std::string HeaderResponse::code_201(void) {
	return ("HTTP/1.1 201 Created\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\nLocation: " + pathfile + "\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: 0\r\n\r\n");
}

std::string HeaderResponse::code_204(void) {
	return ("HTTP/1.1 204 No Content\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\n" + GetConnectionStatut() + "\r\n");
}

std::string HeaderResponse::code_301(void) {
	return ("HTTP/1.1 301 Moved Permanently\r\nLocation: " + GetNewLocation() + "\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Length: 0\r\n" + GetConnectionStatut() + "\r\n");
}

std::string HeaderResponse::code_400(void) {
	return ("HTTP/1.1 400 Bad Request\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n");
}

std::string HeaderResponse::code_403(void) {
	return ("HTTP/1.1 403 Forbidden\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n");
}

std::string HeaderResponse::code_404(void) {
	return ("HTTP/1.1 404 Not Found\r\nDate: "
	+ this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: "+ GetContentType() +"\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n");
}

std::string HeaderResponse::code_405(void) {
	return ("HTTP/1.1 405 Method Not Allowed\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Length: 0\r\nAllow: " + GetMethodAllowed() + "\r\nConnection: closed\r\n\r\n");
}

std::string HeaderResponse::code_411(void) {
	return ("HTTP/1.1 411 Length Required\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + GetConnectionStatut() + "\r\n");
}

std::string HeaderResponse::code_413(void) {
	return ("HTTP/1.1 413 Content Too Large\r\nDate: " + this->GetCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: "
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

std::string HeaderResponse::default_error_page(Error err) {
	std::ostringstream	html;
	std::string			message;

	switch (err)
	{
		case 400:
			message = "Bad Request";
			break;
		case 403:
			message = "Forbidden";
			break;
		case 404:
			message = "Not Found";
			break;
		case 405:
			message = "Method Not Allowed";
			break;
		case 411:
			message = "Length Required";
			break;
		case 413:
			message = "Content Too Large";
			break;
		case 500:
			message = "Internal Server Error";
			break;
		case 501:
			message = "Not Implemented";
			break;
		case 505:
			message = "HTTP Version Not Supported";
			break;
		default:
			message = "Error";
			break;
		}

		html << "<!DOCTYPE html>\n<html lang=\"fr\">\n<head><title>Erreur " << err << "</title>\n"
			<< "<style>body{background:#050510;color:#e2e8f0;font-family:sans-serif;margin:0;min-height:100vh;display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;}</style>\n"
			<< "</head>\n<body>\n"
			<< "<h1>Houston, we have a problem.</h1>\n"
			<< "<h2>" << err << " - " << message << "</h2>\n"
			<< "<p>The page you are looking for has been lost in space.</p>\n"
			<< "</body>\n</html>";

	return (html.str());
}
