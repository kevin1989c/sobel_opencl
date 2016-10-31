

#include <tchar.h>
#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


#define DEFAULT_THRESHOLD  4000

#define DEFAULT_FILENAME "BWstop-sign.ppm"
unsigned int *read_ppm(char *filename, int * xsize, int * ysize, int *maxval){

	if (!filename || filename[0] == '\0') {
		fprintf(stderr, "read_ppm but no file name\n");
		return NULL;  // fail
	}

	FILE *fp;

	fprintf(stderr, "read_ppm( %s )\n", filename);
	fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "read_ppm()    ERROR  file '%s' cannot be opened for reading\n", filename);
		return NULL; // fail 

	}

	char chars[1024];
	//int num = read(fd, chars, 1000);
	int num = fread(chars, sizeof(char), 1000, fp);

	if (chars[0] != 'P' || chars[1] != '6')
	{
		fprintf(stderr, "Texture::Texture()    ERROR  file '%s' does not start with \"P6\"  I am expecting a binary PPM file\n", filename);
		return NULL;
	}

	unsigned int width, height, maxvalue;


	char *ptr = chars + 3; // P 6 newline
	if (*ptr == '#') // comment line! 
	{
		ptr = 1 + strstr(ptr, "\n");
	}

	num = sscanf(ptr, "%d\n%d\n%d", &width, &height, &maxvalue);
	fprintf(stderr, "read %d things   width %d  height %d  maxval %d\n", num, width, height, maxvalue);
	*xsize = width;
	*ysize = height;
	*maxval = maxvalue;

	unsigned int *pic = (unsigned int *)malloc(width * height * sizeof(unsigned int));
	if (!pic) {
		fprintf(stderr, "read_ppm()  unable to allocate %d x %d unsigned ints for the picture\n", width, height);
		return NULL; // fail but return
	}

	// allocate buffer to read the rest of the file into
	int bufsize = 3 * width * height * sizeof(unsigned char);
	if ((*maxval) > 255) bufsize *= 2;
	unsigned char *buf = (unsigned char *)malloc(bufsize);
	if (!buf) {
		fprintf(stderr, "read_ppm()  unable to allocate %d bytes of read buffer\n", bufsize);
		return NULL; // fail but return
	}





	// TODO really read
	char duh[80];
	char *line = chars;

	// find the start of the pixel data.   no doubt stupid
	sprintf(duh, "%d\0", *xsize);
	line = strstr(line, duh);
	//fprintf(stderr, "%s found at offset %d\n", duh, line-chars);
	line += strlen(duh) + 1;

	sprintf(duh, "%d\0", *ysize);
	line = strstr(line, duh);
	//fprintf(stderr, "%s found at offset %d\n", duh, line-chars);
	line += strlen(duh) + 1;

	sprintf(duh, "%d\0", *maxval);
	line = strstr(line, duh);


	fprintf(stderr, "%s found at offset %d\n", duh, line - chars);
	line += strlen(duh) + 1;

	long offset = line - chars;
	//lseek(fd, offset, SEEK_SET); // move to the correct offset
	fseek(fp, offset, SEEK_SET); // move to the correct offset
	//long numread = read(fd, buf, bufsize);
	long numread = fread(buf, sizeof(char), bufsize, fp);
	fprintf(stderr, "Texture %s   read %ld of %ld bytes\n", filename, numread, bufsize);

	fclose(fp);


	int pixels = (*xsize) * (*ysize);
	for (int i = 0; i<pixels; i++) pic[i] = (int)buf[3 * i];  // red channel



	return pic; // success
}

void write_ppm(char *filename, int xsize, int ysize, int maxval, int *pic)
{
	FILE *fp;
	int x, y;

	fp = fopen(filename, "w");
	if (!fp)
	{
		fprintf(stderr, "FAILED TO OPEN FILE '%s' for writing\n");
		exit(-1);
	}



	fprintf(fp, "P6\n");
	fprintf(fp, "%d %d\n%d\n", xsize, ysize, maxval);

	int numpix = xsize * ysize;
	for (int i = 0; i<numpix; i++) {
		unsigned char uc = (unsigned char)pic[i];
		fprintf(fp, "%c%c%c", uc, uc, uc);
	}
	fclose(fp);

}

