OS_TYPE		:= $(shell uname)

LDFLAGS =  -m64 -fPIC
PROC_TYPE   := $(shell uname -m)
CLEARMAKE_FLAGS = -D$(PROC_TYPE)
CXXFLAGS = -m64 -g -O -D_REENTRANT ${CLEARMAKE_FLAGS}

#RPATH will hard code this binary to look for it's libraries in the DSC
#installation directory so it doesn't need the LD_LIBRARY_PATH set on
#the machine that it's executing on
RPATH = -Wl,-rpath=/opt/adc/adc/lib

LUA_INC=../lua/include
LUA_LIB=../lua/lib
PROTOBUF_INC=/vob/adc/router/thirdparty/protobuf/src
PROTOBUF_LIB=/vob/adc/router/thirdparty/protobuf/src/.libs
FLEX_SCTP_INC=/vob/component/flexible_sctp/include
FLEX_SCTP_LIB=/vob/component/flexible_sctp/lib
CLU_ROUTE_MSG_INC=/vob/adc/router/cluster_routing/
DRA_MSG_INC=/vob/adc/router/core/

INC_DIR = -I. -I${LUA_INC} -I${PROTOBUF_INC} -I${FLEX_SCTP_INC}
INC_DIR += -I${CLU_ROUTE_MSG_INC} -I${DRA_MSG_INC}
LIB_DIR = -L${LUA_LIB} -L${PROTOBUF_LIB} -L${FLEX_SCTP_LIB}
LIB = -llua -lflexsctp -ldl -lprotobuf -lrt

OBJECTS = lua_s.o FlexSctpLuaLibrary.o SocketManager.o
OBJECTS += /vob/adc/router/cluster_routing/ClusterRoutingMessage.pb.o
OBJECTS += /vob/adc/router/core/DraMessagePB.pb.o
OBJECTS += SocketLuaLibrary.o ProtoBufLuaLibrary.o ProtoBufUtility.o

all: lua_s

lua_s: ${OBJECTS}
	g++ -g ${LDFLAGS} ${OBJECTS} ${LIB_DIR} ${LIB} ${RPATH} -o lua_s

/vob/adc/router/cluster_routing/ClusterRoutingMessage.pb.o:
	cd /vob/adc/router/cluster_routing; make ClusterRoutingMessage.pb.o

/vob/adc/router/core/DraMessagePB.pb.o:
	cd /vob/adc/router/core; make DraMessagePB.pb.o

%.o : %.cpp
	g++ ${INC_DIR} $(CXXFLAGS) -c $< -o $@

clean:
	rm *o
	rm lua_s
