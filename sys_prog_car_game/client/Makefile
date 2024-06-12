SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
UTL_DIR = ./src/util
INC_DIR = \
		./src \
		-I ./include \

GCCFLAGS += \
	-g \
	-Wall \

# Using pkg-config to get OpenCV flags
CXXFLAGS += $(shell pkg-config --cflags opencv4)
LDFLAGS += \
	-lwiringPi \
	-lpthread \
	$(shell pkg-config --libs opencv4) \

# List of source files
OBJ_SRC += \
	$(SRC_DIR)/client_cpy.c \
	$(SRC_DIR)/handle_car.c \
	$(SRC_DIR)/algorithm.c \
	$(SRC_DIR)/qrdetector.cpp \

# Convert source files to object files
TARGET_OBJ = \
    $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(filter %.c,$(OBJ_SRC)))) \
    $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(filter %.cpp,$(OBJ_SRC))))

GXX = g++

all : runfile

runfile: $(TARGET_OBJ)
	@mkdir -p $(BIN_DIR)
	$(GXX) $(GCCFLAGS) $(CXXFLAGS) -o $(BIN_DIR)/$@ $^ -I$(INC_DIR) $(LDFLAGS)
	ln -sf $(BIN_DIR)/$@ ./client

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	gcc $(GCCFLAGS) -c $< -o $@ -I$(INC_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(GXX) $(GCCFLAGS) $(CXXFLAGS) -c $< -o $@ -I$(INC_DIR)

.c.o:
	gcc -c $< -o $@ -I$(INC_DIR)

.cpp.o:
	$(GXX) -c $< -o $@ -I$(INC_DIR)

-include $(OBJS:.o=.d)

clean:
	@rm -vf $(BIN_DIR)/*
	@rm -vf $(OBJ_DIR)/*.o
	@rm -vf $(SRC_DIR)/*.o
	@rm -vf $(UTL_DIR)/*.o