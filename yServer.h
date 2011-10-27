//
// C++ Interface: server_chara
//
// Description:
//
//
// Author: Arnaud Sevin <Arnaud.Sevin@obspm.fr>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef YSERVER_H
#define YSERVER_H

#include "server_tcp.h"
#include <sys/stat.h>
#include <sys/mman.h>

/**
 * @brief Classe représentant le serveur d'acquisition Yorick
 * @author   Arnaud Sevin <Arnaud.Sevin@obspm.fr>
 * @version  1.0
 * @date     12/10/2011
 */
class YServer : public Server_TCP {
 private:
  ///Etat du serveur
  static bool working;

 public:
  
  YServer();

  /**
   * Permet de créer un serveur PERSEE
   * @param config_filename Emplacement du fichier de config
   */
  YServer(const char *config_filename);
  ~YServer();

 private:

  /**
   * Permet connaitre l'état du serveur
   * @return Vrai s'il écoute les commandes du PC maitre
   */
  bool is_working() {return working;}
  void traite_socket(char keyword[9], int nb_arg, char *arg[MAXARGS]);
  void display_help();

  void start_command();
  static void * execute_command(void *);

};

#endif
