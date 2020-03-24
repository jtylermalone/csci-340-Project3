#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>

// there is some problem with make file so juat coppy gcc line for now and complie that way if u can figure out the problem with the makefile feel free to fix

sem_t mutex; 

typedef struct __node_t {
    
    char * line;
    struct __node_t * next;
    
} node_t;

typedef struct __queue_t { 
    
    node_t *        head;
    node_t *        tail;
    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;

} queue_t;

void Queue_Init(queue_t *q) {
    
    node_t *tmp = malloc(sizeof(node_t));
    tmp->next = NULL;
    q->head = q->tail = tmp;
    
    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);
    
}

void Queue_Enqueue(queue_t * q, char * value) {
    
    node_t *tmp = malloc(sizeof(node_t));  //Get a new node
    if (tmp == NULL) {                           
        perror("malloc");
        exit(1);
    }

    assert(tmp != NULL);
    tmp->line = value;                           //Set the node's contents 
    tmp->next  = NULL;                            //Show it's the tail

    pthread_mutex_lock(&q->tail_lock);          // this is where the lock should be placed
    q->tail->next = tmp;                          //Point old tail to new tail
    q->tail = tmp;                                //Point tail to new node
    pthread_mutex_unlock(&q->tail_lock);
    
}

int Queue_Dequeue(queue_t * q) {

    int rc = -1;
    
    if (q->head->next != NULL) {
        pthread_mutex_lock(&q->head_lock);
        char *value = q->head->next->line;             /* Return the value */
        node_t *tmp = q->head->next;                    /* Save the old head node pointer */
        q->head->next = q->head->next->next;                   /* Reset the head */
        pthread_mutex_unlock(&q->head_lock);      /* Unlock the list */
        free(tmp);                                /* Free the old head node */
        rc = 0;
    }
    
    
    return rc;
    
}

typedef struct {

    queue_t* ptr;

} p_struct_t;

void *producer(void *arg) { // this method needs work

    p_struct_t *mys = arg; // ||THE POINTER TO MY QUEUE IS WRONG ITS NOT PASSING THE POINTER CORRECLTY


    // Open the file for reading
    char *line_buf = NULL; // a pointer to the line being read
    size_t line_buf_sz = 0; //gets the size of the line
    int line_count = 0; // a simple counter that will keep track of the number of lines that are read in from stdin
    ssize_t line_size; // holds the size of each line as it is being read in | needs to be singed size_t cuz the last line is a neg num

    // Get the first line of the file
    sem_wait(&mutex);
    line_size = getline(&line_buf, &line_buf_sz, stdin);
    printf("line[%06d]: chars=%06zd, buf size=%06zu, contents: %s", line_count, line_size, line_buf_sz, line_buf);
    Queue_Enqueue(mys->ptr, line_buf);
    
    sem_post(&mutex);
    
/*
    // Loop through until we are done with the file
    while (line_size >= 0) {
        sem_wait(&mutex);
        line_count++;

        // Show the line details
        //printf("line[%06d]: chars=%06zd, buf size=%06zu, contents: %s", line_count, line_size, line_buf_sz, line_buf);

        line_size = getline(&line_buf, &line_buf_sz, stdin);
        sem_post(&mutex);
    }
*/
    
    // every time the line is larger than the allocated buffer it increases the buffer sz so its probs very large by the end of the file so we will free the allocated line buffer
    free(line_buf);
    line_buf = NULL;

    return NULL;
}

void display_q(node_t *head)
{
    if(head == NULL)
    {
        printf("\n");
    }
    else
    {
        printf("<%s>\n", head->line);
        display_q(head->next);
    }
}

char *trim_ws(char *str) // this method needs work
{
    
    // not sure why this is necessary, but this works
    // if we make a copy of the str parameter and 
    // perform the function on that instead.
	char *return_str = strdup(str);
    char *end;

    if(*return_str == 0){
        return_str[0] = '.';
        return_str[1] = '\0';
        return return_str;
    }

    end = return_str + strlen(return_str) - 1;

    // removing leading whitespace
    while (isspace((unsigned char)*return_str)) {
        return_str++;
    }

    // removing trailing whitespace
    while(end > str && isspace((unsigned char)*end)) {
		end--;
    }

    // truncating trailing whitespace here
    end[1] = '\0';

    return return_str;
	
}

int main(int argc, char *argv[]) {
    printf("---------------------\n");
    if (argc < 2) {
	fprintf(stderr, "usage: spro_mcon <#_of_con>\n");
	exit(1);
    }
    // take all the stuff from the command line and put em in variables 
    int num_threads = atoi(argv[1]);
    //sem init
    sem_init(&mutex, 0, 1);
    //make the thread safe queue 
    queue_t *q;
    q = malloc(sizeof(queue_t));
    Queue_Init(q);

    
/*
    //pass a pointer of the queue to the threads
    p_struct_t vals = {q};

    // make the threads that will interact with the queue
    // the sole producer thread
    pthread_t p1;
    assert(pthread_create(&p1, NULL, producer, &vals) == 0);
    
    assert(pthread_join(p1, NULL) == 0);
*/

    // || so this part below up to line 206 wont be here it will be in the producer but i cant find a way to get all the whitespace out of the line_buf to store it in the queue 


    // Open the file for reading
    char *line_buf = NULL; // a pointer to the line being read
    size_t line_buf_sz = 0; //gets the size of the line
    int line_count = 0; // a simple counter that will keep track of the number of lines that are read in from stdin
    ssize_t line_size; // holds the size of each line as it is being read in | needs to be singed size_t cuz the last line is a neg num
    // Get the first line of the file
    line_size = getline(&line_buf, &line_buf_sz, stdin);
    // Loop through until we are done with the file // || \\ blank line is getline() == 0 
    while (line_size >= 0) {
        printf("in while\n");
        //sem_wait(&mutex);
        line_count++;
        
        trim_ws(line_buf);

        // Show the line details
        printf("line[%06d]: chars=%06zd, buf size=%06zu, contents: <%s>\n", line_count, line_size, line_buf_sz, line_buf);
        
        line_size = getline(&line_buf, &line_buf_sz, stdin);
        
        //sem_post(&mutex);
    }    


    
    return 0;
    
}
