#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv){

	char *exec_args[]={"gzip","-d",NULL};

	execvp("gzip",exec_args);

	perror("error executing command");	

	return 0;
}
