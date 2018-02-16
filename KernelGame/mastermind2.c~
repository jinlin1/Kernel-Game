/* YOUR FILE-HEADER COMMENT HERE */
/**
 *  @Author Jin Hui Lin (hj73293@umbc.edu)
 *
 *  Sources:
 *  https://elixir.free-electrons.com/linux/latest/source/kernel/resource.c#L116
 *  http://elixir.free-electrons.com/linux/v4.12.14/source/kernel/capability.c#L428
 *  http://www.roman10.net/2011/07/28/linux-kernel-programminglinked-list/
 *  https://isis.poly.edu/kulesh/stuff/src/klist/
 *  https://github.com/torvalds/linux/blob/master/include/linux/list.h
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

#define pr_fmt(fmt) "mastermind2: " fmt

#include <linux/capability.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/uidgid.h>
#include <linux/vmalloc.h>

#include "xt_cs421net.h"

/* Copy mm_read(), mm_write(), mm_mmap(), and mm_ctl_write(), along
 * with all of your global variables and helper functions here.
 */
/* YOUR CODE HERE */
#define NUM_PEGS 4
#define STATUS_SIZE 80
#define BUFFER_SIZE 80

struct mm_game {
	bool game_active;
	int target_code[NUM_PEGS];
	unsigned num_guesses;
	char game_status[STATUS_SIZE];
	char *user_view;
	char buffer[BUFFER_SIZE];
	kuid_t uid;
	struct list_head list;
};

static LIST_HEAD(game);

static DEFINE_SPINLOCK(foo_lock);

static int num_colors;

static int num_games;

static int invalid_guess;

static int target_code_change;
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

static struct mm_game *find_game(kuid_t uid)
{

	struct list_head *pos;
	struct mm_game *tmp;

	list_for_each(pos, &game) {
		tmp = list_entry(pos, struct mm_game, list);
		if (uid_eq(tmp->uid, uid)) {
			return tmp;
		}
	}

	tmp = (struct mm_game *)kzalloc(sizeof(struct mm_game), GFP_KERNEL);
	tmp->uid = uid;
	tmp->user_view = vmalloc(PAGE_SIZE);
	scnprintf(tmp->game_status, STATUS_SIZE, "No game yet\n");

	list_add(&(tmp->list), &game);
	return tmp;
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

	spin_lock(&foo_lock);

	if (*ppos >= STATUS_SIZE) {
		count = 0;
	} else {
		count =
		    getMinimum(count,
			       strlen(find_game(current_uid())->game_status) -
			       *ppos);
		retval =
		    copy_to_user(ubuf,
				 find_game(current_uid())->game_status + *ppos,
				 count);
		if (retval < 0) {
			spin_unlock(&foo_lock);
			return -EINVAL;
		}
	}
	(*ppos) += count;
	spin_unlock(&foo_lock);
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

	spin_lock(&foo_lock);

	if (!find_game(current_uid())->game_active) {
		spin_unlock(&foo_lock);
		return -EINVAL;
	}

	if (count < NUM_PEGS) {
		spin_unlock(&foo_lock);
		return -EINVAL;
	}

	for (i = 0; i < NUM_PEGS + 1; i++) {
		tmp_buffer[i] = '\0';
	}

	count = getMinimum(count, BUFFER_SIZE);
	retval = copy_from_user(find_game(current_uid())->buffer, ubuf, count);
	if (retval < 0) {
		spin_unlock(&foo_lock);
		return -EINVAL;
	}

	for (i = 0; i < NUM_PEGS; i++) {
		if (!
		    (find_game(current_uid())->buffer[i] >= '0'
		     && find_game(current_uid())->buffer[i] <=
		     (num_colors + '0'))) {
			spin_unlock(&foo_lock);
			return -EINVAL;
		}
		tmp_buffer[i] = find_game(current_uid())->buffer[i];
		tmp_target[i] = find_game(current_uid())->target_code[i] + '0';
	}

	for (i = 0; i < NUM_PEGS; i++) {
		if (find_game(current_uid())->buffer[i] == tmp_target[i]) {
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

			if (find_game(current_uid())->buffer[i] ==
			    tmp_target[j]) {
				white_pegs++;
				tmp_target[j] = 0;
				break;
			}
		}
	}

	for (i = 0; i < NUM_PEGS; i++) {
		tmp_buffer[i] = find_game(current_uid())->buffer[i];
	}

