#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//refs: https://www.geeksforgeeks.org/c/doubly-linked-list-in-c/

typedef struct Block{
	int is_free;
	int process_id;
	size_t size;
	struct Block* next;
	struct Block* prev;
}Block;

Block* createBlock(int process_id, size_t size){
	Block* newBlock = (Block*)malloc(sizeof(Block));
	newBlock->is_free = 1;
	newBlock->process_id = process_id;
	newBlock->size = size;
	newBlock->next = NULL;
	newBlock->prev = NULL;
	return newBlock;
}

void insertAtBeginning(Block** head, int process_id, size_t size){
	Block* newBlock = createBlock(process_id, size);
	if(*head == NULL){
		*head = newBlock;
		return;
	}
	newBlock->next = *head;
	(*head)->prev = newBlock;
	*head = newBlock;
}

void insertAtEnd(Block** head, int process_id, size_t size){
	Block* newBlock = createBlock(process_id, size);
	if(*head==NULL){
		*head = newBlock;
		return;
	}
	Block* temp = *head;
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = newBlock;
	newBlock->prev = temp;
}

void insertAtPosition(Block** head, int process_id, size_t size, int position)
{
    if (position < 1) {
        printf("Position should be >= 1.\n");
        return;
    }
    // if we are inserting at head
    if (position == 1) {
        insertAtBeginning(head, process_id, size);
        return;
    }
    Block* newBlock = createBlock(process_id, size);
    Block* temp = *head;
    for (int i = 1; temp != NULL && i < position - 1; i++) {
        temp = temp->next;
    }
    if (temp == NULL) {
        printf(
            "Position greater than the number of nodes.\n");
        return;
    }
    newBlock->next = temp->next;
    newBlock->prev = temp;
    if (temp->next != NULL) {
        temp->next->prev = newBlock;
    }
    temp->next = newBlock;
}

void deleteAtBeginning(Block** head)
{
    // checking if the DLL is empty
    if (*head == NULL) {
        printf("The list is already empty.\n");
        return;
    }
    Block* temp = *head;
    *head = (*head)->next;
    if (*head != NULL) {
        (*head)->prev = NULL;
    }
    free(temp);
}

void deleteAtEnd(Block** head)
{
    // checking if DLL is empty
    if (*head == NULL) {
        printf("The list is already empty.\n");
        return;
    }

    Block* temp = *head;
    if (temp->next == NULL) {
        *head = NULL;
        free(temp);
        return;
    }
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->prev->next = NULL;
    free(temp);
}


void deleteAtPosition(Block** head, int position)
{
	if (*head == NULL) {
		printf("The list is already empty.\n");
		return;
	}
	Block* temp = *head;
	if (position == 1) {
		deleteAtBeginning(head);
		return;
	}
	for (int i = 1; temp != NULL && i < position; i++) {
		temp = temp->next;
	}
	if (temp == NULL) {
		printf("Position is greater than the number of "
		 "nodes.\n");
		return;
	}
	if (temp->next != NULL) {
		temp->next->prev = temp->prev;
	}
	if (temp->prev != NULL) {
		temp->prev->next = temp->next;
	}
	free(temp);
}

void printBlock(Block* b){
	printf("process_id: %d\n size: %d\n is_free:%d\n nextPI: %d\n prevPI: %d\n", b->process_id, b->size, b->is_free, b->next->process_id, b->prev->process_id);
}

void printListForward(Block* head)
{
    Block* temp = head;
    printf("Forward List: \n");
	int position = 1;
    while (temp != NULL) {
        printf("position: %d ; isfree: %d ; processid: %d ;  size: %d ;\n", position, temp->is_free, temp->process_id, temp->size);
        temp = temp->next;
		position++;
    }
    printf("\n");
}

void printListReverse(Block* head)
{
	Block* temp = head;
    if (temp == NULL) {
        printf("The list is empty.\n");
        return;
    }
    // Move to the end of the list
    while (temp->next != NULL) {
        temp = temp->next;
    }
    // Traverse backwards
    printf("Reverse List: \n");
    while (temp != NULL) {
        printf("isfree: %d ; processid: %d ;  size: %d ;\n", temp->is_free, temp->process_id, temp->size);
        temp = temp->prev;
    }
    printf("\n");
}

Block* findByPI(Block* head, int process_id)
{
    Block* temp = head;
    while (temp != NULL) {
        if(temp->process_id == process_id)
			return temp;
		temp = temp->next;
    }
	return NULL;
}

Block* findFirstFit(Block* head, int size){
	//block.size >= size
	//NULL = it did not find a free block that fit the given size
	Block* temp = head;
	
    while (temp != NULL) {
		if((temp->is_free ==1) && (temp->size >= size)){
			return temp;
		}
		temp = temp->next;
    }
    return NULL;
}

Block* findBestFit(Block* head, int size){
	//NULL = it did not find a free block that fit the given size
	Block* temp = head;
	Block* tempBF = createBlock(99999, 99999);
	
    while (temp != NULL) {
		if((temp->is_free==1) && (temp->size < tempBF->size) && (temp->size >= size)) {
			tempBF = temp; 
		}
		temp = temp->next;
    }
	
	if(tempBF->size == 99999)
		return NULL;
	
    return tempBF;
}

