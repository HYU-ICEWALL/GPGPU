#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__ || MACOSX
	#include <opencl/opencl.h>
	
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

typedef struct {
	char * name;
	char * profile;
	char * version;
	char * vender;
	cl_int device_num;
	cl_platform_id *platform;
} cl_platform;
typedef struct {
	char * name;
	cl_device_id device;
	cl_bool compile_enabled;
	cl_int platform;
} cl_device;

char * file_read(const char *, size_t *);

char * file_read(const char * path, size_t * size)
{
	FILE * file = fopen(path, "rb");
	char * file_text;

	fseek(file, 0, SEEK_END);
	(*size) = ftell(file);
	rewind(file);

	file_text = (char*)malloc(sizeof(char) * (*size) + sizeof(char));
	(*size) = fread(file_text, sizeof(char), (*size), file);
	fclose(file);

	return file_text;
}

int shrCompareFet(const cl_float * reference, const cl_float * data, const size_t len)
{
	for (int i = 0; i < len; i++)
		if (reference[i] - data[i])
			return 0;
	return 1;
}
void shrFillArray(cl_float * data, cl_int size)
{
	const cl_float scale = 1.0f / (cl_float)RAND_MAX;
	for (int i = 0; i < size; i++)
		data[i] = scale * rand();
}
size_t shrRoundUp(cl_int group_size, cl_int global_size)
{
	cl_int r = global_size % group_size;
	if (!r)
		return global_size;
	else
		return global_size + group_size - r;
}

