/*
 * ch15/sed2/userapp_sed/userapp_sed2.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 15 : Timers, kernel threads and more
 ****************************************************************
 * Brief Description:
 * The user space app for our 'sed' v2 - simple encrypt decrypt - demo driver.
 * Primary demo : kernel thread usage within a driver (or LKM).
 *
 * Summary:
 *                  User-space app
 *                  /    |      |  \
 *          |-------     |      |   -------|
 *op:    encrypt    retrieve  decrypt    destroy
 *      <------------ sed2 driver -------------->
 *by:   [kthread]    [ioctl]  [kthread]   [ioctl]
 *
 * For details, pl refer the book, Ch 15.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "../sed_common.h"

/*
 * destroy_msg
 * Destroys the message within the underlying driver (sed2);
 */
static int destroy_msg(int fd)
{
	if (ioctl(fd, IOCTL_LLKD_SED_IOC_DESTROY_MSG, NULL) == -1) {
		perror("ioctl IOCTL_LLKD_SED_IOC_DESTROY_MSG failed");
		return -1;
	}
	return 0;
}

/*
 * decrypt_msg
 * Decrypts the message that we sent via the 'encrypt' operation. The decrypted
 * message resides within the underlying driver; our kernel thread will decrypt
 * it. To retrieve it, use the 'retrieve' operation.
 */
static int decrypt_msg(int fd, char *msg, char *prg)
{
	struct sed_ds *kd;

	kd = calloc(sizeof(struct sed_ds), 1);
	if (!kd) {
		fprintf(stderr, "%s: calloc() kd failed!\n", prg);
		return -1;
	}
	kd->data_xform = XF_DECRYPT;

	/* The 'returned' data payload will be in the last parameter kd;
	 * iow, it's an "in-out" / "value-result" style parameter
	 */
	if (ioctl(fd, IOCTL_LLKD_SED_IOC_DECRYPT_MSG, kd) == -1) {
		if (errno == ETIMEDOUT) {
			fprintf(stderr,
				"ioctl IOCTL_LLKD_SED_IOC_DECRYPT_MSG failed: *** Operation Timed Out ***\n");
			memset(kd, 0, sizeof(struct sed_ds));
		} else
			perror("ioctl IOCTL_LLKD_SED_IOC_DECRYPT_MSG failed");
		free(kd);
		return -1;
	}

	memcpy(msg, kd->shmem, kd->len);
	free(kd);

	return 0;
}

/*
 * retrieve_msg
 * Retrieves the message from the underlying driver (sed2);
 * the second and third parameters @len and @msg are 'in-out' (or value-result)
 * parameters, return values in effect.
 */
static int retrieve_msg(int fd, int *len, char *msg, char *prg)
{
	struct sed_ds *kd;

	// Send a 'packet' to the underlying driver
	kd = calloc(sizeof(struct sed_ds), 1);
	if (!kd) {
		fprintf(stderr, "%s: calloc() kd failed!\n", prg);
		return -1;
	}
	kd->data_xform = XF_RETRIEVE;

	/* The 'returned' data payload will be in the last parameter kd;
	 * iow, it's an "in-out" / "value-result" style parameter
	 */
	if (ioctl(fd, IOCTL_LLKD_SED_IOC_RETRIEVE_MSG, kd) == -1) {
		if (errno == ETIMEDOUT) {
			fprintf(stderr,
				"ioctl IOCTL_LLKD_SED_IOC_RETRIEVE_MSG failed: *** Operation Timed Out ***\n");
			memset(kd, 0, sizeof(struct sed_ds));
		} else
			perror("ioctl IOCTL_LLKD_SED_IOC_RETRIEVE_MSG failed");
		free(kd);
		return -1;
	}

	*len = kd->len;
	memcpy(msg, kd->shmem, kd->len);
	free(kd);

	return 0;
}

/*
 * encrypt_msg
 * Sends a cleartext message to the underlying driver; there, our kernel thread
 * will encrypt it and store it.
 */
static int encrypt_msg(int fd, char *msg, char *prg)
{
	struct sed_ds *kd;

	kd = calloc(sizeof(struct sed_ds), 1);
	if (!kd) {
		fprintf(stderr, "%s: calloc() kd failed!\n", prg);
		return -1;
	}

	kd->data_xform = XF_ENCRYPT;
	kd->len = strlen(msg);
	memcpy(kd->shmem, msg, strlen(msg));

	/*
	 * Since our sed2 driver uses a kernel thread, we can't simply 'return'
	 * data in the usual fashion... Instead, the user must first write data
	 * to the driver via the 'encrypt' ioctl, and subsequently issue another
	 * 'retrieve' ioctl to obtain the encrypted message in user space.
	 */
	if (ioctl(fd, IOCTL_LLKD_SED_IOC_ENCRYPT_MSG, kd) == -1) {
		perror("ioctl IOCTL_LLKD_SED_IOC_ENCRYPT_MSG failed");
		free(kd);
		return -1;
	}
	free(kd);

	return 0;
}

/*
 * Interesting: I had to leave out the do { ... } while(0) style in the macros
 * below; why? with it, the control statements (like the 'continue' I use) works
 * only within the local do { ... } while(0) and *not* for the actual intended code!
 */
#define CHECK_FOR_ENCRYPT(first) \
	if (first == 1) { \
		printf("You need to first Encrypt the message, thereby pushing it to" \
		" the kernel driver\n"); \
		continue; \
	}
#define CHECK_FOR_DESTROYED(destroyed) \
	if (destroyed) { \
		printf("The message is Destroyed; please restart the app now ...\n"); \
		continue; \
	}

