typedef struct PS PS;
typedef struct DS DS;

struct DS;

struct PS			// Primary server 
{
	int name;		// name of server
	int rate;		// rate of utilisation (percentage)
	int size;		// number of sons to look for EDF 
	DS *son[10];	        // Pointers to the sons of tree
	DS *father;		//useful to recover the deadlines to hight level DS of the tree
};

struct DS			// dual server 
{
	int name;		// Name of server
	int rate;		// rate of utilisation
	int number;		// number of task in the server
	int periods[30];	// Tasks periods of server
	int deadlines[30];	// Next deadline associate with each periods
	PS *son;		// The only son in the tree ( the name 0 if is a task)
	PS *father;		// useful to recover the deadlines to hight level DS of the tree
};
