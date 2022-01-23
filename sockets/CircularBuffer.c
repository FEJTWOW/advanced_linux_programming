#include "CircularBuffer.h"
#include <stdlib.h>
#include <stdio.h>
struct circularBuffer* create(int size)
{
	struct circularBuffer* buffer = (struct circularBuffer*)calloc(1,sizeof(struct circularBuffer));
	buffer->size = size;
	buffer->buffer = (int*)calloc(size,sizeof(int));
	buffer->currentSize = 0;
	buffer->first = 0;
	buffer->last = 0;
	return buffer;
}
void destroy(struct circularBuffer* buffer)
{
	free(buffer->buffer);
	free(buffer);
}
int add(struct circularBuffer* buffer, int element)
{
	if (buffer->currentSize == buffer->size)
	{
		perror("No more place in a buffer!\n");
		return -1;
	}
	buffer->buffer[buffer->last] = element;
	buffer->last = (buffer->last+1)%(buffer->size);
	buffer->currentSize++;
	return 0;
}
int pop(struct circularBuffer* buffer)
{
	int temp = buffer->buffer[buffer->first];
	buffer->buffer[buffer->first] = -1;
	buffer->first = (buffer->first+1)%buffer->size;
	buffer->currentSize--;

	return temp;
}
