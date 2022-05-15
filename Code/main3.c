#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>


int main(){

  /******************* Ouverture et manipulation du fichier ********************/

  printf(" Graphes du Web : \n");
  printf("----------------------------------------------------------------------\n");
  printf(" 1 - wb-cs-stanford          0.6Mo\n");
  printf(" 2 - Stanford                37Mo\n");
  printf(" 3 - Stanford_BerkeleyV2     120Mo\n");
  printf(" 4 - in-2004v2               277Mo\n");
  printf(" 5 - wikipedia-20051105V2    309Mo\n");
  printf(" 6 - wb-edu                  1006Mo\n");
  printf("----------------------------------------------------------------------\n");


  int choice;
  printf(" Choisir un des 6 graphes du web sur lequel effectuer les analyses : ");
  scanf("%d", &choice);

  char filename[] = "";

  switch(choice){
  	case 1:
  		strcpy(filename,"data/wb-cs-stanford.txt");
  		//filename = "data/wb-cs-stanford.txt";
  		break;
  	case 2:
  		strcpy(filename,"data/Stanford.txt");
  		break;
  	case 3:
  		strcpy(filename,"data/Stanford_BerkeleyV2.txt");
  		break;
  	case 4:
  		strcpy(filename,"data/in-2004v2.txt");
  		break;
  	case 5:
  		strcpy(filename, "data/wikipedia-20051105V2.txt");
  		break;
  	case 6:
  		strcpy(filename,"data/wb-edu.txt");
  		break;
  	default:
  		printf("Erreur de choix !");
  }

  FILE *fp;
  if((fp = fopen(filename,"r")) == NULL)

  {
    fprintf(stderr,"[Error] Cannot open the file");
    exit(1);
  }

  // CHOIX DES PARAMETRES
  printf(" Choisir le nombre de nouveaux sommets : ");
  int nbr;
  scanf("%d",&nbr);

  printf(" Choisir la probabilite de transition (p) entre 0 et 1 : ");
  float prob;
  scanf("%f",&prob);

  printf(" Choisir le facteur d'amortissement / Dumping Factor (alpha) entre 0 et 1 : ");
  float alpha;
  scanf("%f",&alpha);

  /*************************** Les variables ***************************/

  // Initialisation des variables de temps d'execution
  clock_t begin, end;
  double time_spent;
  begin = clock();

  // Lecture des donnees
  int n, e;
  char ch;
  char str[100];
  //ch = getc(fp);

 /* while(ch == '#') {
    fgets(str,100-1,fp);

    //printf("%s",str);
    sscanf (str,"%*s %d %*s %d", &n, &e); //nombres de noeud
    ch = getc(fp);
  }*/

  fgets(str,100-1, fp);
  sscanf(str, "%d", &n);
  fgets(str,100-1, fp);
  sscanf(str, "%d", &e);
  //ungetc(ch, fp);


  // DEBUG: Print the number of nodes and edges, skip everything else
  printf("\nInformations sur le graphe choisi :\n\n  Nodes: %d, Edges: %d \n\n", n, e);
  printf("WE REACHED THIS PART OF CODE 1");
  /************************* CSR Structure de la matrice *****************************/

  /* Compressed sparse row format:
     - Val vector: contains 1.0 if an edge exists in a certain row
     - Col_ind vector: contains the column index of the corresponding value in 'val'
     - Row_ptr vector: points to the start of each row in 'col_ind'
  */

  float *val = calloc(e, sizeof(float));
  int *col_ind = calloc(e, sizeof(int));
  int *row_ptr = calloc(n+1, sizeof(int));

  // The first row always starts at position 0
  row_ptr[0] = 0;

  int fromnode, tonode;
  int cur_row = 0;
  int i = 0;
  int j = 0;
  // Elements for row
  int elrow = 0;
  // Cumulative numbers of elements
  int curel = 0;

  FILE *wptr;
   wptr = fopen("iter.txt", "w");
   while(!feof(fp)){

    fscanf(fp,"%d%d",&fromnode,&tonode);

   // printf("From: %d To: %d\n",fromnode, tonode);

    if (fromnode > cur_row) { // change the row
      curel = curel + elrow;
      int k = 0;
      for (k = cur_row + 1; k <= fromnode; k++) {
        row_ptr[k] = curel;
      }
      elrow = 0;
      cur_row = fromnode;
    }

    val[i] = 1.0;
    col_ind[i] = tonode;
    elrow++;
    i++;
    int nods;
    int save = 0;

    for(nods=0; nods<nbr; nods=nods+100){
        save = elrow;
    }
    end = clock();
    time_spent = (double)(end - begin);

    fprintf(wptr, "Nombre d'iteration: %d\n", i);
    fprintf(wptr, "Temp d'execution: %.2fs\n", time_spent/10000);
  }

row_ptr[cur_row+1] = curel + elrow - 1;

  // Fixer la stochastization
  int out_link[n];
  for(i=0; i<n; i++){
    out_link[i] =0;
  }

  int rowel = 0;
  for(i=0; i<n; i++){
        if (row_ptr[i+1] != 0) {
          rowel = row_ptr[i+1] - row_ptr[i];
          out_link[i] = rowel;
        }
   }


  int curcol = 0;
  for(i=0; i<n; i++){
    rowel = row_ptr[i+1] - row_ptr[i];
    for (j=0; j<rowel; j++) {
      val[curcol] = val[curcol] / out_link[i];
      curcol++;
    }
  }


  /******************* INITIALIZATION DE P ET D DAMPING FACTOR ************************/

  // Set the damping factor 'd'
  float d = alpha;

  // Initialize p[] vector
  float p[n];
  for(i=0; i<n; i++){
    //p[i] = 1.0/n;
    p[i] = prob;
  }
  //SOME CHANGES COULD BE DONE HERE

  /*************************** PageRank BOUCLE  **************************/

  // configuration de la condition de sortie et le nombre d'iterations (max) 'k'
  int looping = 1;
  int k = 0;

  // initialisation d'un nouveau vecteur p
  float p_new[n];

  while (looping){

    for(i=0; i<n; i++){
      p_new[i] = 0.0;
    }


    int curcol = 0;

    // Pagerank modifiedie algorithme
    for(i=0; i<n; i++){
      rowel = row_ptr[i+1] - row_ptr[i];
      for (j=0; j<rowel; j++) {
        p_new[col_ind[curcol]] = p_new[col_ind[curcol]] + val[curcol] * p[i];
        curcol++;
      }
    }

    // DANGELING ELEMENTS
    for(i=0; i<n; i++){
      p_new[i] = d * p_new[i] + (1.0 - d) / n;
    }

    // CHECK EXIT CONDITION
    float error = 0.0;
    for(i=0; i<n; i++) {
      error =  error + fabs(p_new[i] - p[i]);
    }
    // EPSILON
    if (error < 0.000001){
      looping = 0;
    }

    // mettre a jour p[]
    for (i=0; i<n;i++){
        p[i] = p_new[i];
    }

    // Nombre d'iterations
    k = k + 1;
}

/*************************** CONCLUSIONS *******************************/

  // STOP TIMER
  end = clock();
  time_spent = (double)(end - begin); // CLOCKS_PER_SEC;

  //
  //Sleep(500);

  // Print resultats
  printf ("\nNumber d'iteration pour converger %d \n\n", k);
  printf ("Valeur finale de pagerank:\n\n[");
  for (i=0; i<n; i++){
    printf("%f ", p[i]);
    if(i!=(n-1)){ printf(", "); }
  }
  printf("]\n\nTEMPS D'EXECUTION': %f seconds.\n", time_spent);


  return 0;
}

