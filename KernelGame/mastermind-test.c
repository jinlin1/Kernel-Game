/**
 *  @Author Jin Hui Lin (hj73293@umbc.edu)
 *  This file is proj1 unit test,    
 *  Ensures writing and reading to /dev/mm and /dev/ 
 *  performs as expected
 *
 *  Sources:
 *  https://stackoverflow.com/questions/9519648/what-is-the-difference-between-map-shared-and-map-private-in-the-mmap-function
 *  https://stackoverflow.com/questions/12124628/endlessly-looping-when-reading-from-character-device
 *  http://www.xml.com/ldd/chapter/book/ch13.html
 *  https://stackoverflow.com/questions/11952693/how-to-call-unix-commands-from-c-program
 *  https://stackoverflow.com/questions/23011683/collecting-return-value-from-shell-script
 *  https://stackoverflow.com/questions/20460670/reading-a-file-to-string-with-mmap
 */

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

int main(void) {

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

	/** 
	 * Unit test 1
	 * Test write "start" to /dev/mm_ctl
	 * Start the program and checks the return value of 
	 * write callback  
	 * Expected result is 5 because 5 characters were written 
	 * to the device.
	 */
	printf("Test 1: Start game\n");
        int returnVal = write(fdMmCtl, "start", 5);
	CHECK_IS_EQUAL(returnVal, 5);

	/** 
	 * Unit test 2
	 * Test write "start\n" to /dev/mm_ctl
	 * Start the program with a newline character 
	 * Error occurs because newline characters should not be accepted
	 * Expected result is -1 and errno is set to EINVAL  
	 */
	errno = 0;
	printf("Test 2: Start game error\n");
	returnVal = write(fdMmCtl, "start\n", 6);
	CHECK_IS_EQUAL(errno, EINVAL); 

	/** 
	 * Unit test 3
	 * Test write "00" to /dev/mm
	 * Error occurs because 00 is not a valid guess
	 * Expected result is -1 and errno is set to EINVAL 
	 */
	errno = 0;
	printf("Test 3: Guess invalid\n");
	returnVal = write(fdMm, "00", 2);
	CHECK_IS_EQUAL(errno, EINVAL); 

	/** 
	 * Unit test 4
	 * Test write "00000" to /dev/mm
	 * Send the guess "00000" to /dev/mm 
	 * Expected result is 5 because 5 bytes were written 
	 * to the device 
	 */
	printf("Test 4: Valid but incorrect guess\n");
        returnVal = write(fdMm, "00000", 5);
	CHECK_IS_EQUAL(returnVal, 5); 


	/** 
	 * Unit test 5
	 * Test the game status in /dev/mm 
	 * Expected result is a game status that says 2 black pegs 
	 * and 0 white pegs 
	 */
	printf("Test 5: Valid but incorrect guess game status check\n");
	if(read(fdMm, buffer,sizeof(buffer)) == -1) {
		printf("Unable to get value from file");
		exit(-1);
	}
	CHECK_IS_STR_EQUAL(buffer, "Guess 1: 2 black peg(s), 0 white peg(s)\n");
	close(fdMm);

	fdMm = open("/dev/mm", O_RDWR); 

	if(fdMm == -1) {
    		printf("Unable to read file \n");
		exit(-1);
  	}

	/** 
	 * Unit test 6
	 * Test write "0012" to /dev/mm
	 * Send the guess "0012" to /dev/mm 
	 * Expected result is 4 because that's how many 
	 * bytes were written
	 */
	printf("Test 6: Valid and correct guess\n");
	returnVal = write(fdMm, "0012", 4);
	CHECK_IS_EQUAL(returnVal, 4); 

	/** 
	 * Unit test 7
	 * Test the game status in /dev/mm 
	 * Expected result is a game status that says 4 black pegs 
	 * and 0 white pegs
	 */ 
	printf("Test 7: Valid and correct guess game status check\n");
	if(read(fdMm, buffer,sizeof(buffer)) == -1) {
		printf("Unable to get value from file");
		exit(-1);
	}
	CHECK_IS_STR_EQUAL(buffer, "Guess 2: 4 black peg(s), 0 white peg(s)\n");	
	close(fdMm);

	char *f;
        const char * file_name = "/dev/mm";

        int fd = open(file_name, O_RDONLY);
    	if(fd < 0) {
     		printf("Error in opening file to read");
		exit(-1);
    	}


	f = (char *) mmap (0, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    	for (int i = 0; i < PAGE_SIZE; i++) {
		buffer[i] = f[i];
    	}

	/** 
	 * Unit test 8
	 * Memory mmap check 
	 * Retrieve the string that is memory mapped 
	 * Check that the string in the memory mapped is the expected string
	 * First guess is 0000 with 2 black pegs 0 white pegs
	 * Second guess is 0012 with 4 black pegs 0 white pegs
	 */
	printf("Test 8: Valid and correct guess string check\n");

	CHECK_IS_STR_EQUAL(buffer, "Guess 1: 0000  | B2 W0 \nGuess 2: 0012  | B4 W0 \n");

	/** 
	 * Unit test 9
	 * Test read from /dev/mm_ctl 
	 * Reading from mm_ctl is not allowed because
	 * read callback not set 
	 * Expected result is -1   
	 */
	printf("Test 9: Read /dev/mm_ctl error\n");
        returnVal = read(fdMmCtl, buffer, sizeof(buffer));
	CHECK_IS_EQUAL(returnVal, -1);

	/** 
	 * Unit test 10
	 * Test write "quit\n" to /dev/mm_ctl 
	 * Error occurs because there is a newline character  
	 * Expected result is -1 with errno set to EINVAL 
	 */
	errno = 0;
	printf("Test 10: Quit game error\n");
        returnVal = write(fdMmCtl, "quit\n", 5);
	CHECK_IS_EQUAL(errno, EINVAL);

	/** 
	 * Unit test 11
	 * Test write "quitnow12" to /dev/mm_ctl 
	 * Write 9 bytes to the device, which is an 
	 * error because buffer is only 8 bytes 
	 * Expected result is -1 with errno set to EINVAL
	 */
	errno = 0;
	printf("Test 11: Write overfill buffer in /dev/mm_ctl\n");
        returnVal = write(fdMmCtl, "quitnow12", 9);
	CHECK_IS_EQUAL(errno, EINVAL);


	/** 
	 * Unit test 12
	 * Test write "quit" to /dev/mm_ctl 
	 * Expected result is 4 because
	 * that is the number of bytes written to device 
	 */
	printf("Test 12: Successfully quit game\n");
        returnVal = write(fdMmCtl, "quit", 4);
	CHECK_IS_EQUAL(returnVal, 4);



	printf("%u tests passed, %u tests failed.\n", test_passed, test_failed);

}
