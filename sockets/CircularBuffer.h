//
// Created by kamil on 20.01.2021.
//

#ifndef SZOKETY_CIRCULARBUFFER_H
#define SZOKETY_CIRCULARBUFFER_H


struct circularBuffer
{
	int first;
	int last;
	int size;
	int currentSize;
	int* buffer;

};

struct circularBuffer* create(int size);
void destroy(struct circularBuffer* buffer);
int add(struct circularBuffer* buffer, int element);
int pop(struct circularBuffer* buffer);



#endif//SZOKETY_CIRCULARBUFFER_H