Block* findWorstFit(Block* head, int size){
	//NULL = it did not find a free block that fit the given size
	Block* temp = head;
	Block* tempWF = createBlock(99999, 0);
	
    while (temp != NULL) {
		//printf("temp_id %d ; temp_size %d ; tempWF_id %d ; tempWF_size %d ; temp->size >= size %d ; temp->size > tempWF->size %d \n", temp->process_id, temp->size, tempWF->process_id, tempWF->size,(temp->size >= size), (temp->size > tempWF->size) );
		if((temp->is_free == 1)&& (temp->size > tempWF->size) && (temp->size >= size)) {
			tempWF = temp; 
		}
		temp = temp->next;
    }
	
	if(tempWF->size == -1)
		return NULL;
	
    return tempWF;
}

int alloc(Block* head, int process_id, int size, int algorithm){
	//return values: -2=no recognized algorithm code ; -1=no block found with the size required ; 0 = unknown error ; 1= well allocated
	//int algorithm: FF=1;BF=2;WF=3;
	Block* temp;
	switch(algorithm){
		case(1):
			temp = findFirstFit(head, size);
			break;
		case(2):
			temp = findBestFit(head, size);
			break;
		case(3):
			temp = findWorstFit(head, size);
			break;
		default:
			return -2;
	}
	if(temp == NULL){
		return -1;
	}
	
	if((temp->is_free==1)&&(temp->size >= size)){
		int prevSize = temp->size;
		temp->size=size;
		temp->is_free = 0;
		temp->process_id = process_id;
		if(prevSize-size>0){
			Block* newBlock = createBlock(-1, prevSize-size);
			newBlock->next = temp->next;
			newBlock->prev = temp;
			if(temp->next != NULL){
				temp->next->prev = newBlock;
			}
			temp->next = newBlock;
		}
		return 1;
	}
	
	return 0;
}

int coalescence(Block* head){
	for(int i=0; i<999; i++){
		Block* temp = head;
		int pos = 1;
		while (temp != NULL) {
			if((temp->is_free ==1) && (temp->next != NULL) && (temp->next->is_free == 1)){
				temp->process_id = -1;
				temp->size +=temp->next->size;
				deleteAtPosition(&head, pos+1);
			}
			temp = temp->next;
			pos++;
		}
	}
    return 0;
}

int freeMem(Block* head, int process_id){
	Block* temp = findByPI(head, process_id);
	if(temp==NULL)
		return -1;
	temp->is_free = 1;
	temp->process_id = -1;
	coalescence(head);
	return 1;	
}

int compact(Block* head, int totalMemory){
	Block* newHead = NULL;
	insertAtEnd(&newHead, -1, totalMemory);
	Block* temp = head;
    while (temp != NULL) {
        if(temp->is_free == 0){
			alloc(newHead, temp->process_id,  temp->size, 1);
		}
        temp = temp->next;
    }
	*head = *newHead;
	return 1;
}

void readFile(char* filepath, Block* head, int* totalMemory, int* algorithm, int* success_alloc){
	FILE* file = fopen(filepath, "r");
	if(file == NULL){
		printf("FILE NOT FOUND");
		return;
	}
	
	char line[200];
	char *lb = " \t\n\r";
	int it = 1;
	
	while(fgets(line, sizeof(line), file) != NULL){
		char *command = strtok(line, lb);
		if(command == NULL){
			it++;
			continue;
		}
		
		if(strcmp(command, "ALLOC") == 0){
			char *pid_str = strtok(NULL, lb);
            char *size_str = strtok(NULL, lb);
			int pid = atoi(pid_str);
            int size = atoi(size_str);
			//printf("ALLOC %d %d\n", pid, size);
			
			int result = alloc(head, pid, size, *algorithm);
			
			if(result == -1)
				break;
			
			*success_alloc++;
		}
		
		if(strcmp(command, "FREE") == 0){
			char *pid_str = strtok(NULL, lb);
			int pid = atoi(pid_str);
			//printf("FREE %d \n", pid);
			freeMem(head, pid);
		}
		
		if(strcmp(command, "COMPACT") == 0){
			//printf("COMPACT\n");
			compact(head, *totalMemory);
		}
	}
}

float memUsed(Block* head){
	Block* temp = head;
    float total = 0.0;
    while (temp != NULL) {
        if(temp->is_free == 0){
			total+= temp->size*1.0;
		}
        temp = temp->next;
    }
    return total;
}

float fExtIndex(Block* head, float fm){
	Block* temp = head;
    int maxSize = 0;
    while (temp != NULL) {
        if((temp->is_free == 1) && (temp->size > maxSize)){
			maxSize= temp->size*1.0;
		}
        temp = temp->next;
    }
	
    return 1 - (maxSize / fm);
}



int main(){
	int totalMemory = 500;	
	int algorithm = 1;
	int success_alloc = 0;
	
	Block* head = NULL;
	insertAtEnd(&head, -1, totalMemory);
	
	readFile("commands.txt", head, &totalMemory, &algorithm, &success_alloc);
	float memused = memUsed(head);
	float memnotused = (totalMemory*1.0f) - memused;
	float porcmemused = (memused/(totalMemory*1.0))*100;
	float fei = fExtIndex(head, memnotused);
	
	
	printf("PROCESOS ASIGNADOS: %d\nMEMORIA UTILIZADA: %.2f\%\nINDICE FRAG. EXT: %.2f\n\n", success_alloc, porcmemused, fei);
	printListForward(head);
	return 0;
}
