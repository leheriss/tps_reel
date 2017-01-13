/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;
RT_TASK tcalcul;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexEtatCamera;
RT_MUTEX mutexModeCalcul;
RT_TASK tbat;
RT_TASK twatch;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexCompteur;
RT_MUTEX mutexComm;

RT_SEM semConnecterRobot;
RT_SEM semWatchdog;
RT_SEM semBatterie;

RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
int mode_calcul_pos = 0;        // 0 -> pas de calcul, 1 -> calcul unique, 2 -> calcul continu
int etatCamera = 1;             // cf plus bas
int compteur = 0;

DRobot *robot;
DMovement *move;
DServer *serveur;
DArena *arena;
DCamera *camera;

int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TCALCUL = 18;      // /!\ a determiner
int PRIORITY_TBAT = 15;
int PRIORITY_TWATCH = 20;
