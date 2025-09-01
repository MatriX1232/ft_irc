CC = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g

INCLUDES = -I/usr/include/ -I./include

NAME = ircserv

SRCS = \
	src/main.cpp \
	src/Server.cpp \
	src/Client.cpp \
	src/Message.cpp \
	src/Channel.cpp \
	src/utils.cpp \
	src/ft_logs.cpp \
	src/Outline.cpp \
	src/commands/parse_message.cpp

OBJS = $(SRCS:%.cpp=%.o)

# BONUS_SRCS = \
# 	src_bonus/main.cpp \

# BONUS_OBJS = $(BONUS_SRCS:%.cpp=%.o)

_BLUE=\e[34m
_PURPLE=\e[35m
_CYAN=\e[36m
_RED=\e[31m
_GREEN=\e[32m
_YELLOW=\e[33m
_WHITE=\e[37m
_BOLD=\e[1m
_ITALIC=\e[3m
_END=\e[0m

PADDING = 50

all: $(NAME)

%.o: %.cpp
	@printf "$(_CYAN)Compiling : $(_YELLOW)%-$(PADDING).$(PADDING)s\r$(_END)" $@
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@printf "$(_GREEN)Build complete: $(_ITALIC)$(_BOLD)$(_PURPLE)$(NAME)$(_END)\n"

# bonus: compile_dep $(BONUS_OBJS)
# 	@$(CC) $(CFLAGS) -o $(NAME) $(BONUS_OBJS) $(DEPS) $(MLXFLAGSN) $(LDFLAGS) $(LIBFT)/libft.a
# 	@printf "$(_GREEN)Build complete: $(_ITALIC)$(_BOLD)$(_PURPLE)$(NAME)$(_END)\n"

# bonus_clean: clean_dep
# 	@rm -rf $(BONUS_OBJS)
# 	@printf "$(_CYAN)Removed all .o object files from: $(_GREEN)src_bonus/$(_END)\n"

# bonus_fclean: bonus_clean
# 	@rm -f $(NAME)
# 	@printf "$(_CYAN)Removed executable: $(_PURPLE)$(NAME)$(_END)\n"

# re_bonus:
# 	@printf "$(_YELLOW)Re-making project$(_END)\n"
# 	@+make bonus_fclean --no-print-directory
# 	@+make bonus --no-print-directory

clean:
	@rm -rf $(OBJS)
	@printf "$(_CYAN)Removed all .o object files from: $(_GREEN)src/$(_END)\n"

fclean: clean
	@rm -f $(NAME)
	@printf "$(_CYAN)Removed executable: $(_PURPLE)$(NAME)$(_END)\n"

re:
	@printf "$(_YELLOW)Re-making project$(_END)\n"
	@+make fclean --no-print-directory
	@+make --no-print-directory

.PHONY: all clean fclean re compile_dep