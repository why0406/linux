SERVICE = service
CLIENT = client

BUILD_DIR = build
OBJS = $(addprefix $(BUILD_DIR)/, orcv_lws.o client.o service.o)
SERVICE_OBJS = $(addprefix $(BUILD_DIR)/, service.o orcv_lws.o)
CLIENT_OBJS = $(addprefix $(BUILD_DIR)/, client.o orcv_lws.o)

CC = g++
CFLAGS = -ggdb3 -Wall -fopenmp
LIBS = -lwebsockets
INCLUDE_PATH = 

.PHONY:all
all: $(SERVICE) $(CLIENT)

$(SERVICE) : $(SERVICE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
$(CLIENT) : $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
$(OBJS) : $(BUILD_DIR)/%.o : %.cpp
	$(CC) -c $< -o $@

.PHONY:clean
clean:
	rm -f $(BUILD_DIR)/*.o $(SERVICE) $(CLIENT)
	@echo "clean over"

