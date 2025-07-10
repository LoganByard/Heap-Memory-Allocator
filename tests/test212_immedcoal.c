// many odd sized allocations and interspersed frees
// tests pass if immediate coalescing has been implemented
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "p3Heap.h"

int main() {
   assert(init_heap(4096) == 0);
   void * ptr[9];
   ptr[0] = balloc(1);
   ptr[1] = (balloc(5));
   ptr[2] = (balloc(8));
   void *p3 = ptr[3] = (balloc(14));
   assert(ptr[0] != NULL);
   assert(ptr[1] != NULL);
   assert(ptr[2] != NULL);
   assert(ptr[3] != NULL);
   
   assert(bfree(ptr[1]) == 0);
   assert(bfree(ptr[0]) == 0);
   assert(bfree(ptr[3]) == 0);
   
   assert((ptr[3] = balloc(23)) == p3);

   ptr[4] = (balloc(1));
   ptr[5] = (balloc(4));
   assert(ptr[4] != NULL);
   assert(ptr[5] != NULL);
   assert(bfree(ptr[5]) == 0);
   
   ptr[6] = (balloc(9));
   assert(ptr[6] != NULL);
   assert(ptr[6]==ptr[5]); // bc freeing ptr[5] coalesced with remaining free space

   ptr[7] = (balloc(33));
   assert(ptr[7] != NULL);
   
   assert(bfree(ptr[4]) == 0); // no coalesce

   ptr[8] = (balloc(55));
   assert(ptr[8] != NULL);

   assert(bfree(ptr[2]) == 0); // coalesce with prev free block arnheap start
   assert(bfree(ptr[7]) == 0); // coalesce with next free block
   assert(bfree(ptr[6]) == 0); // coalese with next free block

   // should find a best fit for balloc(5) before ptr[8]
   assert((ptr[5]=balloc(5)) < ptr[8]); // ptr[5] should find first free block

   exit(0);
}
