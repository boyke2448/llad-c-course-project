TARGET_SRV = bin/dbserver
TARGET_CLI = bin/dbcli

SRC_SRV = $(wildcard src/srv/*.c)
OBJ_SRV = $(SRC_SRV:src/srv/%.c=obj/srv/%.o)

SRC_CLI = $(wildcard src/cli/*.c)
OBJ_CLI = $(SRC_CLI:src/cli/%.c=obj/cli/%.o)

# run: clean default
# 	./$(TARGET_SRV) -f ./mynewdb.db -n -p 8080 &
# 	./$(TARGET_CLI) 127.0.0.1
# 	kill -9 $$(pidof dbserver)

all: clean default
default: $(TARGET_SRV) $(TARGET_CLI)

test: clean default
	./$(TARGET_SRV) -f ./testdb.db -n -p 8080 &
	sleep 1
	gdb --args ./$(TARGET_CLI) 127.0.0.1

clean:
	rm -f obj/srv/*.o
	rm -f obj/cli/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET_SRV): $(OBJ_SRV)
	gcc -o $@ $? -g -I include -std=c11

$(OBJ_SRV): obj/srv/%.o: src/srv/%.c
	gcc -c $< -o $@ -g -I include -std=c11

$(TARGET_CLI): $(OBJ_CLI)
	gcc -o $@ -g -I include $? -std=c11

$(OBJ_CLI): obj/cli/%.o: src/cli/%.c
	gcc -c $< -o $@ -g -I include -std=c11
