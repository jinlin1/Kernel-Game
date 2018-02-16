/**
 *  @Author Jin Hui Lin (hj73293@umbc.edu)
 *  This file is proj1 mastermind.c file,   
 *  Creates a driver in the kernel and creates
 *  callback functions to communicate with the driver
 *  Reading the driver returns the status of the game
 *  Writing to the driver updates status of the game
 *  Driver is memory mapped so the history of the game 
 *  can be viewed 
 *
 *  Sources:
 *  https://stackoverflow.com/questions/9519648/what-is-the-difference-between-map-shared-and-map-private-in-the-mmap-function
 *  https://stackoverflow.com/questions/12124628/endlessly-looping-when-reading-from-character-device
 *  http://www.xml.com/ldd/chapter/book/ch13.html
 *  https://stackoverflow.com/questions/11952693/how-to-call-unix-commands-from-c-program
 *  https://stackoverflow.com/questions/23011683/collecting-return-value-from-shell-script
 *  https://stackoverflow.com/questions/20460670/reading-a-file-to-string-with-mmap
 */

/*
 * This file uses kernel-doc style comments, which is similar to
 * Javadoc and Doxygen-style comments.  See
 * ~/linux/Documentation/kernel-doc-nano-HOWTO.txt for details.
 */

/*
 * Getting compilation warnings?  The Linux kernel is written against
 * C89, which means:
 *  - No // comments, and
 *  - All variables must be declared at the top of functions.
 * Read ~/linux/Documentation/CodingStyle to ensure your project
 * compiles without warnings.
 */

#define pr_fmt(fmt) "MM: " fmt

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define NUM_PEGS 4
#define NUM_COLORS 6

/** true if user is in the middle of a game */
static bool game_active;

/** code that player is trying to guess */
static int target_code[NUM_PEGS];

/** tracks number of guesses user has made */
static unsigned num_guesses;

/** current status of the game */
static char game_status[80];

/** buffer that logs guesses for the current game */
static char *user_view;

static char buffer[80];

/**
 * getMinimum() - returns the minimum of the two numbers  
 * @count1: first count 
 * @count2: second count 
 *
 * Return: returns the lessor of the two numbers 
 */
static size_t getMinimum(size_t count1, size_t count2)
{
	if (count1 <= count2) {
		return count1;
	} else {
		return count2;
	}
}

/**
 * clearBuffer() - overwrites the buffer with a default value  
 * @c: char array  
 * @size: size of char array 
 *
 * Return: replaces the char array with 0 value
 */
void clearBuffer(char *c, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++) {
		c[i] = 0;
	}
}

/**
 * mm_read() - callback invoked when a process reads from
 * /dev/mm
 * @filp: process's file object that is reading from this device (ignored)
 * @ubuf: destination buffer to store output
 * @count: number of bytes in @ubuf
 * @ppos: file offset (in/out parameter)
 *
 * Write to @ubuf the current status of the game, offset by
 * @ppos. Copy the lesser of @count and (string length of @game_status
 * - *@ppos). Then increment the value pointed to by @ppos by the
 * number of bytes copied. If @ppos is greater than or equal to the
 * length of @game_status, then copy nothing.
 *
 * Return: number of bytes written to @ubuf, or negative on error
 */
static ssize_t mm_read(struct file *filp, char __user * ubuf, size_t count,
		       loff_t * ppos)
{
	int retval;

	if (*ppos >= sizeof(game_status)) {
		count = 0;
	} else {
		count = getMinimum(count, strlen(game_status) - *ppos);
		retval = copy_to_user(ubuf, game_status + *ppos, count);
		if (retval < 0) {
			return -EINVAL;
		}
	}
	(*ppos) += count;
	return count;
}

