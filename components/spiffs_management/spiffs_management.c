#include "spiffs_management.h"
#include <dirent.h>

static bool object_created = 0;
cJSON* data_array = NULL;

extern esp_vfs_spiffs_conf_t conf;

static inline int data_len (uint32_t* data)
{
    int len_var = 0;

    while (data[len_var++] != 0);

    return len_var;

}

static inline void find_data_range(int first_val , int* min , int* max) 
{   
    if (*min > first_val) *min = first_val;
    if (*max < first_val) *max = first_val; 
}

void add_remote_option (int option , uint32_t* data , char* data_string) 
{
    if (!object_created) {
        data_array = cJSON_CreateArray();

        object_created = 1;
    }
    char key_buf[20] = {0};
    int iterator = 0 , data_size = 0;

    data_size = data_len(data);

    if (option < 2) {
        if (option == 0) strcpy(key_buf , "Turn Off");
        else             strcpy(key_buf , "Turn On" );
    }
    else {
        sprintf(key_buf , "Temp %d" , option);
    }
    cJSON* itemObject = cJSON_Parse(data_string);
    // error handling???

    cJSON* int_array = cJSON_CreateIntArray(data , data_size);

    cJSON_AddItemToObject(itemObject , key_buf , int_array);


    cJSON_AddItemToArray(data_array , itemObject);

    // data_string = cJSON_Print(data_array);

    // // Print JSON string
    // printf("%s\n", data_string);

    // cJSON_Delete(itemObject);

}

int store_model(char* model_name , char* data_string) 
{
	int ret = 0; 
	size_t total = 0, used = 0 , remaining = 0;

	// check if there is enough size for the new remote file.
	ret = esp_spiffs_info(conf.partition_label, &total, &used);

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	remaining = total - used;

	if (remaining < LEAST_FILE_SIZE) {
		ESP_LOGW(TAG , "Unable to add remote\nNo more space left!!!\n");
		return -1;
	}
	char file_name_buf[50];
	sprintf(file_name_buf , "/spiffs/%s" , model_name);

	int fd = open(file_name_buf , O_WRONLY);

	if (fd == -1) {
		printf("cannot make file\n");
		return -1;
	}

	ssize_t bytes_write = write(fd ,  data_string , strlen(data_string));
	if (bytes_write == -1) {
		printf("unable to write file");
		return -1;
	}

	return 1;



}

char* serialized_it(char* model_name , int min , int max) 
{

    const int numbers[2] = {min , max};
    cJSON *root = cJSON_CreateObject(); // Create root JSON object

    cJSON_AddItemToObject(root, "range", cJSON_CreateIntArray(numbers, 2));
    cJSON_AddItemToObject(root, "Model", cJSON_CreateString(model_name));

    cJSON_AddItemToObject(root, "Temp Range", cJSON_CreateIntArray((const int[]){17, 30}, 2));
    cJSON_AddItemToObject(root , "data" , data_array);

    // Convert cJSON object to string
    char* data_string = cJSON_Print(root);
    cJSON_Delete(root);
    // printf("%s\n", data_string);

    return data_string;


}


const char* read_file(const char* file_name) 
{
	char file_name_buf[50];
	sprintf(file_name_buf , "/spiffs/%s" , file_name);
	ssize_t file_size = 0;
	struct stat st;
	if (stat(file_name_buf , &st) == 0)
	{
		file_size = st.st_size;
		ESP_LOGI(TAG , ">> %s\t %d\n" , file_name , file_size);
	}
	else
	{
		ESP_LOGI(TAG , ">> %sNOT found\nCannot go further!!!\n" , file_name);
		while (1) vTaskDelay(pdMS_TO_TICKS(1000));
	}

	char* file_buff = (char*)malloc(sizeof(char) * file_size);

	int fd = open(file_name_buf , O_RDONLY);

	if (fd == -1) {
		printf("cannot open file\n");
		esp_restart();
	}

	ssize_t bytes_read = 0;

	bytes_read = read(fd , file_buff , file_size);

	ESP_LOGI(TAG , "size: %d , %s" , file_size , file_buff);
	if (bytes_read < file_size) {
		printf("file not read properly!!\n");
	}

	return file_buff;




}

