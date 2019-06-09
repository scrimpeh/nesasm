cmake_minimum_required(VERSION 3.5)
project(NESASM)

add_executable(${PROJECT_NAME} 
	source/alias.c
	source/assemble.c
	source/code.c
	source/command.c
	source/crc.c
	source/defs.h
	source/expr.c
	source/expr.h
	source/externs.h
	source/func.c
	source/inbuilt.c
	source/input.c
	source/inst.h
	source/macro.c
	source/main.c
	source/map.c
	source/mml.c
	source/nes.c
	source/nes.h
	source/output.c
	source/pce.c
	source/pce.h
	source/pcx.c
	source/proc.c
	source/protos.h
	source/symbol.c
	source/vars.h

	source/util/strcasecmp.c
	source/util/strcasecmp.h
)

include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/source
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/source
)
