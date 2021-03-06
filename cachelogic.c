#include "tips.h"

/* The following two functions are defined in util.c */

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w); 

/* return random int from 0..x-1 */
int randomint( int x );

/*
	This function allows the lfu information to be displayed

	assoc_index - the cache unit that contains the block to be modified
	block_index - the index of the block to be modified

	returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
	/* Buffer to print lfu information -- increase size as needed. */
	static char buffer[9];
	sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);

	return buffer;
}

/*
	This function allows the lru information to be displayed

	assoc_index - the cache unit that contains the block to be modified
	block_index - the index of the block to be modified

	returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
	/* Buffer to print lru information -- increase size as needed. */
	static char buffer[9];
	sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);

	return buffer;
}

/*
	This function initializes the lfu information

	assoc_index - the cache unit that contains the block to be modified
	block_number - the index of the block to be modified

*/
void init_lfu(int assoc_index, int block_index)
{
	cache[assoc_index].block[block_index].accessCount = 0;
}

/*
	This function initializes the lru information

	assoc_index - the cache unit that contains the block to be modified
	block_number - the index of the block to be modified

*/
void init_lru(int assoc_index, int block_index)
{
	cache[assoc_index].block[block_index].lru.value = block_index;	// initialize lru value to block index, so that each block will have a different lru value. This means that each block should have a unique lru value, and there will always be a minimum least recently used block
}

//int readWriteCount = 0;	// counts how many reads have been done so far. Used for finding LRU info

// returns a pointer to the block to replace
cacheBlock * replacementPolicy(cacheSet * set) {
	cacheBlock * toReplace = NULL;
	switch (policy) {
	case RANDOM:
		toReplace = &(set->block[randomint(assoc)]);	//(cacheBlock *)set[randomint(assoc)];
		break;

	case LRU:	// find the block with the least lru.value readWriteCount Stamp
		toReplace = (cacheBlock *)(set->block);	//set[0];
		for (int i = 1; i < assoc; i++) {	// there are "assoc" number of blocks in a set
											//lru.value will count which read of the system when that block was last read
											//readWriteCount counts how many reads the system has done
											// when a read is done, the block(s) read have their lru.value set to readWriteCount. readWriteCount is incremented
			if (set->block[i].lru.value < toReplace->lru.value) {
				toReplace = &(set->block[i]);
			}
		}
		// When code gets here, toReplace is a pointer to the LRU block

		break;

	//No need to implement, oh well.
	case LFU:	// find the block with the least accessCount stamp 
		toReplace = &(set->block[0]);
		for (int i = 1; i < assoc; i++) {	// there are "assoc" number of blocks in a set
											//lru.value will count which read of the system when that block was last read
											//readWriteCount counts how many reads the system has done
											// when a read is done, the block(s) read have their lru.value set to readWriteCount. readWriteCount is incremented
			if (set->block[i].accessCount < toReplace->accessCount) {
				toReplace = &(set->block[i]);
			}
		}
		// When code gets here, toReplace is a pointer to the LFU block
		break;

	default:
		printf("Error: unknown replacement policy!!!");
		return NULL;
	}
	return toReplace;
}

void decrementLRUs(cacheSet * set, int value)
{
	for (int i = 0; i < assoc; i++)
	{
		if (set->block[i].lru.value > value)
			set->block[i].lru.value--;
	}
}

