/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headerResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:32 by lomont            #+#    #+#             */
/*   Updated: 2026/04/15 02:11:46 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERRESPONSE_HPP
#define HEADERRESPONSE_HPP

#define HTML_TYPE "text/html; charset=UTF-8"
#define TEXT_TYPE "text/plain"
#define DEFAULT_ERROR_PAGE "./www/default_error.html"
#define DEFAULT_UPLOAD_PATH "/www/uploads/"
#define	FILE_LOCATION "/www"

#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <algorithm>
#include <cstdlib>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <map>
#include "headerRequest.hpp"

struct	config;
class	HeaderRequest;
class	server;

class HeaderResponse
{
	private:
		bool			cookie;
		bool			error;
		bool			parsed;
		bool			isCGI;
		off_t			bodySize;
		size_t			indexLocationConfig;
		std::string		cgi_format;
		std::string		header;
		std::string		buffer;
		std::string		bodySizePrint;
		std::string		pathfile;
		struct config	*config;
		server*			server_instance;
		HeaderRequest	request;
	public:

		//Canonical form
		HeaderResponse(void);
		HeaderResponse(const HeaderResponse&);
		HeaderResponse(HeaderRequest&, struct config*, server*);
		HeaderResponse& operator=(const HeaderResponse&);
		~HeaderResponse(void);

		//Member functions
		void			HandleCGI(void);
		void			HandleGet(void);
		void			HandleAutoIndex(void);
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
		void			FindCGIPathFile(void);
		void			MovedLocation(void);
		void			ParseBody(void);
		void 			CreateImage(const std::string&, std::string&, std::string&);
		bool			is_timeout(const timeval&, int);
		char**			create_env(HeaderRequest&);
		char**			create_args(char*);
		std::string		get_query_string(std::string);
		std::string		get_exec(std::string);
		std::string		FindFileName(void);
		std::string		PerformCGI(void);
		std::string		PerformListing(std::string&);
		std::string		CheckErrors(void);
		std::string		read_cgi_output_with_timeout(int, pid_t, int);

		//Setters
		void			SetFileSize(void);
		void			SetBuffer(std::string);
		void			SetResponseError(Error);

		//Getters
		bool			IsParsed(void);
		void			GetHeaderResponse(void);
		std::string		GetBuffer(void);
		std::string		GetMethodAllowed(void);
		std::string 	GetContentType(void);
		std::string 	GetConnectionStatut(void);
		std::string		GetCurrentTime(void);
		std::string		GetMaxBodySize(void);
		std::string		GetNewLocation(void);

		//Response Header
		std::string 	code_200(void);
		std::string 	code_201(void);
		std::string 	code_204(void);
		std::string 	code_301(void);
		std::string 	code_400(void);
		std::string 	code_403(void);
		std::string 	code_404(void);
		std::string 	code_405(void);
		std::string 	code_411(void);
		std::string 	code_413(void);
		std::string 	code_500(void);
		std::string 	code_501(void);
		std::string 	code_505(void);
		std::string		default_error_page(Error);
};

#endif
