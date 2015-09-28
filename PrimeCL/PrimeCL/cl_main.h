#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef linux || __APPLE__ || MACOSX
#include <opencl/opencl.h>
#include <unistd.h>

#else
#include <CL\opencl.h>
#pragma warning ( disable : 4996 )

#ifdef _WIN64
#pragma comment (lib,"lib/x64/opencl.lib")

#else
#pragma comment (lib,"lib/x86/opencl.lib")
#endif

#endif

#define NO_ERR (0)
#define MEM_SIZE (128)
#define BUFFER_SIZE (256)
#define MAX_SOURCE_SIZE (0x100000)

typedef struct CLplatform {
	char * name;
	char * profile;
	char * version;
	char * vender;
	cl_int device_count;
	cl_platform_id *platform;
} cl_platform;

typedef struct CLdevice {
	char * name;
	cl_device_id device;
	cl_bool compile_enabled;
	cl_int platform;
} cl_device;
