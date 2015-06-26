GZIP		:=	/usr/bin/gzip
LUA_VERSION     :=      5.2.1
LUA_DIRECTORY   :=      lua-${LUA_VERSION}
LUA_TARFILE     :=      ${LUA_DIRECTORY}.tar.gz
LUA_OS          :=	linux
LUA_SRC_DIR     :=      lua/src
LUA_LIB_DIR     :=      lua/lib
LUA_INC_DIR     :=      lua/include
LUA_HEADERS     :=      ${LUA_INC_DIR}/luaconf.h ${LUA_INC_DIR}/lua.h
LUA_HEADERS     :=      ${LUA_HEADERS} ${LUA_INC_DIR}/lualib.h ${LUA_INC_DIR}/lauxlib.h
LUA_LIB         :=      ${LUA_LIB_DIR}/liblua.a

all : lua lua_s

.PHONY : lua
lua :
	#Not linked to CM build. Run Lua build.
	make ${LUA_DIRECTORY}
	make lua_real

lua_real : build_lua ${LUA_HEADERS} ${LUA_LIB}


${LUA_DIRECTORY} : ${LUA_TARFILE}
	echo "${GZIP} -dc ${LUA_TARFILE}"
	${GZIP} -dc ${LUA_TARFILE} | \
	tar xpf -;
	rm -f lua; ln -sf ${LUA_DIRECTORY} lua;
	cat lua/src/Makefile | sed -e 's/gcc/g++/' \
-e 's/^CFLAGS= \(.*\)/CFLAGS= -m64 \1/' \
-e 's/^MYLDFLAGS=/MYLDFLAGS= -m64/' > lua/src/Makefile.new;
	mv lua/src/Makefile lua/src/Makefile.old
	mv lua/src/Makefile.new lua/src/Makefile
	rm -rf lua/lib; mkdir lua/lib;
	rm -rf lua/include; mkdir lua/include;

build_lua :
	cd lua; make ${LUA_OS}


${LUA_INC_DIR}/%.h: ${LUA_SRC_DIR}/%.h
	rm -rf $@
	cp $< $@

${LUA_LIB_DIR}/%.a: ${LUA_SRC_DIR}/%.a
	rm -rf $@
	cp $< $@


.PHONY : lua_s
lua_s : 
	cd lua_s; make

clean :
	rm -rf lua
	rm -rf ${LUA_DIRECTORY}

