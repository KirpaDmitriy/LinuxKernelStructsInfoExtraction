#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/memblock.h>
#include <asm/thread_info.h>
#include <asm/processor.h>
#include <linux/sched.h>

#define MAX_DEBUGFS_SIZE 100
#define DEBUGFS_INTERFACE_DIR "ovo"
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
		unsigned long sp = found_thread.sp;
		unsigned short es = found_thread.es;
		unsigned short ds = found_thread.ds;
		unsigned long iobm = found_thread.io_bitmap;
		unsigned long ts_error_code = found_thread.error_code;
		
		// values to string format:
		const char fields_values_str[MAX_DEBUGFS_SIZE];
		const char format_answer[] = "Stack pointer: %u\nES register: %u\nDS register: %u\nIO: %u\nError code: %u\n";
		size_t string_size = snprintf(NULL, 0, format_answer, sp, es, ds, iobm, ts_error_code) + 1;
		snprintf(fields_values_str, string_size, format_answer, sp, es, ds, iobm, ts_error_code);
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
	size_t split_counter = 0;
	char string_seeked_pid[MAX_DEBUGFS_SIZE];
	char string_seeked_vma[MAX_DEBUGFS_SIZE];
	while((split_counter < vma_buffer_size) && (vma_buffer[split_counter] != ' ')) {
		string_seeked_pid[split_counter] = vma_buffer[split_counter];
		split_counter++;
	}
	string_seeked_pid[split_counter] = '\0';
	printk("VMA seeked PID: %s", string_seeked_pid);
	split_counter++;
	size_t freeze_counter = split_counter;
	while(split_counter < vma_buffer_size) {
		string_seeked_vma[split_counter - freeze_counter] = vma_buffer[split_counter];
		split_counter++;
	}
	string_seeked_vma[split_counter - freeze_counter] = '\0';
	printk("VMA seeked region: %s", string_seeked_vma);
	long seeked_pid = -1, seeked_reg = -1;
	kstrtol(string_seeked_pid, 10, &seeked_pid);
	kstrtol(string_seeked_vma, 10, &seeked_reg);
	printk("VMA seeked PID atoi: %d\n", seeked_pid);
	printk("VMA seeked region atoi: %d\n", seeked_reg);
	struct task_struct *task;
	int found_flag = 0;
	for_each_process(task) {
		if(task->pid == seeked_pid) {
			found_flag = 1;
			break;
		}
	}
	if((found_flag == 1) && (task->mm != NULL) && (seeked_reg != -1)) {
		char result[MAX_DEBUGFS_SIZE];
		struct mm_struct *found_mm = task->mm; // mm containing target virtual memory areas list
		struct vm_area_struct *found_vma = find_vma(found_mm, 10000);
		
		// target fields values:
		unsigned long vm_start = found_vma->vm_start;
		unsigned long vm_end = found_vma->vm_end;
		unsigned long vm_flags = found_vma->vm_flags;
		unsigned long vm_offset = found_vma->vm_pgoff;
		
		// values to string format:
		const char fields_values_str[MAX_DEBUGFS_SIZE];
		const char format_answer[] = "Start address: %u\nEnd address: %u\nFlags: %u\nFile offset: %u\n";
		size_t string_size = snprintf(NULL, 0, format_answer, vm_start, vm_end, vm_flags, vm_offset) + 1;
		snprintf(fields_values_str, string_size, format_answer, vm_start, vm_end, vm_flags, vm_offset);
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
