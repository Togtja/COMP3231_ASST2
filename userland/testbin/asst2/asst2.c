#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#define MAX_BUF 500
char teststr[] = "The quick brown fox jumped over the lazy dog.";
char buf[MAX_BUF];

int
main(int argc, char * argv[])
{
        int fd, r, i, j , k;
        (void) argc;
        (void) argv;
		//Test stdout and stderr
        printf("\n**********\n* File Tester\n");

        snprintf(buf, MAX_BUF, "**********\n* write() works for stdout\n");
        write(1, buf, strlen(buf));
        snprintf(buf, MAX_BUF, "**********\n* write() works for stderr\n");
        write(2, buf, strlen(buf));
		//Test of open file
        printf("**********\n* opening new file \"test.file\"\n");
        fd = open("test.file", O_RDWR | O_CREAT );
        printf("* open() got fd %d\n", fd);
        if (fd < 0) {
                printf("ERROR opening file: %s\n", strerror(errno));
                exit(1);
        }
		//Test of writing to open file
        printf("* writing test string\n");
        r = write(fd, teststr, strlen(teststr));
        printf("* wrote %d bytes\n", r);
        if (r < 0) {
                printf("ERROR writing file: %s\n", strerror(errno));
                exit(1);
        }
		//Test of writing to open file twice
        printf("* writing test string again\n");
        r = write(fd, teststr, strlen(teststr));
        printf("* wrote %d bytes\n", r);
        if (r < 0) {
                printf("ERROR writing file: %s\n", strerror(errno));
                exit(1);
        }
        printf("* closing file\n");
        close(fd);
		//test of opening old file
        printf("**********\n* opening old file \"test.file\"\n");
        fd = open("test.file", O_RDONLY);
        printf("* open() got fd %d\n", fd);
        if (fd < 0) {
                printf("ERROR opening file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* reading entire file into buffer \n");
        i = 0;
        do  {
                printf("* attempting read of %d bytes\n", MAX_BUF -i);
                r = read(fd, &buf[i], MAX_BUF - i);
                printf("* read %d bytes\n", r);
                i += r;
        } while (i < MAX_BUF && r > 0);

        printf("* reading complete\n");
        if (r < 0) {
                printf("ERROR reading file: %s\n", strerror(errno));
                exit(1);
        }
		
        k = j = 0;
        r = strlen(teststr);
        do {
                if (buf[k] != teststr[j]) {
					printf("The buf[%d]: %d" ,k, buf[k]);
					printf("and teststr[%d] %d\n", j, teststr[j]);
                        printf("ERROR  file contents mismatch\n");
                        exit(1);
                }
                k++;
                j = k % r;
        } while (k < i);
        printf("* file content okay\n");

		//Test of lseek
        printf("**********\n* testing lseek\n");
        r = lseek(fd, 5, SEEK_SET);
        if (r < 0) {
                printf("ERROR lseek: %s\n", strerror(errno));
                exit(1);
        }

        printf("* reading 10 bytes of file into buffer \n");
        i = 0;
        do  {
                printf("* attempting read of %d bytes\n", 10 - i );
                r = read(fd, &buf[i], 10 - i);
                printf("* read %d bytes\n", r);
                i += r;
        } while (i < 10 && r > 0);
        printf("* reading complete\n");
        if (r < 0) {
                printf("ERROR reading file: %s\n", strerror(errno));
                exit(1);
        }

        k = 0;
        j = 5;
        r = strlen(teststr);
        do {
                if (buf[k] != teststr[j]) {
                        printf("ERROR  file contents mismatch\n");
                        exit(1);
                }
                k++;
                j = (k + 5)% r;
        } while (k < 5);
		
        printf("* file lseek  okay\n");
        printf("* closing file\n");
		close(fd);
		int filenr = 100; //change this to test more or less files open at the same time
		//Test of multiple files open at once
		printf("\nTEST OF %d OPEN AT THE SAME TIME:\n", filenr);
		int _fd[200]; //MAX should be 128, but to test that everything works we'll over do it
		for (i = 0; i < filenr; i++) {
			//printf("**********\n* opening new file \"test.file\"\n");
			char filename[200];
			snprintf(filename, 200, "%d", i);
			strcat(filename, "test.file");
			_fd[i] = open(filename, O_RDWR | O_CREAT);
			printf("* open() got fd %d\n", _fd[i]);
			if (_fd[i] < 0) {
				printf("ERROR opening file: %s\n", strerror(errno));
				exit(1);
			}

			r = write(_fd[i], teststr, strlen(teststr));
			if (r < 0) {
				printf("ERROR writing file: %s\n", strerror(errno));
				exit(1);
			}

			r = write(_fd[i], teststr, strlen(teststr));
			if (r < 0) {
				printf("ERROR writing file: %s\n", strerror(errno));
				exit(1);
			}
		}
		//Close all the files we opened
		for (i = 0; i < filenr; i++) {
			printf("close fd %d\n",_fd[i]);
			close(_fd[i]);
		}
		//Test if I can open the multiple files I created
		printf("\nTEST OF %d RE-OPEN AND WRITE ALL INTO:\n", filenr);
		for  (int w = 0; w < filenr; w++) {
			char filename[200];
			snprintf(filename, 200, "%d", w);
			strcat(filename, "test.file");
			_fd[w] = open(filename, O_RDONLY);
			printf("* open() got fd %d\n", _fd[w]);
			if (_fd[w] < 0) {
				printf("ERROR opening file: %s\n", strerror(errno));
				exit(1);
			}

			i = 0;
			do {
				r = read(_fd[w], &buf[i], MAX_BUF - i);
				i += r;
			} while (i < MAX_BUF && r > 0);

			if (r < 0) {
				printf("ERROR reading file: %s\n", strerror(errno));
				exit(1);
			}
			k = j = 0;
			r = strlen(teststr);
			do {
				if (buf[k] != teststr[j]) {
					printf("ERROR file contents mismatch\n");
					exit(1);
				}
				k++;
				j = k % r;
			} while (k < i);
			printf("* file content okay\n");
			//close(_fd[w]);
		}

		//Close all the files we opened
		for (int i = 0; i < filenr; i++) {
			close(_fd[i]);
		}
		//Testing of dup
		printf("\nTEST OF DUP2()\n");
		//We create a file
		int og = open("dup1.file", O_RDWR | O_CREAT); //The original file
		printf("* open() got fd %d\n", og);
		//We write a file
		char tempstr[] = "A test of duplication";
		char buf2[MAX_BUF]; 
		char buf3[MAX_BUF];
		r = write(og, tempstr, strlen(tempstr));
		if (r < 0) {
			printf("ERROR writing file: %s\n", strerror(errno));
			exit(1);
		}
		close(og);
		og = open("dup1.file", O_RDWR);
		printf("* open() got fd %d\n", og);
		//Se if we correctly wrote to said file
		i = 0;
		do {
			r = read(og, &buf2[i], MAX_BUF - i);
			i += r;
		} while (i < MAX_BUF && r > 0);

		if (r < 0) {
			printf("ERROR reading file: %s\n", strerror(errno));
			exit(1);
		}
		k = j = 0;
		r = strlen(tempstr);
		do {
			if (buf2[k] != tempstr[j]) {
				printf("ERROR  file contents mismatch\n");
				exit(1);
			}
			k++;
			j = k % r;
		} while (k < i);
		printf("* file content okay\n");
		close(og);
		//we dup that file
		og = open("dup1.file", O_RDWR);
		int dupfd = 10; //Manualy give it an fd
		int dup = dup2(og, dupfd);
		printf("the dup is: %d\n", dup);
		//we check if new file and old file is same
		i = 0;
		do {
			r = read(og, &buf3[i], MAX_BUF - i);
			i += r;
		} while (i < MAX_BUF && r > 0);

		if (r < 0) {
			printf("ERROR reading file: %s\n", strerror(errno));
			exit(1);
		}
		printf("Buff = %s\n", buf3);
		printf("temp = %s\n", tempstr);
		k = j = 0;
		r = strlen(tempstr);
		do {
			if (buf3[k] != tempstr[j]) {
				printf("ERROR  file contents mismatch\n");
				exit(1);
			}
			k++;
			j = k % r;
		} while (k < i);
		printf("* file content okay\n");

		//og and dup has not been closed
		printf("OG AND DUP STILL UP:\n");
		for (i = 0; i < filenr; i++) {
			_fd[i] = open("test.file", O_RDWR | O_CREAT);
			printf("* open() got fd %d\n", _fd[i]);
			if (_fd[i] < 0) {
				printf("ERROR opening file: %s\n", strerror(errno));
				exit(1);
			}

			r = write(_fd[i], teststr, strlen(teststr));
			if (r < 0) {
				printf("ERROR writing file: %s\n", strerror(errno));
				exit(1);
			}

			r = write(_fd[i], teststr, strlen(teststr));
			if (r < 0) {
				printf("ERROR writing file: %s\n", strerror(errno));
				exit(1);
			}
		}

		//Close all the files we opened
		
		for (int i = 0; i < filenr; i++) {
			close(_fd[i]);
		}
		printf("OG AND DUP IS UP:\n");
		close(og);
		printf("OG IS CLOSED:\n");
		close(dup);
		printf("OG AND DUP IS CLOSED:\n");
		//og and dup has been closed
		for (i = 0; i < filenr; i++) {
			_fd[i] = open("test.file", O_RDWR | O_CREAT);
			printf("* open() got fd %d\n", _fd[i]);
			if (_fd[i] < 0) {
				printf("ERROR opening file: %s\n", strerror(errno));
				exit(1);
			}

			r = write(_fd[i], teststr, strlen(teststr));
			if (r < 0) {
				printf("ERROR writing file: %s\n", strerror(errno));
				exit(1);
			}

			r = write(_fd[i], teststr, strlen(teststr));
			if (r < 0) {
				printf("ERROR writing file: %s\n", strerror(errno));
				exit(1);
			}
		}
		for (int i = 0; i < filenr; i++) {
			close(_fd[i]);
		}
        return 0;
}
