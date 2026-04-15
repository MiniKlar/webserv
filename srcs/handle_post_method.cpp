/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_post_method.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 22:12:09 by lomont            #+#    #+#             */
/*   Updated: 2026/04/15 01:53:08 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "headerResponse.hpp"

void HeaderResponse::HandlePost(void) {
	if (this->request.GetAuthorized() == false) {
		this->error = true;
		this->request.SetError(FORBIDDEN);
		pathfile = "ERROR";
		return ;
	}
	else if (this->request.GetError() != OK) {
		this->error = true;
		return ;
	}
	ParseBody();
}

void HeaderResponse::ParseBody() {
	if (this->config->locationConfig) {
		if (this->request.getPairs()["Content-Length:"] == "0") {
			this->buffer = "";
			return ;
		}
		std::string upload_path = this->config->locationConfig[indexLocationConfig].upload_store;
		if (upload_path.empty()) {
			ft_warning("Upload path empty");
			this->request.SetError(FORBIDDEN);
			this->error = true;
			pathfile = "ERROR";
			return ;
		}
		//extraire boundary depuis Content-type
		std::string contentType = this->request.getPairs()["Content-Type:"];
		std::string	boundary = contentType.substr(contentType.find("boundary=") + 10, contentType.find("\r\n") - contentType.find("boundary=") + 10);
		CreateImage(this->request.GetBody(), boundary, upload_path);
	}
}

void HeaderResponse::CreateImage(const std::string& bufferBody, std::string& boundary, std::string& uploadPath) {
	if (boundary.length() == 0) {
		this->error = true;
		this->request.SetError(BAD_REQUEST);
		this->pathfile = "ERROR";
		return ;
	}
	if (bufferBody.length() > 4) {
		size_t startpos = bufferBody.find("name=") + 6;
		size_t endpos = bufferBody.find(";", startpos);
		std::string name = bufferBody.substr(startpos, endpos - startpos - 1);

		startpos = bufferBody.find("filename=") + 10;
		endpos = bufferBody.find("\r\n", startpos);
		std::string filename = bufferBody.substr(startpos, endpos - startpos - 1);
        
		startpos = bufferBody.find("Content-Type:") + 14;
		endpos = bufferBody.find("\r\n", startpos);
		std::string image_png = bufferBody.substr(startpos, endpos - startpos);

		startpos = endpos + 4;
		std::string delimiter = "\r\n" + boundary + "--";
		endpos = bufferBody.find(delimiter, startpos);
		std::string image = bufferBody.substr(startpos, endpos - startpos);

		std::string img_filename = uploadPath + FindFileName();
		img_filename = "." + img_filename;
		ft_logs(img_filename);
		int image_fd = open(img_filename.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IROTH | S_IRUSR | S_IRGRP);
		if (image_fd == -1) {
			this->error = true;
			this->request.SetError(INTERNAL);
			ft_logs("Error while trying to create the new image");
			return ;
		}
		write(image_fd, image.data(), image.size());
		close(image_fd);
		std::stringstream oss;
		oss << image.size();
		bodySizePrint = oss.str();
		this->pathfile = img_filename;
		this->pathfile.erase(pathfile.begin());
		this->buffer = image;
	};
}