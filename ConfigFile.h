#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

#include <string>
#include <map>

using namespace std;

/**
 * @brief Classe permettant de lire un fichier et de la parcourir rapidement
 * @author   Arnaud Sevin <Arnaud.Sevin@obspm.fr>
 * @version  1.0
 * @date     02/06/2008
 */
class ConfigFile {
  ///Ensemble des objets trouvés
  map<string,string> content_;

public:
  /**
   * Permet de lancer l'analyser un fichier
   * @param configFile Emplacement du fichier à analyser
   */
  ConfigFile(string const& configFile);

  
  /**
   * Permet d'accéder à la valeur entière de l'entrée 
   * @param section Section où se trouve l'entrée
   * @param entry Le nom de l'entrée
   * @return La valeur entière de l'entrée 
   */
  long int   const ValueI(string const& section, string const& entry) const;
  
  /**
   * Permet d'accéder à la valeur flottante de l'entrée 
   * @param section Section où se trouve l'entrée
   * @param entry Le nom de l'entrée
   * @return La valeur flottante de l'entrée 
   */
  double     const ValueD(string const& section, string const& entry) const;
  
  /**
   * Permet d'accéder à la valeur de l'entrée 
   * @param section Section où se trouve l'entrée
   * @param entry Le nom de l'entrée
   * @return La valeur de l'entrée 
   */
  string const& ValueS(string const& section, string const& entry) const;

};

#endif
