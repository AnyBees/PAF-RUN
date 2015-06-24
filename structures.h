typedef struct PS PS;
typedef struct DS DS;
typedef struct TSK TSK;
typedef struct lTSK lTSK;

struct DS;

struct PS			// Primary server
{
	int name;		// name of server
	int ratenum;		// rate of utilisation (num/div)
	int rateden;
	int size;		// number of sons to look for EDF
	int son[10];	        // Pointers to the sons of tree
	int father;		//useful to recover the deadlines to hight level DS of the tree
};

struct DS			// dual server
{
	int name;		// Name of server
	int ratenum;		// rate of utilisation (num/div)
	int rateden;
	int number;		// number of task in the server
	int periods[30];	// Tasks periods of server
	int deadlines[30];	// Next deadline associate with each periods
	int son;		// The only son in the tree ( the name 0 if is a task)
	int father;		// useful to recover the deadlines to hight level DS of the tree
};

struct TSK {
	float WCET;
	int number;
	float period;
	float deadline;
	int complete;
	_Bool active;
	int next;
	int previous;
};

struct lTSK {
	TSK *first;
};
