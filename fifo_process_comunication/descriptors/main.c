#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int flags;
	int accessMode;


	for (int i=0; i<1023; ++i)
	{
		flags = fcntl(i, F_GETFL);
		if (flags == -1) { continue; }
		accessMode = flags & O_ACCMODE;
		switch (accessMode)
		{
			case O_WRONLY: { printf("File is write-only mode\n"); break; }
			case O_RDONLY: { printf("File is in read-only mode\n"); break; }
			case O_RDWR: { printf("File is in read-write mode!\n"); break; }
			default: break;
		}
		if (flags & O_APPEND) { printf("O_APPEND flag is set"); }
		if (flags & O_NONBLOCK) { printf("O_NONBLOCK flag is set"); }
		//if (flags & O_NOATIME ) { printf("NOATIME flag is set")} nie widzi NOATIME
		if (flags & O_ASYNC) { printf("O_ASYNC flag is set"); }
		if (flags & O_DIRECTORY) { printf("O_DIRECTORY flag is set"); }
		if (flags & O_SYNC) { printf("O_SYNC flag is set"); }
		printf("\n");
		/*   (Some  other  UNIX  implementations allow fcntl() to modify other flags, such as O_SYNC.)
		 * w Kerrisku jest taka linia i własnie nie wiem czy powinienem jakby sprawdzać wszystkie dostępne
		 * flagi jakie mam czy tylko te co w Kerrisku */

	}
	return 0;
}
