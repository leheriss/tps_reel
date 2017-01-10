#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void envoyer(void * arg) {
    DMessage *msg;
    int err;

    while (1) {
        rt_printf("tenvoyer : Attente d'un message\n");
        if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
            rt_printf("tenvoyer : envoi d'un message au moniteur\n");
            serveur->send(serveur, msg);
            msg->free(msg);
        } else {
            rt_printf("Error msg queue write: %s\n", strerror(-err));
        }
    }
}

void connecter(void * arg) {
    int status;
    DMessage *message;

    rt_printf("tconnect : Debut de l'exécution de tconnect\n");

    while (1) {
        rt_printf("tconnect : Attente du sémaphore semConnecterRobot\n");
        rt_sem_p(&semConnecterRobot, TM_INFINITE);
        rt_printf("tconnect : Ouverture de la communication avec le robot\n");
        status = robot->open_device(robot);

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        etatCommRobot = status;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            status = robot->start_insecurely(robot);
            if (status == STATUS_OK){
                rt_printf("tconnect : Robot démarrer\n");
            }
        }

        message = d_new_message();
        message->put_state(message, status);

        rt_printf("tconnecter : Envoi message\n");
        message->print(message, 100);

        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }
    }
}

void communiquer(void *arg) {
    DMessage *msg = d_new_message();
    int communicationStatus = 1;
    int num_msg = 0;

    rt_printf("tserver : Début de l'exécution de serveur\n");
    serveur->open(serveur, "8000");
    rt_printf("tserver : Connexion\n");

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommMoniteur = 0;
    rt_mutex_release(&mutexEtat);

    while (communicationStatus > 0) {
        rt_printf("tserver : Attente d'un message\n");
        communicationStatus = serveur->receive(serveur, msg);
 		rt_mutex_acquire(&mutexEtat, TM_INFINITE);
 		if (communicationStatus > 0)
		    etatCommMoniteur = STATUS_OK;
	    rt_mutex_release(&mutexEtat);        
        num_msg++;
        if (communicationStatus > 0) {
            switch (msg->get_type(msg)) {
                case MESSAGE_TYPE_ACTION:
                    rt_printf("tserver : Le message %d reçu est une action\n",
                            num_msg);
                    DAction *action = d_new_action();
                    action->from_message(action, msg);
                    switch (action->get_order(action)) {
                        case ACTION_CONNECT_ROBOT:
                            rt_printf("tserver : Action connecter robot\n");
                            rt_sem_v(&semConnecterRobot);
                            break;
                        case ACTION_FIND_ARENA:
                        	rt_printf("tserver : Action trouver arene\n");
                       		rt_mutex_acquire(&mutexEtatCamera, TM_INFINITE);
    						etatCamera = FIND_ARENA;
    						rt_mutex_release(&mutexEtatCamera);
                        	break;
                        case ACTION_ARENA_IS_FOUND:
                        	rt_printf("tserver : Action arène trouvée\n");
                       		rt_mutex_acquire(&mutexEtatCamera, TM_INFINITE);
    						etatCamera = ARENA_IS_FOUND;
    						rt_mutex_release(&mutexEtatCamera);
                        	break;
                        case ACTION_ARENA_FAILED:
                        	rt_printf("tserver : Action trouver arene failed\n");
                       		rt_mutex_acquire(&mutexEtatCamera, TM_INFINITE);
    						etatCamera = ARENA_CANCEL;
    						rt_mutex_release(&mutexEtatCamera);
                        	break;
                        case ACTION_COMPUTE_CONTINUOUSLY_POSITION:
                        	rt_printf("tserver : Action chercher position\n");
                       		rt_mutex_acquire(&mutexModeCalcul, TM_INFINITE);
                       		if (mode_calcul_pos == NON_CALCUL)
    							mode_calcul_pos = CALCUL_CONTINU;
    						else
    							mode_calcul_pos = NON_CALCUL;
    						rt_mutex_release(&mutexModeCalcul);                        	
                    }
                    break;
                case MESSAGE_TYPE_MOVEMENT:
                    rt_printf("tserver : Le message reçu %d est un mouvement\n",
                            num_msg);
                    rt_mutex_acquire(&mutexMove, TM_INFINITE);
                    move->from_message(move, msg);
                    move->print(move);
                    rt_mutex_release(&mutexMove);
                    break;
            }
        }
        else {
        	serveur->close(serveur);
        	rt_printf("tserver : Relance l'exécution de serveur\n");
    		serveur->open(serveur, "8000");
        }
    }
}