/**
 * mm_write() - callback invoked when a process writes to /dev/mm
 * @filp: process's file object that is reading from this device (ignored)
 * @ubuf: source buffer from user
 * @count: number of bytes in @ubuf
 * @ppos: file offset (ignored)
 *
 * If the user is not currently playing a game, then return -EINVAL.
 *
 * If @count is less than NUM_PEGS, then return -EINVAL. Otherwise,
 * interpret the first NUM_PEGS characters in @ubuf as the user's
 * guess. For each guessed peg, calculate how many are in the correct
 * value and position, and how many are simply the correct value. Then
 * update @num_guesses, @game_status, and @user_view.
 *
 * <em>Caution: @ubuf is NOT a string; it is not necessarily
 * null-terminated.</em> You CANNOT use strcpy() or strlen() on it!
 *
 * Return: @count, or negative on error
 */
static ssize_t mm_write(struct file *filp, const char __user * ubuf,
			size_t count, loff_t * ppos)
{
	int i, j, retval;
	char tmp_target[NUM_PEGS];
	char tmp_buffer[NUM_PEGS + 1];
	size_t black_pegs = 0;
	size_t white_pegs = 0;
	if (!game_active) {
		return -EINVAL;
	}

	if (count < NUM_PEGS) {
		return -EINVAL;
	}

	for (i = 0; i < NUM_PEGS + 1; i++) {
		tmp_buffer[i] = '\0';
	}

	count = getMinimum(count, sizeof(buffer));
	retval = copy_from_user(buffer, ubuf, count);
	if (retval < 0) {
		return -EINVAL;
	}

	for (i = 0; i < NUM_PEGS; i++) {
		if (!(buffer[i] >= '0' && buffer[i] <= '9')) {
			return -EINVAL;
		}
		tmp_buffer[i] = buffer[i];
		tmp_target[i] = target_code[i] + '0';
	}

	for (i = 0; i < NUM_PEGS; i++) {
		if (buffer[i] == tmp_target[i]) {
			black_pegs++;
			tmp_target[i] = 0;
			tmp_buffer[i] = 0;
		}
	}

	for (i = 0; i < NUM_PEGS; i++) {

		if (tmp_buffer[i] == 0) {
			continue;
		}

		for (j = 0; j < NUM_PEGS; j++) {

			if (buffer[i] == tmp_target[j]) {
				white_pegs++;
				tmp_target[j] = 0;
				break;
			}
		}
	}

	for (i = 0; i < NUM_PEGS; i++) {
		tmp_buffer[i] = buffer[i];
	}

	num_guesses++;
	clearBuffer(game_status, sizeof(game_status));
	scnprintf(game_status, sizeof(game_status),
		  "Guess %d: %zu black peg(s), %zu white peg(s)\n", num_guesses,
		  black_pegs, white_pegs);
	scnprintf(buffer, sizeof(buffer), "Guess %d: %s  | B%zu W%zu \n",
		  num_guesses, tmp_buffer, black_pegs, white_pegs);
	scnprintf(user_view, sizeof(buffer), "%s%s", user_view, buffer);
	return count;

}

/**
 * mm_mmap() - callback invoked when a process mmap()s to /dev/mm
 * @filp: process's file object that is mapping to this device (ignored)
 * @vma: virtual memory allocation object containing mmap() request
 *
 * Create a read-only mapping from kernel memory (specifically,
 * @user_view) into user space.
 *
 * Code based upon
 * <a href="http://bloggar.combitech.se/ldc/2015/01/21/mmap-memory-between-kernel-and-userspace/">http://bloggar.combitech.se/ldc/2015/01/21/mmap-memory-between-kernel-and-userspace/</a>
 *
 * You do not need to modify this function.
 *
 * Return: 0 on success, negative on error.
 */
