#pragma OPENCL EXTENSION cl_khr_icd  : enable

__kernel void DotProduct(__global double* a, __global double* b, __global double* c, long iNumElements)
{
	int iGID = get_global_id(0);

	if (iGID >= iNumElements)
		return;

	int iInOffset = iGID << 2;
	c[iGID] = a[iInOffset] * b[iInOffset]
		+ a[iInOffset + 1] * b[iInOffset + 1]
		+ a[iInOffset + 2] * b[iInOffset + 2]
		+ a[iInOffset + 3] * b[iInOffset + 3];
}