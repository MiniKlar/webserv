/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:32 by lomont            #+#    #+#             */
/*   Updated: 2026/03/03 00:27:27 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERRESPONSE_HPP
#define HEADERRESPONSE_HPP

#define PAGE_404 "./html/error.html"
#define UPLOAD_LOCATION "./html/drive/"
#define	FILE_LOCATION "./html"

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <cstdio>
#include <map>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include "HeaderRequest.hpp"
#include "utils.hpp"

enum	Code {
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	NOT_FOUND = 404
};

class HeaderResponse
{
	private:
		HeaderRequest	request;
		std::map<std::string, std::string>	Map;
		off_t			fileSize;
		std::string		stringFileSize;
		std::string		content;
		enum Code		code;
	public:
		HeaderResponse(void);
		HeaderResponse(const HeaderResponse& other);
		HeaderResponse(HeaderRequest&);
		HeaderResponse& operator=(const HeaderResponse& other);
		~HeaderResponse(void);
		char*		SearchFileRequested(std::string&);
		std::string getCurrentTime(void);
		std::string code_200(void);
		std::string code_201(void);
		std::string code_204(void);
		std::string code_404(void);

		//getters
		void		getContentLength(std::string&); //setter in reality
		std::string getContent(void);
		std::string getConnectionStatut(void);
		std::string getLocation(void);
		off_t& GetFileSize(void);
		Code GetCode(void);
};

#endif
