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
#include "server_tcp.h"

int decompose_arguments(char* string, char* arg[]);
void ecrire(bool verbous, char *args, ...);

Server_TCP::Server_TCP()
{
  /**************************************/
  /*     Initialisation des variables   */
  /**************************************/
  this->server_run = false;

  this->debug = 0;
  
  this->log = 0;
  this->logfilename = "/tmp/null.log";
    
  char hostname[128];
  cerr << "warning : hostname not define ! try to find it... ";
  if(gethostname(hostname, 128)){ 
    cerr << "not found ! please define it in the config file\n";
    exit(0);
  } 
  this->server_ip = hostname;
  cerr << hostname << " found\n";

  this->server_port = 1501;
  cerr << "warning : port not define ! using default one : " << this->server_port << endl;

  /**************************************/
  /*   Creation du socket               */
  /**************************************/
  this->sock=createSocket( );
}

Server_TCP::Server_TCP(const char *config_filename)
{
  /**************************************/
  /*     Initialisation des variables   */
  /**************************************/
  this->server_run = false;

  /**************************************/
  /*     Lecture de la configuration    */
  /**************************************/
  ConfigFile cf(config_filename);

  try{
    this->debug = cf.ValueI("general", "debug");

    this->log = cf.ValueI("general", "log");
    this->logfilename = cf.ValueS("general", "logfilename");

    if(this->log){
      // Creation du fichier ou écrasement
      this->fichierlog.open(this->logfilename.c_str(), ios::trunc);
    
      /* --- Cas d'erreur d'ouverture du fichier --- */
      if ( !this->fichierlog ){
	cerr << "Erreur de creation du fichier" << endl;
	this->log=0;
      }
    }

    this->server_port = cf.ValueI("serveur", "port");
  } catch (string msg)  {
    cerr << "ERREUR (serveur_TCP) : " << msg << endl;
  }

  try{
    this->server_ip = cf.ValueS("serveur", "ip");
  } catch ( string tmp ){
    char hostname[128];
    cerr << "warning : hostname not define ! try to find it... ";
    if(gethostname(hostname, 128)){ 
      cerr << "not found ! please define it in the config file\n";
      exit(0);
    } 
    this->server_ip = hostname;
    cerr << hostname << " found\n";
  }
  /**************************************/
  /*   Creation du socket               */
  /**************************************/
  this->sock=createSocket( );
}

/*!
  \fn Server_TCP::start_thread(void *)
*/
void * Server_TCP::start_thread(void *server_arg)
{
  Server_TCP *server = (Server_TCP *)server_arg;
  server->listen_TCP();
  return (void*)0L;
}

