#include <stdio.h>
#include <windows.h>
#include "dirpath.h"
#include "open_notify.h"

int __wrap_open(const char *pathname, int flags, ...);
int __real_open(const char *pathname, int flags, ...);

int __wrap_open(const char *pathname, int flags, ...)
{
	mode_t mode = 0;
	enum access_type at = ACCESS_READ;

	if(flags & O_WRONLY || flags & O_RDWR)
		at = ACCESS_WRITE;
	if(open_notify(at, pathname) < 0)
		return -1;

	if(flags & O_CREAT) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	} else {
		DWORD attributes;

		attributes = GetFileAttributesA(pathname);

		/* If there was an error getting the file attributes, or if we
		 * are trying to open a normal file, we want to fall through to
		 * the __real_open case. Only things that we know are
		 * directories go through the special dirpath logic.
		 */
		if(attributes != INVALID_FILE_ATTRIBUTES) {
			if(attributes & FILE_ATTRIBUTE_DIRECTORY) {
				return win32_add_dirpath(pathname);
			}
		}
	}
	return __real_open(pathname, flags, mode);
}
