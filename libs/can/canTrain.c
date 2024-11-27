#include "canTrain.h"

/**
 * @brief Crée et initialise une structure t_TrainInfo pour le train.
 *
 * Cette fonction alloue dynamiquement une structure t_TrainInfo, initialise
 * ses membres à des valeurs par défaut, configure le mutex associé et
 * appelle la fonction d'initialisation CAN.
 *
 * @return Un pointeur vers la structure t_TrainInfo initialisée. NULL si l'allocation échoue.
 */
t_TrainInfo* create_train_can() {
    t_TrainInfo* train_can = malloc(sizeof(t_TrainInfo));
    pthread_mutex_init(&(train_can->trainInfo_mutex), NULL);
    train_can->train.distance = 0;
    train_can->train.vit_consigne = 0;
    train_can->train.vit_mesuree = 0;
    train_can->train.nb_impulsions = 0;
    initCan();
    return train_can;
}

/**
 * @brief Initialise la configuration pour l'utilisation du bus CAN.
 *
 * Cette fonction configure l'interface CAN de la Raspberry Pi, initialise des filtres CAN 
 * pour restreindre les messages à lire, et met en place les structures nécessaires 
 * pour la communication sur le bus CAN.
 *
 * @note Vérifiez que le nom de l'interface CAN est correct au niveau du système 
 * avant d'utiliser cette fonction.
 */
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

/**
 * @brief Envoie une consigne de vitesse limite sur le bus CAN.
 *
 * Cette fonction prépare et envoie un message CAN contenant une consigne 
 * de vitesse limite. Si la vitesse dépasse la limite maximale autorisée, 
 * elle est automatiquement ramenée à cette limite.
 *
 * @param vitesseLimite La vitesse limite souhaitée, en cm/s.
 *                      Elle sera plafonnée à MAX_CONSIGNE_VITESSE_AUTORISEE si nécessaire.
 *
 * @return Un entier indiquant le résultat de la transmission.
 *         0 en cas de succès, une valeur négative en cas d'erreur.
 */
int writeVitesseLimite(float vitesseLimite){
	uCAN1_MSG message;
	
	if(vitesseLimite > MAX_CONSIGNE_VITESSE_AUTORISEE) //vitesse supérieur à 50 cm/s
	    vitesseLimite = MAX_CONSIGNE_VITESSE_AUTORISEE;                   
	
	message.frame.id  = MC_ID_CONSIGNE_VITESSE_LIMITE;
	message.frame.dlc = MC_DLC_CONSIGNE_VITESSE_LIMITE;
	MESCAN_SetData8(&message, cdmc_consigneVitesseLimite, vitesseLimite);
	
	return 	canLinux_transmit(CANLINUX_PRIORITY_HIGH, &message);
}

/**
 * @brief Envoie une consigne de vitesse et de sens de déplacement sur le bus CAN.
 *
 * Cette fonction prépare et envoie un message CAN contenant une consigne de vitesse 
 * et le sens de déplacement associé. Si la vitesse dépasse la limite maximale autorisée,
 * elle est automatiquement ramenée à cette limite.
 *
 * @param vitesse La vitesse souhaitée, en cm/s. Elle sera plafonnée à MAX_CONSIGNE_VITESSE_AUTORISEE si nécessaire.
 * @param sense Le sens de déplacement (0 pour un sens, 1 pour l'autre).
 *
 * @return Un entier indiquant le résultat de la transmission.
 *         0 en cas de succès, une valeur négative en cas d'erreur.
 */
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

/**
 * @brief Thread de lecture des messages CAN.
 *
 * Cette fonction lit en continu les messages reçus sur le bus CAN et effectue des
 * actions en fonction des identifiants des messages. Elle met à jour les informations
 * de train et l'odométrie en conséquence.
 *
 * @param args Pointeur vers une structure contenant les arguments pour le thread.
 *             - `train_info` : Informations du train à mettre à jour.
 *             - `odo` : Structure d'odométrie à réinitialiser si nécessaire.
 *
 * @return NULL. Cette fonction est exécutée dans un thread et ne retourne rien directement.
 */
void *lectureCan(void * args) {
    //Definition d'une variable de type message can	
    thread_args * arguments = (thread_args *) args;
    t_TrainInfo * train_can = arguments->train_info;
    odometrie * odo = arguments->odo;

    uCAN1_MSG recCanMsg;

    while(1){
        if(canLinux_receive(&recCanMsg, 1)){
            switch (recCanMsg.frame.id){
                case MC_ID_SCHEDULEUR_MESURES :
                    pthread_mutex_lock(&(train_can->trainInfo_mutex));
                    train_can->train.vit_mesuree= (int)MESCAN_GetData8(&recCanMsg, cdmc_vitesseMesuree);/** le nbre d'implusion envoyé ici est le nombre d'impulsion entre 2 mesures **/
                    train_can->train.nb_impulsions+= train_can->train.vit_mesuree;
                    train_can->train.distance= PAS_ROUE_CODEUSE * (train_can->train.nb_impulsions);
                    train_can->train.vit_consigne= (float)MESCAN_GetData8(&recCanMsg, cdmc_vitesseConsigneInterne);

                    //fprintf(stderr, "Actualisation: Postition courante : %lf cm, Vit: %d cm/s\n", train.train.distance, train.train.vit_mesuree);
                    pthread_mutex_unlock(&(train_can->trainInfo_mutex));
                    break;
                case MC_ID_EBTL2_RECEIVED : //balise
                    pthread_mutex_lock(&(train_can->trainInfo_mutex));
                    train_can->train.nb_impulsions = 0;

                    //fprintf(stderr, "\nBalise : %X", recCanMsg.frame.data5);
                    //fprintf(stderr, "Actualisation: Postition courante : %lf cm, Vit: %d cm/s\n\n", train.train.distance, train.train.vit_mesuree);
                    pthread_mutex_unlock(&(train_can->trainInfo_mutex));
                    
                    // Reset the odometrie
                    pthread_mutex_lock(&(odo->mutex));
                    reset_odometrie(odo);
                    pthread_mutex_unlock(&(odo->mutex));

                    break;
                default :
                    fprintf(stderr, "La trame lue a pour ID %X \n",recCanMsg.frame.id);
            }
        }
    }
}