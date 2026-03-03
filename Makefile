# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/15 23:56:24 by lomont            #+#    #+#              #
#    Updated: 2026/03/03 01:19:58 by lomont           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

C++ = g++-15
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

RM = rm -f

SRC = ./main.cpp \
	./webserv.cpp \
	./client.cpp \
	./HeaderRequest.cpp \
	./HeaderResponse.cpp \
	./utils.cpp \

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(C++) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(C++) $(CFLAGS) -c $< -o $@

make: make all

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY : all clean fclean re
