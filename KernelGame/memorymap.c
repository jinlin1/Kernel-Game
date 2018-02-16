#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/user.h>

int main(int argc, char const *argv[])
{
    char *f;
    char c;
    const char * file_name = "/dev/mm";

    int fd = open(file_name, O_RDONLY);
    if(fd < 0) {
      printf("ERROR");
    }


    f = (char *) mmap (0, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    for (int i = 0; i < PAGE_SIZE; i++) {
	printf("%c", f[i]);
    }

    return 0;
}
