/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:00:02 by lomont            #+#    #+#             */
/*   Updated: 2026/03/29 18:42:19 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERREQUEST_HPP
#define HEADERREQUEST_HPP

#define UPLOAD_FOLDER "/www/uploads/"

#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include "utils.hpp"

enum Error {
	OK = 0,
	MOVED_PERMANENTLY = 301,
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	NOT_ALLOWED = 405,
	LENGTH = 411,
	BODY_TOO_LARGE = 413,
	INTERNAL = 500,
	NOT_IMPLEMENTED = 501,
	VERSION = 505
};

enum Method {
	GET,
	POST,
	DELETE,
	OTHER
};

struct config;

class HeaderRequest
{
	private:
		Method												method;
		std::map<std::string, std::string>					headerPair;
		std::string											body;
		bool												_delete;
		Error												error;
		bool												isCGI;
		bool												authorized;
	public:
		//constructors
		HeaderRequest();
		HeaderRequest(Error err);
		HeaderRequest(std::string);

		//Copy constructor & assignment operator
		HeaderRequest(const HeaderRequest&);
		HeaderRequest& operator=(const HeaderRequest&);

		//Destructor
		~HeaderRequest(void);

		//Member functions
		void ParseHeaderRequest(std::string&);
		void ParseFirstLine(std::string&);
		void			CleanHeader(void);

		//getters
		std::map<std::string, std::string>& getPairs(void);
		Method								GetMethod(void);
		Error								GetError(void);
		bool								GetDeleteSocket(void);
		std::string							GetBody(void);
		bool								GetIsCGI(void);
		bool								GetAuthorized(void);

		//setters
		void								SetMethod(std::string&);
		void								SetError(Error err);
		void								SetBody(std::string str);
};

#endif