static void menu_drive(int fd, char *buf, char *prg)
{
	static int first = 1;
	int choice = 0, len = 0, ret, destroyed = 0;
	char *msg = calloc(MAX_DATA, 1);

	if (!msg) {
		fprintf(stderr, "%s: calloc (msg) failed\n", prg);
		close(fd);
		exit(EXIT_FAILURE);
	}

	printf
	    ("---< Welcome to the SED (Simple Encrypt Decrypt) v2 User mode app >---\n"
	     "((c) 'Learn Linux Kernel Development', Kaiwan N Billimoria, Packt)\n\n");

	/* Check, is the driver already holding a message packet? */
	ret = retrieve_msg(fd, &len, msg, prg);
	if (ret == 0 && len > 0) {
		printf("Detected that the driver already holds a message:\n"
		       "len = %d, msg = \"%.*s\"\n"
		       "RECOMMENDATION: Destroy the message and restart app.",
		       len, len, msg);
		first = 0;
	} else
		printf("The message we shall work with is:\n\"%s\"\n", buf);

	while (1) {
		printf("\n   ***  Menu  ***\n\
  --- Message Control ---\n\
1. Encrypt the message\n\
2. Retrieve the message (from the driver)\n\
3. Decrypt the message (that was encrypted in (1))\n\
4. Destroy the message\n\
     --- Kernel Logs ---\n\
5. View the kernel log (via dmesg(1))\n\
6. Clear the kernel log (via sudo)\n\
7. Quit\n\
>  ");
		if (scanf("%d", &choice) == EOF) {
			printf
			    ("%s: reading your choice failed; pl try again...\n",
			     prg);
			continue;
		}

		switch (choice) {
		case 1:	/* Encrypt the message (in the kernel driver) */
			CHECK_FOR_DESTROYED(destroyed);
			ret = encrypt_msg(fd, buf, prg);
			if (ret == -1) {
				if (errno == EBADRQC)
					fprintf(stderr,
						"Invalid request, message is already encrypted\n");
				else
					fprintf(stderr,
						"%s: *** encrypt op failed (see kernel logs) ***\n",
						prg);
				first = 1;
			} else {
				printf
				    ("\n---> Message ENCRYPTED in the kernel driver; retrieve to see <---\n"
				     "     (ioctl IOCTL_LLKD_SED_IOC_ENCRYPT_MSG successful)\n");
				first = 0;
			}
			break;
		case 2:	/* Retrieve the (en|de-crypted) message (from the kernel driver) */
			CHECK_FOR_DESTROYED(destroyed);
			CHECK_FOR_ENCRYPT(first);
			ret = retrieve_msg(fd, &len, msg, prg);
			if (ret == -1)
				fprintf(stderr,
					"%s: *** retrieve op failed (see kernel logs) ***\n",
					prg);
			else
				printf
				    ("\n---> Message RETRIEVED from the kernel driver <---\n"
				     "     (ioctl IOCTL_LLKD_SED_IOC_RETRIEVE_MSG successful)\n"
				     "     len=%d, msg=\"%.*s\"\n", len, len,
				     msg);
			break;
		case 3:	/* Decrypt the message (in the kernel driver) */
			CHECK_FOR_DESTROYED(destroyed);
			CHECK_FOR_ENCRYPT(first);
			ret = decrypt_msg(fd, msg, prg);
			if (ret == -1) {
				if (errno == EBADRQC)
					fprintf(stderr,
						"Invalid request, message is already decrypted\n");
				else
					fprintf(stderr,
						"%s: *** decrypt op failed (see kernel logs) ***\n",
						prg);
			} else
				printf
				    ("\n---> Message DECRYPTED in the kernel driver; retrieve to see <---\n"
				     "     (ioctl IOCTL_LLKD_SED_IOC_DECRYPT_MSG successful)\n");
			break;
		case 4:	/* Destroy the message (in the kernel driver) */
			CHECK_FOR_DESTROYED(destroyed);
			CHECK_FOR_ENCRYPT(first);
			ret = destroy_msg(fd);
			if (ret == -1)
				fprintf(stderr,
					"%s: *** destroy op failed (see kernel logs) ***\n",
					prg);
			else {
				printf
				    ("\n---> Message DESTROYED within the kernel driver <---\n"
				     "     (ioctl IOCTL_LLKD_SED_IOC_DESTROY_MSG successful)\n"
				     "     Restart this app please ...\n");
				destroyed = 1;
			}
			break;
		case 5:	/* View kernel log (dmesg) */
			printf("---> View kernel log : dmesg(1) <---\n");
			if (system("dmesg") < 0)
				fprintf(stderr,
					"%s: *** system(3) on dmesg(1) failed ***\n",
					prg);
			break;
		case 6:	/* Clear kernel log (sudo dmesg -C) */
			printf("---> Clear kernel log : sudo dmesg -C <---\n");
			if (system("sudo dmesg -C") < 0)
				fprintf(stderr,
					"%s: *** system(3) on sudo dmesg -C failed ***\n",
					prg);
			break;
		case 7:	/* Exit */
			printf("---> Exiting on user choice <---\n");
			free(msg);
			close(fd);
			exit(EXIT_SUCCESS);
		default:
			printf("---> Unknown choice (%d) <---\n", choice);
			getchar();
		}		// switch
	}			// while (1)
}

int main(int argc, char **argv)
{
	int fd;
	char buf[MAX_DATA];

	if (argc < 3) {
		fprintf(stderr, "Usage: %s device_file message_to_encrypt\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}
	if (strlen(argv[2]) == 0 || strlen(argv[2]) > MAX_DATA) {
		fprintf(stderr, "%s: invalid message\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	memcpy(buf, argv[2], MAX_DATA);

	fd = open(argv[1], O_RDWR, 0);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	printf("device opened: fd=%d\n", fd);

	menu_drive(fd, buf, argv[0]);
	exit(EXIT_SUCCESS);	// should not reach here..
}
