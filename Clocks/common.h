#define MAXBUF     1024
#define CHUNK      255
#define MAXTX2CPUS 14

typedef int Status;

typedef enum 
{
	SUCCESS,
	INSUFFICIENT_RESOURCES,
	GENERIC_ERROR
} STATUS_VALUES;