/*!
  \fn Server_TCP::listen_TCP()
*/
void Server_TCP::listen_TCP()
{
  if(csock<0) return;
  server_run = true;

  while(server_run) {  // main accept() loop
    struct sockaddr_in csin;
    socklen_t csinsize=sizeof(csin);

    /**************************************/
    /*     Ecoute du du port TCP          */
    /**************************************/
    csock=accept(sock,(struct sockaddr *)&csin,&csinsize);

    if ( csock == -1) {
      //perror("ERREUR : ouverture du socket impossible");
      continue;
    }

    /**************************************/
    /*         GO, GO, GO                 */
    /**************************************/
    client_ip = inet_ntoa(csin.sin_addr);
    ecrire(debug, (const char*)"adresse du client: %s\n",client_ip.c_str());

    /**************************************/
    /*     Declaration des variables      */
    /**************************************/
    char    *arg[MAXARGS];
    int     nb_arg;
    char    first_buffer[11+NBCHAR_SIZE];
    char    keyword[9];
    int     msglength, recvlength;
	
    /**********************************/
    /*     procedure mot de passe     */
    /**********************************/
    if((recvlength=recv(csock,first_buffer,11+NBCHAR_SIZE, 0))==-1){
      send_err("\nERREUR : reception impossible");
      continue;
    }
	
    if((strncmp(first_buffer,"SESAME_OPEN",11))!=0) {
      ecrire(debug, "\n%s %d\n",first_buffer,recvlength);
      send_err("\nERREUR : mot de passe incorrect\n");
      continue;
    }
	
    /**********************************/
    /*     Reponse : ok, je suis pret */
    /**********************************/
    msglength=strtol(first_buffer+11, NULL,0);
    strcpy( first_buffer, "SESAME_PRET");
    send( csock, first_buffer, strlen(first_buffer), 0);

    char buffer[msglength];
    /**********************************/
    /*    Reception de la commande    */
    /**********************************/
    if((recvlength=recv(csock,buffer,msglength, 0))==-1) {
      send_err("\nERREUR : reception impossible");
      continue;
    }
    buffer[recvlength]='\0';
    if(log){
      char dateheure[20];   //buffer contenant la chaîne finale
      time_t time_of_day;    //valeur contenant le temps écoulé en secondes depuis 01/01/1970
      time( &time_of_day );  //récupère le temps passé
      strftime(dateheure, 20, "%d/%m/%Y %H:%M:%S", localtime(&time_of_day));
      fichierlog << dateheure << " " << client_ip << "\t" << buffer << endl;
    }
	
    if(recvlength<8){
      send_err("A pas compris!, tapez MAN_SERV pour plus d'information\n");
    } else {
      /**********************************/
      /*  Analyse de la commande        */
      /**********************************/
      buffer[recvlength]='\0';
      strncpy( keyword, buffer, 8 );
      keyword[8]='\0';
	
      char arguments[msglength-7];
      strncpy( arguments, buffer+8, msglength-8);
      arguments[msglength-8]='\0';
	
      nb_arg=decompose_arguments(arguments,arg);
	
      ecrire(debug, "Mot-clef:  ---%s---\n", keyword );
      int argument = 0;
      while(argument!=nb_arg)    {
	ecrire(debug, "arg%d: %s\n",argument,arg[argument]);
	argument++;
      }
	
      traite_socket(keyword, nb_arg, arg);

      for(int i=0;i<nb_arg;i++){
	delete arg[i];
	arg[i]=0L;
      }
    }
  }
}


/*!
  \fn Server_TCP::is_listen()
*/
bool Server_TCP::is_listening()
{
  return server_run;
}


/*!
  \fn Server_TCP::start_listen()
*/
void Server_TCP::start_listen()
{
  /**************************************/
  /*     Ecoute du du port TCP          */
  /**************************************/
  pthread_t tcp_loop;
  pthread_create (&tcp_loop, NULL, Server_TCP::start_thread, this);

  server_run=true;
}
/*!
  \fn Server_TCP::stop_listen()
*/
void Server_TCP::stop_listen()
{
  if (sock>=0)  close(sock);
  server_run=false;
}

/*!
  \fn Server_TCP::traite_socket( char keyword[9], int nb_arg, char *arg[MAXARGS] )
*/
void Server_TCP::traite_socket( char keyword[9], int nb_arg, char *arg[MAXARGS] )
{
  /**********************************/
  /*  Execution de la commande      */
  /**********************************/
  if(strcmp( keyword, "OK_READY" )==0){
    send_fini ( "SESAME pret.\n" );
  } else if(strcmp( keyword, "MAN_SERV" )==0){
    display_help( );
    send_fini ( "SESAME pret.\n" );
  } else if( (strcmp( keyword, "STP_SERV" )==0) ){
    send_fini ( "SESAME shut down.\n" );
    server_run=false;
  } else {
    send_err("A pas compris!, tapez MAN_SERV pour plus d'information\n");
  }
}


