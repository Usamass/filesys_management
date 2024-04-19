#include <spiffs_management.h>
#include <file_system.h>

const char* const FILES[3] = {"orient.json" , "kenwood.json" , "dawlance.json"};


void app_main () 
{
	file_system_init();

	char* file_buffer = read_file(FILES[2]);

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}




}