#include <unistd.h>
#include <iostream>

void *memoryBegin;
void *memoryEnd;
int isInitialized = 0;

struct block 
{
	int isAvailable;
	int size;
};

void initialize()
{
	memoryBegin = sbrk(0);
	memoryEnd = memoryBegin;
	isInitialized = 1;
}

void *malloc(long numberOfBytes) 
{
    void *currentPosition;
    struct block *currentPositionBlock;
    void *blockAddress;

    if(!isInitialized) 
    {
        initialize();
    }

    numberOfBytes = numberOfBytes + sizeof(struct block);
    blockAddress = 0;
    currentPosition = memoryBegin;

    while(currentPosition != memoryEnd)
    {
        currentPositionBlock = (struct block*)currentPosition;

        if(currentPositionBlock->isAvailable)
        {
            if(currentPositionBlock->size >= numberOfBytes)
            {
                currentPositionBlock->isAvailable = 0;
                blockAddress = currentPosition;
                break;
            }
        }

        currentPosition = currentPosition + currentPositionBlock->size;
    }

    if(!blockAddress)
    {
        if (sbrk(numberOfBytes) == (void*)-1)
        {
        	return 0;
        }

        blockAddress = memoryEnd;

        currentPositionBlock = (block*)blockAddress;
        currentPositionBlock->size = numberOfBytes;
        currentPositionBlock->isAvailable = 0;

        memoryEnd += numberOfBytes;
    }

    blockAddress += sizeof(struct block);
    return blockAddress;
}

void free(void *address) 
{
	struct block *block = (struct block*)(address - sizeof(struct block));
	block->isAvailable = 1;
	return;
}

void *realloc(void *address, long numberOfBytes)
{
    void *blockAddress = address - sizeof(struct block);
    struct block *block = (struct block*)blockAddress;

    if (numberOfBytes < block->size)
    	return 0;

    void *reallocated = malloc(numberOfBytes);

    for (int i = 0; i < block->size; i++)
    {
        ((char*)reallocated)[i] = ((char*)address)[i];
    }

    free(address);
    return reallocated;
}