using namespace std;

cl_int ConvertToString(const char *pFileName, std::string &str);

int _tmain(int argc, _TCHAR* argv[])
{
	int thresh = DEFAULT_THRESHOLD;
	char *filename;
	filename = strdup(DEFAULT_FILENAME);

	if (argc > 1) {
		if (argc == 3)  { // filename AND threshold
			filename = strdup(argv[1]);
			thresh = atoi(argv[2]);
		}
		if (argc == 2) { // default file but specified threshhold

			thresh = atoi(argv[1]);
		}

		fprintf(stderr, "file %s    threshold %d\n", filename, thresh);
	}


	int xsize, ysize, maxval;
	unsigned int *pic = read_ppm(filename, &xsize, &ysize, &maxval);


	int numbytes = xsize * ysize * 3 * sizeof(int);
	int *result = (int *)malloc(numbytes);

	int DATA_SIZE = xsize * ysize;


	cl_int			iStatus = 0;			
	cl_uint			uiNumPlatforms = 0;				
	cl_platform_id	Platform = NULL;		
	size_t			uiSize = 0;			
	cl_int			iErr = 0;		
	char			*pName = NULL;				
	cl_uint			uiNumDevices = 0;			
	cl_device_id	*pDevices = NULL;			
	cl_context		Context = NULL;			
	cl_command_queue	CommandQueue = NULL;			
	const char		*pFileName = "sobel_kernel.cl";	
    string			strSource = "";		
	const char		*pSource;						
	size_t			uiArrSourceSize[] = { 0 };			
	cl_program		Program = NULL;			
	cl_mem			inputA = NULL;			
	cl_mem			output = NULL;			
	cl_kernel		Kernel = NULL;				



	// query the platform
	iStatus = clGetPlatformIDs(0, NULL, &uiNumPlatforms);
	if (CL_SUCCESS != iStatus)
	{
		cout << "Error: Getting platforms error" << endl;
		return 0;
	}


	// get platform
	if (uiNumPlatforms > 0)  
	{
		
		cl_platform_id *pPlatforms = (cl_platform_id *)malloc(uiNumPlatforms * sizeof(cl_platform_id));

		// get the platform
		iStatus = clGetPlatformIDs(uiNumPlatforms, pPlatforms, NULL);
		Platform = pPlatforms[0];	// get the first platform
		free(pPlatforms);			// release space
	}


	iErr = clGetPlatformInfo(Platform, CL_PLATFORM_VERSION, 0, NULL, &uiSize);


	pName = (char *)alloca(uiSize * sizeof(char));

	// get version name
	iErr = clGetPlatformInfo(Platform, CL_PLATFORM_VERSION, uiSize, pName, NULL);
	cout << pName << endl;



	/*try to get Gpu device*/
	iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);
	if (uiNumDevices == 0)	
	{
		cout << "Error: No GPU device available." << endl;
		return 0;
	}
	else
	{
		pDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id));

		iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, uiNumDevices, pDevices, NULL);
	}


	/*create context*/
	Context = clCreateContext(NULL, 1, pDevices, NULL, NULL, NULL);
	if (Context == NULL)
	{
		cout << "Error: Can not create context" << endl;
		return 0;
	}

	/*create command queue*/
	CommandQueue = clCreateCommandQueue(Context, pDevices[0], 0, NULL);
	if (NULL == CommandQueue)
	{
		cout << "Error: Can not create CommandQueue" << endl;
		return 0;
	}


	/*create program*/
	iStatus = ConvertToString(pFileName, strSource);

	pSource = strSource.c_str();			
	uiArrSourceSize[0] = strlen(pSource);	

	
	Program = clCreateProgramWithSource(Context, 1, &pSource, uiArrSourceSize, NULL);
	if (Program  == NULL)
	{
		cout << "Error: Can not create program" << endl;
		return 0;
	}

	/*compile program*/
	iStatus = clBuildProgram(Program, 1, pDevices, NULL, NULL, NULL);
	if (iStatus != CL_SUCCESS)	
	{
		cout << "Error: Can not build program" << endl;
		char szBuildLog[16384];
		clGetProgramBuildInfo(Program, *pDevices, CL_PROGRAM_BUILD_LOG, sizeof(szBuildLog), szBuildLog, NULL);

		cout << "Error in Kernel: " << endl << szBuildLog;
		clReleaseProgram(Program);
		cout << xsize << endl;
		cout << ysize << endl;
		system("pause");
		return 0;
	}

	/*create input and output buffer*/
	Kernel = clCreateKernel(Program,"sobel", NULL); //kernel enter
	if (NULL == Kernel)
	{
		cout << "Error: Can not create kernel" << endl;
		return 0;
	}


	inputA = clCreateBuffer(Context, CL_MEM_READ_ONLY, DATA_SIZE*sizeof(int), NULL, NULL);
	output = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, DATA_SIZE* sizeof(int), NULL, NULL);

	/*load data into buffer*/
	clEnqueueWriteBuffer(CommandQueue, inputA, CL_TRUE, 0, sizeof(int) * DATA_SIZE, pic, 0, NULL, NULL);

	/*set argument list to kerne command*/
	clSetKernelArg(Kernel, 0, sizeof(int), &xsize);
	clSetKernelArg(Kernel, 1, sizeof(int), &ysize);
	clSetKernelArg(Kernel, 2, sizeof(int), &thresh);
	clSetKernelArg(Kernel, 3, sizeof(cl_mem), &inputA);
	clSetKernelArg(Kernel, 4, sizeof(cl_mem), &output);

	size_t global_work_size[2] = { (ysize - 1), (xsize - 1) };
	
	iStatus = clEnqueueNDRangeKernel(CommandQueue,Kernel,2,NULL,global_work_size,  NULL,0,NULL,NULL);
	if (CL_SUCCESS != iStatus)
	{
		cout << "Error: Can not run kernel" << endl;
		system("pause");
		return 0;
	}

	clFinish(CommandQueue);

	/*copy data from output buffer to result*/
	iStatus = clEnqueueReadBuffer(CommandQueue,	output,	CL_TRUE,0,sizeof(int) * DATA_SIZE,result,0,NULL,NULL);
	 if (CL_SUCCESS != iStatus)
	 {
		 cout << "Error: Can not reading result buffer" << endl;
		 system("pause");
		 return 0;
	 }

	

	 cout << xsize << endl;
	 cout << ysize << endl;


	 write_ppm("result.ppm", xsize, ysize, 255, result);
	 cout << "sobel done" << endl;
	


	 /*release*/
	clReleaseKernel(Kernel);
	clReleaseProgram(Program);
	clReleaseMemObject(inputA);
	clReleaseMemObject(output);
	clReleaseCommandQueue(CommandQueue);
    clReleaseContext(Context);


	system("pause");
	return 0;
}

/*convert cl file into string*/
cl_int ConvertToString(const char *pFileName, std::string &Str)
{
	size_t		uiSize = 0;
	size_t		uiFileSize = 0;
	char		*pStr = NULL;
	std::fstream fFile(pFileName, (std::fstream::in | std::fstream::binary));


	if (fFile.is_open())
	{
		fFile.seekg(0, std::fstream::end);
		uiSize = uiFileSize = (size_t)fFile.tellg();  
		fFile.seekg(0, std::fstream::beg);
		pStr = new char[uiSize + 1];

		if (NULL == pStr)
		{
			fFile.close();
			return 0;
		}

		fFile.read(pStr, uiFileSize);				
		fFile.close();
		pStr[uiSize] = '\0';
		Str = pStr;

		delete[] pStr;

		return 0;
	}

	cout << "Error: Failed to open cl file\n:" << pFileName << endl;

	return -1;
}






