# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: root <root@student.42.fr>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/12 16:19:52 by msolinsk          #+#    #+#              #
#    Updated: 2025/03/11 23:17:17 by root             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME_CLIENT = client
NAME_SERVER = server

CC = clang++
CFLAGS = -Wall -Wextra -Werror -std=c++98
RM = rm -f

FILES = simple_server.cpp \
		simple_client.cpp \
		Client.cpp \
		Server.cpp

OBJ = $(FILES:.cpp=.o)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

all: $(NAME_CLIENT)	$(NAME_SERVER)

$(NAME_CLIENT): simple_client.o Client.o
	$(CC) $(CFLAGS) Client.o simple_client.o -o $(NAME_CLIENT)

$(NAME_SERVER): simple_server.o Server.o
	$(CC) $(CFLAGS) Server.o simple_server.o -o $(NAME_SERVER)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME_CLIENT) $(NAME_SERVER)

re: fclean all

# run :
# 	@make
# 	# cmd.exe /c start wsl.exe ./$(NAME_CLIENT)
# 	./$(NAME_SERVER) 25565
 
.PHONY: all clean fclean re run
