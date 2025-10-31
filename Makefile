FORMAT  = clang-format

CC 		= gcc
CFLAGS  = -Wall -Wno-dangling-pointer -O3 
DCFLAGS  = -Wall -Wno-dangling-pointer -g 
TARGET 	= boom.bin
DTARGET 	= dboom.bin
LIBS = -lm

SRC_DIR = src
OBJ_DIR = obj
DOBJ_DIR = dobj

SRCS = $(wildcard ${SRC_DIR}/*.c)
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))
DOBJS = $(patsubst %.c,$(DOBJ_DIR)/%.o,$(notdir $(SRCS)))

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o ${TARGET} $^ ${LIBS}

${OBJ_DIR}/%.o : ${SRC_DIR}/%.c | ${OBJ_DIR}
	${CC} ${CFLAGS} -c $< -o $@ 

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

all: ${TARGET}

${DOBJ_DIR}/%.o : ${SRC_DIR}/%.c | ${DOBJ_DIR}
	${CC} ${DCFLAGS} -c $< -o $@ 

${DOBJ_DIR}:
	mkdir -p ${DOBJ_DIR}

debug: ${DOBJS}
	${CC} ${DCFLAGS} -o ${DTARGET} $^ ${LIBS}

format:
	${FORMAT} -i ${SRC_DIR}/* || true

clean: format
	${RM} -rf ${OBJ_DIR}
	${RM} ${TARGET}
	${RM} -rf ${DOBJ_DIR}
	${RM} ${DTARGET}
