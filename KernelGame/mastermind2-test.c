/**
 *  @Author Jin Hui Lin (hj73293@umbc.edu)
 *  This file is proj2 unit test,    
 *  Ensures writing and reading to /dev/mm and /dev/ 
 *  performs as expected
 *
 *  Sources:
 *  https://elixir.free-electrons.com/linux/latest/source/kernel/resource.c#L116
 *  http://elixir.free-electrons.com/linux/v4.12.14/source/kernel/capability.c#L428
 *  http://www.roman10.net/2011/07/28/linux-kernel-programminglinked-list/
 *  https://isis.poly.edu/kulesh/stuff/src/klist/
 *  https://github.com/torvalds/linux/blob/master/include/linux/list.h
 */

/**
 * Initial Setup:
 * ------------
 * sudo adduser bob
 * id bob
 * id
 * ------------
 * Run on command line: sudo adduser bob
 * Ensure bob has user id of 1001 
 * by running on command line: id bob
 * Ensure current user has user id of 1000
 * by running on command line: id
 *
 * Steps to run through test:
 * -----------
 * ./master-test
 * sudo ./master-test
 * su bob
 * ./master-test
 * -----------
 * First run the test under user id 1000
 * Then proceed to run test under root
 * Finally log in to bob and run test 
 * with user id 1001  
 */

#define _BSD_SOURCE

#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/fsuid.h>
#include "cs421net.h"

char buffer[PAGE_SIZE];
unsigned test_passed;
unsigned test_failed;

#define CHECK_IS_NOT_NULL(ptrA) \
        do { \
                if ((ptrA) != NULL) { \
                        test_passed++; \
                } else { \
                        test_failed++; \
                        printf("%d: FAIL\n", __LINE__); \
                } \
        } while(0);

#define CHECK_IS_EQUAL(valA,valB) \
        do { \
                if ((valA) == (valB)) { \
                        test_passed++; \
                } else { \
                        test_failed++; \
                        printf("%d: FAIL\n", __LINE__); \
                } \
        } while(0);

/**
 * Check if strings are equals 
 * Increments test_passed if strings match
 * and test_failed if strings failed
 *
 * @param strings a and b 
 * @return non
 *
 */
void CHECK_IS_STR_EQUAL(char* a, char* b) {
        int ret = strcmp(a, b);
        if(ret == 0) {
                test_passed++;
        } else {
                test_failed++;
        }
}

/** 
 * Unit test 1
 * Test write "start" to /dev/mm_ctl
 * Start the program and checks the return value of 
 * write callback  
 * Expected result is 5 because 5 characters were written 
 * to the device.
 */
void unitTest1(int fdMmCtl) {
	printf("Test 1: Start game\n");
        int returnVal = write(fdMmCtl, "start", 5);
	CHECK_IS_EQUAL(returnVal, 5);
}

/** 
 * Unit test 2
 * Test write "start\n" to /dev/mm_ctl
 * Start the program with a newline character 
 * Error occurs because newline characters should not be accepted
 * Expected result is -1 and errno is set to EINVAL  
 */
void unitTest2(int fdMmCtl) {
	errno = 0;
	printf("Test 2: Start game error\n");
	if(write(fdMmCtl, "start\n", 6)) {}
	CHECK_IS_EQUAL(errno, EINVAL); 
}

/** 
 * Unit test 3
 * Test write "00" to /dev/mm
 * Error occurs because 00 is not a valid guess
 * Expected result is -1 and errno is set to EINVAL 
 */
void unitTest3(int fdMm) {
	errno = 0;
	printf("Test 3: Guess invalid\n");
	if(write(fdMm, "00", 2)) {}
	CHECK_IS_EQUAL(errno, EINVAL); 
}

/** 
 * Unit test 4
 * Test write "00000" to /dev/mm
 * Send the guess "00000" to /dev/mm 
 * Expected result is 5 because 5 bytes were written 
 * to the device 
 */

void unitTest4(int fdMm) {
	printf("Test 4: Valid but incorrect guess\n");
        int returnVal = write(fdMm, "00000", 5);
	CHECK_IS_EQUAL(returnVal, 5); 
}

/** 
 * Unit test 5
 * Test the game status in /dev/mm 
 * Expected result is a game status that says 2 black pegs 
 * and 0 white pegs 
 */
void unitTest5(int fdMm) {
	printf("Test 5: Valid but incorrect guess game status check\n");
	if(read(fdMm, buffer,sizeof(buffer)) == -1) {
		printf("Unable to get value from file");
		exit(-1);
	}
	CHECK_IS_STR_EQUAL(buffer, "Guess 1: 2 black peg(s), 0 white peg(s)\n");
}

/** 
 * Unit test 6
 * Test write "0012" to /dev/mm
 * Send the guess "0012" to /dev/mm 
 * Expected result is 4 because that's how many 
 * bytes were written
 */
void unitTest6(int fdMm) {
	printf("Test 6: Valid and correct guess\n");
	int returnVal = write(fdMm, "0012", 4);
	CHECK_IS_EQUAL(returnVal, 4); 
}

