/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 23:24:12 by lomont            #+#    #+#             */
/*   Updated: 2026/04/08 01:19:27 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerResponse.hpp"
#include "utils.hpp"

static void free_envp(char**);
static void free_args(char**);

std::string HeaderResponse::get_exec(std::string path) {
	std::string		line;
	std::ifstream	file;

	if (*path.begin() != '.')
		path = "." + path;
	file.open(path.c_str());
	if (!file.is_open()) {
		ft_warning("Can't open script");
		return "";
	}
	std::getline(file, line);
	if (line[0] != '#' || line[1] != '!')
		return "";
	else {
		line.erase(0,2);
		size_t pos = line.find_first_not_of(" \r\t");
		if (pos != std::string::npos)
			line = line.substr(pos);
	}
	return line;
}

std::string HeaderResponse::get_query_string(std::string uri) {
	size_t pos = uri.find("?");
	if (pos != std::string::npos)
		return uri.substr(pos + 1);
	return "";
}

char** HeaderResponse::create_env(HeaderRequest& request, std::string& path) {
	char**				envp;
	std::ostringstream	oss;
	std::string			tmp_env[7] = {"REQUEST_METHOD=", "QUERY_STRING=", "SCRIPT_NAME=", "CONTENT_LENGTH=", "CONTENT_TYPE=", "SCRIPT_FILENAME=", "REDIRECT_STATUS=200"};

	tmp_env[0].append(request.getPairs()["Method:"]);
	tmp_env[1].append(get_query_string(request.getPairs()["Request-Target:"]));
	if (path.find("?") != std::string::npos)
		path.resize(path.find("?"));
	tmp_env[2].append(request.getPairs()["Request-Target:"]);
	oss << request.GetBody().length();
	tmp_env[3].append(oss.str());
	tmp_env[4].append(request.getPairs()["Content-Type:"]);
	if (path.find_last_of("/") != std::string::npos)
		tmp_env[5].append(path.substr(path.find_last_of("/"), path.size() - pathfile.find_last_of("/")));
	else
		tmp_env[5].append(path);
	envp = new char*[7 + 1];
	if (!envp)
		return NULL;
	for (int i = 0; i < 7; i++) {
		envp[i] = strdup(tmp_env[i].c_str());
		if (!envp[i]) {
			free_envp(envp);
			return NULL;
		}
	}
	envp[7] = NULL;
	return (envp);
}

char** HeaderResponse::create_args(char* path)
{
	char**		args;
	std::string tmp_args[2];
	std::string script_path(path);

	script_path = "." + script_path.substr(script_path.find("=") + 1, script_path.length() - script_path.find("="));
	tmp_args[0] = get_exec(this->pathfile);
	if (tmp_args[0].empty())
		return NULL;
	tmp_args[1] = script_path;
	args = new char*[2 + 1];
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

std::string HeaderResponse::PerformCGI()
{
	int			pipe_in[2];
	int			pipe_out[2];
	std::string buffer;

	bool is_post = false;
	if (this->request.GetMethod() == POST)
		is_post = true;
	char** envp = create_env(this->request, this->pathfile);
	if (!envp) {
		SetResponseError(INTERNAL);
		return "";
	}
	char** args = create_args(envp[5]);
	if (!args) {
		free_envp(envp);
		SetResponseError(INTERNAL);
		return "";
	}
	if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1) {
		free_envp(envp);
		free_args(args);
		SetResponseError(INTERNAL);
		return "";
	}
	if (!is_post) {
		close(pipe_in[0]);
		close(pipe_in[1]);
	}
	this->pathfile = "." + this->pathfile;
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
			free_envp(envp);
			free_args(args);
			close(pipe_in[0]);
			close(pipe_out[1]);
		}
        exit(1);
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
		buffer = read_cgi_output_with_timeout(pipe_out[0], pid, 20);
    }
	free_envp(envp);
	free_args(args);
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
    char		buf[4096];
    timeval		start;
    std::string buffer;

    gettimeofday(&start, NULL);
    while (1)
    {
		ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0)
            buffer.append(buf, n);
        else if (n == -1 && errno != EAGAIN)
            return "";

        int ret = waitpid(pid, NULL, WNOHANG);
        if (ret == pid)
        {
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
