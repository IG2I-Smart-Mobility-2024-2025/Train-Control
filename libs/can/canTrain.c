#include "canTrain.h"

void initCan(){
    // Definition du nom de l'interface can du raspberry pi. A controler au niveau systeme.
	char *NomPort = "can0";
	//Definition d'un filtre CAN pour preciser les identifiant a lire
	struct can_filter rfilter[2]; //Le filtre sera limite ici a une variable

	//Initialisation du filtre
	rfilter[0].can_id   = MC_ID_SCHEDULEUR_MESURES; //On indique que l'on veut lire ces trames CAN
	rfilter[0].can_mask = CAN_SFF_MASK;
    rfilter[1].can_id   = MC_ID_EBTL2_RECEIVED; //On indique que l'on veut lire ces trames CAN
	rfilter[1].can_mask = CAN_SFF_MASK;

    /* Creation du descripteur de port a utilise pour communiquer sur le bus CAN */
    canLinux_init_prio(NomPort);
	// Mise en place d'un filtre
	canLinux_initFilter(rfilter, sizeof(rfilter));
}

int writeVitesseLimite(float vitesseLimite){
	uCAN1_MSG message;
	
	if(vitesseLimite > MAX_CONSIGNE_VITESSE_AUTORISEE) //vitesse supérieur à 50 cm/s
	    vitesseLimite = MAX_CONSIGNE_VITESSE_AUTORISEE;                   
	
	message.frame.id  = MC_ID_CONSIGNE_VITESSE_LIMITE;
	message.frame.dlc = MC_DLC_CONSIGNE_VITESSE_LIMITE;
	MESCAN_SetData8(&message, cdmc_consigneVitesseLimite, vitesseLimite);
	
	return 	canLinux_transmit(CANLINUX_PRIORITY_HIGH, &message);
}

int writeVitesseConsigne(unsigned int vitesse, unsigned char sense){
	uCAN1_MSG consigneVitesse;

	if(vitesse>MAX_CONSIGNE_VITESSE_AUTORISEE) //vitesse supérieur à 50 cm/s
		vitesse = MAX_CONSIGNE_VITESSE_AUTORISEE;
	
	consigneVitesse.frame.id  = MC_ID_CONSIGNE_VITESSE;
	consigneVitesse.frame.dlc = MC_DLC_CONSIGNE_VITESSE;
	MESCAN_SetData8(&consigneVitesse, cdmc_consigneVitesse, vitesse);
	//MESCAN_SetBit(&consigneVitesse, bdmc_sensDeplacementLoco, sense);
	MESCAN_SetBit(&consigneVitesse, bdmc_consigneSensDeplacement, sense);
	//MESCAN_SetBit(&consigneVitesse, 0, sense);	

	return 	canLinux_transmit(CANLINUX_PRIORITY_HIGH, &consigneVitesse);
}

void *lectureCan(){
    //Definition d'une variable de type message can
	uCAN1_MSG recCanMsg;

    while(1){
        if(canLinux_receive(&recCanMsg, 1)){
            switch (recCanMsg.frame.id){
                case MC_ID_SCHEDULEUR_MESURES :
                    pthread_mutex_lock(&train.trainInfo_mutex);
                    train.train.vit_mesuree= (int)MESCAN_GetData8(&recCanMsg, cdmc_vitesseMesuree);/** le nbre d'implusion envoyé ici est le nombre d'impulsion entre 2 mesures **/
                    train.train.nb_impulsions+= train.train.vit_mesuree;
                    train.train.distance= PAS_ROUE_CODEUSE * (train.train.nb_impulsions);
                    train.train.vit_consigne= (float)MESCAN_GetData8(&recCanMsg, cdmc_vitesseConsigneInterne);

                    //fprintf(stderr, "Actualisation: Postition courante : %lf cm, Vit: %d cm/s\n", train.train.distance, train.train.vit_mesuree);
                    pthread_mutex_unlock(&train.trainInfo_mutex);
                    break;
                case MC_ID_EBTL2_RECEIVED : //balise
                    pthread_mutex_lock(&train.trainInfo_mutex);
                    train.train.nb_impulsions = 0;

                    //fprintf(stderr, "\nBalise : %X", recCanMsg.frame.data5);
                    //fprintf(stderr, "Actualisation: Postition courante : %lf cm, Vit: %d cm/s\n\n", train.train.distance, train.train.vit_mesuree);
                    pthread_mutex_unlock(&train.trainInfo_mutex);
                    break;
                default :
                    fprintf(stderr, "La trame lue a pour ID %X \n",recCanMsg.frame.id);
            }
        }
    }
}