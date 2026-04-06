/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headerRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:00:02 by lomont            #+#    #+#             */
/*   Updated: 2026/04/06 15:55:56 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERREQUEST_HPP
#define HEADERREQUEST_HPP

#define UPLOAD_FOLDER "/www/uploads/" //TODO create default uploads folder?

#include <map>
#include <string>

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
		bool												_delete;
		bool												authorized;
		Error												error;
		Method												method;
		std::string											body;
		std::map<std::string, std::string>					headerPair;
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
		bool								GetDeleteSocket(void);
		bool								GetAuthorized(void);
		Error								GetError(void);
		Method								GetMethod(void);
		std::string							GetBody(void);
		std::map<std::string, std::string>& getPairs(void);

		//setters
		void								SetMethod(std::string&);
		void								SetError(Error err);
		void								SetBody(std::string str);
};

#endif
