#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "p3230267-p3230111-pizza.h"

void *customers_thread (void *threadId){
	int id = *(int *)threadId;										//περνάμε το id κάθε πελάτη στην μέθοδο
	free(threadId);													//αποδέσμευση της μνήμης απο την μεταβλητή

	struct timespec start_time_customer_service1, end_time_customer_service1;
	clock_gettime(CLOCK_REALTIME, &start_time_customer_service1);	//ρολόι για χρόνο εξυπηρέτησης

	int cnt_tel = 0;
	struct timespec start_time, end_time; 
	clock_gettime(CLOCK_REALTIME, &start_time); 					//βοηθητικό ρολόι για διάρκεια αναμονής πελάτη για τηλεφωνητή

	pthread_mutex_lock(&mutex_tel); 								//κλείδωμα mutex για τηλεφωνητή
	while (N_tel == 0) { 											//έλεγχος περίπτωσης μη διαθεσιμότητας τηλεφωνητών
		cnt_tel++;
		if (cnt_tel == 1) { 										//συνθήκη για εκτύπωση μόνο μια φορά
			pthread_mutex_lock(&mutex_prints); 						//κλείδωμα mutex για εκτύπωση οτι δεν υπάρχει διαθέσιμος τηλεφωνητής
			printf("Ο πελάτης με αριθμό παραγγελίας %d περιμένει έναν τηλεφωνητή μέχρι να γίνει διαθέσιμος.\n", id);	
			pthread_mutex_unlock(&mutex_prints);					//ξεκλείδωμα mutex για εκτύπωση
		}
		pthread_cond_wait(&cond_tel, &mutex_tel);  					//σήμα να περιμένουν τα επόμενα νήματα
	}
	N_tel--;														//δέσμευση 1 τηλεφωνητή
	pthread_mutex_unlock(&mutex_tel);								//ξεκλείδωμα mutex για τηλεφωνητή

	clock_gettime(CLOCK_REALTIME, &end_time); 						// κλείσιμο  βοηθητικού ρολογιού
	float waitingTime_order = end_time.tv_sec - start_time.tv_sec;

	pthread_mutex_lock(&mutex_prints);							//κλείδωμα mutex για εκτύπωση χρόνου αναμονής πελάτη για να παραγγείλει
	printf("Ο πελάτης με αριθμό παραγγελίας %d περίμενε %.2f λεπτά μέχρι να παραγγείλει.\n", id, waitingTime_order);
	pthread_mutex_unlock(&mutex_prints);						// ξεκλείδωμα mutex για εκτύπωση

	unsigned int seed = Seed + time(NULL);						//αρχικοποίηση του Seed ώστε να ειναι διαφορετικό στο κάθε thread 

	int num_pizzas = rand_r(&seed) % (N_orderhigh - N_orderlow + 1) + N_orderlow;		//υπολογισμός αριθμών πιτσων 
	int margarita = 0, peperoni = 0, special = 0;

	for (int i = 0; i < num_pizzas; i++) {  					// υπολογισμός τύπου κάθε πίτσας ανάλογα με τις πιθανότητες της εκφώνησης 
		float pizza_type_chance = rand_r(&seed) % 101;
		if (pizza_type_chance <= Pm)
			margarita++;					//αναλογα σε ποιο αριθμητικό διάστημα βρίσκεται ο τυχαίος αριθμός, αποφασίζεται ο τύπος πίτσας
		else if (pizza_type_chance <= Pm + Pp)
			peperoni++;
		else
			special++;
	}

	int paying_delay = rand_r(&seed) % (T_paymenthigh - T_paymentlow + 1) + T_paymentlow;			//υπολογισμός καθυστέρησης
	sleep(paying_delay);										//κοιμίζουμε το thread για το δίαστημα της καθυστέρησης

	float NoMoney_chance = rand_r(&seed) % 101;					//πιθανοτητα ακύρωσης παραγγελίας λόγω μη επαρκόμενων χρημάτων


	if (NoMoney_chance <= P_fail) { 		//υπολογισμός στατιστικών στην περίπτωση που αποτύχει η παραγγελία λόγω ανεπάρκιας χρημάτων 
		pthread_mutex_lock(&mutex_stats);
		apotyxies++;
		pthread_mutex_unlock(&mutex_stats);

		pthread_mutex_lock(&mutex_prints);
		printf("Η παραγγελία με αριθμό %d απέτυχε.\n", id);	//κλείδωμα και ξεκλείδωμα των κατάλληλων mutex για στατιστικά και εκτύπωση
		pthread_mutex_unlock(&mutex_prints);

		pthread_mutex_lock(&mutex_tel);
		N_tel++;												//αποδέσμευση του τηλεφωνητή 
		pthread_cond_signal(&cond_tel);
		pthread_mutex_unlock(&mutex_tel);

		pthread_exit(NULL);										//έξοδος του πελάτη απο το πρόγραμμα καθώς τελείωσε η διαδικασία του
	} else {
		pthread_mutex_lock(&mutex_stats);
		epitixies++;
		pthread_mutex_unlock(&mutex_stats);

		pthread_mutex_lock(&mutex_prints);
		printf("Η παραγγελία με αριθμό %d καταχωρήθηκε.\n", id);
		pthread_mutex_unlock(&mutex_prints);
	}

	clock_gettime(CLOCK_REALTIME, &end_time_customer_service1); 				//τέλος ρολόι για χρόνο εξυπηρέτησης μεχρι καταχώρηση
	float waitingTime_customer_service = end_time_customer_service1.tv_sec - start_time_customer_service1.tv_sec;

	struct timespec start_time_customer_service2, end_time_customer_service2;		//ρολόι για service
	clock_gettime(CLOCK_REALTIME, &start_time_customer_service2);

	struct timespec start_packing, end_packing; 							//ρολοι μεχρι πακεταρισμα
	clock_gettime(CLOCK_REALTIME, &start_packing);
	
	struct timespec start_delivery, end_delivery; 						//ρολόι μεχρι delivery 
	clock_gettime(CLOCK_REALTIME, &start_delivery);

	pthread_mutex_lock(&mutex_stats); 									//mutex lock για υπολογισμό στατιστικων
	total_pizzas_sold += num_pizzas;
	total_margarita += margarita;
	total_peperoni += peperoni;
	total_special += special;
	total_revenue += margarita * Cm + peperoni * Cp + special * Cs;
	pthread_mutex_unlock(&mutex_stats);

	pthread_mutex_lock(&mutex_tel);
	N_tel++; 														//αποδέσμευση τηλεφωνητή
	
	pthread_cond_signal(&cond_tel); 					//ενημέρωση του επόμενου νήματος που περιμένει πως απελευθερώθηκε τηλεφωνητής
	pthread_mutex_unlock(&mutex_tel);


	int cnt_cook = 0;
	pthread_mutex_lock(&mutex_cook); 								//κλείδωμα mutex για cooks 
	while (N_cook == 0) {									 //ελεγχος περίπτωσης μη διαθεσιμότητας παρασκευαστών
		cnt_cook++;
		if(cnt_cook == 1){
			pthread_mutex_lock(&mutex_prints);	//κλείδωμα mutex και εκτύπση μια φορά το μήνυμα αν δεν υπάρχει διαθέσιμος παρασκευαστής 
			printf("Η παραγγελία με αριθμό %d περιμένει έναν παρασκευαστή μέχρι να γίνει διαθέσιμος.\n",id);
			pthread_mutex_unlock(&mutex_prints);
		}
		pthread_cond_wait(&cond_cook, &mutex_cook); 			//σήμα να περιμένουν τα επόμενα νήματα
	}
	N_cook--; 													//δέσμευση του παρασκευαστή
	pthread_mutex_unlock(&mutex_cook); 							//ξεκλείδωμα mutex για cooks

	sleep(Tprep);												//δημιουργία καθυστέρησης για τον χρόνο ετοιμασίας πίτσων

	pthread_mutex_lock(&mutex_cook);	
	N_cook++;													//αποδέσμευση του παρασκευαστή
	pthread_cond_signal(&cond_cook);					//ενημέρωση του επόμενου νήματος που περιμένει πως απελευθερώθηκε παρασκευαστής
	pthread_mutex_unlock(&mutex_cook);
	
	pthread_mutex_lock(&mutex_oven);						//κλείδωμα mutex για φούρνους 
	int cnt_oven = 0;
	while (N_oven == 0 || N_oven - num_pizzas < 0) { 		//ελεγχος μη διαθεσιμότητας φούρνων σε περίπτωση που δεν χωράει η παραγγελία στους διαθέσιμους φούρνους 
		cnt_oven++;
		if(cnt_oven == 1){
			pthread_mutex_lock(&mutex_prints); 			//κλείδωμα mutex και εκτύπση μια φορά το μήνυμα αν δεν υπάρχουν διαθέσιμοι φούρνοι και πόσοι χρειάζονται εκείνη την στιγμή σε κάθε παραγγελία
			printf("Η παραγγελία με αριθμό %d αυτή την στιγμή περιμένει %d φούρνους να γίνουν διαθέσιμοι.\n", id, num_pizzas - N_oven);
			pthread_mutex_unlock(&mutex_prints);					//ξεκλείδωμα mutex
		}
		pthread_cond_wait(&cond_oven, &mutex_oven); 				//σήμα να περιμένουν τα επόμενα νήματα
	}
	N_oven -= num_pizzas;											//δέσμευση 1 φούρνο ανά πίτσα 
	pthread_mutex_unlock(&mutex_oven);								//ξεκλείδωμα mutex


	sleep(Tbake);											//δημιουργία καθυστέρησης για τον χρόνο ψησίματος

	struct timespec start_time_cold, end_time_cold; 		//ρολόι υπολογισμού χρόνου κρυώματος 
	clock_gettime(CLOCK_REALTIME, &start_time_cold);


	int cnt_packer = 0;
	pthread_mutex_lock(&mutex_packer);						//mutex του πακεταριστή
	while (N_packer == 0) {									//έλεγχος περίπτωσης μη διαθεσιμότητας πακεταριστών 
		cnt_packer++;		
		if(cnt_packer == 1){
			pthread_mutex_lock(&mutex_prints); 		//κλείδωμα mutex και εκτύπση μια φορά το μήνυμα αν δεν υπάρχει διαθέσιμος πακεταρηστής
			printf("Η παραγγελία με αριθμό %d περιμένει έναν υπάλληλο πακεταρίσματος να γίνει διαθέσιμος.\n",id);
			pthread_mutex_unlock(&mutex_prints); 			// ξεκλείδωμα mutex
		}	 
		pthread_cond_wait(&cond_packer, &mutex_packer);		//σήμα να περιμένουν τα επόμενα νήματα
	}
	N_packer--;
	pthread_mutex_unlock(&mutex_packer); 	//ξεκλείδωμα mutex του πακεταριστή

	sleep(Tpack); 							//δημιουργία καθυστέρησης για τον χρόνο πακεταρίσματος

	pthread_mutex_lock(&mutex_packer);
	N_packer++;								//αποδέσμευση πακεταριστή 
	pthread_cond_signal(&cond_packer);		//ειδοποίηση για το επόμενο νήμα να ξυπνήσει
	pthread_mutex_unlock(&mutex_packer);

		
	clock_gettime(CLOCK_REALTIME, &end_packing); 							//τελος ρολογιου packing time 
	float packing_time = end_packing.tv_sec - start_packing.tv_sec;


	float totalWaitingPacking = packing_time + waitingTime_customer_service; 		//υπολογισμός συνολικου χρόνου προετοιμασίας
	pthread_mutex_lock(&mutex_prints);
	printf("Η παραγγελία με αριθμό %d ετοιμάστηκε σε %.2f λεπτά.\n", id, totalWaitingPacking);
	pthread_mutex_unlock(&mutex_prints);

	pthread_mutex_lock(&mutex_oven); 					//αποδέσμευση φούρνων
	N_oven+= num_pizzas; 
	pthread_cond_broadcast(&cond_oven); //broadcast για να ξυπνήσουμε όλα τα threads και να μπει η πρώτη παραγγελία που χωράει και προλάβει
	pthread_mutex_unlock(&mutex_oven);

	int cnt_deliver = 0;
	pthread_mutex_lock(&mutex_deliver); 					//mutex διανομέα
	while (N_deliver == 0) { 						//ελεγχος περίπτωσης μη διαθεσιμότητας διανομέων
		cnt_deliver++;
		if(cnt_deliver == 1){
			pthread_mutex_lock(&mutex_prints);		//κλείδωμα mutex και εκτύπωση μια φορά μηνύματος για το ότι δεν υπάρχουν διαθέσιμοι διανομείς
			printf("Η παραγγελία με αριθμό %d περιμένει έναν διανομέα να γίνει διαθέσιμος.\n",id);
			pthread_mutex_unlock(&mutex_prints); 		//ξεκλείδωμα mutex για εκτυπωση
		}	
		pthread_cond_wait(&cond_deliver, &mutex_deliver); 			//σήμα να περιμένουν τα επόμενα νήματα
	}
	N_deliver--; 													//δέσμευση διανομέα
	pthread_mutex_unlock(&mutex_deliver); 						//ξεκλείδωμα mutex διανομέα


	int delivery_duration = rand_r(&seed) % (Tdelhigh - Tdellow + 1) + Tdellow;		//καθορισμός χρόνος delivery
	sleep(delivery_duration); 									//χρόνος του διανομέα να παει την παραγγελια 

	clock_gettime(CLOCK_REALTIME, &end_delivery); 					//τελος ρολογιου delivery
	float actual_delivery_time = end_delivery.tv_sec - start_delivery.tv_sec;

	float TotalWaitingDelivery = actual_delivery_time + waitingTime_customer_service; 		//συνολικος χρονος παραδοσης
	pthread_mutex_lock(&mutex_prints);								//κλείδωμα mutex για εκτύπωση 
	printf("Η παραγγελία με αριθμό %d παραδόθηκε σε %.2f λεπτά.\n", id, TotalWaitingDelivery);
	pthread_mutex_unlock(&mutex_prints); 							//ξεκλείδωμα mutex για εκτύπωση

	clock_gettime(CLOCK_REALTIME, &end_time_customer_service2); //κλεισιμο ρολογιου service
	clock_gettime(CLOCK_REALTIME, &end_time_cold); //κλεισιμο ρολογιου κρυώματος 

	float waitingTime_customer_service_2nd_phase = end_time_customer_service2.tv_sec - start_time_customer_service2.tv_sec;

	float total_customer_service = waitingTime_customer_service + waitingTime_customer_service_2nd_phase;

	if (total_customer_service > megistos_xronos_ejipiretisis) { //υπολογισμός μέγιστου χρόνου εξυπηρέτησης
		megistos_xronos_ejipiretisis = total_customer_service;
	}
	synolikos_xronos_ejipiretisis += total_customer_service;//αθροισμα συνολικου χρόνου εξυπηρετησης για τον υπολογισμο μο

	float waitingTime_cold = end_time_cold.tv_sec - start_time_cold.tv_sec;

	if (waitingTime_cold > megistos_xronos_krywmatos) {//υπολογισμός μέγιστου χρόνου κρυώματος
		megistos_xronos_krywmatos = waitingTime_cold;
	}
	synolikos_xronos_krywmatos += waitingTime_cold;//αθροισμα συνολικου χρόνου κρυώματος για τον υπολογισμο μο

	sleep(delivery_duration); 					//επιστροφη διανομέα

	pthread_mutex_lock(&mutex_deliver);
	N_deliver++; 								//αποδέσμευση διανομέα 
	pthread_cond_signal(&cond_deliver);
	pthread_mutex_unlock(&mutex_deliver);

	pthread_exit(NULL);		//έξοδος του νήματος/πελάτη απο την διαδικασία
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Δεν υπάρχουν αρκετά ορίσματα\n");
        exit(0);
    }
 
    int Ncust = atoi(argv[1]); 					//ορισμα 1ο(αριθμος πελατων)
    Seed = atoi(argv[2]);						//ορισμα 2ο(seed)
 
    pthread_t* customers = malloc(Ncust * sizeof(pthread_t));
    if (customers == NULL) {
		printf("Δεν υπάρχει αρκετή μνήμη\n");		//έλεγχος αν υπάρχει αρκετή μνήμη για να δεσμεύσουμε για τα νήματα
        exit(-1);
    }
 
    for (int i = 0; i < Ncust; i++) {
        int* id = malloc(sizeof(int));
        if (id == NULL) {						//έλεγχος αν υπάρχει αρκετή μνήμη για να δεσμεύσουμε για τα id
            printf("Δεν υπάρχει αρκετή μνήμη\n");
            exit(-1);
        }
 
        *id = i + 1;
 		int rc = pthread_create(&customers[i], NULL, customers_thread, id); //δημιουργια νηματων, πέρασμα στην μέθοδο customers_thread και έλεγχος αν γίνεται κάποιο λάθος 
        if (rc != 0) {
            printf("Λάθος κατά την δημιουργία νήματος(Πελάτη)\n");
            free(id);
            exit(0);
        }
 		
 		unsigned int seedp = Seed + i ; 				//δεχομαστε τον σπορο 
        int kathisterisi = rand_r(&seedp) % (T_orderhigh - T_orderlow +1) + T_orderlow; 
        sleep(kathisterisi);
    }
 
    for (int i = 0; i < Ncust; i++) { 				//περιμένουμε να τελειώσουν όλα τα νήματα προτού εκτυπώσουμε τα τελικά στατιστικά
        pthread_join(customers[i], NULL);
    }

	printf("Συνολικά έσοδα: %d $ \n", total_revenue);

	printf("Αριθμός Μαργαριτών που πουλήθηκαν: %d \n", total_margarita);
	
	printf("Αριθμός Πεπερόνι που πουλήθηκαν: %d \n", total_peperoni);
	
	printf("Αριθμός Σπέσιαλ που πουλήθηκαν: %d \n", total_special);
	
	printf("Συνολικός αριθμός πιτσών που πουλήθηκαν: %d \n", total_margarita + total_peperoni + total_special);

										//εκτύπωση τελικών στατιστικών. 
	printf("Οι επιτυχημένες παραγγελίες ήταν: %d ενώ οι αποτυχημένες ήταν: %d \n", epitixies, apotyxies);
	
	printf("Μέγιστος χρόνος εξυπηρέτησης επιτυχμένων παραγγελιών ήταν: %.2f λεπτά\n",megistos_xronos_ejipiretisis);
	
	if(epitixies!=0){					//έλεγχος για περίπτωση διαίρεσης με 0
		printf("Μέσος χρόνος εξυπηρέτησης επιτυχημένων παραγγελιών ήταν: %.2f λεπτά \n", synolikos_xronos_ejipiretisis/epitixies);
		printf("Μεσος χρόνος κρυώματος των παραγγελιών ήταν: %.2f λεπτά\n",synolikos_xronos_krywmatos/epitixies);
	}else{
		printf("Όλες οι παραγγελίες απέτυχαν. \n");
	}
	printf("Μέγιστος χρόνος κρυώματος των παραγγειών ήταν: %.2f λεπτά\n",megistos_xronos_krywmatos);
	
    free(customers);					//αποδέσμευση μνήμης
    return 0;
}
