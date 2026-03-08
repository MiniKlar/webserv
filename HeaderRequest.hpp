/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:00:02 by lomont            #+#    #+#             */
/*   Updated: 2026/03/08 02:57:32 by lomont           ###   ########.fr       */
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

enum Method {
	GET,
	POST,
	DELETE,
	HEAD,
	OTHER
};

class HeaderRequest
{
	private:
		std::map<std::string, std::string>					Map;
		std::string 										_startLine;
		std::string 										_requestTarget;
		std::string											pathFileCreated;
		std::string											method;
		Method												_method;
		const char*											_body;
	public:
		HeaderRequest(void);
		HeaderRequest(std::string);
		HeaderRequest(const HeaderRequest&);
		HeaderRequest& operator=(const HeaderRequest&);
		~HeaderRequest(void);
		void ParseHeaderRequest(std::string&);
		void ParseStartLine(void);
		void printDebug(void);
		void CreateImage(std::string&, std::string&);
		std::string FindFileName(void);

		//getters
		std::map<std::string, std::string>& getPairs(void);
		std::string& GetRequestTarget(void);
		std::string& GetPathImageCreated(void);
		Method	GetMethod(void);

		//setters
		void	SetBody(const std::string);
};

#endif
