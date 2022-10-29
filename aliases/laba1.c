#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/memblock.h>


#define MAX_DEBUGFS_SIZE 100
#define DEBUGFS_INTERFACE_DIR "did"
#define DEBUGFS_MEMORY_BLOCK_INTERFACE_FILE "memory_block"


static char debugfs_buffer[MAX_DEBUGFS_SIZE];
static unsigned long debugfs_buffer_size = 0;


char* read_whole_file(char* filename) {
	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
  	fclose(f);
	
  	string[fsize] = 0;
	
  	return string;
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char* get_file_info(char* path) {
  char* result1 = concat("1: ", read_whole_file(path));
  char* result2 = concat("2: ", read_whole_file(path));
  return concat(result1, result2);
}


ssize_t read_interface(struct file * file, char __user * buff, size_t count, loff_t * offset) {
	pr_info("Reading");
	static int read_status = 0;
	if(read_status != 0) {
		read_status = 0;
		return 0;
	}
	copy_to_user(buff, debugfs_buffer, debugfs_buffer_size);
	read_status = debugfs_buffer_size;
	return read_status;
}


ssize_t write_interface(struct file * file, const char __user * buff, size_t count, loff_t * offset) {
	pr_info("Writing");
	if(count <= MAX_DEBUGFS_SIZE) debugfs_buffer_size = count;
	else debugfs_buffer_size = MAX_DEBUGFS_SIZE;
	copy_from_user(debugfs_buffer, buff, debugfs_buffer_size);
	return debugfs_buffer_size;
}


static const struct file_operations memory_block_file_interface = {
	.owner=THIS_MODULE,
	.read=read_interface,
	.write=write_interface,
};


static int __init initer(void) {
	pr_info("Initializing debugfs interface");
	struct dentry* interface_dir = debugfs_create_dir(DEBUGFS_INTERFACE_DIR, NULL);
	struct dentry* memory_block_interface = debugfs_create_file(DEBUGFS_MEMORY_BLOCK_INTERFACE_FILE, 0777, interface_dir, NULL, &memory_block_file_interface);
	return 0;
}

static void __exit goodbye(void) {
	pr_info("Bye!");
}

module_init(initer);
module_exit(goodbye);

MODULE_LICENSE("GPL");