	find_game(current_uid())->num_guesses++;
	memset(find_game(current_uid())->game_status, '\0', STATUS_SIZE);
	if (black_pegs == NUM_PEGS) {
		find_game(current_uid())->game_active = false;
		scnprintf(find_game(current_uid())->game_status, STATUS_SIZE,
			  "Correct! Game over\n");
		scnprintf(find_game(current_uid())->buffer, BUFFER_SIZE,
			  "Correct! Game over\n");
		scnprintf(find_game(current_uid())->user_view, BUFFER_SIZE,
			  "%s%s", find_game(current_uid())->user_view,
			  find_game(current_uid())->buffer);

	} else {
		scnprintf(find_game(current_uid())->game_status, STATUS_SIZE,
			  "Guess %d: %zu black peg(s), %zu white peg(s)\n",
			  find_game(current_uid())->num_guesses, black_pegs,
			  white_pegs);
		scnprintf(find_game(current_uid())->buffer, BUFFER_SIZE,
			  "Guess %d: %s  | B%zu W%zu \n",
			  find_game(current_uid())->num_guesses, tmp_buffer,
			  black_pegs, white_pegs);
		scnprintf(find_game(current_uid())->user_view, BUFFER_SIZE,
			  "%s%s", find_game(current_uid())->user_view,
			  find_game(current_uid())->buffer);

	}
	spin_unlock(&foo_lock);
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
	unsigned long page =
	    vmalloc_to_pfn(find_game(current_uid())->user_view);

	spin_lock(&foo_lock);

	if (size > PAGE_SIZE) {
		spin_unlock(&foo_lock);
		return -EIO;
	}
	vma->vm_pgoff = 0;
	vma->vm_page_prot = PAGE_READONLY;
	if (remap_pfn_range(vma, vma->vm_start, page, size, vma->vm_page_prot)) {
		spin_unlock(&foo_lock);
		return -EAGAIN;
	}

	spin_unlock(&foo_lock);
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

	spin_lock(&foo_lock);