/** 
 * Unit test 7
 * Test the game status in /dev/mm 
 * Expected result is a game status that says 4 black pegs 
 * and 0 white pegs
 */ 
void unitTest7(int fdMm) {
	printf("Test 7: Valid and correct guess game status check\n");

	memset(buffer, '\0', sizeof(buffer));
	if(read(fdMm, buffer,sizeof(buffer)) == -1) {
		printf("Unable to get value from file");
		exit(-1);
	}
	CHECK_IS_STR_EQUAL(buffer, "Correct! Game over\n");
}

/** 
 * Unit test 8
 * Memory mmap check 
 * Retrieve the string that is memory mapped 
 * Check that the string in the memory mapped is the expected string
 * First guess is 0000 with 2 black pegs 0 white pegs
 * Second guess is 0012 with 4 black pegs 0 white pegs
 */
void unitTest8(int fd) {
	char *f;
	printf("Test 8: Valid and correct guess string check\n");

	memset(buffer, '\0', sizeof(buffer));
	f = (char *) mmap (0, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    	for (int i = 0; i < PAGE_SIZE; i++) {
		buffer[i] = f[i];
    	}
	CHECK_IS_STR_EQUAL(buffer, "Guess 1: 0000  | B2 W0 \nCorrect! Game over\n");
}

/** 
 * Unit test 9
 * Test read from /dev/mm_ctl 
 * Reading from mm_ctl is not allowed because
 * read callback not set 
 * Expected result is -1   
 */
void unitTest9(int fdMmCtl) {
	printf("Test 9: Read /dev/mm_ctl error\n");
        int returnVal = read(fdMmCtl, buffer, sizeof(buffer));
	CHECK_IS_EQUAL(returnVal, -1);
}

/** 
 * Unit test 10
 * Test write "quit\n" to /dev/mm_ctl 
 * Error occurs because there is a newline character  
 * Expected result is -1 with errno set to EINVAL 
 */
void unitTest10(int fdMmCtl) {
	errno = 0;
	printf("Test 10: Quit game error\n");
        if(write(fdMmCtl, "quit\n", 5)){}
	CHECK_IS_EQUAL(errno, EINVAL);
}

/** 
 * Unit test 11
 * Test write "quitnow12" to /dev/mm_ctl 
 * Write 9 bytes to the device, which is an 
 * error because buffer is only 8 bytes 
 * Expected result is -1 with errno set to EINVAL
 */
void unitTest11(int fdMmCtl) {
	errno = 0;
	printf("Test 11: Write overfill buffer in /dev/mm_ctl\n");
        if(write(fdMmCtl, "quitnow12", 9)){}
	CHECK_IS_EQUAL(errno, EINVAL);
}

/** 
 * Unit test 12
 * Test write "colors 5" to /dev/mm_ctl 
 * Expected result is errno EACCES
 * because permission is not granted to user 
 */
void unitTest12(int fdMmCtl) {
	errno = 0;
	printf("Test 12: Change color error\n");
        if(write(fdMmCtl, "colors 5", 8)){}
	CHECK_IS_EQUAL(errno, EACCES);
}

/** 
 * Unit test 13
 * Test send "4444" as interrupt  
 * Expected result is true 
 * and the target code is changed to 4444 
 * and global count for code change goes up by 1
 */
void unitTest13(void) {
	char buffer[4] = {'4', '4', '4', '4'};
	printf("Test 13: Sending new target code\n");
	CHECK_IS_EQUAL(true, cs421net_send(buffer, 4));
	usleep(1000);
}

/** 
 * Unit test 14
 * Test send "44444" as interrupt  
 * Expected result is true 
 * and but target code is not changed
 * and invalid count goes up by 1 
 */
void unitTest14(void) {
	char buffer[5] = {'4', '4', '4', '4', '4'};
	printf("Test 14: Sending new invalid target code\n");
	CHECK_IS_EQUAL(true, cs421net_send(buffer, 5));
	usleep(1000);
}

/** 
 * Unit test 15
 * Check the contents in the stats
 */
void unitTest15(int stats) {
	printf("Test 15: Verify contents in stats\n");
	memset(buffer, '\0', sizeof(buffer));
	if(read(stats, buffer,sizeof(buffer)) == -1) {
		printf("Unable to get value from file");
		exit(-1);
	}
	CHECK_IS_STR_EQUAL(buffer, "CS421 Mastermind Stats\nNumber of pegs: 4\nNumber of colors: 6\nNumber of times code was changed: 1\nNumber of invalid code change attempts: 1\nNumber of games started: 1\n");
}

/** 
 * Unit test 16
 * Test write "quit" to /dev/mm_ctl 
 * Expected result is 4 because
 * that is the number of bytes written to device 
 */
void unitTest16(int fdMmCtl) {
	printf("Test 16: Successfully quit game\n");
        int returnVal = write(fdMmCtl, "quit", 4);
	CHECK_IS_EQUAL(returnVal, 4);
}

/** 
 * Unit test 17
 * Test write "colors 1" to /dev/mm_ctl 
 * Expected result is errno EINVAL 
 * because permission is set but 1 is not valid color
 */
void unitTest17(int fdMmCtl) {
	errno = 0;
	printf("Test 17: Invalid color change error under root\n");
        if(write(fdMmCtl, "colors 1", 8)){}
	CHECK_IS_EQUAL(errno, EINVAL);
}

/** 
 * Unit test 18
 * Test write "colors  " to /dev/mm_ctl 
 * Expected result is errno EINVAL 
 * because command isn't 8 bytes 
 */
void unitTest18(int fdMmCtl) {
	errno = 0;
	printf("Test 18: Not correct color change command under root\n");
        if(write(fdMmCtl, "colors  ", 9)){}
	CHECK_IS_EQUAL(errno, EINVAL);
}

/** 
 * Unit test 19
 * Test write "colors 7" to /dev/mm_ctl 
 * Expected result is 8 bytes 
 */
void unitTest19(int fdMmCtl) {
	printf("Test 19: Successfully change color under root\n");
        int returnVal = write(fdMmCtl, "colors 7", 8);
	CHECK_IS_EQUAL(returnVal, 8);

}

/** 
 * Unit test 20
 * Test write "9000" to /dev/mm
 * Expected result is errno EINVAL 
 * because the color ranges from 0 to 7
 */

void unitTest20(int fdMm) {
	errno = 0;
	printf("Test 20: Invalid color guess\n");
        if(write(fdMm, "9000", 4)){}
	CHECK_IS_EQUAL(errno, EINVAL); 
}

/** 
 * Unit test 20
 * Test write "0777" to /dev/mm
 * Send the guess "0777" to /dev/mm 
 * Expected result is 4 because that's how many 
 * bytes were written
 */
void unitTest21(int fdMm) {
	printf("Test 21: Valid guess with new color\n");
	int returnVal = write(fdMm, "0777", 4);
	CHECK_IS_EQUAL(returnVal, 4); 
}

/** 
 * runTest 
 * Run this test when uid is 1000 
 */
void runTest(void) {

	int fdMm;
	int fdMmCtl;

 	fdMm = open("/dev/mm", O_RDWR); 

	if(fdMm == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}

	fdMmCtl = open("/dev/mm_ctl", O_WRONLY);

	if(fdMmCtl == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}

	unitTest1(fdMmCtl);
	unitTest2(fdMmCtl);
	unitTest3(fdMm);
	unitTest4(fdMm);
	unitTest5(fdMm);
	close(fdMm);
	fdMm = open("/dev/mm", O_RDWR); 
	if(fdMm == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}
	unitTest6(fdMm);
	unitTest7(fdMm);
	close(fdMm);

        const char * file_name = "/dev/mm";
        int fd = open(file_name, O_RDONLY);
    	if(fd < 0) {
     		printf("Error in opening file to read");
		exit(-1);
    	}
	unitTest8(fd);
	unitTest9(fdMmCtl);
	unitTest10(fdMmCtl);
	unitTest11(fdMmCtl);
	unitTest12(fdMmCtl);
	unitTest13();
	unitTest14();

	int stat = open("/sys/devices/platform/mastermind/stats", O_RDONLY);
	unitTest15(stat);

	unitTest16(fdMmCtl);

	printf("Player:%d, %u tests passed, %u tests failed.\n", getuid(), test_passed, test_failed);

	close(stat);
	close(fd);
	close(fdMm);
	close(fdMmCtl);
}

/** 
 * runTestRoot 
 * Run this test when uid is 0 
 */
void runTestRoot(void) {

	int fdMm;
	int fdMmCtl;

 	fdMm = open("/dev/mm", O_RDWR); 

	if(fdMm == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}

	fdMmCtl = open("/dev/mm_ctl", O_WRONLY);

	if(fdMmCtl == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}

	unitTest17(fdMmCtl);
	unitTest18(fdMmCtl);
	unitTest19(fdMmCtl);
	printf("Player:%d, %u tests passed, %u tests failed.\n", getuid(), test_passed, test_failed);

	close(fdMm);
	close(fdMmCtl);
}

/** 
 * runTestBob 
 * Run this test when uid is 1001 
 */
void runTestBob(void) {

	int fdMm;
	int fdMmCtl;

 	fdMm = open("/dev/mm", O_RDWR); 

	if(fdMm == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}

	fdMmCtl = open("/dev/mm_ctl", O_WRONLY);

	if(fdMmCtl == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}

        if(write(fdMmCtl, "start", 5)){}
 	unitTest20(fdMm);	
	unitTest21(fdMm);

	printf("Player:%d, %u tests passed, %u tests failed.\n", getuid(), test_passed, test_failed);

	close(fdMm);
	close(fdMmCtl);
}

int main(void) {

	cs421net_init();
	uid_t id;
	id = getuid();
	switch (id) {
		case 0:
			runTestRoot();
			break;
		case 1000:
			runTest();
			break;
		case 1001:
			runTestBob();
			break;
		default:
			break;
	}
}
