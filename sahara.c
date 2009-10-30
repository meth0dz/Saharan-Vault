// Released and coded by meth0dz under the GPLv2 License.
// I can be reached at meth0dz_@hotmail.com.

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("meth0dz");
MODULE_DESCRIPTION("Exports sys_call_table to the kernel namespace.");


// Must be supplied in little endian format
static long sct_addr = 0;
module_param(sct_addr, long, 0);

static ssize_t read_handler(struct file * file, char __user * buffer, size_t length, loff_t * offset);
static ssize_t write_handler(struct file * file, const char __user * buffer, size_t length, loff_t * offset);
static int ioctl_handler(struct inode * inode, struct file * file, unsigned int ioctl_number, unsigned long ioctl_param);
static int open_handler(struct inode * inode, struct file * file);
static int release_handler(struct inode * inode, struct file * file);

void *get_interrupt_handler(int index);
int replace_sys_call_table(void * ptr_sct);
void * mem_scan(void * buffer, long buffer_len, void * token, long token_length);

typedef struct __attribute__ ((packed)) {
        uint16_t base_lo;
        uint16_t sel;
        uint8_t  always0;
        uint8_t  flags;
        uint16_t base_hi;
} idt_entry_t;

typedef struct __attribute__ ((packed)) {
        uint16_t limit;
        uint32_t base;
} idt_ptr_t;

unsigned long new_sys_call_table[357];

int init_module(void)
{	
	const char * device_name = "Sahara";
	struct file_operations fops = 
	{
		.read = &read_handler,
		.write = &write_handler,
		.ioctl = &ioctl_handler,
		.open = &open_handler,
		.release = &release_handler
	};
	void * sys_call = get_interrupt_handler(0x80);
	replace_sys_call_table(sys_call);
	register_chrdev(0, device_name, &fops);
	 
	return 0;
}



void cleanup_module(void)
{	
	return;
}


/*
** This function is responsible for returning data about the current state of Sahara.  It can return 
** information such as, is it currently in operation, or chillin, as well as a list of all intercepted 
** calls and their parameters.
*/
static ssize_t read_handler(struct file * file, char __user * buffer, size_t length, loff_t * offset)
{

	return 0;
}

/* 
** This function is responsible for allowing a user to put the module in and out of operation.  As well
** as allowing the user to set the directory and userland binary that is to be controlled.
*/
static ssize_t write_handler(struct file * file, const char __user * buffer, size_t length, loff_t * offset)
{

	return 0;
}

/*
** Probably not going to do shit.
*/
static int ioctl_handler(struct inode * inode, struct file * file, unsigned int ioctl_number, unsigned long ioctl_param)
{

	return 0;
}

/*
** This gives control of the driver to someone.
*/
static int open_handler(struct inode * inode, struct file * file)
{

	return 0;
}

/*
** Allows a user to say that he is done using the driver.
*/
static int release_handler(struct inode * inode, struct file * file)
{

	return 0;
}

// By Napalm[a]Netcore2k.com under the GPL
void *get_interrupt_handler(int index)
{
        idt_ptr_t idtp;
        idt_entry_t *entry;

        // save idt pointer and get ref to idt entry for index
        asm("sidt %0" : "=m"(idtp));
        entry = &((idt_entry_t *)idtp.base)[index];

        return (void *)((entry->base_hi << 16) + entry->base_lo);
}

int replace_sys_call_table(void * ptr_sys_call)
{
	// Remember little endian
	void * loc, * def = "\xFF\x14\x85";

	// If the user supplied a possible address for the sys_call_table
	// We will try to use it first
	if (sct_addr) {
		if (loc = mem_scan(ptr_sys_call, 0x1FF, (void*)sct_addr, 4)) {
			printk(KERN_ALERT "Found using user supplied address!\n");
			//apply_replacement(loc, new_sys_call_table);
			return 0;
		}
	}

	// Otherwise, we will just scan through memory looking for 
	// 0x8514ff
	if (loc = mem_scan(ptr_sys_call, 0x1FF, def, 3)) {
		printk(KERN_ALERT "Found using general memory scan!\n");
		// In my memory, it's laid out 4 in front of the searched string
		//apply_replacement(loc + 4, new_sys_call_table);
		return 0;
	}
	
	return -1;
}

// Is endianess correct here?
void apply_replacement(void * addr, void * rep_data)
{
	*(unsigned long*)addr = (unsigned long)rep_data;
	return;
}


// We don't want to rely on memmem or strstr being exported into the kernel
// as this could potentially make the software unusable on some systems
void * mem_scan(void * buffer, long buffer_len, void * token, long token_len)
{
	int i, j;
	if (buffer && token) {
		for (i = 0; i < buffer_len; i++, buffer++) {
			if (*(unsigned char*)buffer == *(unsigned char*)token) {
				for (j = 1; j < token_len; j++) 
					if (((unsigned char*)buffer)[j] != ((unsigned char*)token)[j]) break;
						if (j == token_len) return buffer;
			}
		}
	}
	return NULL;
}
