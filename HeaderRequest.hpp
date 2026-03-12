/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:00:02 by lomont            #+#    #+#             */
/*   Updated: 2026/03/12 00:17:47 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERREQUEST_HPP
#define HEADERREQUEST_HPP

#define UPLOAD_FOLDER "./www/uploads/"

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
	BAD_REQUEST = 400,
	NOT_FOUND = 404,
	NOT_ALLOWED = 405,
	LENGTH = 411,
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

class HeaderRequest
{
	private:
		Method												method;
		std::map<std::string, std::string>					headerPair;
		const char*											body;
		bool												_delete;
		Error												error;
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
		void printDebug(void);
		void CreateImage(std::string&, std::string&);
		void			CleanHeader(void);
		std::string FindFileName(void);

		//getters
		std::map<std::string, std::string>& getPairs(void);
		Method								GetMethod(void);
		Error								GetError(void);
		bool								GetDeleteSocket(void);

		//setters
		void	SetBody(const std::string);
		void	SetMethod(std::string&);
		void	SetError(Error err);
};

#endif
