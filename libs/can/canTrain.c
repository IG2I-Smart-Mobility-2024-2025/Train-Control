#include "canTrain.h"

/**
 * @brief Create a can train object
 * 
 * @param port Port can
 * @param callback Function to call when a beacon is passed
 * @return can_train_t* The can train object
 */
can_train_t* create_can_train(char * port, callback_function callback) {
    can_train_t* res = (can_train_t*) malloc(sizeof(can_train_t));
    if (res == NULL) {
        fprintf(stderr, "Memory allocation failed for can_train_t\n");
        return NULL;
    }

    // Set callback arg to null
    res->callback_arg = NULL;
    res->callback = callback;

    // Initialize the can odometrie
    CAN_ODO(res).distance = 0;
    CAN_ODO(res).vit_consigne = 0;
    CAN_ODO(res).vit_mesuree = 0;
    CAN_ODO(res).nb_impulsions = 0;
    pthread_mutex_init(&(CAN_ODO(res).can_odometrie_mutex), NULL);

    // Initialize the can port
    initCan(port);

    return res;
}

/**
 * @brief Initialise le port CAN
 * 
 * @param port Port can (can0 par defaut)
 */
void initCan(char * port) {
    // Definition d'un port CAN si celui passé en parametre est NULL
    char* NomPort = port ? port : "can0";

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
 * @brief Ecrit la vitesse limite du train sur le bus can
 * 
 * @param vitesseLimite 
 * @return int 1 si l'envoi a reussi, 0 sinon
 */
int writeVitesseLimite(float vitesseLimite) {
	uCAN1_MSG message;
	
	if(vitesseLimite > MAX_CONSIGNE_VITESSE_AUTORISEE) //vitesse supérieur à 50 cm/s
	    vitesseLimite = MAX_CONSIGNE_VITESSE_AUTORISEE;                   
	
	message.frame.id  = MC_ID_CONSIGNE_VITESSE_LIMITE;
	message.frame.dlc = MC_DLC_CONSIGNE_VITESSE_LIMITE;
	MESCAN_SetData8(&message, cdmc_consigneVitesseLimite, vitesseLimite);
	
	return canLinux_transmit(CANLINUX_PRIORITY_HIGH, &message);
}

/**
 * @brief Ecrit la vitesse consigne du train sur le bus can
 * 
 * @param vitesse Vitesse consigne
 * @param sense Sens de deplacement
 * @return int 1 si l'envoi a reussi, 0 sinon
 */
int writeVitesseConsigne(unsigned int vitesse, unsigned char sense) {
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
 * @brief Met à jour l'odométrie du train
 * 
 * @param can_odo Odométrie du train
 * @param recCanMsg Message can pour tirer les informations
 */
void updateCanOdometrie(can_odometrie_t* can_odo, uCAN1_MSG recCanMsg) {
    // On prend la mutex pour eviter les problemes de concurrence
    pthread_mutex_lock(&(can_odo->can_odometrie_mutex));

    // maj des informations du train
    can_odo->vit_mesuree = (int)MESCAN_GetData8(&recCanMsg, cdmc_vitesseMesuree);/** le nbre d'implusion envoyé ici est le nombre d'impulsion entre 2 mesures **/
    can_odo->nb_impulsions += can_odo->vit_mesuree;
    can_odo->distance = PAS_ROUE_CODEUSE * (can_odo->nb_impulsions);
    can_odo->vit_consigne = (float)MESCAN_GetData8(&recCanMsg, cdmc_vitesseConsigneInterne);

    // On rend la mutex
    pthread_mutex_unlock(&(can_odo->can_odometrie_mutex));
}

/**
 * @brief Thread de lecture de CAN
 * 
 * @param args Structure de type can_train_t
 */
void* lectureCan(void * args) {
	// On recupere les informations du train
    can_train_t * can_train = (can_train_t *) args;
    // Definition d'une variable de type message can
    uCAN1_MSG recCanMsg;

    while(1){
        if(canLinux_receive(&recCanMsg, 1)){
            switch (recCanMsg.frame.id){
                // Pas de mesure
                case MC_ID_SCHEDULEUR_MESURES :
                    
                    // On met a jour les informations du train
                    updateCanOdometrie(&(can_train->can_odometrie), recCanMsg);
                    
                    break;
                // On passe sur une balise
                case MC_ID_EBTL2_RECEIVED : //balise
                    
                    // On remet le nombre d'impulsions a 0
                    CAN_ODO(can_train).nb_impulsions = 0;

                    // On appelle la fonction de callback avec le bon argument
                    can_train->callback(can_train->callback_arg);

                    break;
                
                default :
                    fprintf(stderr, "La trame lue a pour ID %X \n",recCanMsg.frame.id);
            }
        }
    }
}

/**
 * @brief Delete a can train object
 * 
 * @param can_train 
 * @return int 0 if the can train was deleted successfully, -1 otherwise
 */
int delete_can_train(can_train_t * can_train) {
    if (can_train == NULL) return -1;
    pthread_mutex_destroy(&(CAN_ODO(can_train).can_odometrie_mutex));
    free(can_train);
    return 0;
}