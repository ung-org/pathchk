#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int pathchk(const char *pathname, int posix_max, int dangerous_globs)
{
	size_t pathlen = strlen(pathname);
	size_t chklen = 0;
	char chkpath[pathlen + 1];
	char chkname[pathlen + 1];

	if (dangerous_globs && pathlen == 0) {
		fprintf(stderr, "pathchk: empty pathname\n");
		return 0;
	}

	if (pathname[0] == '/') {
		strcpy(chkpath, "/");
		chklen = 1;
	} else {
		memset(chkpath, 0, sizeof(chkpath));
		for (chklen = 0; pathname[chklen] != '/' && chklen < pathlen; chklen++) {
			chkpath[chklen] = pathname[chklen];
		}
	}

	while (chklen < pathlen) {
		memset(chkname, 0, sizeof(chkname));
		size_t namelen = 0;
		for (size_t namelen = 0; pathname[namelen + chklen] != '/' && namelen + chklen < pathlen; namelen++) {
			chkname[namelen] = pathname[namelen + chklen];
		}

		#if 0
		printf("checking '%s' in '%s'\n", chkname, chkpath);
		#endif

		size_t path_max = posix_max ? _POSIX_PATH_MAX : pathconf(chkpath, _PC_PATH_MAX);
		if (namelen + chklen + 1 > path_max) {
			fprintf(stderr, "pathchk: %s/%s is longer than %s\n", chkpath, chkname, posix_max ? "_POSIX_PATH_MAX" : "PATH_MAX");
		}

		size_t name_max = posix_max ? _POSIX_NAME_MAX : pathconf(chkpath, _PC_NAME_MAX);
		if (namelen > name_max) {
			fprintf(stderr, "pathchk: %s/%s is longer than %s\n", chkpath, chkname, posix_max ? "_POSIX_NAME_MAX" : "NAME_MAX");
		}

		if (!posix_max && !(access(chkpath, X_OK) == 0)) {
			fprintf(stderr, "pathchk: %s is not searchable\n", chkpath);
		}

		if (posix_max) {
			/* check portable character set */
		} else {
			/* check directory allowed characters */
		}

		if (dangerous_globs && chkname[0] == '-') {
			fprintf(stderr, "pathchk: %s begins with '-'\n", chkname);
		}

		strcat(chkpath, "/");
		strcat(chkpath, chkname);
		chklen += (namelen + 2);
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	int posix_max = 0;
	int dangerous_globs = 0;
	int c;

	setlocale(LC_ALL, "");

	while ((c = getopt(argc, argv, "pP")) != -1) {
		switch (c) {
		case 'p':
			posix_max = 1;
			break;

		case 'P':
			dangerous_globs = 1;
			break;

		default:
			return 1;
		}
	}

	if (optind >= argc) {
		return 1;
	}

	int ret = 0;
	do {
		ret |= pathchk(argv[optind++], posix_max, dangerous_globs);
	} while (argv[optind]);

	return ret;
}
