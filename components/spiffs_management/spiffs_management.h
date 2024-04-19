#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <file_system.h>
#include <esp_system.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <cJSON.h>
#include <inttypes.h>
#include <stdbool.h>

#define TAG "MAIN"
#define LEAST_FILE_SIZE 16000 //kbs

typedef enum {
	POWER,
	TEMPERATURE
}opr_e;

// char* get_model(cJSON* data , int value);
// char* remote_operation(cJSON* data , enum option);

const char* read_file(const char* file_name);
const char* get_model(char* file_buffer , int value);
bool remote_option(opr_e operation , int option , char* file_buffer , uint32_t* data_buff);

// functions for adding a new remote and its options
void add_remote_option (int option , uint32_t* data , char* data_string);
int store_model(char* model_name , char* data_string);


// utility function.
char* serialized_it(char* model_name , int min , int max);