/*
	This is the primary function you are filling out,
	You are free to add helper functions if you need them

	@param addr 32-bit byte address
	@param data a pointer to a SINGLE word (32-bits of data)
	@param we	if we == READ, then data used to return
				information back to CPU

				if we == WRITE, then data used to
				update Cache/DRAM
*/
void accessMemory(address addr, word* data, WriteEnable we) {
	/* Declare variables here */
	//readWriteCount ++;
	/* handle the case of no cache at all - leave this in */
	if(assoc == 0) {
		accessDRAM(addr, (byte*)data, WORD_SIZE, we);
		return;
	}
	
	 // How many bits are in the (physical) address.
	 // If the cache is direct mapped, then 
	unsigned int offsetLength = uint_log2(block_size);
	unsigned int indexLength  = uint_log2(set_count);
	unsigned int tagLength    = 32 - indexLength - offsetLength;
	
	unsigned int offsetMask =   (1<<(offsetLength)) - 1;
	unsigned int indexMask  =  ((1<<(indexLength )) - 1) << offsetLength;
	unsigned int tagMask    = (((1<<(tagLength   )) - 1) << offsetLength) << indexLength;
	
	printf ("Address is as such: 0x%x\n", addr);
	printf ("   %d bit tag    %d bit index    %d bit offset\n", tagLength, indexLength, offsetLength);
	
	
	/*
	if (setCount == 1) {
		//fully associative
		
	} else if (setCount == assoc) {
		//Direct mapped
		
	}
	*/
	unsigned int tag, index, offset;
	offset = addr & offsetMask;
	index = (addr & indexMask ) >> offsetLength;	//(addr >> uint_log2(block_size)) & uint_log2(set_count); //First "substring" address to block_size
	tag =  ((addr & tagMask   ) >> offsetLength) >> indexLength;	//addr >> (uint_log2(block_size) + uint_log2(set_count));
	
	
	
	cacheSet * set = &(cache[index]);	// find which set to access
	cacheBlock * blockToAccess = NULL;
	
	//Find which row of set corresponds to the c
	
	
	
	
	if (we == WRITE) {
		//cache[]
		printf("It's a write!");
		
		
		// FIND WHICH BLOCK TO WRITE TO
		// Do this by: Iterate through set, and check if any block in set is invalid
		for (int i = 0; i < assoc; i++) {
			if (set->block[i].tag == tag) {	// cache hit!!!
				blockToAccess->data[offset    ] = (*data >> 24) & 0xFF;
				blockToAccess->data[offset + 1] = (*data >> 16) & 0xFF;
				blockToAccess->data[offset + 2] = (*data >>  8) & 0xFF;
				blockToAccess->data[offset + 3] = (*data      ) & 0xFF;
				blockToAccess->accessCount += 1;
				decrementLRUs(set, blockToAccess->lru.value);
				blockToAccess->lru.value = assoc - 1;	//readWriteCount++;
				return;
			}
			if (set->block[i].valid == INVALID) {
				// If a block in the set is invalid, then use this to write to
				blockToAccess = &(set->block[i]);
				blockToAccess->valid = VALID;
				goto writeBlockToMem;
			}
		}
		
		// if code gets here, then no block in the set is invalid.
		// So we have to replace a valid block, based on the replacement policy
		//CHOOSE BLOCK TO REPLACE:
		cacheBlock * toReplace = replacementPolicy(set);

		// at this point, we know which block we want to replace. toReplace points to it
		
		
		// REPLACE THE BLOCK toReplace
		// if the block's dirty bit is set, then we must save it to memory before replacing
		if (toReplace->dirty == DIRTY) {
			//save cache block to memory
			
			
			// save every byte in block to replace. Save the entire block from the cache to memory (because it's about to be replaced)
			int addrToSave = (((toReplace->tag)<<indexLength)<<offsetLength) | (index << offsetLength);
			for (int i = 0; i < block_size; i++) {	// number of bytes in the block is block_size
				accessDRAM(addrToSave, (byte*)&(toReplace->data[i]), BYTE_SIZE, WRITE);
				addrToSave = addrToSave + i;
			}
			//at this point, the entire block should be saved to DRAM
			//addrToSave = (tag << indexLength) << offsetLength;
			//addrToSave = //addrToSave | (index << offsetLength);
			blockToAccess->dirty = VIRGIN;
		}
		
		//if (memory_sync_policy == WRITE_BACK) {
			// if th
		//}
		blockToAccess = toReplace;
		
		
		/////////////////////////////WRITE THE BLOCK FROM CPU TO CACHE///////////////////////////////////////////
		writeBlockToMem:	// toReplace is now set to the block to replace

		//blockToAccess->dirty = DIRTY;
		//if (blockToAccess->tag != tag) {	// cache miss
		blockToAccess->tag = tag;
		//}
		
		// LOAD BLOCK FROM MEMORY TO CACHE
		// Load the entire cache block from memory to cache before writing from CPU's data to cache block 
		int addrToSave = addr & (tagMask | indexMask);		// address to save to will have same tag and index as addr
		
		TransferUnit transferUnit = uint_log2(block_size); //Determine how many bytes we need to copy from memory to fill the block.
		accessDRAM(addrToSave, (byte*)&(blockToAccess), transferUnit, READ);
		
		//Write to the block from the CPU's data
		blockToAccess->data[offset    ] = (*data >> 24) & 0xFF;
		blockToAccess->data[offset + 1] = (*data >> 16) & 0xFF;
		blockToAccess->data[offset + 2] = (*data >>  8) & 0xFF;
		blockToAccess->data[offset + 3] = (*data      ) & 0xFF;
		decrementLRUs(set, blockToAccess->lru.value);
		blockToAccess->lru.value = assoc - 1;	//readWriteCount++;
		blockToAccess->accessCount += 1;
		blockToAccess->dirty = DIRTY;
		//for (int i = 0; i < block_size; i++) {
		//	blockToAccess->data[i] = data[i];
		//}
		
		// Write data from CPU to DRAM (it write through policy)
		if (memory_sync_policy == WRITE_THROUGH) {	// if there is a write through policy, then also write the data to DRAM
			accessDRAM (addr, (byte *)data, WORD_SIZE, WRITE);
			blockToAccess->dirty = VIRGIN;
		}
		
		
	} else if (we == READ) {
		printf("It's a read!\n");
		// FIND WHICH BLOCK TO read from
		// Do this by: Iterate through set, and check if any block in set is invalid
		for (int i = 0; i < assoc; i++) 
		{
			if (set->block[i].tag == tag && set->block[i].valid == VALID)
			{
				memcpy(data, &(set->block[i].data[offset]), 4);	// WORD_SIZE corresponds to 4 bytes
				decrementLRUs(set, set->block[i].lru.value);
				set->block[i].lru.value = assoc - 1;	//readWriteCount++;
				set->block[i].accessCount++;
				return;
			}
		}

		printf("Read miss\n");
		//accessDRAM(addr, (byte*)data, WORD_SIZE, READ);

		cacheBlock * toReplace = replacementPolicy(set);

		
		// REPLACE THE BLOCK toReplace
		// if the block's dirty bit is set, then we must save it to memory before replacing
		if (toReplace->dirty == DIRTY) {
			printf("Dirty block\n");
			//save cache block to memory
			
			
			// save every byte in block to replace. Save the entire block from the cache to memory (because it's about to be replaced)
			TransferUnit transferUnit = uint_log2(block_size); //Determine how many bytes we need to copy from memory to fill the block.
			int addrToSave = (((toReplace->tag)<<indexLength)<<offsetLength) | (index << offsetLength);
			accessDRAM(addrToSave, (byte*)&(toReplace->data), transferUnit, WRITE);
			//at this point, the entire block should be saved to DRAM
			//addrToSave = (tag << indexLength) << offsetLength;
			//addrToSave = //addrToSave | (index << offsetLength);
			blockToAccess->dirty = VIRGIN;
		}
		
		//if (memory_sync_policy == WRITE_BACK) {
			// if th
		//}
		blockToAccess = toReplace;
		
		blockToAccess->tag = tag;
		
		
		//Nothing in cache. Load from memory.
		//Load from memory into cache
		//accessDRAM(addr, (byte*)toReplace, WORD_SIZE, READ);
		
		// LOAD BLOCK FROM MEMORY TO CACHE
		// Load the entire cache block from memory to cache before reading to CPU's from cache block 
		int addrToSave = addr & (tagMask | indexMask);		// address to save to will have same tag and index as addr
			
		
		TransferUnit transferUnit = uint_log2(block_size); //Determine how many bytes we need to copy from memory to fill the block.
		accessDRAM(addr, (byte *)&(blockToAccess->data), transferUnit, READ);

		memcpy(data, &(blockToAccess->data[offset]), 4);	// WORD_SIZE correspondsto 4 bytes
		
		//set valid bit
		toReplace->valid = VALID;
		//set LRU and accessCount
		//set->block[i].lru.value = readWriteCount++;
		//set->block[i].accessCount++;
		decrementLRUs(set, blockToAccess->lru.value);
		toReplace->lru.value = assoc - 1;	//readWriteCount++;
		toReplace->accessCount++;
		//Now load from cache to requested address.
		//memcpy(data, &(toReplace->data[offset]), 4);

	} else {
		printf ("Error: neither read nor write!!!\n");
	}

	//cacheBlock block = cache[index].block[offset]; 


	//if (block.dirty) {
		
	//} 

	/*
	You need to read/write between memory (via the accessDRAM() function) and
	the cache (via the cache[] global structure defined in tips.h)

	Remember to read tips.h for all the global variables that tell you the
	cache parameters

	The same code should handle random, LFU, and LRU policies. Test the policy
	variable (see tips.h) to decide which policy to execute. The LRU policy
	should be written such that no two blocks (when their valid bit is VALID)
	will ever be a candidate for replacement. In the case of a tie in the
	least number of accesses for LFU, you use the LRU information to determine
	which block to replace.

	Your cache should be able to support write-through mode (any writes to
	the cache get immediately copied to main memory also) and write-back mode
	(and writes to the cache only gets copied to main memory when the block
	is kicked out of the cache.

	Also, cache should do allocate-on-write. This means, a write operation
	will bring in an entire block if the block is not already in the cache.

	To properly work with the GUI, the code needs to tell the GUI code
	when to redraw and when to flash things. Descriptions of the animation
	functions can be found in tips.h
	*/

	/* Start adding code here */

	//Define and calculate fields



	/* This call to accessDRAM occurs when you modify any of the
	 cache parameters. It is provided as a stop gap solution.
	 At some point, ONCE YOU HAVE MORE OF YOUR CACHELOGIC IN PLACE,
	 THIS LINE SHOULD BE REMOVED.
	*/

	/*
	if () {
		
		
	}

	if (memory_sync_policy == WRITE_BACK) {
		
		
	}
	*/
	//accessDRAM(addr, (byte*)data, WORD_SIZE, we);

}
