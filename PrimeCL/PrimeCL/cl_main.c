#include "cl_main.h"

int initilize(cl_platform * platforms, cl_int * platform_count, cl_device * devices)
{
	cl_int err = NO_ERR, size = 0;
	cl_uint device_count = 0, device_count_temp = 0;
	cl_int index, _index;
	char buffer[BUFFER_SIZE];

	cl_platform_id * platform = (cl_platform_id)malloc(sizeof(cl_platform_id));
	cl_device_id * device = NULL;

	err |= clGetPlatformIDs(0, NULL, platform_count);
	err |= clGetPlatformIDs((*platform_count), platform, NULL);
	printf("openCL Platform detected: %d\n", (*platform_count));

	platforms = (cl_platform*)realloc(platforms, sizeof(cl_platform) * (*platform_count));
	for (index = 0; index < (*platform_count); index++)
	{
		platforms[index].platform = platform[index];

		err |= clGetPlatformInfo(platform[index], CL_PLATFORM_NAME, BUFFER_SIZE, buffer, &size);
		printf("Platform%d name: %s\n", index, buffer);
		platforms[index].name = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platforms[index].name, buffer);

		err |= clGetPlatformInfo(platform[index], CL_PLATFORM_VENDOR, BUFFER_SIZE, buffer, &size);
		printf("\tPlatform%d vender: %s\n", index, buffer);
		platforms[index].vender = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platforms[index].vender, buffer);

		err |= clGetPlatformInfo(platform[index], CL_PLATFORM_PROFILE, BUFFER_SIZE, buffer, &size);
		printf("\tPlatform%d profile: %s\n", index, buffer);
		platforms[index].profile = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platforms[index].profile, buffer);

		err |= clGetPlatformInfo(platform[index], CL_PLATFORM_VERSION, BUFFER_SIZE, buffer, &size);
		printf("\tPlatform%d version: %s\n", index, buffer);
		platforms[index].version = (char*)malloc(sizeof(char) * strlen(buffer) + sizeof(char));
		strcpy(platforms[index].version, buffer);

		err |= clGetDeviceIDs(platform[index], CL_DEVICE_TYPE_ALL, 0, 0, &device_count_temp);
		printf("\tin Platform%d, Devices detected: %d\n", index, device_count_temp);
		platforms[index].device_count = device_count_temp;

		device = (cl_device_id*)malloc(sizeof(cl_device_id) * device_count_temp);
		err |= clGetDeviceIDs(platform[index], CL_DEVICE_TYPE_ALL, device_count_temp, device, &device_count_temp);

		devices = (cl_device*)realloc(devices, sizeof(cl_device) * (device_count += device_count_temp));

		for (_index = 0; _index < device_count_temp; _index++)
		{
			char * device_name_temp = (char*)malloc(sizeof(char) * BUFFER_SIZE);

			err |= clGetDeviceInfo(device[_index], CL_DEVICE_NAME, BUFFER_SIZE, device_name_temp, &size);
			printf("\t\tDevice%d name: %s\n", _index, device_name_temp);

			err |= clGetDeviceInfo(device[_index], CL_DEVICE_COMPILER_AVAILABLE, BUFFER_SIZE, buffer, &size);
			if (buffer[0] == CL_TRUE)
				printf("\t\tDevice%d support kernel compile!\n", _index);
			else
				printf("\t\tDevice%d does not support kernel compile!\n", _index);

			devices[device_count - device_count_temp + _index].platform = index;
			devices[device_count - device_count_temp + _index].device = device[_index];
			devices[device_count - device_count_temp + _index].compile_enabled = buffer[0];

			devices[device_count - device_count_temp + _index].name = (char*)malloc(sizeof(char) * strlen(device_name_temp) + sizeof(char));
			strcpy(devices[device_count - device_count_temp + _index].name, device_name_temp);
		}
		if (err)
			return -1;
	}
}
int main(int argc, char * argv[])
{
	cl_int err = NO_ERR, _index, size, platform_count;
	cl_platform * platforms = (cl_platform*)malloc(sizeof(cl_platform));
	cl_device * devices = (cl_device *)malloc(sizeof(cl_device));
	initilize(platforms, &platform_count, devices);

	printf("=========================Devices=========================\n");
	for (_index = 0; _index < platforms->device_count; _index++)
		printf("%d. %s,\tcompile Enabled: %s\n", _index, devices[_index].name, devices[_index].compile_enabled ? "TRUE" : "FALSE");

	int input;
	do {
		printf("input number of device to work : ");
		scanf("%d", &input);
		if (input < platforms->device_count)
		{
			printf("%s-%s selected.\n", platforms[devices[input].platform].name, devices[input].name);

			char * kernel_source = file_read("./dotproduct.cl", &size), result_string[MEM_SIZE];
			size_t kernel_source_size = size;

			cl_context context = clCreateContext(NULL, 1, &devices[input].device, NULL, NULL, &err);
			cl_command_queue queue = clCreateCommandQueue(context, devices[input].device, 0, &err);
			cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, &kernel_source_size, &err);

			err |= clBuildProgram(program, 1, &devices[input].device, NULL, NULL, NULL);
			cl_kernel kernel = clCreateKernel(program, "DotProduct", &err);

			if (err)
			{
				printf("\tkernel creation fail.\n");
				continue;
			}
			else
				printf("\tkernel created!\n");

			long t_ele;
			do {
				printf("\tinput size(over 1024): ");
				scanf("%ld", &t_ele);
			} while (t_ele < 1024);

			cl_long NmEle = t_ele, szLocalSize = 256, szGlobalSize = 0;//shrRoundUp((size_t)szLocalSize, NmEle);

			void *arg_0 = (void*)malloc(sizeof(cl_double4) * szGlobalSize),
				*arg_1 = (void*)malloc(sizeof(cl_double4) * szGlobalSize),
				*arg_2 = (void*)malloc(sizeof(cl_double) * szGlobalSize);

			if (arg_0 == NULL || arg_1 == NULL || szGlobalSize * 4 > SIZE_MAX)
			{
				printf("\tout of size.\n");
				continue;
			}

			//shrFillArray((double*)arg_0, 4 * NmEle);
			//shrFillArray((double*)arg_1, 4 * NmEle);

			printf("\t%d vectors created!\n\tdot product calculating...\t", szGlobalSize);

			__int64 nStartcount;
			_asm rdtsc
			_asm lea ebx, nStartcount
			_asm mov dword ptr[ebx], eax
			_asm mov dword ptr[ebx + 4], edx

			cl_mem mem_arg_0 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_double4) * szGlobalSize, NULL, &err);
			cl_mem mem_arg_1 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_double4) * szGlobalSize, NULL, &err);
			cl_mem mem_arg_2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_double) * szGlobalSize, NULL, &err);

			err |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&mem_arg_0);
			err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&mem_arg_1);
			err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&mem_arg_2);
			err |= clSetKernelArg(kernel, 3, sizeof(cl_long), (void *)&NmEle);

			err |= clEnqueueWriteBuffer(queue, mem_arg_0, CL_FALSE, 0, sizeof(cl_double4) * szGlobalSize, arg_0, 0, NULL, NULL);
			err |= clEnqueueWriteBuffer(queue, mem_arg_1, CL_FALSE, 0, sizeof(cl_double4) * szGlobalSize, arg_1, 0, NULL, NULL);

			err |= clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &szGlobalSize, &szLocalSize, 0, NULL, NULL);

			err |= clEnqueueReadBuffer(queue, mem_arg_2, CL_TRUE, 0, sizeof(cl_double) * szGlobalSize, arg_2, 0, NULL, NULL);

			__int64 nEndCount;
			_asm rdtsc
			_asm lea ebx, nEndCount
			_asm mov dword ptr[ebx], eax
			_asm mov dword ptr[ebx + 4], edx

			for (_index = 0; _index < 10; _index++)
				if (((cl_double*)arg_2)[_index] < 0.00001)
				{
					printf("fail\n");
					break;
				}
			if (_index == 10)
				printf("success.\n\texecution time: %.4lf(ms)\n", (double)(nEndCount - nStartcount) / 2000000.0);

			err |= clFlush(queue);
			err |= clFlush(queue);
			err |= clFinish(queue);
			err |= clFinish(queue);
			err |= clReleaseKernel(kernel);
			err |= clReleaseProgram(program);

			err |= clReleaseMemObject(mem_arg_0);
			err |= clReleaseMemObject(mem_arg_1);
			err |= clReleaseMemObject(mem_arg_2);

			err |= clReleaseCommandQueue(queue);
			err |= clReleaseContext(context);

			free(arg_0);
			free(arg_1);
			free(arg_2);
			free(kernel_source);
		}
		else if (!(input + 1))
			break;
		else
			printf("Device%d does not exist\n", input);
		int temp_c;
		while ((temp_c = getchar()) != EOF && temp_c != '\n');
	} while (1);

	return 0;
}