void deplacer(void *arg) {
    int status = 1;
    int gauche;
    int droite;
    DMessage *message;

    rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);

    while (1) {
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tmove : Activation périodique\n");

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommRobot;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            rt_mutex_acquire(&mutexMove, TM_INFINITE);
            switch (move->get_direction(move)) {
                case DIRECTION_FORWARD:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_LEFT:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
                case DIRECTION_RIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_STOP:
                    gauche = MOTEUR_STOP;
                    droite = MOTEUR_STOP;
                    break;
                case DIRECTION_STRAIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
            }
            rt_mutex_release(&mutexMove);

            status = robot->set_motors(robot, gauche, droite);

            if (status != STATUS_OK) {
                rt_mutex_acquire(&mutexEtat, TM_INFINITE);
                etatCommRobot = status;
                rt_mutex_release(&mutexEtat);

                message = d_new_message();
                message->put_state(message, status);

                rt_printf("tmove : Envoi message\n");
                if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                    message->free(message);
                }
            }
        }
    }
}

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
    void *msg;
    int err;

    msg = rt_queue_alloc(msgQueue, size);
    memcpy(msg, &data, size);

    if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
        rt_printf("Error msg queue send: %s\n", strerror(-err));
    }
    rt_queue_free(&queueMsgGUI, msg);

    return err;
}


//Utilise variables globales : mode_calcul_pos (RW), etatCamera (RW), camera(W), arena(RW)
//Fonction periodique a lancer toutes les 600ms
void calcul_position (void *arg) {

    rt_printf("tcalcul : Debut de l'éxecution periodique à 600ms\n");
    rt_task_set_periodic(NULL, TM_NOW, 600000000);
    
    camera->open(camera);
    
    arena=NULL;
    	  	
    	
    	
		DImage *img = d_new_image();
		DJpegimage *jpegimg = d_new_jpegimage();
		DPosition *position = d_new_position();
		int cpt;
		int found;
		int mode_calc;
		int aux_etatCamera;
		
		while(1){
		DMessage *message = d_new_message();
		
		rt_mutex_acquire(&mutexEtat, TM_INFINITE);
	    int status = etatCommMoniteur;
 		rt_mutex_release(&mutexEtat); 
		
        rt_task_wait_period(NULL);
        rt_printf("tcalcul : Execution periodique\n");
        		
		if (status == STATUS_OK) {

		
		cpt = 0;
		found = 0;
		
	   	rt_mutex_acquire(&mutexEtatCamera, TM_INFINITE);
		aux_etatCamera = etatCamera;
		rt_mutex_release(&mutexEtatCamera);	
		
		if (aux_etatCamera == FIND_ARENA) {
			    rt_printf("tcalcul : Recherche de l'arene\n");
				while(cpt < 3 && !(found)) {
					d_camera_get_frame(camera, img);
					if ((arena = d_image_compute_arena_position (img)) != NULL)
						found = 1;
					else
						cpt ++;
				}
				if (found) {
					d_imageshop_draw_arena (img, arena);
					jpegimg -> compress(jpegimg, img);
					
					message->put_jpeg_image(message, jpegimg);
									
					rt_printf("tcalcul : Envoi arene\n");
             	   	if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
           	  	       message->free(message);
                	}
        		   	rt_mutex_acquire(&mutexEtatCamera, TM_INFINITE);
                	etatCamera = WAIT_USER;
        			rt_mutex_release(&mutexEtatCamera);	
				}
		}
		else if (aux_etatCamera == WAIT_USER) {
			    rt_printf("tcalcul : Attente de la validation ou non de l'arene\n");
		}
		else { //IS_FOUND, ARENA_CANCEL ou WORKING
				rt_mutex_acquire(&mutexModeCalcul, TM_INFINITE);
                mode_calc = mode_calcul_pos;
                rt_mutex_release(&mutexModeCalcul);
                
				if (mode_calc != NON_CALCUL) {
				    rt_printf("tcalcul : calcul de la position\n");			
					if (aux_etatCamera == ARENA_CANCEL) {
						arena = NULL;
	        		   	rt_mutex_acquire(&mutexEtatCamera, TM_INFINITE);
						etatCamera = WORKING;
	        			rt_mutex_release(&mutexEtatCamera);	
					}
					if (aux_etatCamera == ARENA_IS_FOUND) {
	        		   	rt_mutex_acquire(&mutexEtatCamera, TM_INFINITE);
						etatCamera = WORKING;
	        			rt_mutex_release(&mutexEtatCamera);	
					}
					while (cpt < 3 && !(found)) {
						d_camera_get_frame (camera, img);
						//à cause d'un problème dans le code de compute_robot_position, il faut
						//utiliser arene = NULL pour que la detection ne crashe pas
						if ((position = d_image_compute_robot_position (img, NULL)) != NULL)
							found = 1;
						else
							cpt ++;
					}
					if (found) {
						d_imageshop_draw_position (img, position);
						jpegimg -> compress(jpegimg, img);
					
						message->put_jpeg_image(message, jpegimg);				
						rt_printf("tcalcul : Envoi image avec position\n");
		         	   	if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
		       	  	       message->free(message);
		            	}
		            	
						message = d_new_message();
		            	message->put_position(message, position);
						rt_printf("tcalcul : Envoi position\n");
		         	   	if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
		       	  	       message->free(message);
		            	}
					}
				}
		}
		}

	}
	d_image_free(img);

}













