NAME    = ircserv

CXX     = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = .
SRCS    = main.cpp \
          server/Server.cpp \
		  client/Client.cpp \
		  commands/CommandHandler.cpp \
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
