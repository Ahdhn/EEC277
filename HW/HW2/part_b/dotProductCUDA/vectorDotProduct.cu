//Perform two float vector dot product on gpu and cpu and produce the average absolute error 
//By: Ahmed H. Mahmoud

#include <stdio.h>
#include <cuda_runtime.h>
#include <conio.h>

__global__ void
DorProduct(const double *d_vect1, const double *d_vect2, int len, double*d_res)
{
	//we just launch one register and let it do all the work through a loop

    //int idx = blockDim.x * blockIdx.x + threadIdx.x;	
	//printf("\n idx =%d", idx);
	d_res[0] = 0;
	for (size_t i = 0; i < len; i++){		
		d_res[0] += d_vect1[i] * d_vect2[i];
	}
}


int main(void)
{
	// set the device to my GT 610
	int deviceCount;
	cudaGetDeviceCount(&deviceCount);
	int dev = 1;
	cudaSetDevice(dev);	
	    
	//length of vectors 
	int len = 100000;	
	size_t size = len * sizeof(double);

    //allocate stuff on the host     
	double*h_vect1 = new double[len];
	double*h_vect2 = new double[len];
	double*h_res = new double[1];
	
	srand ( time(NULL) ); // activate for different experiments
    //initialize stuff on the host
	for (int i = 0; i < len; ++i){ 
		h_vect1[i] = double(rand() / (double)RAND_MAX);
		h_vect2[i] = double(rand() / (double)RAND_MAX);
    }
	h_res[0] = 0;
	

	// Error code to check return values for CUDA calls
	cudaError_t err = cudaSuccess;


	//allocate stuff on the device     
	double *d_vect1 = NULL;
    err = cudaMalloc((void **)&d_vect1, size);
    if (err != cudaSuccess){
		fprintf(stderr, "Failed to allocate device d_vect1 (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
		
	double *d_vect2 = NULL;
    err = cudaMalloc((void **)&d_vect2, size);
	if (err != cudaSuccess){ 
        fprintf(stderr, "Failed to allocate device d_vect2 (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

	// Allocate result on GPU
	double*d_res = NULL;
	err = cudaMalloc((void **)&d_res, sizeof(double));
	if (err != cudaSuccess){
		fprintf(stderr, "Failed to allocate device d_res (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	   
    // Copy the from host memory to the device memory 
    err = cudaMemcpy(d_vect1, h_vect1, size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy h_vect1 from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_vect2, h_vect2, size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy h_vect2 from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

	err = cudaMemcpy(d_res, h_res, sizeof(double), cudaMemcpyHostToDevice);//useless but meh
	if (err != cudaSuccess){
		fprintf(stderr, "Failed to copy h_res from host to device (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

    //launch CUDA Kernel        
	dim3 blockSize(1,1,1), gridSize(1,1,1);	
	DorProduct <<<1, 1 >>>(d_vect1, d_vect2, len, d_res);
    err = cudaGetLastError();

    if (err != cudaSuccess){
        fprintf(stderr, "Failed to launch vectorAdd kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
	cudaDeviceSynchronize();
		
	
	err = cudaMemcpy(h_res, d_res, sizeof(double), cudaMemcpyDeviceToHost);
	if (err != cudaSuccess){
		fprintf(stderr, "Failed to copy d_res from device to host (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

    //get cpu results 	
	double *inter_vect = new double[len];
	//make sure it is not converted into mulitply-add 
	double cpu_res(0);
	for (int i = 0; i < len; ++i){ 
		inter_vect[i] = h_vect1[i] * h_vect2[i];	
		//cpu_res += h_vect1[i] * h_vect2[i];
    }
	
	for (int i = 0; i < len; ++i){
		cpu_res += inter_vect[i];
	}    

    // Free device global memory
    err = cudaFree(d_vect1);
    if (err != cudaSuccess){
		fprintf(stderr, "Failed to free device d_vect1 (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

    err = cudaFree(d_vect2);
    if (err != cudaSuccess){
		fprintf(stderr, "Failed to free device d_vect2 (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	    
    // Free host memory
    free(h_vect1);
    free(h_vect2);    
	//free(inter_vect);
	    
    err = cudaDeviceReset();

    if (err != cudaSuccess){
        fprintf(stderr, "Failed to deinitialize the device! error=%s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }


	/// cal the error
	double error_cpu_gpu = abs(cpu_res - h_res[0]);




    printf("\nDone\n");

	printf("\n\nPress Any Key To Exit!!!");
	_getch();

    return 0;
}

