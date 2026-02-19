// TODO: implement flexible linked list
#ifndef LLST_H_
#define LLST_H_

#define LLSTDEF static inline



// Bulid struct Node
typedef struct LLNode {
	void* data;
	struct LLNode *next;
} LLNode;

//Bulid struct Linked List
typedef struct {
	LLNode *head;			// pointer to the head
	LLNode *tail;			// pointer to the tail
	uint32_t length;			// Number of elements in the array
	uint16_t element_size;  // size of the element in byte
} LLST;

LLSTDEF LLST LLST_create(uint16_t element_size);
LLSTDEF void LLST_append(LLST* llst, void* item);
// LLSTDEF LLST LLST_delete(LLST* llst, size_t index);


#endif // LLST_H_

#ifdef LLST_IMPLEMENTATION

LLSTDEF LLST LLST_create(uint16_t element_size){
	return (LLST){
		.head = NULL,
		.tail = NULL,
		.element_size = element_size,
		.length = 0
	};
}

LLSTDEF void LLST_append(LLST* llst, void* item){
	void* data = (void*)malloc(llst->element_size);
	memcpy(data, item, llst->element_size);
	LLNode* node = (LLNode*)malloc(sizeof(LLNode));
	node->next = NULL;
	node->data = data;
	if(llst->length == 0){
		llst->head = node;
		llst->tail = node;

	}else{
		llst->tail->next = node;
		llst->tail = node;
	}
	llst->length++;	
}

#endif // LLST_IMPLEMENTATION