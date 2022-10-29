#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/memblock.h>
#include <asm/thread_info.h>
#include <asm/processor.h>
#include <linux/sched.h>

#define MAX_DEBUGFS_SIZE 100
#define DEBUGFS_INTERFACE_DIR "aub"
#define DEBUGFS_MEMORY_BLOCK_INTERFACE_FILE "mb"


static char debugfs_buffer[MAX_DEBUGFS_SIZE];
static unsigned long debugfs_buffer_size = 0;


ssize_t read_interface(struct file * file, char __user * buff, size_t count, loff_t * offset) {
	pr_info("Reading");
	static int read_status = 0;
	if(read_status != 0) {
		read_status = 0;
		return 0;
	}
	printk("Input : %s\n", debugfs_buffer);
	long transferred;
	kstrtol(debugfs_buffer, 10, &transferred);
	printk("Atoi: %d\n", transferred);
	int seeked_pid = transferred;
	struct task_struct *task;
	for_each_process(task) {
		if(task->pid == seeked_pid) {
			printk("%s\n", task->comm);
			break;
		}
	}
	copy_to_user(buff, "kek", 3);
	read_status = 3;
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
	.write=write_interface
};


static int __init initer(void) {
	pr_info("Initializing debugfs interface");
	struct dentry* interface_dir = debugfs_create_dir(DEBUGFS_INTERFACE_DIR, NULL);
	struct dentry* interface_file = debugfs_create_file(DEBUGFS_MEMORY_BLOCK_INTERFACE_FILE, 0777, interface_dir, NULL, &memory_block_file_interface);
	// struct thread_info* cti = current_thread_info();
	// pr_info("Thread status %d\n", cti->status);
	return 0;
}

static void __exit goodbye(void) {
	pr_info("Bye!");
}

module_init(initer);
module_exit(goodbye);

MODULE_LICENSE("GPL");
