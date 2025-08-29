CXX = c++
CXXFLAGS = -Wall -Wextra -std=c++98 -O2 $(INCLUDE)

OBJ_DIR = obj/
SRC_DIR = src/
INCLUDE = -Iinclude

SRC =		$(SRC_DIR)main.cpp								\
			$(SRC_DIR)commands/CommandHandler.cpp			\
			$(SRC_DIR)channel/Channel.cpp					\
			$(SRC_DIR)replies/Replies.cpp					\
			$(SRC_DIR)server/Server.cpp 					\
			$(SRC_DIR)server/Listener.cpp					\
			$(SRC_DIR)server/Connection.cpp 				\
			$(SRC_DIR)start_func/args_check.cpp				\
			$(SRC_DIR)start_func/server_start.cpp			\
			$(SRC_DIR)user/User.cpp							\
			$(SRC_DIR)signals/signals.cpp					\
			$(SRC_DIR)logger/Logger.cpp						\

OBJ = $(addprefix $(OBJ_DIR), $(SRC:.cpp=.o))

NAME = ircserv

BENDER_SRC = $(SRC_DIR)bender/Bender.cpp						\
			$(SRC_DIR)bender/bender_types/BenderFactory.cpp		\
			$(SRC_DIR)bender/bender_types/BobBender.cpp			\
			$(SRC_DIR)bender/bender_types/GandhiBender.cpp		\
			$(SRC_DIR)bender/bender_types/SatanBender.cpp		\
			$(SRC_DIR)bender/infochan/ChannelInfo.cpp			\
			$(SRC_DIR)bender/bender_signals/bender_sig.cpp		\
			$(SRC_DIR)bender/bender_helper.cpp					\
			$(SRC_DIR)bender/main.cpp

BENDER_OBJ = $(addprefix $(OBJ_DIR), $(BENDER_SRC:.cpp=.o))
BENDER_NAME = bender

all: $(NAME)

$(NAME): $(OBJ)
	@echo ""${BLUE}$(NAME)""${NC} Compiling... "\c"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo ""${GREEN}Complete""$(NC)""

bender: $(BENDER_OBJ)
	@echo ""${BLUE}$(BENDER_NAME)""${NC} Compiling... "\c"
	@$(CXX) $(CXXFLAGS) $(BENDER_OBJ) -o $(BENDER_NAME)
	@echo ""${GREEN}Complete""$(NC)""

$(OBJ_DIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

WHITE	=	'\033[0;37m'
YELLOW	=	'\033[0;33m'
BLUE	=	'\033[0;34m'
GREEN	=	'\033[0;32m'
RED		=	'\033[0;31m'
NC		=	'\033[0m'

log: CXXFLAGS += -DLOG
log: re

clean:
	@rm -rf $(OBJ_DIR)
	@echo ""${GREEN}Clean Complete""$(NC)""

fclean: clean
	@rm -f $(NAME) $(BENDER_NAME)
	@echo ""${GREEN}Fclean Complete""$(NC)""

re: fclean all

.PHONY: all clean fclean re debug bender
