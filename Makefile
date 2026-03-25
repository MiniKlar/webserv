# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/15 23:56:24 by lomont            #+#    #+#              #
#    Updated: 2026/03/26 00:10:03 by lomont           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CXX ?= g++-15
BASE_CFLAGS = -Wall -Wextra -Werror -g -std=c++98 -pedantic -fsanitize=address
SANITIZE ?= 0

ifeq ($(SANITIZE),1)
SANITIZE_FLAGS = -fsanitize=address
endif

CFLAGS = $(BASE_CFLAGS) $(SANITIZE_FLAGS)

RM = rm -f
OBJ_DIR = obj

SRC = ./srcs/main.cpp \
	./srcs/webserv.cpp \
	./srcs/client.cpp \
	./srcs/HeaderRequest.cpp \
	./srcs/HeaderResponse.cpp \
	./srcs/utils.cpp \

OBJ = $(SRC:./srcs/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: ./srcs/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

clean:

fclean: clean
	$(RM) $(NAME)
	$(RM) -r $(OBJ_DIR)

re: fclean all

.PHONY : all clean fclean re
