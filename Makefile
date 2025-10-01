NAME = ft_strace
CC = gcc
CFLAGS = -Wall -Werror -Wextra -g
RM = rm -f
LIB = -lftprintf

SRC = src/main.c src/decoder.c aux/auxiliars.c aux/ft_split.c src/print_syscall.c src/syscall_table.c \
		src/error_table.c src/flags_table.c src/signals_table.c src/read_string.c aux/ft_itoa.c src/peekdata.c \
		src/free_syscall.c src/ft_strace.c src/syscall_tableSixFout.c
OBJS = $(SRC:.c=.o)

FT_PRINTF_DIR = lib/printf
FT_PRINTF = $(FT_PRINTF_DIR)/libftprintf.a

all: $(NAME)

$(NAME): $(OBJS) $(FT_PRINTF)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) -L$(FT_PRINTF_DIR) $(LIB) -lm

$(FT_PRINTF):
	@make -C $(FT_PRINTF_DIR)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	@ make clean -C $(FT_PRINTF_DIR)

fclean: clean
	$(RM) $(NAME)
	@make fclean -C $(FT_PRINTF_DIR)

re: fclean all

.PHONY: all clean fclean re $(FT_PRINTF)