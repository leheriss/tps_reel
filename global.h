/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

//constantes pour mode_calcul_pos
#define NON_CALCUL 0
#define CALCUL_CONTINU 1

//constantes pour etat_camera
#define WAIT_USER 0
#define WORKING 1
#define FIND_ARENA 2
#define ARENA_IS_FOUND 3
#define ARENA_CANCEL 4

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tServeur;
extern RT_TASK tconnect;
extern RT_TASK tmove;
extern RT_TASK tenvoyer;
extern RT_TASK tcalcul;
extern RT_TASK tbat;
extern RT_TASK twatch;

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexEtatCamera;
extern RT_MUTEX mutexModeCalcul;
extern RT_MUTEX mutexCompteur;
extern RT_MUTEX mutexComm;

/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semWatchdog;
extern RT_SEM semBatterie;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int mode_calcul_pos;
extern int etatCamera; 
extern int compteur;
extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TCALCUL;
extern int PRIORITY_TBAT;
extern int PRIORITY_TWATCH;

#endif	/* GLOBAL_H */

