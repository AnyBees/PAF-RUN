typedef struct SP SP;
typedef struct SD SD;

struct SD;

struct SP			// Serveur primaire
{
	int nom;		// Identifie le serveur
	int taux;		// Taux d'utilisation (pourcentage)
	int taille;		// Nombre de fils à regarder pour EDF
	SD *fils[10];	// Pointeurs vers les fils dans l'arbre
	SD *pere;		// Utile pour remonter les deadlines aux SD plus haut dans l'arbre (de nom 0 si racine)
};

struct SD			// Serveur dual
{
	int nom;			// Identifie le serveur
	int taux;			// Taux d'utilisation
	int nombre;			// Nombre de taches dans le serveur
	int periodes[30];	// Periode des taches du serveur
	int deadlines[30];	// Prochaine deadline associee a chaque periode
	SP *fils;			// L'unique fils dans l'arbre (de nom 0 si tache)
	SP *pere;			// Utile pour remonter les deadlines aux SD plus haut dans l'arbre
};
