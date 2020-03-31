#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>

// there is some problem with make file so juat coppy gcc line for now and complie that way if u can figure out the problem with the makefile feel free to fix

//new 
// Definition of a queue


typedef struct {
    int             fill;                                    
    int             use;                                                                       
    char**           buffer;                                 
    pthread_mutex_t queue_lock;
    sem_t           empty;
    sem_t           full;
} QUEUE;

// Thread data producer

typedef struct {

    QUEUE* q;
    
} producer_data;

// Thread data consumer

typedef struct {
    QUEUE* q;
    
} consumer_data;

struct node {
    char *line;
    struct node *next;
};

// rather than using a queue struct, I think maybe all 
// we need is these next 5 lines... this is if, indeed,
// we just need to create the queue in the main process
// without using threads.
struct node *front = NULL;
struct node *rear = NULL;
sem_t empty;
sem_t full;
pthread_mutex_t count_lock;
pthread_mutex_t head_lock;
int number_of_words = 0;
int lines_in_file;

// Queue management functions

void put(char* line)
{
    struct node *new_line = malloc(sizeof(struct node));
    new_line->line = line;
    new_line->next = NULL;
    if (rear == NULL) {
        front = new_line;
        rear = new_line;
    }
    else {
        rear->next = new_line;
        rear = rear->next;
    }
    //assert(pthread_mutex_unlock(&queue_lock) == 0);
}

struct node* get()
{
    struct node *tmp;

    //assert(sem_wait(&q->full) == 0);
    //assert(pthread_mutex_lock(&q->queue_lock) == 0);
    assert(pthread_mutex_lock(&head_lock) == 0);
    tmp = front;
    front = front->next;
    //printf("\n\nline: %s", tmp->line);
    assert(pthread_mutex_unlock(&head_lock) == 0);
    //assert(pthread_mutex_unlock(queue_lock) == 0);
    //assert(sem_post(empty) == 0);

    return tmp;
    
}

int word_count(char* line) {

    int words = 0;
    int i = 0;
    for (i = 0; line[i]; i++) {
        if (line[i] == 32)
            words++;
    }
    if (i > 0)
        words++;

    return words;
}


char* trim_ws(char *str){ // this method needs work

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

// Thread functions:

int count_lines(){

    int count = 0;
    char *line_buf = NULL; // a pointer to the line being read
    size_t line_buf_sz = 0; //gets the size of the line
    

    for (ssize_t line_size = getline(&line_buf, &line_buf_sz, stdin); line_size >= 0; line_size = getline(&line_buf, &line_buf_sz, stdin)){

        if (line_size != 0) // Increment count if this character is newline 
            count = count + 1; 
    }

    fseek(stdin,0,SEEK_END);
    return count;

}

void *consumer_function(void * arg) {

    int thread_id = (int*)arg;
    struct node *temp;

    while (lines_in_file >= 1) {
        temp = get();
        char *line = trim_ws(temp->line);
        int words_in_line = word_count(line);
        printf("thread_id: %d\n", thread_id);
        printf("line: %s", line);
        assert(pthread_mutex_lock(&count_lock) == 0);
        number_of_words = number_of_words + words_in_line;
        assert(pthread_mutex_unlock(&count_lock) == 0);
        
        lines_in_file = lines_in_file - 1;
        printf("\ncurrent word count: %d\n", number_of_words);
        printf("remaining lines: %d\n", lines_in_file);
    }
    //printf("%s\n", get());
    printf("**********************\n");
}

int main(int argc, char *argv[]) {

    if (argc < 1) {
	fprintf(stderr, "usage: spro_mcon <#_of_con>\n");
	exit(1);
    }
    // take all the stuff from the command line and put em in variables 
    int num_threads = atoi(argv[1]);
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    
    assert(pthread_mutex_init(&count_lock, NULL) == 0);
    assert(pthread_mutex_init(&head_lock, NULL) == 0);
    char *line_buf = NULL; // a pointer to the line being read
    size_t line_buf_sz = 0; //gets the size of the line
    int line_count = 0; 
    ssize_t line_size; 
	//QUEUE q = {0, 0, buffer, PTHREAD_MUTEX_INITIALIZER};
    while(line_size = getline(&line_buf, &line_buf_sz, stdin) >= 0){
        // trimming trailing/leading whitespace and putting on queue
        if (line_buf[0] != '\n') {
            put(trim_ws(line_buf)); 
            lines_in_file++; // incrementing number of lines
        }
    }
    printf("\nlines in file: %d\n", lines_in_file);

    for (int i = 0; i < num_threads; i++) {

        if (pthread_create(&threads[i], NULL, &consumer_function, (void*)i) != 0) {
            printf("Unable to create thread\n");
            exit(1);
        }

    }
    
    assert(sem_init(&empty, 0, lines_in_file) == 0);
    assert(sem_init(&full, 0, 0) == 0);
    
    /*
    // displaying each line by running through queue
    struct node *temp;
    temp = front;
    printf("\n");
    int total_words = 0;
    while (temp != NULL) {
        printf("line: %s\n", temp->line);
        printf("# of words in line: %d\n", word_count(temp->line));
        total_words = total_words + word_count(temp->line);
        temp = temp->next;
    }
    */
   
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    printf("\ntotal words in file: %d\n", number_of_words);

    //make the queue 
    char* buffer[lines_in_file];
    QUEUE q = {0, 0, buffer, PTHREAD_MUTEX_INITIALIZER};
    

    //data to be passed to threads
    producer_data pd = {&q};
    consumer_data cd = {&q};

    // make the threads that will interact with the queue
    // the sole producer thread
    pthread_t p1;
    //assert(pthread_create(&p1, NULL, producer_fn, (void*)&pd) == 0);
    
    //assert(pthread_join(p1, NULL) == 0);

    return 0;
    
}
