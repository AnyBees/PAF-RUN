#include "structures.h"
#include <stdio.h>
#include <stdlib.h>

SD serveursDuaux[30];
SP serveursPrimaires[30];

int pack(int first, int last, int pPos);	// Regroupe les serveurs duaux et renvoie le nombre de serveurs crees
int dual(int first, int last, int dPos);	// Cree le dual de chaque serveur primaire
int reduce(int first, int last, int pPos, int dPos);
int affiched(SD serveur);
int affichep(SP serveur);

int main(int argc, char* argv[]){

	int i;
	int periode;
	int res;
	int taux;
	int tauxTotal;
	int number;
	int pPos;
	int dPos;
	int pCrees;

	SP leaf;
	leaf.nom = 0;
	leaf.fils[0] = 0;
	leaf.pere = 0;

	SD root;
	root.nom = 0;
	root.fils = 0;
	root.pere = 0;

	/* Input des taches */

	if (argc != 2) {
		printf("Utilisation : %s nombre de taches !\n", argv[0]);
		return 1;
	}

	number = atoi(argv[1]);

	tauxTotal = 0;

	for( i = 0 ; i<number ; i++){
		printf("Periode et taux d'utilisation (entre 1 et 99) de la tache numero %d ?\n", i+1);
		res = scanf("%d %d", &periode, &taux);
		if(res != 2){
			printf("Utilisation : Periode Taux !\n");
			return 1;
		}
		if(taux > 99){
			printf("Le taux d'utilisation doit etre inferieur a 100 !\n");
			return 1;
		}
		serveursDuaux[i].nom = i+1;
		serveursDuaux[i].taux = taux;
		serveursDuaux[i].nombre = 1;
		tauxTotal += taux;
		serveursDuaux[i].periodes[0] = periode;
		serveursDuaux[i].deadlines[0] = periode;
		serveursDuaux[i].fils = &leaf;
	}

	if (tauxTotal % 100 != 0){
		printf("Le taux d'utilisation total n'est pas un multiple de 100 (%d) \n", tauxTotal);
		return 1;
	}

	dPos = number;
	pPos = 0;

	pCrees = pack(0, dPos, pPos);
	pPos += pCrees;

	while(pCrees > 1){
		dPos += pCrees;
		pCrees = reduce(pPos-pCrees, pPos, pPos, dPos-pCrees);
		pPos += pCrees;
		
	}

	serveursPrimaires[pPos-1].pere = &root;

	for(i = 0 ; i<dPos ; i++){
		affiched(serveursDuaux[i]);
	}

	for(i = 0 ; i<pPos ; i++){
		affichep(serveursPrimaires[i]);			
	}
	
	return 0;

}

int pack(int first, int last, int pPos){
	int i;
	int j;
	int place;

	/* On initialise avec le premier SD dans un SP */

	int nombreServeurs = 1;
	serveursPrimaires[pPos].nom = 31+pPos;
	serveursPrimaires[pPos].taux = serveursDuaux[first].taux;
	serveursPrimaires[pPos].taille = 1;
	serveursPrimaires[pPos].fils[0] = &(serveursDuaux[first]);
	serveursDuaux[first].pere = &(serveursPrimaires[pPos]);

	/* On parcourt la liste des autres SD du niveau le plus haut et on l'ajoute dans le premier SP pouvant le contenir */

	for(i = first+1 ; i < last ; i++){
		place = 0;
		for(j = pPos ; j < pPos+nombreServeurs ; j++){
			if(serveursPrimaires[j].taux + serveursDuaux[i].taux <= 100){
				serveursPrimaires[j].taux += serveursDuaux[i].taux;
				serveursPrimaires[j].fils[serveursPrimaires[j].taille] = &(serveursDuaux[i]);
				serveursPrimaires[j].taille++;
				serveursDuaux[i].pere = &(serveursPrimaires[j]);
				place = 1;
				break;
			}
		}
		if(!place){
			serveursPrimaires[pPos+nombreServeurs].nom = 31+pPos+nombreServeurs;
			serveursPrimaires[pPos+nombreServeurs].taux = serveursDuaux[i].taux;
			serveursPrimaires[pPos+nombreServeurs].taille = 1;
			serveursPrimaires[pPos+nombreServeurs].fils[0] = &(serveursDuaux[i]);
			serveursDuaux[i].pere = &(serveursPrimaires[pPos+nombreServeurs]);
			nombreServeurs++;
		}
	}

	return nombreServeurs;

}
		
int dual(int first, int last, int dPos){
	int i;
	int j;
	int k;
	int nombre;

	for(i = 0 ; i < last-first ; i++){
		serveursDuaux[dPos+i].nom = dPos+i+1;
		serveursDuaux[dPos+i].taux = 100-serveursPrimaires[first+i].taux;
		nombre = 0;
		for(j = 0 ; j < serveursPrimaires[first+i].taille ; j++){
			for(k = 0 ; k < serveursPrimaires[first+i].fils[j]->nombre ; k++){
				serveursDuaux[dPos+i].periodes[nombre+k] = serveursPrimaires[first+i].fils[j]->periodes[k];
				serveursDuaux[dPos+i].deadlines[nombre+k] = serveursPrimaires[first+i].fils[j]->deadlines[k];
			}
			nombre += k;
		}
		serveursDuaux[dPos+i].fils = &serveursPrimaires[first+i];
		serveursPrimaires[first+i].pere = &serveursDuaux[dPos+i];
		serveursDuaux[dPos+i].nombre = nombre;
	}

	return 1;

}

int reduce(int first, int last, int pPos, int dPos){
	dual(first, last, dPos);
	return pack(dPos, dPos+last-first, pPos);
}

int affiched(SD serveur){
	int i;
	printf("\nServeur numero %d de taux %d. ", serveur.nom, serveur.taux);
	if(serveur.fils->nom == 0)
		printf("C'est une feuille, ie une tache reelle, de pere %d et de periode %d \n", serveur.pere->nom, serveur.periodes[0]);
	else{
		printf("Son fils est le serveur numero %d, son pere le serveur numero %d et ses périodes sont ", serveur.fils->nom, serveur.pere->nom);
		for(i = 0 ; i < serveur.nombre ; i++)
			printf("%d ", serveur.periodes[i]);
		printf("\n");
	}
	return 1;
}

int affichep(SP serveur){
	int i;
	printf("\nServeur numero %d de taux %d. ", serveur.nom, serveur.taux);
	if(serveur.pere->nom == 0)
		printf("C'est la racine, ");
	else
		printf("Son pere est le serveur numero %d, ", serveur.pere->nom);
	printf("ses fils sont de numero ");
	for(i = 0 ; i < serveur.taille ; i++)
		printf("%d ", serveur.fils[i]->nom);
	printf("\n");
	return 1;
}

