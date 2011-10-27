/***************************************************************************
 *   Copyright (C) 2007 by Arnaud Sevin
 *   Arnaud.Sevin@obspm.fr
 * 
 *   With the help of Eric Gendron, Zoltan Hubert, Denis Peret 
 *
 * Copyright (c) 1998, Regents of the University of California
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/
#ifndef SERVER_TCP_H
#define SERVER_TCP_H

/**
   @author Arnaud Sevin <Arnaud.Sevin@obspm.fr>
*/

#include <iostream>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <pthread.h>

#include "ConfigFile.h"

// How many caracteres used for size of message
#define NBCHAR_SIZE 4 

// define the number of arguments maximum  
#define MAXARGS     31

/**
 * Define the time out in sec
 */
#define TCPTIMEOUT 60

// define max size of answer
#define MAX_MESSAGE 9999

using namespace std;

class Server_TCP{
 protected:
  int  debug;
  int  log;
  string logfilename;
  ofstream fichierlog;
  bool server_run;
  int  server_port;
  string server_ip;
  string client_ip;
  int  sock;
  int  csock;

 public:
  Server_TCP();
  Server_TCP(const char *config_filename);

  virtual ~Server_TCP() {}

  bool is_listening();
  virtual void start_listen();
  void stop_listen();
  static void *start_thread(void *);
  virtual void listen_TCP();

 protected:
  int  createSocket();
  virtual void traite_socket(char keyword[9], int nb_arg, char *arg[MAXARGS]);
  void die( const char *mess );
  virtual void display_help();
  void send_err( const char *args, ... );
  void send_fini( const char *args, ... );
  void send_info( const char *args, ... );
  void ecrire(bool verbous, const char *args, ...);
  int decompose_arguments(char* string, char* arg[]);
};

#endif
