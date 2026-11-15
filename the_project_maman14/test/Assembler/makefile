CC = gcc
CFLAGS = -ansi -Wall -pedantic -g
SRCS = main.c parser.c symbol_table.c cmd.c output_files.c hash_map.c directive.c errors.c mcro.c lst.c used_labels_list.c
TARGET = assembler

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
