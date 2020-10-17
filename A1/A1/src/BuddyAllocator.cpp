#include "BuddyAllocator.h"
#include <iostream>
#include <cmath>
#include <algorithm>
using namespace std;

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
	Start = new char[_total_memory_length]; // 
	cout<<Start<<endl;
	basic_block_size = _basic_block_size;
	total_memory_size = _total_memory_length;
	int k;
	k = ceil(log2((_total_memory_length*1.0)/(_basic_block_size)))+1;
	FreeList.resize(k);
	FreeList[k-1].head = (BlockHeader*) Start; //Tpye cast, convert pointer
	FreeList[k-1].head->block_size = _total_memory_length;
	FreeList[k-1].head->next = NULL;
	
	for (int i = 0; i < k-1 ; i++){
		FreeList[i].head = NULL;
	
	}
	
}

BuddyAllocator::~BuddyAllocator (){
	delete Start;
}

char* BuddyAllocator::alloc(int _length) {
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
  int x=_length+sizeof(BlockHeader);
  if(x>total_memory_size){
	  return 0;
  }
  unsigned int BS = ceil(log2(x/basic_block_size))+1;
  BlockHeader* H;
  if(FreeList[BS].head){
    H = FreeList[BS].head;
	FreeList[BS].remove(H);
	return (char*) H + sizeof(BlockHeader);;	  
  }
  unsigned int j = BS;
  
  while(FreeList[BS].head == 0 && BS < FreeList.size()){ 
	j++;
	
	if(j>FreeList.size()){ 
		
		return 0;
	}
		
		while(j>BS && j<FreeList.size()){
			if(!FreeList[j].head){j++;continue;}
			
			BlockHeader* Old = FreeList[j].head;
			BlockHeader* New = this->split(Old);
			FreeList[j].remove(Old);
			FreeList[j-1].insert(New);
			FreeList[j-1].insert(Old);
			
			
			j--;
			}
			
		
  } 
  H = FreeList[BS].head;

  FreeList[BS].remove(H);

  char* T = (char*) H ;
  T = T + sizeof(BlockHeader);
  
   return T;
}

int BuddyAllocator::free(char* _a) {
  char* N = _a - sizeof(BlockHeader);
  BlockHeader* Block = (BlockHeader*) N;
  int index = log2(Block->block_size/basic_block_size);
  if(index<0){index=0;}
  BlockHeader* Block1 = FreeList[index].head;
 
  FreeList[index].insert(Block);
  
  bool Done = false;
   BlockHeader* Curr = FreeList[index].head;
   Block = Curr->next;
  while(!Done && index < FreeList.size()){
	  Done = true;
	   
	  
	  while(Curr->next != NULL){
		  Block = Curr->next;
		  if(arebuddies(Curr,Block)){
			  Done = false;
			   
			  BlockHeader* Big = merge(Curr,Block);
			  FreeList[index].remove(Curr);

			  FreeList[index].remove(Block);
			  
			  Big->next = NULL;
			  FreeList[index+1].insert(Big);
			
			  index++;
			
			  if(!FreeList[index].head){break;}
			  Curr = FreeList[index].head;
	
			  Block=NULL;
			  
		  }
		  else{
		 
		  Curr = Curr->next;
		  }
	  }
	  
	  
  }
 
}

BlockHeader* BuddyAllocator::split(BlockHeader* Old){

	char* New = (char*) Old;
	int tempS= Old->block_size / 2;
	New += tempS;
	Old->block_size = Old->block_size / 2 ;
	BlockHeader* New2 = (BlockHeader*) New;
	
	New2->block_size = tempS;
	return New2;
	
}

bool BuddyAllocator::arebuddies (BlockHeader* block1, BlockHeader* block2){
	int index =log2(block1->block_size/basic_block_size);
	
	BlockHeader* Curr = FreeList[index].head;
	char* B1 = (char*) block1;
	char* B2 = (char*) block2;
	if(!block2){return false;}
	
	char* startingPoint = Start;
	
	
	int Buddy = basic_block_size*pow(2,index); //offset of possible buddies
	//check if its a left or right block, to check for the correct possible buddy
	// cehck if the possition in memory relative to the number of possible n sized block is odd or even 
	
	
	//With the
	if( (B1 - startingPoint) == 0  ){
		
		 if((B1 + block2->block_size) == B2){
			return true; 
		 }
	
	}
	else if( (B2 - startingPoint) == 0  ){
		
		 if((B2 + block2->block_size) == B1){
			return true; 
		 }
	
	}
	
	
	else if( ((B1 - startingPoint) / (block2->block_size) % 2 ==0)){
		
		 //odd position look for a buddy to the right 
		if( (B1 + block2->block_size) == B2){
			
			return true;
		}
	}
	
	else if (((B1 - startingPoint) / (block2->block_size) % 2 !=0)){
		//odd position look for a buddy in the left 
		if( (B1 - block2->block_size) == B2){

			return true;
		}
		
	}
	return false;
}

BlockHeader* BuddyAllocator::merge (BlockHeader* block1, BlockHeader* block2){
	BlockHeader* New;
	int S = block1->block_size*2;
	if(block1>block2){
		New = block2;
	}
	else{
		New=block1;
	}	
	New->block_size=S;
	
	
	return New;
}

void BuddyAllocator::printlist (){
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  for (int i=0; i<FreeList.size(); i++){
    cout << "[" << i <<"] (" << ((1<<i) * basic_block_size) << ") : ";  // block size at index should always be 2^i * bbs
    int count = 0;
    BlockHeader* b = FreeList [i].head;
    // go through the list from head to tail and count
    while (b){
      count ++;
      // block size at index should always be 2^i * bbs
      // checking to make sure that the block is not out of place
      if (b->block_size != (1<<i) * basic_block_size){
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit (-1);
      }
      b = b->next;
    }
    cout << count << endl;  
  }
}