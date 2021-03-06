#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>



// global variables
int word_total = 0;
int line_count = 0;
pthread_mutex_t count_lock;
pthread_mutex_t get_lock;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int fin = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

struct node {
    char *line;
    struct node *next;
};


// queue definition
typedef struct {
    struct node *   head;
    struct node *   tail;
} QUEUE;

// Thread data producer

typedef struct {
    QUEUE* q;
    int thread_id;   
} consumer_data;

// Queue management functions

void put(QUEUE *q,
         char*    line)
{
    // this method adds items to queue
    struct node *new_line = malloc(sizeof(struct node));
    new_line->line = line;
    new_line->next = NULL;

    // if queue is empty
    if (q->tail == NULL) {
        q->head = new_line;
        q->tail = new_line;
    }
    // if queue isn't empty
    else {
        q->tail->next = new_line;
        q->tail = q->tail->next;
    }
    
}

char* get(QUEUE * q)
{

    // this tmp node is used to return
    // the char* line. we need to be able
    // to return the line after we pop the
    // first element from the queue.
    struct node *tmp;

    assert(pthread_mutex_lock(&get_lock) == 0);
    
    // get head node
    tmp = q->head;
    // assign head to next node
    q->head = q->head->next;

    char * line = tmp->line;

    assert(pthread_mutex_unlock(&get_lock) == 0);

    // i'm pretty sure that freeing here
    // isn't not effective and smart
    free(tmp);
    return line;  
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

int word_count(char* line) {

    int words = 0;
    int i = 0;
    for (i = 0; line[i]; i++) {
        // if current character in line
        // is a space, we increment words.
        if (line[i] == 32)
            words++;
    }
    // must increment one more time because
    // the number of words in a line is the
    // number of spaces + 1
    if (i > 0)
        words++;

    return words;
}

//thread function

void thr_exit() {
            pthread_mutex_lock(&m);
            fin = 1;
            pthread_cond_signal(&cond);        
            pthread_mutex_unlock(&m);
}

void *consumer_function(void * arg) {

    // getting data from arguments
    consumer_data *data = (consumer_data*)arg;
    QUEUE *q = data->q;
    int thread_id = data->thread_id;

    char *line;
    char* end = "++..";
    

    // line count gets decremented each
    // time through this while loop
    while (line_count > 0 && fin == 0) {
        // getting line from queue
        line = get(q);

        //if the end signal is sent end all the threads
        if(strcmp(line, end) == 0){
            thr_exit();
            return NULL;
        }

        line_count = line_count - 1;

        // trimming leading/trailing whitespace
        char *trimmed_line = trim_ws(line);

        // counting words in line
        int words_in_line = word_count(trimmed_line);
        
        // printing required info
        printf("Thread ID: %d | Line: <%s>\n", thread_id, trimmed_line);

        // adding to overall word total
        assert(pthread_mutex_lock(&count_lock) == 0);
        word_total = word_total + words_in_line;
        assert(pthread_mutex_unlock(&count_lock) == 0);
    }
    // if we don't return here, the compiler gives
    // a warning: "control reaches end of non-void function"
    return NULL;
}

void sleep_main() {
    //sleep the parent functions and make sure all threads fin
    pthread_mutex_lock(&m);
    while (fin == 0)
        pthread_cond_wait(&cond, &m);
    pthread_mutex_unlock(&m);
}


int main(int argc, char *argv[]) {

    if (argc < 1) {
	fprintf(stderr, "usage: spro_mcon <#_of_con>\n");
	exit(1);
    }
    // take all the stuff from the command line and put em in variables 
    int num_threads = atoi(argv[1]);

    //read in all the lines from stdin
    // Open the file for reading
    char *line_buf = NULL; // a pointer to the line being read
    size_t line_buf_sz = 0; //gets the size of the line
    ssize_t line_size; // holds the size of each line as it is being read in | needs to be singed size_t cuz the last line is a neg num
    char* trimed = NULL;

    //init mutex's as well as the queue
    assert(pthread_mutex_init(&count_lock, NULL) == 0);
    assert(pthread_mutex_init(&get_lock, NULL) == 0);
    QUEUE q = {NULL, NULL};

    // Get the first line of the file
    line_size = getline(&line_buf, &line_buf_sz, stdin);
    //printf("line[%06d]: chars=%06zd, buf size=%06zu, contents: %s\n", line_count, line_size, line_buf_sz, line_buf);
    

    // Loop through until we are done with the file
    while (line_size >= 0) {

        if(line_size > 1) {//skip any line that is empty
            line_count++;
            trimed = trim_ws(line_buf); // trim any whitespace from the line at the front and back but not spaces inbetween words
            put(&q, trimed);
            //printf("line[%06d]: chars=%06zd, buf size=%06zu, contents: %s\n", line_count, line_size, line_buf_sz, line_buf);
        }

        line_size = getline(&line_buf, &line_buf_sz, stdin);
    }

    // every time the line is larger than the allocated buffer it increases the buffer sz so its probs very large by the end of the file so we will free the allocated line buffer
    free(line_buf);
    line_buf = NULL;

    char* end = "++..";

    line_count++;
    put(&q, end);// the files can't have any non alpha chars so we can use that to send signals || '+' for us is gonna be the end of the file

    // make the threads that will interact with the queue
    // array that will hold all threads
    pthread_t threads[num_threads];
    // array that will hold each thread's parameters
    consumer_data cd[num_threads];

    //create each thread and pass it the args needed
    for (int i = 0; i < num_threads; i++) {
        // passing the same queue to each thread
        cd[i].q = &q;
        // passing thread's ID
        cd[i].thread_id = i;
        if (pthread_create(&threads[i], NULL, &consumer_function, &cd[i]) != 0) {
            printf("Unable to create thread\n");
            exit(1);
        }
    }

    sleep_main();

    // joining threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("final word total: %d\n", word_total);

    return 0;
    
}
