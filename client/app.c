#include <stdio.h>

#define DEBUGFS_TS "/sys/kernel/debug/ovo/ts"
#define DEBUGFS_VM "/sys/kernel/debug/ovo/vm"

void my_printf(FILE* fp) {
	char current;
	while((current = fgetc(fp)) != EOF) putchar(current);
}

int main(int argc, char *argv[]) {
	if((argc < 3) || (argc > 4)) {
		printf("I accept 2-3 arguments. The first one must be required struct code (0 for thread_struct, 1 for vm_area_struct) and 1-2 ids of chosen structure\n");
		return 0;
	}
	if(strcmp(argv[1], "0") == 0) { // thread_struct case
		if(argc != 3) {
			printf("For thread_struct I accept only 1 id: its process PID\n");
			return 0;
		}
		FILE *tsf;
		tsf = fopen(DEBUGFS_TS, "w+");
		fprintf(tsf, argv[2]);
		my_printf(tsf);
		fclose(tsf);
	}
	else if(strcmp(argv[1], "1") == 0) { // vm_area_struct
		if(argc != 4) {
			printf("For thread_struct I accept 2 ids: its process PID and VM area address\n");
			return 0;
		}
		FILE *tsf;
		tsf = fopen(DEBUGFS_VM, "w+");
		fprintf(tsf, "%s %s", argv[2], argv[3]);
		my_printf(tsf);
		fclose(tsf);
	}
	else {
		printf("For now I only know 2 structures: thread_struct (code 0), vm_area_struct (code 1). Code is the first command line argument\n");
	}
	return 0;
}
