//plug_dir, _("./",plug_dir());
plug_in,"yServer";

extern server_obj;
/* DOCUMENT server_obj
   server = server_obj(configfile)

   This function create a server_obj
   
   SEE ALSO:
 */


extern start_server;
/* DOCUMENT start_server
   start_server, server_obj

   This function starts the server_obj
   
   SEE ALSO:
 */

extern stop_server;
/* DOCUMENT stop_server
   stop_server, server_obj

   This function stops the server_obj
   
   SEE ALSO:
 */

func pause(i) { system, swrite(format="sleep %f",i/1000.); }
