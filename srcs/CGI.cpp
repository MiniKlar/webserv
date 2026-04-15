/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 23:24:12 by lomont            #+#    #+#             */
/*   Updated: 2026/04/15 02:08:18 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerResponse.hpp"
#include "utils.hpp"
#include "webserv.hpp"

static void free_envp(char**);
static void free_args(char**);
static void free_all(char **, char **);

void HeaderResponse::HandleCGI(void) {
	FindCGIPathFile();
	std::string buffer = PerformCGI();
	ft_logs(buffer);
	if (buffer.empty()) {
		ft_logs("buffer empty");
		this->error = true;
		this->request.SetError(INTERNAL);
		pathfile = "ERROR";
		return ;
	}
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos) {
		ft_logs("no header found empty");
		this->error = true;
		this->request.SetError(INTERNAL);
		pathfile = "ERROR";
		return ;
	}
	pos += 4;

	std::string	cgi_headers = buffer.substr(0, pos);
	this->buffer = buffer.substr(pos);

	this->bodySize = this->buffer.size();
	std::ostringstream oss;
	oss << bodySize;
	this->bodySizePrint = oss.str();

	std::string header;
	if (size_t pos = cgi_headers.find("Status:") != std::string::npos) {
		pos += 7;
		this->header = "HTTP/1.1 " + cgi_headers.substr(pos, cgi_headers.find("\r\n", pos) + 2 - pos);
		std::cout << "this->header = " << this->header << std::endl;
	}
	else
		this->header = "HTTP/1.1 200 OK\r\n";
    this->header += "Date: " + this->GetCurrentTime() + "\r\n";
    this->header += "Server: Webserv\r\n";
    this->header += "Content-Length: " + this->bodySizePrint + "\r\n";
	this->header += cgi_headers;
	parsed = true;
}

std::string HeaderResponse::get_exec(std::string path) {
	if (*path.begin() != '.')
		path = "." + path;
	std::ifstream	file;
	file.open(path.c_str());
	if (!file.is_open()) {
		ft_warning("Can't open script");
		return "";
	}
	std::string line;
	std::getline(file, line);
	if (line[0] != '#' && line[1] != '!' && line.find("<?php") == std::string::npos)
		return "";
	else {
		if (line.find("<?php") != std::string::npos)
			line = "/usr/bin/php";
		else {
			line.erase(0,2);
			size_t pos = line.find_first_not_of(" \r\t");
			if (pos != std::string::npos)
				line = line.substr(pos);
		}
	}
	return line;
}

std::string HeaderResponse::get_query_string(std::string uri) {
	size_t pos = uri.find("?");
	if (pos != std::string::npos)
		return uri.substr(pos + 1);
	return "";
}

char** HeaderResponse::create_env(HeaderRequest& request) {
	std::vector<std::string> tmp_env;
	LocationConfig conf = this->config->locationConfig[indexLocationConfig];
	tmp_env.push_back("REQUEST_METHOD=" + request.getPairs()["Method:"]);
	tmp_env.push_back("SCRIPT_NAME=" + this->pathfile.substr(conf.location.length() + conf.root.length() + 1, this->pathfile.length() - conf.root.length() - conf.location.length()));
	tmp_env.push_back("PATH_INFO=" + request.getPairs()["Request-Target:"].substr(request.getPairs()["Request-Target:"].find(this->cgi_format) + this->cgi_format.length()));
	tmp_env.push_back("QUERY_STRING=" + get_query_string(request.getPairs()["Request-Target:"]));
	std::ostringstream	oss;
	oss << request.GetBody().length();
	tmp_env.push_back("CONTENT_LENGTH=" + oss.str());
	tmp_env.push_back("CONTENT_TYPE=" + request.getPairs()["Content-Type:"]);
	std::string to_push_in_env;
	std::string key;
	for (std::map<std::string, std::string>::iterator it = this->request.getPairs().begin(); it != this->request.getPairs().end(); it++) {
		if (it->first.find("Method:") || it->first.find("Request-Target:")) {
			to_push_in_env.append("HTTP_");
			key = it->first;
			for (int i = 0; key[i]; i++) {
				if (key[i] == '-')
					key[i] = '_';
				else if (islower(key[i]))
					key[i] = toupper(key[i]);
			}
			key.resize(key.length() - 1);
			to_push_in_env.append(key + "=");
			key.clear();
			to_push_in_env.append(it->second);
			tmp_env.push_back(to_push_in_env);
			to_push_in_env.clear();
		}
	}
	size_t size_vector = tmp_env.size();
	//allouer tableau de char
	char** envp = new char*[size_vector + 1];
	if (!envp)
		return NULL;
	for (size_t i = 0; i < size_vector; i++) {
		envp[i] = strdup(tmp_env[i].c_str());
		if (!envp[i]) {
			free_envp(envp);
			return NULL;
		}
	}
	envp[size_vector] = NULL;
	return (envp);
}

char** HeaderResponse::create_args(char* script_name)
{
	std::string tmp_args[2];

	tmp_args[0] = get_exec(this->pathfile);
	if (tmp_args[0].empty())
		return NULL;
	std::string script_path(script_name);
	script_path = "./" + script_path.substr(script_path.find("=") + 1, script_path.length() - script_path.find("="));
	tmp_args[1] = script_path;
	char** args = new char*[2 + 1];
	if (!args)
		return NULL;
	for (int i = 0; i < 2; i++) {
		args[i] = strdup(tmp_args[i].c_str());
		if (!args[i]) {
			free_args(args);
			return NULL;
		}
	}
	args[2] = NULL;
	return args;
}

std::string HeaderResponse::PerformCGI(void)
{
	bool is_post = false;
	if (this->request.GetMethod() == POST)
		is_post = true;
	char** envp = create_env(this->request);
	if (!envp) {
		SetResponseError(INTERNAL);
		return "";
	}
	char** args = create_args(envp[1]);
	if (!args) {
		free_envp(envp);
		SetResponseError(INTERNAL);
		return "";
	}

	int	pipe_in[2];
	int	pipe_out[2];
	if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1) {
		free_all(envp, args);
		SetResponseError(INTERNAL);
		return "";
	}
	if (!is_post) {
		close(pipe_in[0]);
		close(pipe_in[1]);
	}
	this->pathfile = "." + this->pathfile;
	std::string	buffer;
    pid_t pid = fork();
    if (pid == 0)
    {
		if (is_post) {
			dup2(pipe_in[0], STDIN_FILENO);
			close(pipe_in[1]);
		}

        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[0]);
		pathfile.resize(pathfile.find_last_of("/"));
        chdir(this->pathfile.c_str());
        if (execve(args[0], args, envp) == -1) {
			free_all(envp, args);
			close(pipe_in[0]);
			close(pipe_out[1]);
			delete this->server_instance;
			exit(1);
		}
    }
    else
    {
		if (is_post) {
			write(pipe_in[1], this->request.GetBody().c_str(), this->request.GetBody().size());
			close(pipe_in[0]);
			close(pipe_in[1]);
		}
		close(pipe_out[1]);
		fcntl(pipe_out[0], F_SETFL | O_NONBLOCK);
		buffer = read_cgi_output_with_timeout(pipe_out[0], pid, 4);
    }
	free_all(envp, args);
	close(pipe_out[0]);
	return buffer;
}

bool HeaderResponse::is_timeout(const timeval& start, int sec_limit)
{
    timeval	now;

    gettimeofday(&now, NULL);
    return now.tv_sec - start.tv_sec > sec_limit;
}

std::string HeaderResponse::read_cgi_output_with_timeout(int fd, pid_t pid, int timeout_sec)
{
	int			wstatus;
    char		buf[4096];
    timeval		start;
    std::string buffer;

    gettimeofday(&start, NULL);
    while (1)
    {
		ssize_t n;

        int ret = waitpid(pid, &wstatus, WNOHANG);

        if (ret == pid)
        {
			if (WIFSIGNALED(wstatus)) {
				std::cout << "tu return ici" << std::endl;
				return "";
			}
            while ((n = read(fd, buf, sizeof(buf))) > 0)
                buffer.append(buf, n);
            return buffer;
        }

        if (is_timeout(start, timeout_sec))
        {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            ft_warning("CGI script killed due to timeout");
            return "";
        }
        usleep(10000);
    }
}

void HeaderResponse::FindCGIPathFile(void) {
	std::string request_target = this->request.getPairs()["Request-Target:"];
	std::map<std::string, std::string> cgi_extension = this->config->locationConfig[indexLocationConfig].cgi_handlers;

	for (std::map<std::string, std::string>::iterator it = cgi_extension.begin(); it != cgi_extension.end(); it++) {
		size_t pos_extension = request_target.find(it->first);
		if (pos_extension != std::string::npos) {
				this->pathfile = request_target.substr(0, it->first.length() + pos_extension);
				this->cgi_format = it->first;
		}
	}
	this->pathfile = this->config->locationConfig[indexLocationConfig].root + this->pathfile;
}

static void free_all(char **envp, char **args)
{
	free_args(args);
	free_envp(envp);
}

static void free_envp(char** envp) {
	for (int i = 0; envp[i]; i++)
		free (envp[i]);
	delete[] envp;
}

static void free_args(char** args) {
	for (int i = 0; args[i]; i++)
		free (args[i]);
	delete[] args;
}
