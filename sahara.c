// Released and coded by meth0dz under the GPLv2 License.
// I can be reached at meth0dz_[at]hotmail.com.

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>

#define SYS_CALL_TABLE_ENTRIES 357
#define UNUSED(x) x

MODULE_LICENSE("GPL");
MODULE_AUTHOR("meth0dz");
MODULE_DESCRIPTION("Creates a sandbox that allows users to run unreliable, unpredictable or even potentially dangeorus software on their system in a controlled environment.");

// Must be supplied in little endian format
static long orig_sct_addr = 0;
module_param(orig_sct_addr, long, 0);

static ssize_t read_handler(struct file * file, char __user * buffer, size_t length, loff_t * offset);
static ssize_t write_handler(struct file * file, const char __user * buffer, size_t length, loff_t * offset);
static int ioctl_handler(struct inode * inode, struct file * file, unsigned int ioctl_number, unsigned long ioctl_param);
static int open_handler(struct inode * inode, struct file * file);
static int release_handler(struct inode * inode, struct file * file);

static void * get_interrupt_handler(int index);
static void * get_ptr_to_sct(void * ptr_sct);
static void apply_replacement(void * addr, void * rep_data);
static void * mem_scan(void * buffer, long buffer_len, void * token, long token_length);
static int create_replacement_sct(void);

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


static unsigned long new_sys_call_table[SYS_CALL_TABLE_ENTRIES];
static void * ptr_to_sct = NULL;
static long major = 0;
static const char * device_name = "Sahara";

int init_module(void)
{	
	struct file_operations fops = 
	{
		.read = &read_handler,
		.write = &write_handler,
		.ioctl = &ioctl_handler,
		.open = &open_handler,
		.release = &release_handler
	};
	void * sys_call_function;
	if ((major = register_chrdev(0, device_name, &fops)) > 0) {
		sys_call_function = get_interrupt_handler(0x80);
		if ((ptr_to_sct = get_ptr_to_sct(sys_call_function))) {
			if (create_replacement_sct()) {
				return 0;
			}
		}
	}
	printk(KERN_ALERT "Saharan Vault failed to initialize properly.\n");
	return 1;
}



void cleanup_module(void)
{	unregister_chrdev(major, device_name);
	return;
}


/*
** This function is responsible for returning data about the current state of 
** Sahara.  It can return information such as, is it currently in operation, or
** chillin, as well as a list of all intercepted calls and their parameters.
*/
static ssize_t read_handler(struct file * file, char __user * buffer, size_t length, loff_t * offset)
{

	return 0;
}

/* 
** This function is responsible for allowing a user to put the module in and out
** of operation.  As well as allowing the user to set the directory and userland
** binary that is to be controlled.
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
	apply_replacement(ptr_to_sct, new_sys_call_table);
	return 0;
}

/*
** Allows a user to say that he is done using the driver.
*/
static int release_handler(struct inode * inode, struct file * file)
{
	apply_replacement(ptr_to_sct, (void*)orig_sct_addr);
	return 0;
}

// By Napalm[at]Netcore2k.net under the GPL
void *get_interrupt_handler(int index)
{
        idt_ptr_t idtp;
        idt_entry_t *entry;

        // save idt pointer and get ref to idt entry for index
        asm("sidt %0" : "=m"(idtp));
        entry = &((idt_entry_t *)idtp.base)[index];

        return (void *)((entry->base_hi << 16) + entry->base_lo);
}

void * get_ptr_to_sct(void * ptr_sys_call_func)
{
	// Remember little endian
	void * ptr_to_sct, * def = "\xFF\x14\x85";

	// If the user supplied a possible address for the sys_call_table
	// We will try to use it first
	if (orig_sct_addr) {
		if ((ptr_to_sct = mem_scan(ptr_sys_call_func, 0x1FF, (void*)orig_sct_addr, 4))) {
			printk(KERN_ALERT "Found using user supplied address!\n");
			return ptr_to_sct;
		}
	}

	// Otherwise, we will just scan through memory looking for 
	// 0x8514ff
	if ((ptr_to_sct = mem_scan(ptr_sys_call_func, 0x1FF, def, 3))) {
		printk(KERN_ALERT "Found using general memory scan!\n");
		ptr_to_sct += 4;
		orig_sct_addr = (long)*(unsigned long*)ptr_to_sct;
		return ptr_to_sct;
	}
	
	return (void*)0;
}

void apply_replacement(void * addr, void * rep_data)
{
	*(unsigned long*)addr = (unsigned long)rep_data;
	return;
}


// We don't want to rely on memmem or strstr being exported into the kernel
// as this could potentially make the software unusable on some systems
void * mem_scan(void * buffer, long buffer_len, void * token, long token_len)
{
	int i = 0, j = 0, track = 0;
	if (buffer && token) {
		for (i = 0; i < buffer_len; i++, buffer++) {
			track = *(unsigned char*)buffer == ((unsigned char*)token)[j++] ? track + 1 : 0;
			if (!track) j = 0;
			else if (j == token_len) return (buffer - token_len);
		}
	}
	return NULL;
}

int create_replacement_sct(void)
{
	int i = 0;
	for (; i < SYS_CALL_TABLE_ENTRIES; i++) 
		((unsigned long*)new_sys_call_table)[i] = (*(unsigned long**)(ptr_to_sct))[i];	
	return 1;
}

