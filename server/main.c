#include"./server.h"
#if defined(_WIN32) || defined(_WIN64)
int __cdecl main(int argc, char* argv[]){
	server();
	return 0;
}
#elif defined(__unix__) || defined(__linux__) || defined(_POSIX_VERSION)
int main(int argc, char* argv[]){
	server();
	return 0;
}
#endif

