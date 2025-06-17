/*
 * downloader.c - entire multithread download program
 * Author: Cheran Balakrishnan
 *
 * Description:
 * This program runs threads to download multiple urls stored in a txt file and can be downloaded in the project folder or create a specific sub folder to download to
 *
 * Main functionalities:
 * - Take list of file URLS
 * - use real HTTP downloading
 * - Creates threads to download files
 * - Handle multiple concurrent downloads
 * - Gracefully waits for all threads to finish
 * - show time stamp and download percentage
 * - command-line interface 
 *
 * First install curl library: 
 * sudo apt update
 * sudo apt install libcurl4-openssl-dev
 * 
 * Compile: gcc downloader.c -o downloader -lcurl
 * Run: ./downloader
 * 
 * COMMAND LINE INTERFACE: 
 * 
 * for a specific folder run: ./downloader -o folder_name
 * 
 * for a specific txt file to read from run: ./downloader file_name.txt 
 * 
 * for a specific folder and txt file run: ./downloader file_name.txt -o folder_name
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <errno.h>


#define MAX_ARRAY 1000
#define MAX_CHAR_READ 500
#define MAX_THREADS 5
#define FILE_DOWNLOADS_SIZE 50
#define FULL_FILE_PATHWAY_SIZE 200

char *urls[MAX_ARRAY];//array to store all the file names from downloads.txt

int default_file_num = 0;//If the link has no file name inside, then a default name with this integer in the title is made,incrementing for each new default file name

char *file_pathway = ".";//default file pathway if user does not use "-o" in terminal, storing downloaded files to project directory

//initialize a mutex lock so global default_file_num can be used one thread at a time
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

//initialize a mutex lock so each thread can use the global array file_names[] separately
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

//initialize a mutex lock so each thread can print in terminal separately
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;



/**
 * extract_file_name() - Extracts the name of a file from a url
 * 
 * char *url: url that the file name is going to get extracted from
 * ---------------------------------------------------  
 * 
 * Description:
 * When when naming the file where the url's downloaded bytes will be stored, the file name in 
 * the link or if no file name is described then a default name will be returned from this function.
 *           
 */
char *extract_file_name (char *url){

    const char *last_slash = strrchr(url,'/');//find address in url string of the last '/'

    if (last_slash != NULL && *(last_slash + 1) != '\0'){//check if strrchr() detected a '/' and if after the slash there exists a file name

        char *file_name = strdup(last_slash + 1);//set the file name as the words after '/'

        char *question_mark = strrchr(file_name,'?');//check if a '?' exists after the file name

        if (question_mark != NULL){//if question mark exists set it to '\0' so when reading the file name it does not go past the '?'

            *question_mark = '\0';
        }

        return file_name;
    }

    else{//if no '/' exists

        char *default_file_name = malloc(40);//allocate memory for a default name

        pthread_mutex_lock(&count_mutex);//lock mutex

        snprintf(default_file_name,40,"File_%d.txt",default_file_num);//set a default name 

        pthread_mutex_unlock(&count_mutex);//unlock mutex

        default_file_num++;//increment number so the next default name can have a different number

        return default_file_name;

    }

}


/**
 * progress_callback() - Used under a curl session this function prints the progress percentage when downloading the file
 * 
 * void *clientp,curl_off_t total,curl_off_t now,curl_off_t ultotal,curl_off_t ulnow: parameters needed in curl_easy_setopt to use this function
 * 
 * But the parameter this function only uses is:
 * 
 * curl_off_t total: Total amount of bytes from url.
 * 
 * curl_off_t now: Amount of bytes downloaded from url.
 * 
 * ---------------------------------------------------  
 * 
 * Description:
 * Function is set under the curl session settings and displays the progress percentage when downloading each file
 *           
 */
int progress_callback(void *clientp,curl_off_t total,curl_off_t now,curl_off_t ultotal,curl_off_t ulnow){

    if (total > 0){//check if total file size greater than 0 bytes

        double percent = ((double)(now / total))*100;//divide the total by the amount of bytes retrieved now and multiply by 100 for percent
        printf("\rProgress: %.2lf%% ",percent);//overwrite the line to update percent
        fflush(stdout);//for immediate print to terminal
    }
    return 0;
}



/**
 * Download_file() - Downloads files under a thread
 * 
 * void *args: A pointer that has NULL value needed in parameter so thread can use the function
 * 
 * ---------------------------------------------------  
 * 
 * Description:
 * Function creates a curl session, creates file and downloads a URL link from the global URLs array and continues 
 * to download links until there are no more links left to download
 *           
 */
