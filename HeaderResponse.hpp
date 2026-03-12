/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:32 by lomont            #+#    #+#             */
/*   Updated: 2026/03/12 01:02:54 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERRESPONSE_HPP
#define HEADERRESPONSE_HPP

#define HTML_TYPE "text/html; charset=UTF-8"
#define DEFAULT_ERROR_PAGE "./www/default_error.html"
#define DEFAULT_UPLOAD_PATH "./www/uploads/"
#define	FILE_LOCATION "./www"

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

struct config;

class HeaderResponse
{
	private:
		HeaderRequest	request;
		struct config	*config;
		std::string		header;
		std::string		buffer;
		std::string		bodySizePrint;
		off_t			bodySize;
		std::string		pathfile;
		size_t			indexLocationConfig;
		bool			error;
		bool			parsed;
		// std::map<std::string, std::string>	Map;
		// off_t			bufferSize;
		// std::string		stringFileSize;
		// std::string		content;
		// bool			empty;
		// bool			deleteSocket;
	public:
		HeaderResponse(void);
		HeaderResponse(const HeaderResponse&);
		HeaderResponse(HeaderRequest&, struct config*);
		HeaderResponse& operator=(const HeaderResponse&);
		~HeaderResponse(void);
		//char*		SearchFileRequested(std::string&);

		//void		CheckConnectionStatut(void);
		void			GetHeaderResponse(void);

		std::string		CheckErrors(void);
		void			CheckMethod(void);
		void			OpenBodyFile(void);

		void			CleanHeader(void);

		//bool			IsEmpty(void);
		void			FindPath(void);
		void			FindLocation(void);

		//Setters
		void			SetBodySize(void);

		//Getters
		std::string		GetBuffer(void);
		std::string		GetMethodAllowed(void);
		std::string		getCurrentTime(void);
		bool			IsParsed(void);

		std::string 	getLocation(void);
		off_t& 			GetFileSize(void);
		void			getContentLength(std::string&); //setter in reality
		std::string 	getContent(void);
		std::string 	getConnectionStatut(void);

		//Response Header
		std::string 	code_200(void);
		std::string 	code_201(void);
		std::string 	code_204(void);
		std::string 	code_303(void);
		std::string 	code_400(void);
		std::string 	code_404(void);
		std::string 	code_405(void);
		std::string 	code_411(void);
		std::string 	code_500(void);
		std::string 	code_501(void);
		std::string 	code_505(void);
};

#endif
