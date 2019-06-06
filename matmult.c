/*
 * Matrix multiplication with and without blocking.
 *
 * Intel® Core™ i5-6600 CPU @ 3.30GHz
 *
 * $ ./matmult -n 1024 -v 0
 * Time elapsed: 3.052755 seconds.
 * $ ./matmult -n 1024 -v 1
 * Time elapsed: 0.746337 seconds.
 * $ ./matmult -n 1024 -v 2
 * Time elapsed: 9.882309 seconds.
 * $ ./matmult -n 1024 -v 3
 * Time elapsed: 0.698795 seconds.
 */
#include "common.h"

/* You can modify following definitions to try out different settings. */
#define T double
#define BLOCK 16

/* Break matrices start alignment with following offsets: */
#define A_OFFSET NITEMS(BLOCK_SIZE * 2, T)
#define B_OFFSET NITEMS(BLOCK_SIZE * 1, T)
#define C_OFFSET NITEMS(BLOCK_SIZE * 0, T)

/* Useful macro for accessing row-major 2D arrays of size n×n. */
#define M(a, i, j) a[(i) * n + (j)]

typedef T *restrict T_p;

static void fill(T *dst, int size) {
  for (int i = 0; i < size; i++)
      dst[i] = 1;
}

/* ijk (& jik) */
static __noinline void multiply0(int n, T_p a, T_p b, T_p c) {
  double sum = 0.0;

  for(int i=0; i<n; i++) {
    for(int j=0; j<n; j++) {
      sum = 0.0;
      for(int k=0; k<n; k++) {
        sum += M(a, i, k) * M(b, k, j);
      }
      M(c, i, j) = sum;
    }
  }

}

/* kij (& ikj) */
static __noinline void multiply1(int n, T_p a, T_p b, T_p c) {
  double r;

  for(int k=0; k<n; k++) {
    for(int i=0; i<n; i++) {
      r = M(a, i, k);
      for(int j=0; j<n; j++) {
        M(c, i, j) += r * M(b, k, j);
      }
    }
  }
}

/* jki (& kji) */
static __noinline void multiply2(int n, T_p a, T_p b, T_p c) {
  double r;

  for(int j=0; j<n; j++) {
    for(int k=0; k<n; k++) {
      r = M(b, k, j);
      for(int i=0; i<n; i++) {
        M(c, i, j) += M(a, i, k) * r;
      }
    }
  }
}

/* BLOCK*BLOCK tiled version */
static __noinline void multiply3(int n, T_p a, T_p b, T_p c) {

  for (int i = 0; i < n; i += BLOCK)
  {
    for (int j = 0; j < n; j += BLOCK)
    {
      for (int k = 0; k < n; k += BLOCK)
      {
        /* BLOCK * BLOCK elements mini matrix multiplication */
        for (int i1 = i; i1 < i + BLOCK; i1++)
        {
          for (int j1 = j; j1 < j + BLOCK; j1++)
          {
            for (int k1 = k; k1 < k + BLOCK; k1++)
            {
              M(c, i1, j1) += M(a, i1, k1) * M(b, k1, j1);
            }
          }
        }
      }
    }
  }
}

typedef void (*matmult_t)(int n, T *a, T *b, T *c);

static matmult_t multiply[4] = {multiply0, multiply1, multiply2, multiply3};

int main(int argc, char **argv) {
  int opt, variant = -1, n = 0;
  bool err = false;

  while ((opt = getopt(argc, argv, "n:v:")) != -1) {
    if (opt == 'n')
      n = atoi(optarg);
    else if (opt == 'v')
      variant = atoi(optarg);
    else
      err = true;
  }

  if (err || n == 0 || variant < 0 || variant >= 4)
    fail("Usage: %s -n size -v variant\n", argv[0]);

  if (n % BLOCK)
    fail("Matrix size (%d) must be divisible by %d!", n, BLOCK);

  size_t size = n * n * sizeof(T);
  size_t pagesize = getpagesize();

  /* Allocate space for each matrix plus extra space (of page size). */
  T *a = malloc_page_aligned(size + pagesize);
  T *b = malloc_page_aligned(size + pagesize);
  T *c = malloc_page_aligned(size + pagesize);

  printf("Generate 2 matrices %d x %d (%ld KiB each)\n", n, n, size >> 10);

  fill(a, n * n + NITEMS(pagesize, T));
  fill(b, n * n + NITEMS(pagesize, T));
  bzero(c, size + pagesize);
  flush_cache();

  printf("Performing matrix multiplication.\n");

  _timer_t timer;
  timer_reset(&timer);
  timer_start(&timer);
  multiply[variant](n, a + A_OFFSET, b + B_OFFSET, c + C_OFFSET);
  timer_stop(&timer);
  timer_print(&timer);

  free(a);
  free(b);
  free(c);

  return 0;
}
