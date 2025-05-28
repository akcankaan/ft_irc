NAME    = ircserv

CXX     = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = .
SRCS    = main.cpp \
          server/Server.cpp \
		  client/Client.cpp \
		  command/CommandHandler.cpp \
		  command/commands/password.cpp \
		  command/commands/nick.cpp \
		  command/commands/user.cpp \
		  command/commands/join.cpp \
		  command/commands/privmsg.cpp \
		  command/commands/kick.cpp \
		  command/commands/topic.cpp \
		  command/commands/invite.cpp \
		  command/commands/mode.cpp \
		  command/commands/quit.cpp \
		  command/commands/part.cpp \
		  channel/Channel.cpp

OBJS    = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
