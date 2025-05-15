# ========== Ayarlar ==========
NAME     = ircserv
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# ========== Dosyalar ==========
SRCS = main.cpp \
       server/Server.cpp \
       client/Client.cpp

OBJS = $(SRCS:.cpp=.o)

# ========== Kurallar ==========

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

# ========== Ekstra ==========
.PHONY: all clean fclean re
