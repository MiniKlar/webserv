/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:32 by lomont            #+#    #+#             */
/*   Updated: 2026/02/21 04:09:55 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef HEADERRESPONSE_HPP
#define HEADERRESPONSE_HPP

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <cstdio>

#include "HeaderRequest.hpp"

class HeaderResponse
{
	private:
		HeaderRequest& _request;
	public:
		HeaderResponse(HeaderRequest& _request);
		~HeaderResponse(void);
		std::string getCurrentTime(void);
		std::string code_200(void);
		std::string getContentLength(void);
		std::string getConnectionStatut(void);
};

#endif
