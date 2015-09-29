#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#pragma warning (disable : 4067)
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
	cl_platform_id platform;
} cl_platform;

typedef struct CLdevice {
	char * name;
	cl_device_id device;
	cl_bool compile_enabled;
	cl_int platform;
} cl_device;

static cl_bool initilized = CL_FALSE;
static cl_bool save_log = CL_FALSE;

int initilize(cl_platform **, cl_int *, cl_device **, cl_int *);
int Compile(const cl_platform **, const cl_int *, const cl_device **, const cl_int *, const cl_int);

void show_initilize(void);
void show_help(void);

void show_platform_list(const cl_platform *, cl_int, cl_bool);
void show_device_list(const cl_device *, cl_int, cl_bool);
