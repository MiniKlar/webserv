/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:00:02 by lomont            #+#    #+#             */
/*   Updated: 2026/02/21 04:11:56 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef HEADERREQUEST_HPP
#define HEADERREQUEST_HPP

#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>

class HeaderRequest
{
	private:
		std::vector<std::pair<std::string, std::string> >	Map;
		std::string 										_startLine;
		std::string 										_method;
		std::string 										_requestTarget;
		std::string 										_body;
	public:
		HeaderRequest(char* buffer, ssize_t bytes_read);
		~HeaderRequest(void);
		void ParseHeaderRequest(std::string& _header);
		void ParseStartLine(void);
		void printDebug(void);
		std::vector<std::pair<std::string, std::string> >& getPairs(void);
};

#endif