	for (i = 0; i < BUFFER_SIZE; i++) {
		find_game(current_uid())->buffer[i] = 0;
	}
	count = getMinimum(count, BUFFER_SIZE);
	retval = copy_from_user(find_game(current_uid())->buffer, ubuf, count);
	if (retval < 0) {
		spin_unlock(&foo_lock);
		return -EINVAL;
	}
	if (find_game(current_uid())->buffer[0] == 's'
	    && find_game(current_uid())->buffer[1] == 't'
	    && find_game(current_uid())->buffer[2] == 'a'
	    && find_game(current_uid())->buffer[3] == 'r'
	    && find_game(current_uid())->buffer[4] == 't'
	    && find_game(current_uid())->buffer[5] == 0) {
		find_game(current_uid())->target_code[0] = 0;
		find_game(current_uid())->target_code[1] = 0;
		find_game(current_uid())->target_code[2] = 1;
		find_game(current_uid())->target_code[3] = 2;
		find_game(current_uid())->game_active = true;
		find_game(current_uid())->num_guesses = 0;
		for (i = 0; i < PAGE_SIZE; i++) {
			find_game(current_uid())->user_view[i] = 0;
		}
		memset(find_game(current_uid())->game_status, '\0',
		       STATUS_SIZE);
		num_games++;
		scnprintf(find_game(current_uid())->game_status, STATUS_SIZE,
			  "Starting game\n");
		spin_unlock(&foo_lock);
		return count;
	} else if (find_game(current_uid())->buffer[0] == 'q'
		   && find_game(current_uid())->buffer[1] == 'u'
		   && find_game(current_uid())->buffer[2] == 'i'
		   && find_game(current_uid())->buffer[3] == 't'
		   && find_game(current_uid())->buffer[4] == 0) {
		if (find_game(current_uid())->game_active) {
			find_game(current_uid())->game_active = false;
			memset(find_game(current_uid())->game_status, '\0',
			       STATUS_SIZE);
			scnprintf(find_game(current_uid())->game_status,
				  STATUS_SIZE,
				  "Game over. The code was %d%d%d%d.\n",
				  find_game(current_uid())->target_code[0],
				  find_game(current_uid())->target_code[1],
				  find_game(current_uid())->target_code[2],
				  find_game(current_uid())->target_code[3]);
		}
		spin_unlock(&foo_lock);
		return count;
	} else if (find_game(current_uid())->buffer[0] == 'c'
		   && find_game(current_uid())->buffer[1] == 'o'
		   && find_game(current_uid())->buffer[2] == 'l'
		   && find_game(current_uid())->buffer[3] == 'o'
		   && find_game(current_uid())->buffer[4] == 'r'
		   && find_game(current_uid())->buffer[5] == 's'
		   && find_game(current_uid())->buffer[6] == ' '
		   && find_game(current_uid())->buffer[8] == 0) {

		if (!file_ns_capable(filp, &init_user_ns, CAP_SYS_ADMIN)) {
			spin_unlock(&foo_lock);
			return -EACCES;
		}

		if (find_game(current_uid())->buffer[7] < '2'
		    || find_game(current_uid())->buffer[7] > '9') {
			spin_unlock(&foo_lock);
			return -EINVAL;
		}

		num_colors = find_game(current_uid())->buffer[7] - '0';
		spin_unlock(&foo_lock);
		return count;

	} else {
		spin_unlock(&foo_lock);
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
 * cs421net_top() - top-half of CS421Net ISR
 * @irq: IRQ that was invoked (ignored)
 * @cookie: Pointer to data that was passed into
 * request_threaded_irq() (ignored)
 *
 * If @irq is CS421NET_IRQ, then wake up the bottom-half. Otherwise,
 * return IRQ_NONE.
 */
static irqreturn_t cs421net_top(int irq, void *cookie)
{
	/* Part 4: YOUR CODE HERE */
	if (irq == CS421NET_IRQ) {
		return IRQ_WAKE_THREAD;
	} else {
		return IRQ_NONE;
	}
}

/**
 * cs421net_bottom() - bottom-half to CS421Net ISR
 * @irq: IRQ that was invoked (ignore)
 * @cookie: Pointer that was passed into request_threaded_irq()
 * (ignored)
 *
 * Fetch the incoming packet, via cs421net_get_data(). If:
 *   1. The packet length is exactly equal to the number of digits in
 *      the target code, and
 *   2. If all characters in the packet are valid ASCII representation
 *      of valid digits in the code, then
 * Set the target code to the new code, and increment the number of
 * types the code was changed remotely. Otherwise, ignore the packet
 * and increment the number of invalid change attempts.
 *
 * During Part 5, update this function to change all codes for all
 * active games.
 *
 * Remember to add appropriate spin lock calls in this function.
 *
 * <em>Caution: The incoming payload is NOT a string; it is not
 * necessarily null-terminated.</em> You CANNOT use strcpy() or
 * strlen() on it!
 *
 * Return: always IRQ_HANDLED
 */
static irqreturn_t cs421net_bottom(int irq, void *cookie)
{
	/* Part 4: YOUR CODE HERE */
	struct list_head *pos;
	struct mm_game *tmp;
	size_t size = 0;
	const char *data;
	int i;
	spin_lock(&foo_lock);
	data = cs421net_get_data(&size);
	if (data == NULL) {
		invalid_guess++;
		spin_unlock(&foo_lock);
		return IRQ_HANDLED;
	}
	if (size != NUM_PEGS) {
		invalid_guess++;
		spin_unlock(&foo_lock);
		return IRQ_HANDLED;
	}

	for (i = 0; i < NUM_PEGS; i++) {
		if (data[i] < '0' || data[i] > (num_colors + '0')) {
			invalid_guess++;
			spin_unlock(&foo_lock);
			return IRQ_HANDLED;
		}
	}

	list_for_each(pos, &game) {
		tmp = list_entry(pos, struct mm_game, list);
		for (i = 0; i < NUM_PEGS; i++) {
			tmp->target_code[i] = data[i] - '0';
		}
	}

	target_code_change++;

	spin_unlock(&foo_lock);
	return IRQ_HANDLED;
}

/**
 * mm_stats_show() - callback invoked when a process reads from
 * /sys/devices/platform/mastermind/stats
 *
 * @dev: device driver data for sysfs entry (ignored)
 * @attr: sysfs entry context (ignored)
 * @buf: destination to store game statistics
 *
 * Write to @buf, up to PAGE_SIZE characters, a human-readable message
 * containing these game statistics:
 *   - Number of pegs (digits in target code)
 *   - Number of colors (range of digits in target code)
 *   - Number of valid network messages (see Part 4)
 *   - Number of invalid network messages (see Part 4)
 *   - Number of active players (see Part 5)
 * Note that @buf is a normal character buffer, not a __user
 * buffer. Use scnprintf() in this function.
 *
 * @return Number of bytes written to @buf, or negative on error.
 */
static ssize_t mm_stats_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	/* Part 3: YOUR CODE HERE */
	spin_lock(&foo_lock);
	memset(buf, '\0', PAGE_SIZE);
	scnprintf(buf, PAGE_SIZE,
		  "CS421 Mastermind Stats\nNumber of pegs: %d\nNumber of colors: %d\nNumber of times code was changed: %d\nNumber of invalid code change attempts: %d\nNumber of games started: %d\n",
		  NUM_PEGS, num_colors, target_code_change, invalid_guess,
		  num_games);
	spin_unlock(&foo_lock);
	return strlen(buf);
}

static DEVICE_ATTR(stats, S_IRUGO, mm_stats_show, NULL);

/**
 * mastermind_probe() - callback invoked when this driver is probed
 * @pdev platform device driver data
 *
 * Return: 0 on successful probing, negative on error
 */
static int mastermind_probe(struct platform_device *pdev)
{

	int retval;
	/* Copy the contents of your original mastermind_init() here. */
	/* YOUR CODE HERE */

	num_colors = 6;
	pr_info("Initializing the game.\n");

	retval = misc_register(&device_mm);
	if (retval < 0) {
		return retval;
	}
	retval = misc_register(&device_mm_ctl);
	if (retval < 0) {
		goto deregister_mm;
	}
	retval =
	    request_threaded_irq(CS421NET_IRQ, cs421net_top, cs421net_bottom,
				 IRQF_SHARED, KBUILD_MODNAME, pdev);
	if (retval < 0) {
		goto deregister_mm_ctl;
	}
	/*
	 * You will need to integrate the following resource allocator
	 * into your code. That also means properly releasing the
	 * resource if the function fails.
	 */
	retval = device_create_file(&pdev->dev, &dev_attr_stats);
	if (retval) {
		pr_err("Could not create sysfs entry\n");
		goto free_threaded_irq;
	}
	cs421net_enable();
	return retval;

free_threaded_irq:free_irq(CS421NET_IRQ, pdev);
deregister_mm_ctl:misc_deregister(&device_mm_ctl);
deregister_mm:misc_deregister(&device_mm);
	return retval;

}

/**
 * mastermind_remove() - callback when this driver is removed
 * @pdev platform device driver data
 *
 * Return: Always 0
 */
static int mastermind_remove(struct platform_device *pdev)
{
	/* Copy the contents of your original mastermind_exit() here. */
	/* YOUR CODE HERE */

	struct mm_game *tmp;
	struct list_head *pos, *q;

	pr_info("Freeing resources.\n");

	list_for_each_safe(pos, q, &game) {
		tmp = list_entry(pos, struct mm_game, list);
		list_del(pos);
		vfree(tmp->user_view);
		kfree(tmp);
	}

	misc_deregister(&device_mm);
	misc_deregister(&device_mm_ctl);
	device_remove_file(&pdev->dev, &dev_attr_stats);
	cs421net_disable();
	free_irq(CS421NET_IRQ, pdev);
	return 0;
}

static struct platform_driver cs421_driver = {
	.driver = {
		   .name = "mastermind",
		   },
	.probe = mastermind_probe,
	.remove = mastermind_remove,
};

static struct platform_device *pdev;

/**
 * cs421_init() -  create the platform driver
 * This is needed so that the device gains a sysfs group.
 *
 * <strong>You do not need to modify this function.</strong>
 */
static int __init cs421_init(void)
{
	pdev = platform_device_register_simple("mastermind", -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	return platform_driver_register(&cs421_driver);
}

/**
 * cs421_exit() - remove the platform driver
 * Unregister the driver from the platform bus.
 *
 * <strong>You do not need to modify this function.</strong>
 */
static void __exit cs421_exit(void)
{
	platform_driver_unregister(&cs421_driver);
	platform_device_unregister(pdev);
}

module_init(cs421_init);
module_exit(cs421_exit);

MODULE_DESCRIPTION("CS421 Mastermind Game++");
MODULE_LICENSE("GPL");
