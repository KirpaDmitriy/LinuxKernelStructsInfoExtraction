#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/memblock.h>
#include <asm/thread_info.h>
#include <asm/processor.h>
#include <linux/sched.h>

#define MAX_DEBUGFS_SIZE 100
#define DEBUGFS_INTERFACE_DIR "banan"
#define DEBUGFS_THREAD_STRUCTURE_INTERFACE_FILE "ts"
#define DEBUGFS_VMA_INTERFACE_FILE "vm"


static char ts_buffer[MAX_DEBUGFS_SIZE];
static char vma_buffer[MAX_DEBUGFS_SIZE];
static unsigned long ts_buffer_size = 0;
static unsigned long vma_buffer_size = 0;


// Thread srtuct info interface methods:

ssize_t ts_read_interface(struct file * file, char __user * buff, size_t count, loff_t * offset) {
	pr_info("Reading TS Info");
	static int read_status = 0;
	if(read_status != 0) {
		read_status = 0;
		return 0;
	}
	printk("TS read input : %s\n", ts_buffer);
	long seeked_pid = 1;
	kstrtol(ts_buffer, 10, &seeked_pid);
	printk("TS read input atoi: %d\n", seeked_pid);
	struct task_struct *task;
	int found_flag = 0;
	for_each_process(task) {
		if(task->pid == seeked_pid) {
			found_flag = 1;
			break;
		}
	}
	if(found_flag == 1) {
		struct thread_struct found_thread = task->thread; // target thread structure
		
		// target fields values:
		unsigned long ts_error_code = found_thread.error_code;
		
		// values to string format:
		const char fields_values_str[MAX_DEBUGFS_SIZE];
		const char format_answer[] = "Error code: %u\n";
		size_t string_size = snprintf(NULL, 0, format_answer, ts_error_code) + 1;
		snprintf(fields_values_str, string_size, format_answer, ts_error_code);
		copy_to_user(buff, fields_values_str, string_size);
		read_status = string_size;
	}
	else {
		copy_to_user(buff, "Nothing found\n", 15);
		read_status = 15;
	}
	return read_status;
}


ssize_t ts_write_interface(struct file * file, const char __user * buff, size_t count, loff_t * offset) {
	pr_info("Writing PID to explore TS");
	if(count <= MAX_DEBUGFS_SIZE) ts_buffer_size = count;
	else ts_buffer_size = MAX_DEBUGFS_SIZE;
	copy_from_user(ts_buffer, buff, ts_buffer_size);
	return ts_buffer_size;
}


// VMA struct info interface methods:

ssize_t vma_read_interface(struct file * file, char __user * buff, size_t count, loff_t * offset) {
	pr_info("Reading VMA Info");
	static int read_status = 1;
	if(read_status != 0) {
		read_status = 0;
		return 0;
	}
	printk("VMA read input: %s\n", vma_buffer);
	long seeked_pid = 1;
	kstrtol(vma_buffer, 10, &seeked_pid);
	printk("VMA read input atoi: %d\n", seeked_pid);
	struct task_struct *task;
	int found_flag = 0;
	for_each_process(task) {
		if(task->pid == seeked_pid) {
			found_flag = 1;
			break;
		}
	}
	if(found_flag == 1) {
		char result[MAX_DEBUGFS_SIZE];
		struct vm_area_struct *found_vma = task->mm->mmap; // target virtual memory areas list
		
		// target fields values:
		unsigned long vm_start = found_vma->vm_start;
		unsigned long vm_end = found_vma->vm_end;
		
		// values to string format:
		const char fields_values_str[MAX_DEBUGFS_SIZE];
		const char format_answer[] = "Start address: %u\nEnd address: %u\n";
		size_t string_size = snprintf(NULL, 0, format_answer, vm_start, vm_end) + 1;
		printk("StrSz: %d", string_size);
		snprintf(fields_values_str, string_size, format_answer, vm_start, vm_end);
		copy_to_user(buff, fields_values_str, string_size);
		read_status = string_size;
	}
	else {
		copy_to_user(buff, "Nothing found\n", 15);
		read_status = 15;
	}
	return read_status;
}


ssize_t vma_write_interface(struct file * file, const char __user * buff, size_t count, loff_t * offset) {
	pr_info("Writing VMA Identifiers");
	if(count <= MAX_DEBUGFS_SIZE) vma_buffer_size = count;
	else vma_buffer_size = MAX_DEBUGFS_SIZE;
	copy_from_user(vma_buffer, buff, vma_buffer_size);
	return vma_buffer_size;
}


// Setting up debugfs IO dir, attaching interface methods to debugfs files

static const struct file_operations ts_file_interface = {
	.owner=THIS_MODULE,
	.read=ts_read_interface,
	.write=ts_write_interface
};

static const struct file_operations vma_file_interface = {
	.owner=THIS_MODULE,
	.read=vma_read_interface,
	.write=vma_write_interface
};

static int __init initer(void) {
	pr_info("Initializing debugfs TS and VMA info interface");
	struct dentry* interface_dir = debugfs_create_dir(DEBUGFS_INTERFACE_DIR, NULL);
	debugfs_create_file(DEBUGFS_THREAD_STRUCTURE_INTERFACE_FILE, 0777, interface_dir, NULL, &ts_file_interface);
	debugfs_create_file(DEBUGFS_VMA_INTERFACE_FILE, 0777, interface_dir, NULL, &vma_file_interface);
	return 0;
}

static void __exit goodbye(void) {
	pr_info("Hasta la vista, baby!");
}

module_init(initer);
module_exit(goodbye);

MODULE_LICENSE("GPL");
