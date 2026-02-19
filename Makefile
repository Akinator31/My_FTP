
SRC_DIR     	=	src
BUILD_DIR   	=	build

NAME         	= 	myftp
TEST_NAME		=	unit_tests

COLOR_BLUE    := $(shell printf "\033[1;34m")
COLOR_GREEN   := $(shell printf "\033[1;32m")
COLOR_RED     := $(shell printf "\033[1;31m")

all: $(NAME)

$(NAME):
	@echo "$(COLOR_RED)Building $(NAME)$(COLOR_RESET)"
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)
	@cp build/myftp .
	@echo "$(COLOR_GREEN)Project built successfully!$(COLOR_RESET)"

clean:
	@$(RM) -rf $(BUILD_DIR)
	@echo "$(COLOR_GREEN)Object files cleaned!$(COLOR_RESET)"

fclean: clean
	@$(RM) $(NAME)
	@$(RM) -r $(BUILD_DIR)
	@echo "$(COLOR_GREEN)Project cleaned!$(COLOR_RESET)"

tests_run:
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR) --target $(TEST_NAME)
	@./$(BUILD_DIR)/$(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re