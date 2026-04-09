/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_delete_method.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 22:10:55 by lomont            #+#    #+#             */
/*   Updated: 2026/04/09 22:11:23 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "headerResponse.hpp"

void HeaderResponse::HandleDelete(void) {
	FindPathFile();
	DeleteFile();
}

void HeaderResponse::DeleteFile(void) {
	std::string path = "." + pathfile;
	if (remove(path.c_str()) == -1) {
		ft_warning("Error: can't delete file; file not found");
		this->request.SetError(NOT_FOUND);
		pathfile = "ERROR";
		this->error = true;
		return;
	}
	ft_logs("File deleted");
}