int main(int argc, char * argv[])
{
	srand(0);
	cl_int err = NO_ERR, size = 0;
	cl_uint platform_num = 0, device_num = 0, device_num_temp = 0;
	size_t _index, __index, _temp;

	cl_platform_id *platform = (cl_platform_id)malloc(sizeof(cl_platform_id));
	cl_platform *platform_info = (cl_platform*)malloc(sizeof(cl_platform));
	cl_device_id *device = NULL;
	cl_device *devices = (cl_device *)malloc(sizeof(cl_device));
	
	char buffer[BUFFER_SIZE];

	err |= clGetPlatformIDs(0, NULL, &platform_num);
	err |= clGetPlatformIDs(platform_num, platform, NULL);
	printf("openCL Platform detected: %d\n", platform_num);

	platform_info = (cl_platform*)realloc(platform_info, sizeof(cl_platform) * platform_num);
	for (_index = 0; _index < platform_num; _index++)
	{
		platform_info[_index].platform = platform[_index];

		err |= clGetPlatformInfo(platform[_index], CL_PLATFORM_NAME, BUFFER_SIZE, buffer, &size);
		printf("Platform%d name: %s\n", _index, buffer);
		platform_info[_index].name = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platform_info[_index].name, buffer);

		err |= clGetPlatformInfo(platform[_index], CL_PLATFORM_VENDOR, BUFFER_SIZE, buffer, &size);
		printf("\tPlatform%d vender: %s\n", _index, buffer);
		platform_info[_index].vender = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platform_info[_index].vender, buffer);

		err |= clGetPlatformInfo(platform[_index], CL_PLATFORM_PROFILE, BUFFER_SIZE, buffer, &size);
		printf("\tPlatform%d profile: %s\n", _index, buffer);
		platform_info[_index].profile = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platform_info[_index].profile, buffer);

		err |= clGetPlatformInfo(platform[_index], CL_PLATFORM_VERSION, BUFFER_SIZE, buffer, &size);
		printf("\tPlatform%d version: %s\n", _index, buffer);
		platform_info[_index].version = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platform_info[_index].version, buffer);

		err |= clGetDeviceIDs(platform[_index], CL_DEVICE_TYPE_ALL, 0, 0, &device_num_temp);
		printf("\tin Platform%d, Devices detected: %d\n", _index, device_num_temp);
		platform_info[_index].device_num = device_num_temp;

		device = (cl_device_id*)malloc(sizeof(cl_device_id) * device_num_temp);
		err |= clGetDeviceIDs(platform[_index], CL_DEVICE_TYPE_ALL, device_num_temp, device, &device_num_temp);

		devices = (cl_device*)realloc(devices, sizeof(cl_device) * (device_num += device_num_temp));

		for (__index = 0; __index < device_num_temp; __index++)
		{
			char * device_name_temp = (char*)malloc(sizeof(char) * BUFFER_SIZE);

			err |= clGetDeviceInfo(device[__index], CL_DEVICE_NAME, BUFFER_SIZE, device_name_temp, &size);
			printf("\t\tDevice%d name: %s\n", __index, device_name_temp);

			err |= clGetDeviceInfo(device[__index], CL_DEVICE_COMPILER_AVAILABLE, BUFFER_SIZE, buffer, &size);
			if (buffer[0] == CL_TRUE)
				printf("\t\tDevice%d support kernel compile!\n", __index);
			else
				printf("\t\tDevice%d does not support kernel compile!\n", __index);

			devices[device_num - device_num_temp + __index].platform = _index;
			devices[device_num - device_num_temp + __index].device = device[__index];
			devices[device_num - device_num_temp + __index].compile_enabled = buffer[0];

			devices[device_num - device_num_temp + __index].name = (char*)malloc(sizeof(char) * strlen(device_name_temp) + sizeof(char));
			strcpy(devices[device_num - device_num_temp + __index].name, device_name_temp);
		}
		if (err)
			return -1;
	}

	printf("=========================Devices=========================\n");
	for (_index = 0; _index < device_num; _index++)
		printf("%d. %s,\tcompile Enabled: %s\n", _index, devices[_index].name, devices[_index].compile_enabled ? "TRUE": "FALSE");
	
	int input;
	do	{
		printf("input number of device to work : ");
		scanf("%d", &input);
		if (input < device_num)
		{
			printf("%s-%s selected.\n", platform_info[devices[input].platform].name, devices[input].name);

			char * kernel_source = file_read("./test.cl", &size), result_string[MEM_SIZE];
			size_t kernel_source_size = size;
			printf("==src==\n%s\==end==\n", kernel_source);
			cl_context context = clCreateContext(NULL, 1, &devices[input].device , NULL, NULL, &err);
			cl_command_queue queue = clCreateCommandQueue(context, devices[input].device, 0, &err);
			cl_mem memory_object = clCreateBuffer(context, CL_MEM_READ_WRITE, BUFFER_SIZE, buffer, &size);
			cl_program program = clCreateProgramWithSource(context, 1,(const char **)&kernel_source, &kernel_source_size, &err);
			err |= clBuildProgram(program, 1, &devices[input].device, NULL, NULL, NULL);

			cl_kernel kernel = clCreateKernel(program, "DotProduct", &err);
			cl_int NmEle = 1277944;
			cl_int szLocalSize = 256, szGlobalSize = shrRoundUp((int)szLocalSize, NmEle);
			void *arg_0 = (void*)malloc(sizeof(cl_float4) * szGlobalSize),
				*arg_1 = (void*)malloc(sizeof(cl_float4) * szGlobalSize),
				*arg_2 = (void*)malloc(sizeof(cl_float) * szGlobalSize),
				*result = (void*)malloc(sizeof(cl_float) * NmEle);
			shrFillArray((cl_float*)arg_0, 4 * NmEle);
			shrFillArray((cl_float*)arg_1, 4 * NmEle);

			cl_mem mem_arg_0 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalSize * 4, NULL, &err);
			cl_mem mem_arg_1 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalSize * 4, NULL, &err);
			cl_mem mem_arg_2 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalSize, NULL, &err);
			err |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&mem_arg_0);
			err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&mem_arg_1);
			err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&mem_arg_2);
			err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&NmEle);

			err |= clEnqueueWriteBuffer(queue, mem_arg_0, CL_FALSE, 0, sizeof(cl_float) * szGlobalSize * 4, arg_0, 0, NULL, NULL);
			err |= clEnqueueWriteBuffer(queue, mem_arg_0, CL_FALSE, 0, sizeof(cl_float) * szGlobalSize * 4, arg_1, 0, NULL, NULL);

			err |= clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &szGlobalSize, &szLocalSize, 0, NULL, NULL);

			err |= clEnqueueReadBuffer(queue, mem_arg_2, CL_TRUE, 0, sizeof(cl_float) * szGlobalSize, arg_2, 0, NULL, NULL);

			for (int i = 0, j = 0; i < NmEle; i++)
			{
				((cl_float*)result)[i] = 0.0f;
				for (int k = 0; k < 4; k++, j++)
					((cl_float*)result)[i] += ((cl_float*)arg_0)[j] * ((cl_float*)arg_1)[j];
			}

			if (shrCompareFet((const cl_float *)result, (const cl_float*)arg_2, (size_t)NmEle))
				printf("success\n");

			//err |= clEnqueueTask(queue, kernel, 0, NULL, NULL);
			//err |= clEnqueueReadBuffer(queue, memory_object, CL_TRUE, 0, MEM_SIZE * sizeof(char), result_string, 0, NULL, NULL);

			err |= clFlush(queue);
			err |= clFlush(queue);
			err |= clFinish(queue);
			err |= clFinish(queue);
			err |= clReleaseKernel(kernel);
			err |= clReleaseProgram(program);
			err |= clReleaseMemObject(memory_object);
			err |= clReleaseCommandQueue(queue);
			err |= clReleaseContext(context);

			free(kernel_source);
		}
		else if (!(input + 1))
			break;
		else
			printf("Device%d does not exist\n", input);
		int temp_c;
		while ((temp_c = getchar()) != EOF && temp_c != '\n');
	 } while(1);

	return 0;
}
