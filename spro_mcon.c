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

struct node {
    char *line;
    struct node *next;
};


typedef struct {
    //int             fill;                                    
    //int             use;                                     
    //int             q_len;                                   
    //char**           buffer;                                 
    pthread_mutex_t queue_lock;
    //sem_t           empty;
    //sem_t           full;
    struct node *   head;
    struct node *   tail;
} QUEUE;


// Thread data producer

typedef struct {

    QUEUE* q;
    
} producer_data;

// Thread data consumer

typedef struct {
    QUEUE* q;
    int thread_id;   
} consumer_data;

// Queue management functions

int word_total = 0;
int line_count = 0;
pthread_mutex_t count_lock;

void put(QUEUE *q,
         char*    line)
{

    struct node *new_line = malloc(sizeof(struct node));
    new_line->line = line;
    new_line->next = NULL;
    if (q->tail == NULL) {
        q->head = new_line;
        q->tail = new_line;
    }
    else {
        q->tail->next = new_line;
        q->tail = q->tail->next;
    }

}

char* get(QUEUE * q)
{
    struct node *tmp;

    //assert(sem_wait(&q->full) == 0);
    //assert(pthread_mutex_lock(&q->queue_lock) == 0);
    //assert(pthread_mutex_lock(&q->queue_lock) == 0);

    tmp = q->head;
    q->head = q->head->next;
    //printf("\n\nline: %s", tmp->line);
    //assert(pthread_mutex_unlock(&q->queue_lock) == 0);
    //assert(pthread_mutex_unlock(queue_lock) == 0);
    //assert(sem_post(empty) == 0);

    return tmp->line;
    
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
// Thread functions producer


/*
// Thread functions consumer

void * stage_1_fn(void * args)
{

    STAGE_1_DATA* data = (STAGE_1_DATA*)args;

    for (int i = 0; i < data->n_items; i++) {

        put(data->q_out, i);
        printf("Stage1: %d\n", i);
        
    }

    put(data->q_out, -1);
    
    printf("Stage1: Terminating\n");
    
    return NULL;
        
}
*/

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

void *consumer_function(void * arg) {

    consumer_data *data = (consumer_data*)arg;
    QUEUE *q = data->q;
    int thread_id = data->thread_id;
    printf("thread_id: %d\n", thread_id);
    //struct node *temp;
    char *line;

    while (line_count > 0) {
        line = get(q);
        if (line == "..")
            break;
        char *trimmed_line = trim_ws(line);
        int words_in_line = word_count(trimmed_line);
        
        //printf("words in line: %d\n", words_in_line);
        printf("thread_id: %d\ | ", thread_id);
        printf("line: %s", line);
        assert(pthread_mutex_lock(&count_lock) == 0);
        word_total = word_total + words_in_line;
        line_count = line_count - 1;
        assert(pthread_mutex_unlock(&count_lock) == 0);
        
        printf(" | current word count: %d\n", word_total);
        //printf("remaining lines: %d\n", lines_in_file);
        //printf("line count: %d\n", line_count);
        
    }
    //printf("%s\n", get());

}

int count_lines(){

    int count = 0;
/*
    char *line_buf = NULL; // a pointer to the line being read
    size_t line_buf_sz = 0; //gets the size of the line
    

    for (ssize_t line_size = getline(&line_buf, &line_buf_sz, stdin); line_size >= 0; line_size = getline(&line_buf, &line_buf_sz, stdin)){

        if (line_size != 0) // Increment count if this character is newline 
            count = count + 1; 
    }
*/

    char ch;
    while((ch=fgetc(stdin))!=EOF) {
      if(ch=='\n')
         count++;
   }
    return count;

}

/*
void display_q(QUEUE* q, int len_q){
    printf("contents of queue\n");
    for(int i = 0; i < len_q + 1; i++){
        printf("line %d: <%s>\n", i, q->buffer[i]);
    }  
}*/

int main(int argc, char *argv[]) {

    if (argc < 1) {
	fprintf(stderr, "usage: spro_mcon <#_of_con>\n");
	exit(1);
    }
    // take all the stuff from the command line and put em in variables 
    int num_threads = atoi(argv[1]);

    
    int lines_in_file = 2699;//count_lines();
    printf("lc: %d\n", lines_in_file);
    //make the queue 
    char* buffer[lines_in_file];
    //QUEUE q = {0, 0, lines_in_file, buffer, PTHREAD_MUTEX_INITIALIZER, NULL, NULL};
    //assert(sem_init(&q.empty, 0, lines_in_file) == 0);
    //assert(sem_init(&q.full, 0, 0) == 0);
    
    

    //read in all the lines from stdin

    // Open the file for reading
    char *line_buf = NULL; // a pointer to the line being read
    size_t line_buf_sz = 0; //gets the size of the line
    //int line_count = 0; // a simple counter that will keep track of the number of lines that are read in from stdin
    ssize_t line_size; // holds the size of each line as it is being read in | needs to be singed size_t cuz the last line is a neg num
    char* trimed = NULL;

    pthread_mutex_t lock;
    assert(pthread_mutex_init(&lock, NULL) == 0);
    QUEUE q = {&lock, NULL, NULL};
    

    // Get the first line of the file
    line_size = getline(&line_buf, &line_buf_sz, stdin);
    //printf("line[%06d]: chars=%06zd, buf size=%06zu, contents: %s\n", line_count, line_size, line_buf_sz, line_buf);

    // Loop through until we are done with the file
    while (line_size >= 0) {
        if (line_buf[0] != '\n') {//skip any line that is empty
            line_count++;
            trimed = trim_ws(line_buf); // trim any whitespace from the line at the front and back but not spaces inbetween words
            put(&q, trimed);

            //tessting stuff
            // Show the line details
            //printf("line[%06d]: chars=%06zd, buf size=%06zu, contents: <%s>\n", line_count, line_size, line_buf_sz, trimed);
        }

        line_size = getline(&line_buf, &line_buf_sz, stdin);
    }

    // every time the line is larger than the allocated buffer it increases the buffer sz so its probs very large by the end of the file so we will free the allocated line buffer
    free(line_buf);
    line_buf = NULL;

    char* end = "..";
    put(&q, end);// the files can't have any non alpha chars so we can use that to send signals || '+' for us is gonna be the end of the file
      
    printf(" linecount: %d\n", line_count);
    //data to be passed to threads

    //display_q(&q, line_count);

    //consumer_data cd = {&q};

    

    // make the threads that will interact with the queue
    // the sole producer thread
    
    printf("before making threads... line count: %d\n", line_count);

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        //printf("i: %d\n", i);
        consumer_data cd = {&q, i};
        if (pthread_create(&threads[i], NULL, &consumer_function, &cd) != 0) {
            printf("Unable to create thread\n");
            exit(1);
       
        }

    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("----------------- word total: %d ---------------\n", word_total);

    return 0;
    
}
