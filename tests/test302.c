// check for coalesce free space
#include <assert.h>
#include <stdlib.h>
#include "p3Heap.h"

int main() {
   assert(init_heap(4096) == 0);
   void * ptr[4];

   ptr[0] = balloc(800);
   assert(ptr[0] != NULL);

   ptr[1] = balloc(800);
   assert(ptr[1] != NULL);

   ptr[2] = balloc(800);
   assert(ptr[2] != NULL);

   ptr[3] = balloc(800);
   assert(ptr[3] != NULL);

   while (balloc(800) != NULL)
     ;

   assert(bfree(ptr[2]) == 0);
   ptr[2] = balloc(2400);
   assert(ptr[2] == NULL);   // alloc fails

   assert(bfree(ptr[1]) == 0);
   ptr[2] = balloc(2400);
   assert(ptr[2] == NULL);  // alloc fails
   ptr[2] = balloc(1600);
   assert(ptr[2] != NULL);  // alloc succeeds

   assert(bfree(ptr[2]) == 0);
   assert(bfree(ptr[3]) == 0);
   ptr[2] = balloc(2400);
   assert(ptr[2] != NULL);

   exit(0);
}
