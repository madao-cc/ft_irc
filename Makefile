NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
CPPFLAGS := -I include

# Explicit source list (no shell find). List each .cpp file one by one as required.
SRCS := \
	src/channel/Channels.cpp \
	src/client/Client.cpp \
	src/commands/invite.cpp \
	src/commands/join.cpp \
	src/commands/kick.cpp \
	src/commands/mode.cpp \
	src/commands/nick.cpp \
	src/commands/part.cpp \
	src/commands/pass.cpp \
	src/commands/privmsg.cpp \
	src/commands/quit.cpp \
	src/commands/topic.cpp \
	src/commands/user.cpp \
	src/modes/modes.cpp \
	src/server/main.cpp \
	src/server/Server.cpp

all: $(NAME)

$(NAME): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(NAME)

clean:
	@rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re


