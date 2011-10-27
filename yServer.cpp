//
// C++ Implementation: server_persee
//
// Description:
//
//
// Author: Arnaud Sevin <Arnaud.Sevin@obspm.fr>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "yServer.h"
#include "yapi.h"
#include "pstdlib.h"
#include <sstream>

// declare class static object
bool YServer::working=false;

YServer::YServer() : Server_TCP() {}

YServer::YServer(const char *config_filename) : Server_TCP(config_filename)
{
  working = false;
  /**************************************/
  /*     Lecture de la configuration    */
  /**************************************/
  try{
    ConfigFile cf(config_filename);
  } catch (string msg) {
    cerr << "ERREUR (YServer): " << msg << endl;
  }
}


YServer::~YServer()
{
  working = false;
}


/*!
  \fn YServer::traite_socket(char keyword[9], int nb_arg, char *arg[MAXARGS])
*/
void YServer::traite_socket(char keyword[9], int nb_arg, char *arg[MAXARGS])
{
  /**************************************/
  /*     Declaration des variables      */
  /**************************************/
  short int Err;

  /**********************************/
  /*  Traitemant de la commande     */
  /**********************************/
  if( working ){
    send_info("Server is acquiring...\n");
  } else if(strcmp( keyword, "OK_READY" )==0){
  } else if(strcmp( keyword, "HALT_NOW" )==0){
    system("halt");
  } else if(strcmp( keyword, "RESET_OS" )==0){
    system("reboot");
  } else if(strcmp( keyword, "MAN_SERV" )==0){
    display_help( );
  } else if(strcmp( keyword, "YCOMMAND" )==0){
    long dims[2] = {1,1};
    ystring_t *cmd = ypush_q(dims);

    cmd[0] = p_strcpy("");
    for(int i=0; i<nb_arg; i++) {
      if(i!=0) cmd[0] = p_strncat(cmd[0], "_", sizeof(char));
      cmd[0] = p_strncat(cmd[0], arg[i], strlen(arg[i])* sizeof(char));
    }
    cmd[0] = p_strncat(cmd[0], " ", sizeof(char));

    stringstream buff;
    buff << "Execute command : " <<  cmd[0];
    cout << buff.str();
    cout.flush();
    //y_print(buff.str().c_str(),0); 

    send_info(buff.str().c_str());
    //yexec_include(0, 1);
    start_command();
    int i;
    while(working){
      for(i=0;i<10000&&working;i++) usleep(1000);
      if(i==10000){
	send_info(".");
	cout << ".";
	cout.flush();
      }
    }
    send_info(" done\n");
    cout << " done" << endl;

  } else {
    send_info("ERROR : not understood !, type MAN_SERV for more information\n");
  }
  /**********************************/
  /*     Envoi de la reponse        */
  /**********************************/
  if(csock>=0) send_fini ( "yServer Ready.\n" );  
}

/*!
  \fn YServer::display_help()
*/
void YServer::display_help()
{
  send_info("----HELP FONCTION----\n");
  send_info("* OK_READY : Verifier que le serveur fonctionne\n");
  send_info("* MAN_SERV : Affiche cette aide\n");
  send_info("* YCOMMAND_func_att1_att2...   : Execute la fonction avec les attributs donnés \n");
}

/*!
  \fn YServer::start_command()
*/
void YServer::start_command()
{
  /**************************************/
  /*     Ecoute du du port TCP          */
  /**************************************/
  working=true;
  pthread_t tcp_loop;
  pthread_create (&tcp_loop, NULL, YServer::execute_command, NULL);

}

/*!
  \fn YServer::execute_command(void *)
*/
void * YServer::execute_command( void *)
{
  yexec_include(0, 1);
  working=false;
  return (void*)0L;
}


extern "C" {

  typedef struct tServer {
    YServer *server_obj;
  } tServer;

  void server_free(void *yObj) {
    tServer *server = (tServer *)yObj;
    server->server_obj->stop_listen();
    delete server->server_obj;
  }

  char g_ObjectName[32] = "SESAME Server Object";
  static y_userobj_t server_yObj = { g_ObjectName, &server_free, 0, 0, 0, 0 };

  void Y_server_obj(int argc) {
    try {

      tServer *server = (tServer *) ypush_obj(&server_yObj, sizeof(tServer));
      if(argc==1){
	server->server_obj = new YServer();
      } else if (argc==2) {
	ystring_t config_filename = ygets_q(argc-1);
	server->server_obj = new YServer(config_filename);
      } else {
	server->server_obj=NULL;
	throw "Bad call";
      }
    } catch ( string msg ) {
      y_error(msg.c_str());
    }
    catch ( char const * msg ) {
      y_error(msg);
    }
  }

  void Y_start_server(int argc) {
    tServer *server = (tServer *)yget_obj(argc-1,&server_yObj);
    if(server->server_obj!=NULL) 
      server->server_obj->start_listen();
  }
  
  void Y_stop_server(int argc) {
    tServer *server = (tServer *)yget_obj(argc-1,&server_yObj);
    if(server->server_obj!=NULL) 
      server->server_obj->stop_listen();
  }

}
