#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "funmod.h"

void get_vars(int fd)
{
	printf("Trying to get values\n");
	query_arg_t q;
	if (ioctl(fd, QUERY_GET_VARIABLES, &q)) {
		perror("query_apps ioctl get");
	} else {
		printf("Foo:    %d\n", q.foo);
		printf("Foobar: %d\n", q.foobar);
		printf("Baz:    %d\n", q.baz);
	}
}

void clr_vars(int fd)
{
	if (ioctl(fd, QUERY_CLR_VARIABLES) == -1)
		perror("query_apps ioctl clr");
}

void set_vars(int fd)
{
	int v;
	query_arg_t q;

	printf("Enter foo: ");
	scanf("%d", &v);
	getchar();
	q.foo = v;
	printf("Enter foobar: ");
	scanf("%d", &v);
	getchar();
	q.foobar = v;
	printf("Enter baz: ");
	scanf("%d", &v);
	getchar();
	q.baz = v;

	if (ioctl(fd, QUERY_SET_VARIABLES, &q) == -1)
		perror("query_apps ioctl set");
}

int main(int argc, char **argv)
{
	char *file_name = "/dev/query";
	int fd;
	enum
	{
		e_get,
		e_clr,
		e_set
	} option;

	if (argc == 1) {
		option = e_get;

	} else if (argc == 2) {
		if (strcmp(argv[1], "-g") == 0)
			option = e_get;
		else if (strcmp(argv[1], "-c") == 0)
			option = e_clr;
		else if (strcmp(argv[1], "-s") == 0)
			option = e_set;
		else {
			printf("-g get, -s set, -c clear\n");
			return 1;
		}
	} else {
		printf("-g get, -s set, -c clear\n");
		return 1;
	}
	if ((fd = open(file_name, O_RDWR)) == -1) {
		perror("query_apps open");
		return 2;
	}

	switch (option) {
	case e_get:
		get_vars(fd);
		break;
	case e_clr:
		clr_vars(fd);
		break;
	case e_set:
		set_vars(fd);
		break;
	}

	close(fd);
	return 0;
}
