#include "Ackerman.h"
#include "BuddyAllocator.h"
#include <getopt.h>
#include <string>  

void easytest(BuddyAllocator* ba){
  // be creative here
  // know what to expect after every allocation/deallocation cycle

  // here are a few examples
  ba->printlist();
  // allocating a byte




 char * mem = ba->alloc (100);
 
  // now print again, how should the list look now
  ba->printlist ();
  char * mem2 = ba->alloc(100);
  ba->printlist ();
  char * mem3 = ba->alloc(100);
  ba->printlist ();
  char * mem4 = ba->alloc(100);
  ba->printlist ();

  
  int a = ba->free(mem);
  ba->printlist ();
  int b = ba->free(mem2);
  ba->printlist ();
  b= ba->free(mem3);
  ba->printlist ();
  b= ba->free(mem4);
    ba->printlist ();

 

  // ba->free (mem); // give back the memory you just allocated
 // shouldn't the list now look like as in the beginning

}

int main(int argc, char ** argv) {

  int basic_block_size = 128, memory_length = 256*1024*1024;
  int M;
  int a=0;

  while(( M =getopt(argc,argv,"b:s:")) != -1){
	  switch (M){
		  case 'b' :
			a =atof(optarg);
			if(a<basic_block_size){
				break;
			}
			else{
				basic_block_size = a;
			}
			break;
		  case 's' :
			a = atof(optarg);

			if(a<memory_length){
				
				break;
			}
			else {
				memory_length = a;
				
			}
			break;
	  } 
  }
  // create memory manager
  BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);

  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  easytest (allocator);
  delete allocator;
  BuddyAllocator * allocator1 = new BuddyAllocator(basic_block_size, memory_length);

  // stress-test the memory manager, do this only after you are done with small test cases
  Ackerman* am = new Ackerman ();
  am->test(allocator1); // this is the full-fledged test. 
  
  // destroy memory manager
  delete allocator1;
}