static int mm_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
	unsigned long page = vmalloc_to_pfn(user_view);
	if (size > PAGE_SIZE)
		return -EIO;
	vma->vm_pgoff = 0;
	vma->vm_page_prot = PAGE_READONLY;
	if (remap_pfn_range(vma, vma->vm_start, page, size, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

/**
 * mm_ctl_write() - callback invoked when a process writes to
 * /dev/mm_ctl
 * @filp: process's file object that is writing to this device (ignored)
 * @ubuf: source buffer from user
 * @count: number of bytes in @ubuf
 * @ppos: file offset (ignored)
 *
 * Copy the contents of @ubuf, up to the lesser of @count and 8 bytes,
 * to a temporary location. Then parse that character array as
 * following:
 *
 *  start - Start a new game. If a game was already in progress, restart it.
 *  quit  - Quit the current game. If no game was in progress, do nothing.
 *
 * If the input is none of the above, then return -EINVAL.
 *
 * <em>Caution: @ubuf is NOT a string; it is not necessarily
 * null-terminated.</em> You CANNOT use strcpy() or strlen() on it!
 *
 * Return: @count, or negative on error
 */
static ssize_t mm_ctl_write(struct file *filp, const char __user * ubuf,
			    size_t count, loff_t * ppos)
{
	int retval;
	int i;
	for (i = 0; i < sizeof(buffer); i++) {
		buffer[i] = 0;
	}
	count = getMinimum(count, sizeof(buffer));
	retval = copy_from_user(buffer, ubuf, count);
	if (retval < 0) {
		return -EINVAL;
	}
	if (buffer[0] == 's' && buffer[1] == 't' && buffer[2] == 'a'
	    && buffer[3] == 'r' && buffer[4] == 't' && buffer[5] == 0) {
		target_code[0] = 0;
		target_code[1] = 0;
		target_code[2] = 1;
		target_code[3] = 2;
		game_active = true;
		num_guesses = 0;
		for (i = 0; i < PAGE_SIZE; i++) {
			user_view[i] = 0;
		}
		clearBuffer(game_status, sizeof(game_status));
		scnprintf(game_status, sizeof(game_status), "Starting game\n");
		return count;
	} else if (buffer[0] == 'q' && buffer[1] == 'u' && buffer[2] == 'i'
		   && buffer[3] == 't' && buffer[4] == 0) {
		if (game_active) {
			game_active = false;
			clearBuffer(game_status, sizeof(game_status));
			scnprintf(game_status, sizeof(game_status),
				  "Game over. The code was %d%d%d%d.\n",
				  target_code[0], target_code[1],
				  target_code[2], target_code[3]);
		}
		return count;
	} else {
		return -EINVAL;
	}
}

static const struct file_operations file_mm = {
	.read = mm_read,
	.write = mm_write,
	.mmap = mm_mmap
};

static struct miscdevice device_mm = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mm",
	.fops = &file_mm,
	.mode = 0666
};

static const struct file_operations file_mm_ctl = {
	.write = mm_ctl_write
};

static struct miscdevice device_mm_ctl = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mm_ctl",
	.fops = &file_mm_ctl,
	.mode = 0666
};

/**
 * mastermind_init() - entry point into the Mastermind kernel module
 * Return: 0 on successful initialization, negative on error
 */
static int __init mastermind_init(void)
{
	pr_info("Initializing the game.\n");
	user_view = vmalloc(PAGE_SIZE);
	if (!user_view) {
		pr_err("Could not allocate memory\n");
		return -ENOMEM;
	}

	/* YOUR CODE HERE */
	if(misc_register(&device_mm) != 0) {
		goto free_user_view;
	}
	if(misc_register(&device_mm_ctl) != 0) {
		goto deregister_mm;
	}
	scnprintf(game_status, sizeof(game_status), "No game yet\n");
	return 0;

	deregister_mm: misc_deregister(&device_mm);
	free_user_view: vfree(user_view);	
	return -1;
}

/**
 * mastermind_exit() - called by kernel to clean up resources
 */
static void __exit mastermind_exit(void)
{
	pr_info("Freeing resources.\n");
	vfree(user_view);

	/* YOUR CODE HERE */
	misc_deregister(&device_mm);
	misc_deregister(&device_mm_ctl);
}

module_init(mastermind_init);
module_exit(mastermind_exit);

MODULE_DESCRIPTION("CS421 Mastermind Game");
MODULE_LICENSE("GPL");
