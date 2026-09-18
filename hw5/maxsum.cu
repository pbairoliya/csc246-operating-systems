// Elapsed Real Time for input-5.txt: 
  //real	0m1.668s
  //user	0m0.049s
  //sys	  0m1.375s
// Type of GPU:rtx2070


/**
 * @file maxsum.cu cuda version of max sum program allows to calculate larger array mcuh faster
 * @author Pratik Bairoliya pbairol@ncsu.edu
 * @date 2022-11-13
 * HW 5 
 */


#include <stdio.h>
#include <stdbool.h>
#include <cuda_runtime.h>

// Input sequence of values.
int *vList;

// Number of values on the list.
int vCount = 0;

// Capacity of the list of values.
int vCap = 0;

// General function to report a failure and exit.
static void fail(char const *message)
{
  fprintf(stderr, "%s\n", message);
  exit(1);
}

// Print out a usage message, then exit.
static void usage()
{
  printf("usage: maxsum [report]\n");
  exit(1);
}

// Read the list of values.
__host__ void readList()
{
  // Set up initial list and capacity.
  vCap = 5;
  vList = (int *)malloc(vCap * sizeof(int));

  // Keep reading as many values as we can.
  int v;
  while (scanf("%d", &v) == 1)
  {
    // Grow the list if needed.
    if (vCount >= vCap)
    {
      vCap *= 2;
      vList = (int *)realloc(vList, vCap * sizeof(int));
    }

    // Store the latest value in the next array slot.
    vList[vCount++] = v;
  }
}


/**
 * @brief checksum method that is ran on the gpu each thread runs this method and finds its own contiguous maximum sum
 * 
 * @param cudaList the list on the device with all the values of the array we are finidng the maximum sum of
 * @param resultsList the maximum sum each thread found put into the array
 * @param vCount the count of integers in the vlist
 * @param report  the boolean value that tells us if we want to report the thread max sum
 * @return __global__ 
 */
__global__ void checkSum(int *cudaList, int *resultsList, int vCount, bool report)
{

  int idx = blockDim.x * blockIdx.x + threadIdx.x;
  int currMax = 0;
  int maxSum = 0;
  if (idx < vCount)
  {
    currMax = 0;
    for (int i = idx; i < vCount; i++)
    {
      currMax += cudaList[i];

      if (maxSum < currMax)
      {
        maxSum = currMax;
      }
    }
    if (report)
    {
      printf("I'm thread %d. The maximum sum I found is %d.\n", idx, maxSum);
    }
    resultsList[idx] = maxSum;
  }
}


/**
 * main method that is ran on the host and then calls global function that is ran on the gpu
 * 
 * @param argc number of arguments
 * @param argv the arguemnts itself in a string array
 * @return int the exit status of the program
 */
int main(int argc, char *argv[])
{
  // error messaging 
  if (argc < 1 || argc > 2)
    usage();

  // If there's an argument, it better be "report"
  bool report = false;
  if (argc == 2)
  {
    if (strcmp(argv[1], "report") != 0)
      usage();
    report = true;
  }

  //reading in the values from the txt file into vlist
  readList();

  // Add code to allocate memory on the device and copy over the list.
  int *cudaList = NULL;
  //gpu allocation of the cuda list
  cudaMalloc((void **)&cudaList, vCap * sizeof(int));
  //copying host vlist to gpu cuda list
  cudaMemcpy(cudaList, vList, vCap * sizeof(int), cudaMemcpyHostToDevice);

  // Add code to copy the list over to the device.

  // Block and grid dimensions.
  int threadsPerBlock = 100;
  // Round up for the number of blocks we need.
  int blocksPerGrid = (vCount + threadsPerBlock - 1) / threadsPerBlock;

  // Add code to allocate space on the device to hold the results.
  int *resultsList = NULL;
  cudaMalloc((void **)&resultsList, vCount * sizeof(int));

  // Run our kernel on these block/grid dimensions (you'll need to add some parameters)
  checkSum<<<blocksPerGrid, threadsPerBlock>>>(cudaList, resultsList, vCount, report);

  //checking if cuda connection is succesful
  if (cudaGetLastError() != cudaSuccess)
    fail("Failure in CUDA kernel execution.");

  // allocating the host list 
  int *hostResultsList = (int *)malloc(vCount * sizeof(int));
  //copying device list to host list 
  cudaMemcpy(hostResultsList, resultsList, vCount * sizeof(int), cudaMemcpyDeviceToHost);

  // and report the final largest product
  int actualMaxSum = 0;
  //algorithm to find the largest maxsum in the hostReuslts list
  for (int i = 0; i < vCount; i++)
  {
    if (actualMaxSum <= hostResultsList[i])
    {
      actualMaxSum = hostResultsList[i];
    }
  }
  //print the maximum sum
  printf("Maximum Sum: %d\n", actualMaxSum);

  // Free memory on the device and the host.
  cudaFree(resultsList);
  cudaFree(cudaList);
  free(vList);
  free(hostResultsList);
  cudaDeviceReset();

  return 0;
}