const char* get_model(char* file_buff , int value) 
{

	char* model_buf = (char*)malloc(sizeof(char) * 50);

	cJSON* root = cJSON_Parse(file_buff);

	if (root == NULL) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
		free(file_buff);
    }

	cJSON *range = cJSON_GetObjectItem(root, "range");
	cJSON *model = cJSON_GetObjectItem(root, "Model");


	int lower_range , upper_range;
	
	lower_range = cJSON_GetArrayItem(range, 0)->valueint;
	upper_range = cJSON_GetArrayItem(range, 1)->valueint;

	if (value >= lower_range && value <= upper_range) {
		strcpy(model_buf , model->valuestring);
	}
	else {
		strcpy(model_buf , "UNKNOWN_MODEL");

	}
	cJSON_Delete(root);

	return model_buf;

}

bool remote_option(opr_e operation , int option , char* file_buffer , uint32_t* data_buff) {

	cJSON* root = cJSON_Parse(file_buffer);
	cJSON *item = NULL;
	char tem_buf[20] = {0};
	int itr = 0;


	if (root == NULL) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
    }

	cJSON *data = cJSON_GetObjectItem(root, "data");

	if (operation == POWER) {
		if (option) {
			cJSON *dataItem = cJSON_GetArrayItem(data, 1); // Assuming there's only one item in "data"
			cJSON *turn_on = cJSON_GetObjectItem(dataItem, "Turn On");

            cJSON_ArrayForEach(item, turn_on) {
				data_buff[itr] = item->valueint;
				itr++;
                // printf("%d ", item->valueint);
            }

		}
		else {
			cJSON *dataItem = cJSON_GetArrayItem(data, 0); // Assuming there's only one item in "data"
			cJSON *turn_off = cJSON_GetObjectItem(dataItem, "Turn Off");

            cJSON_ArrayForEach(item, turn_off) {
                data_buff[itr] = item->valueint;
				itr++;
            }


		}

	}
	else if (operation == TEMPERATURE) {
		sprintf(tem_buf , "Temp %d" , option);

		cJSON *tem_range = cJSON_GetObjectItem(root, "Temp Range");
		int lower_tem_range , upper_tem_range;
		
		lower_tem_range = cJSON_GetArrayItem(tem_range, 0)->valueint;
		upper_tem_range = cJSON_GetArrayItem(tem_range, 1)->valueint;
		
		int tem_index = (option - lower_tem_range) +2;

		cJSON *dataItem = cJSON_GetArrayItem(data, tem_index); // Assuming there's only one item in "data"
		cJSON *tem_val = cJSON_GetObjectItem(dataItem, tem_buf);

		cJSON_ArrayForEach(item, tem_val) {
            data_buff[itr] = item->valueint;
			itr++;
        }


	}
	itr = 0;

	for (int i = 0 ; data_buff[i] != 0; i++) {
		printf("[%d]: %ld\n" , i , data_buff[i]);
	}

	// do {
	// 	printf("%ld" , raw_data[itr]);
	// 	itr++;
	// } while(raw_data[itr] != 0);


	cJSON_Delete(root);

	return 0;

}

int add_remote(const char* remote_model) 
{ 
	int ret = 0; 
	size_t total = 0, used = 0 , remaining = 0;

	// check if there is enough size for the new remote file.
	ret = esp_spiffs_info(conf.partition_label, &total, &used);

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	remaining = total - used;

	if (remaining < LEAST_FILE_SIZE) {
		ESP_LOGW(TAG , "Unable to add remote\nNo more space left!!!\n");
		return -1;
	}
	char file_name_buf[50];
	sprintf(file_name_buf , "/spiffs/%s" , remote_model);

	int fd = open(file_name_buf , O_WRONLY);

	if (fd == -1) {
		printf("cannot make file\n");
		return -1;
	}


    
	// check if there is already a file with the same name.
	// if yes include and maintain version number.
	// return file discriptor of the newly created model file.
	return fd;
	
}
					//  tmp range  ,        raw data     , model range, 
bool add_remote_options(int option , uint32_t* data_buff , int val) 
{
	bool ret = 0;
	// user will record data in sequence (Turn Off , Turn On , Temp (data)).
	// function will store the data to the file in json format.
	// function will return status (whether the data has been stored successfully for not).
	return ret;
}









	