/*!
  \fn Server_TCP::createSocket()
*/
int Server_TCP::createSocket()
{
  /**************************************/
  /*  mise en place du socket d'ecoute  */
  /**************************************/
  struct hostent* local;
  local=gethostbyname(server_ip.c_str());
  if(local==NULL)
    die("\nerreur au gesthostbyname\n");

  char*   localIP;
  localIP = inet_ntoa (*(struct in_addr *)*local->h_addr_list);
  ecrire(debug, "\nname: %s adress: %s\n",local->h_name,localIP);

  struct sockaddr_in adresse_serveur;
  memset(&adresse_serveur,0,sizeof(struct sockaddr_in));

  adresse_serveur.sin_family = AF_INET;
  adresse_serveur.sin_port = htons(server_port);
  memcpy( (char *)&adresse_serveur.sin_addr.s_addr,
	  local->h_addr_list[0],local->h_length);

  int sock=0;
  sock=socket(local->h_addrtype,SOCK_STREAM,0); //cree la socket d'ecoute
  if (sock < 0)
    die("\nError at socket()");

  struct timeval      timeout;
  timeout.tv_sec  = TCPTIMEOUT;
  timeout.tv_usec  = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))==-1) {
    die("ERROR : can't define the TCP socket timeout\n");
  }

  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))==-1) {
    die("ERROR : can't define the TCP socket timeout\n");
  }

  int yes=1;
  // lose the pesky "Address already in use" error message
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1) {
    die("ERROR : can't lose the pesky \"Address already in use\" error message\n");
  }

  //effectue la liaison de la socket avec l'adresse
  if (bind(sock,(struct sockaddr *)&adresse_serveur,sizeof(adresse_serveur))==-1)
    die("\nError at bind()");

  //ecoute la socket
  if (listen(sock,10)==-1)
    die("\nError au listen()");

  return sock;
}


/*!
  \fn Server_TCP::die( const char *mess )
*/
void Server_TCP::die( const char *mess )
{
  perror(mess);
  close(csock);
  csock = -1;
  throw "server die";
}


/*!
  \fn Server_TCP::display_help()
*/
void Server_TCP::display_help()
{
  send_info("----HELP FONCTION----\n");
  send_info("* STP_SERV : Stop le service\n");
}


/*!
  \fn Server_TCP::send_err( const char *args, ... )
*/
void Server_TCP::send_err( const char *args, ... )
{
  char buffer[MAX_MESSAGE], info[MAX_MESSAGE];
  va_list args_list;

  va_start(args_list, args);
  vsprintf(info,args, args_list);
  va_end(args_list);

  if(log){
    char dateheure[20];   //buffer contenant la chaîne finale
    time_t time_of_day;    //valeur contenant le temps écoulé en secondes depuis 01/01/1970
    time( &time_of_day );  //récupère le temps passé
 
    strftime(dateheure, 20, "%d/%m/%Y %H:%M:%S", localtime(&time_of_day));
    fichierlog << dateheure << " ERROR : " << info;
  }

  sprintf(buffer,"SESAME_FINI%4dERROR : %s",strlen(info)+8,info);
  send( csock, buffer, strlen(buffer), 0);
  close(csock);
}


/*!
  \fn Server_TCP::send_fini( const char *args, ... )
*/
void Server_TCP::send_fini( const char *args, ... )
{
  char buffer[MAX_MESSAGE], info[MAX_MESSAGE];
  va_list args_list;

  va_start(args_list, args);
  vsprintf(info,args, args_list);
  va_end(args_list);

  sprintf(buffer,"SESAME_FINI%4dREADY : %s",strlen(info)+8,info);
  send( csock, buffer, strlen(buffer), 0);
  close(csock);
}


/*!
  \fn Server_TCP::send_info( const char *args, ... )
*/
void Server_TCP::send_info(const char *args, ...) {
  char buffer[MAX_MESSAGE], info[MAX_MESSAGE];
  va_list args_list;

  va_start(args_list, args);
  vsprintf(info,args, args_list);
  va_end(args_list);

  sprintf(buffer,"SESAME_INFO%4d%s",strlen(info),info);
  send( csock, buffer, strlen(buffer), 0);
}

/*!
  \fn Server_TCP::ecrire(int verbous, const char *args, ...)
*/
void Server_TCP::ecrire(bool verbous, const char *args, ...) {
  if (verbous) {
    va_list args_list;

    va_start(args_list, args);
    vprintf(args, args_list);
    va_end(args_list);
  }
}

/*!
  \fn Server_TCP::decompose_arguments(char* string, char* arg[])
*/
int Server_TCP::decompose_arguments(char* string, char* arg[])
{
  char * pch;
  int i=0;
  pch = strtok (string,"_");
  while (pch != NULL)
    {
      arg[i] = new char[strlen(pch)];
      strcpy(arg[i],pch);
      i++;
      pch = strtok (NULL, "_");
    }
  return i;
}
