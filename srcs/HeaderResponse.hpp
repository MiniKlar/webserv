/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:32 by lomont            #+#    #+#             */
/*   Updated: 2026/03/25 23:55:00 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERRESPONSE_HPP
#define HEADERRESPONSE_HPP

#define HTML_TYPE "text/html; charset=UTF-8"
#define IMG_TYPE "multipart/form-data"
#define DEFAULT_ERROR_PAGE "/www/default_error.html"
#define DEFAULT_UPLOAD_PATH "/www/uploads/"
#define	FILE_LOCATION "/www"

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
		std::string		pathfile;
		size_t			indexLocationConfig;
		off_t			bodySize;
		bool			error;
		bool			parsed;
		bool			cookie;
	public:

		//Canonical form
		HeaderResponse(void);
		HeaderResponse(const HeaderResponse&);
		HeaderResponse(HeaderRequest&, struct config*);
		HeaderResponse& operator=(const HeaderResponse&);
		~HeaderResponse(void);

		//Member functions
		void			HandleCGI(void);
		void			HandleGet(void);
		void			HandlePost(void);
		void			HandleDelete(void);
		void			CreateResponse(void);
		void			OpenFile(void);
		void			CleanHeader(void);
		void			FindPathFile(void);
		void			FindFileLocation(void);
		void			SearchErrorPage(void);
		void			DeleteFile(void);
		void			CheckMethod(void);
		void			ParseBody(void);
		void 			CreateImage(const std::string&, std::string&, std::string&);
		std::string		FindFileName(void);
		std::string		PerformCGI(void);
		std::string		CheckErrors(void);

		//Setters
		void			SetFileSize(void);
		void			SetBuffer(std::string);

		//Getters
		bool			IsParsed(void);
		void			GetHeaderResponse(void);
		std::string		GetBuffer(void);
		std::string		GetMethodAllowed(void);
		std::string 	GetContentType(void);
		std::string 	GetConnectionStatut(void);
		std::string		GetCurrentTime(void);
		std::string		GetMaxBodySize(void);

		//Response Header
		std::string 	code_200(void);
		std::string 	code_201(void);
		std::string 	code_204(void);
		std::string 	code_303(void);
		std::string 	code_400(void);
		std::string 	code_403(void);
		std::string 	code_404(void);
		std::string 	code_405(void);
		std::string 	code_411(void);
		std::string 	code_413(void);
		std::string 	code_500(void);
		std::string 	code_501(void);
		std::string 	code_505(void);
};

#endif
