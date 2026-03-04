# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/15 23:56:24 by lomont            #+#    #+#              #
#    Updated: 2026/03/03 22:33:30 by lomont           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

C++ = g++-15
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

RM = rm -f
OBJ_DIR = obj

SRC = ./main.cpp \
	./webserv.cpp \
	./client.cpp \
	./HeaderRequest.cpp \
	./HeaderResponse.cpp \
	./utils.cpp \

OBJ = $(SRC:./%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(C++) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: ./%.cpp
	@mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -c $< -o $@

make: make all

clean: $(OBJ_DIR)
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)
	$(RM) -r $(OBJ_DIR)

re: fclean all

.PHONY : all clean fclean re