void *Download_file(void *args){

    while(1){

        char *temp_url_name = NULL;//url to download


        pthread_mutex_lock(&queue_mutex);//lock mutex
        
        for (int j = 0 ; j<MAX_ARRAY;j++){//Go through all the URL's

            if (urls[j] != NULL){//if a URL is found in the array

                temp_url_name = urls[j];//set the url name to the temp pointer
                urls[j] = NULL;//now set the url pointer in the URL array to NULL
                break; //end for loop
            }
        }

        pthread_mutex_unlock(&queue_mutex);//unlock mutex lock

        if (temp_url_name == NULL){//check if there are no more urls left to download
            free(temp_url_name);
            break;
        }

        char *file_name = extract_file_name(temp_url_name);//extract file name from URL


        char full_file_pathway[FULL_FILE_PATHWAY_SIZE];

        snprintf(full_file_pathway,FULL_FILE_PATHWAY_SIZE,"%s/%s",file_pathway,file_name);


        CURL *curl = curl_easy_init();//create a curl session for one download request

        FILE *file_downloader =fopen(full_file_pathway,"wb");//open and create a file that is going to be written in binary from download link


        if (file_downloader == NULL){//check for fopen error
            perror("fopen failed");
            free(file_name);
            free(temp_url_name);
            return NULL;
        }


        if (curl == NULL){//check if curl session initialized properly
            fprintf(stderr,"curl_easy_init() error\n");
            free(file_name);
            free(temp_url_name);
            return NULL;
        }

        curl_easy_setopt(curl, CURLOPT_URL, temp_url_name);//Set URL in curl session 

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file_downloader);//Set the download data to output to the file opened in file_downloader

        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);//If download finds itself in a 404 not found or 403 Forbidden then curl session will treat as a error    

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//If the server redirects to another link then follow the new location 

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);//set the curl to output progress(orginally disabled in curl options)

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);//output the progress of the curl session to the progress_callback function

        
        CURLcode res = curl_easy_perform(curl);//download data in curl session
        
        //time variable to output timestamps when each download finishes
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);

        pthread_mutex_lock(&print_mutex);//lock mutex


        if (res != CURLE_OK) {//check if the download was not succesful
            fprintf(stderr, "Download failed for %s: %s\n", temp_url_name, curl_easy_strerror(res));

        } else {//check if the download was succesful
            printf("Downloaded %s [%02d:%02d:%02d]\n", file_name,tm_info->tm_hour,tm_info->tm_min,tm_info->tm_sec);
        }

        pthread_mutex_unlock(&print_mutex);//unlock mutex

        curl_easy_cleanup(curl);//clean up the curl session
        
        fclose(file_downloader);//close file with downloaded data
        free(file_name);//free the file name
        free(temp_url_name);//free url name
    }

    return NULL;

}

/**
 * free_name_array() - free's memory from urls array
 * 
 * char **urls: The urls array
 * 
 * ---------------------------------------------------  
 * 
 * Description:
 * frees dynamically stored url strings
 *           
 */
void free_name_array(char **urls){

    int i = 0;//In case the array is getting free at a specific part due to error possibilities when creating threads

    while (urls[i] != NULL){//continue looping through the file_name array and freeing each name until the list reaches NULL
        free(urls[i++]);
    }

    return;
}



/**
 * main() 
 * ---------------------------------------------------
 * - takes in any custom commands such as "-o" and creates a folder in the project directory
 * - creates threads to download url links
 * - joins threads and ensures no memory leak
 * - check for error througout the entire process         
 * 
 */
int main(int argc, char *argv[]){

    srand(time(NULL));//set the rand() to be a range of any random number with a large range 

    char line[MAX_CHAR_READ];//array used to store file names temporarily when reading from downloads.txt

    pthread_t threads[MAX_THREADS];//array to store all the threads

    int j = 0;//used to keep track of indexes when adding to the file_name array

    char *file_downloads = malloc(FILE_DOWNLOADS_SIZE);

    if (argc >1){//if command argument count is more than jus ./program

        for (int i = 0; i<argc;i++){//search through the argv array for "-o" to then set the file_pathway variable from "." to a folder where the files can download to

            if (strcmp(argv[i],"-o") == 0 && argv[i+1] != NULL){
                file_pathway = argv[i+1];

                printf("%s",file_pathway);
                if (mkdir(file_pathway, 0755) == -1 && errno != EEXIST) {//
                    perror("mkdir failed");
                    free(file_downloads);
                    return 1;
                }
            }
        }
    }

    if (argc > 1 && strcmp(argv[1],"-o")!= 0){
        snprintf(file_downloads,FILE_DOWNLOADS_SIZE,"%s",argv[1]);
    }

    else{
        snprintf(file_downloads,FILE_DOWNLOADS_SIZE,"%s","downloads.txt");
    }
   

    FILE *downloads = fopen(file_downloads,"r");//open downloads.txt and read from file
    

    if (downloads == NULL){//check for fopen error
        perror("fopen failed");
        free(file_downloads);
        return 1;
    }
    

    while (fgets(line,MAX_CHAR_READ,downloads)!=NULL){//read through the "downloads.txt" file until the file ends 

        line[strcspn(line,"\r")] = 0;//after reading, remove the "\r" character read from each line

        urls[j] = strdup(line);//duplicate string and let the file_names[] point to that memory
        
        j++;    

    }

    urls[j] = NULL;//set the end of the file_names array to NULL


    for (int i = 0; i<MAX_THREADS;i++){

        if (pthread_create(&threads[i], NULL,Download_file,NULL) != 0){//create the thread and ensure in the function parameter, the file name is inside
            free_name_array(urls);
            free(file_downloads);
            fclose(downloads);
            perror("pthread_create error");
            return 1;
        }

    }

    //loop through all the threads created which is equivalent to the amount of file names read and join all the threads
    for (int i = 0;i<MAX_THREADS; i++){

        if (pthread_join(threads[i], NULL) != 0){
            free_name_array(urls);
            free(file_downloads);
            fclose(downloads);
            perror("pthread_create error");
            return 1;
        }

    }

    free_name_array(urls);//free all the strings stored in the file_names[]

    free(file_downloads);

    fclose(downloads);//close file

    return 0;

}