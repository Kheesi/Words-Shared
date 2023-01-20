CPPFLAGS	:=	-D_XOPEN_SOURCE -D_FORTIFY_SOURCE=2
CFLAGS		:=	-std=c18 -Wpedantic \
							-Wall -Wextra -Wconversion -Wwrite-strings -Wstrict-prototypes -Werror \
							-fstack-protector-all -fpie \
							-O2 -g
LFLAGS		:=	-Wl,-z,relro,-z,now -pie

SRC_DIR		:=	src
INC_DIR		:=	inc

BIN_DIR		:=	bin
BLD_DIR		:=	build

SRCS			:=	$(shell find $(SRC_DIR) -type f -name \*.c -not -name t_\*.c)
BINS			:=	$(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

T_SRCS		:= 	$(shell find $(SRC_DIR) -type f -name t_\*.c)
T_BINS		:=	$(T_SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

TARGET		:=	main

#---

$(TARGET): $(BLD_DIR) $(BINS) $(T_BINS)
	@echo "bins  :" $(BINS)
	@echo "t_bins:" $(T_BINS)
	@echo -e "\n\x1b[32m> \x1b[34mCompiling \"$@\" ...\x1b[0m"
	$(CC) -o $(BLD_DIR)/$@ $(BINS) $(T_BINS) $(LFLAGS)
	@echo -e "\x1b[32m> Successfully compiled \"$@\" !\x1b[0m\n"

all: $(TARGET) lib$(TARGET).a lib$(TARGET).so

#---

lib%.a: $(BLD_DIR) $(BINS)
	@echo -e "\n\x1b[32m> \x1b[34mCompiling \"$@\" ...\x1b[0m"
	ar -r $(BLD_DIR)/$@ $(BINS)
	ranlib $(BLD_DIR)/$@
	@echo -e "\x1b[32m> Successfully compiled \"$@\" !\x1b[0m\n"

lib%.so: $(BLD_DIR) $(BINS)
	@echo -e "\n\x1b[32m> \x1b[34mCompiling \"$@\"...\x1b[0m"
	$(CC) -o $(BLD_DIR)/$@ $(BINS) $(LFLAGS) -shared
	@echo -e "\x1b[32m> Successfully compiled \"$@\" !\x1b[0m\n"

#---

$(BIN_DIR)/t_%.o: $(SRC_DIR)/%.c $(BIN_DIR)
	$(CC) -c $< -o $@ -I$(INC_DIR) $(CFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(BIN_DIR)
	$(CC) -c $< -o $@ -I$(INC_DIR) $(CFLAGS) -fPIC

#---

$(BIN_DIR):
	@mkdir $@

$(BLD_DIR):
	@mkdir $@
	
#---

run:
	@$(BLD_DIR)/$(TARGET)

clean:
	rm -rf $(BIN_DIR) $(BLD_DIR)
	tar -czvf $(shell pwd | grep -Eo '[^/]+$$').tar.gz src inc Makefile