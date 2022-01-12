#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void parallelMerge(int *, int *, int, int, int);
void parallelMergeSort(int *, int *, int, int);

void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    /* create temp arrays */
    int L[n1], R[n2];

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there
    are any */
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    /* Copy the remaining elements of R[], if there
    are any */
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

/* l is for left index and r is right index of the
sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r)
{
    if (l < r) {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l + (r - l) / 2;

        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        merge(arr, l, m, r);
    }
}

void printArray(int A[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}

int main(int argc, char** argv) {
	
	/********** Create and populate the array **********/
	int n = atoi(argv[1]);
	int *original_array = malloc(n * sizeof(int));
	
	int c;
	srand(time(NULL));
	printf("This is the unsorted array: ");
	for(c = 0; c < n; c++) {
		
		original_array[c] = rand() % n;
		printf("%d ", original_array[c]);
		
		}

	printf("\n");
	printf("\n");
	
	/********** Initialize MPI **********/
	int world_rank;
	int world_size;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		
	/********** Divide the array in equal-sized chunks **********/
	int size = n/world_size;
	
	/********** Send each subarray to each process **********/
	int *sub_array = malloc(size * sizeof(int));
	MPI_Scatter(original_array, size, MPI_INT, sub_array, size, MPI_INT, 0, MPI_COMM_WORLD);
	
	/********** Perform the mergesort on each process **********/
	int *tmp_array = malloc(size * sizeof(int));
	parallelMergeSort(sub_array, tmp_array, 0, (size - 1));
	
	/********** Gather the sorted subarrays into one **********/
	int *sorted = NULL;
	if(world_rank == 0) {
		
		sorted = malloc(n * sizeof(int));
		
		}
	
	MPI_Gather(sub_array, size, MPI_INT, sorted, size, MPI_INT, 0, MPI_COMM_WORLD);
	
	/********** Make the final mergeSort call **********/
	clock_t begining,end;
	if(world_rank == 0) {
		
		int *other_array = malloc(n * sizeof(int));
		begining = clock();
		parallelMergeSort(sorted, other_array, 0, (n - 1));
		end = clock();
		/********** Display the sorted array **********/
		printf("This is the sorted array: ");
		for(c = 0; c < n; c++) {
			
			printf("%d ", sorted[c]);
			
			}
			
		printf("\n");
		printf("\n");
			
		/********** Clean up root **********/
		free(sorted);
		free(other_array);
			
		}
	
	/********** Finalize MPI **********/
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	float elapsed = (float)(end - begining) / CLOCKS_PER_SEC * 1000;
	printf("Parallel needed %f miliseconds \n\n", elapsed);	
	
	printf("Given array is \n");
    	printArray(original_array, n);
  	clock_t s_begining = clock();
    	mergeSort(original_array, 0, n - 1);
  	clock_t s_end = clock();
    	printf("\nSorted array is \n");
    	printArray(original_array, n);	

	float s_elapsed = (float)(s_end - s_begining) / CLOCKS_PER_SEC * 1000;
        printf("Serial needed %f miliseconds \n\n", s_elapsed);

	free(original_array);
        free(sub_array);
        free(tmp_array);
}

/********** Merge Function **********/
void parallelMerge(int *a, int *b, int l, int m, int r) {
	int h, i, j, k;
	h = l;
	i = l;
	j = m + 1;
	
	while((h <= m) && (j <= r)) {
		
		if(a[h] <= a[j]) {
			
			b[i] = a[h];
			h++;
			
			}
			
		else {
			
			b[i] = a[j];
			j++;
			
			}
			
		i++;
		
		}
		
	if(m < h) {
		
		for(k = j; k <= r; k++) {
			
			b[i] = a[k];
			i++;
			
			}
			
		}
		
	else {
		
		for(k = h; k <= m; k++) {
			
			b[i] = a[k];
			i++;
			
			}
			
		}
		
	for(k = l; k <= r; k++) {
		
		a[k] = b[k];
		
		}
		
	}

/********** Recursive Merge Function **********/
void parallelMergeSort(int *a, int *b, int l, int r) {
	int m;
	
	if(l < r) {
		
		m = (l + r)/2;
		
		parallelMergeSort(a, b, l, m);
		parallelMergeSort(a, b, (m + 1), r);
		parallelMerge(a, b, l, m, r);
		
	}
		
}
