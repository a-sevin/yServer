/***************************************************************************
 *   Copyright (C) 2007 by Arnaud Sevin
 *   Arnaud.Sevin@obspm.fr
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
#include "ConfigFile.h"

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

string trim(string const& source, char const* delims = " \t\r\n") {
	string result(source);
	string::size_type index = result.find_last_not_of(delims);
	if (index != string::npos)
		result.erase(++index);

	index = result.find_first_not_of(delims);
	if (index != string::npos)
		result.erase(0, index);
	else
		result.erase();
	return result;
}

ConfigFile::ConfigFile(string const& configFile) {
	try {
		ifstream file(configFile.c_str());
		string line;
		string name;
		string value;
		string inSection;
		int posEqual;
		while (getline(file, line)) {

			if (!line.length())
				continue;

			if (line[0] == '#')
				continue;
			if (line[0] == ';')
				continue;

			if (line[0] == '[') {
				inSection = trim(line.substr(1, line.find(']') - 1));
				continue;
			}

			posEqual = line.find('=');
			name = trim(line.substr(0, posEqual));
			value = trim(line.substr(posEqual + 1));

			content_[inSection + '/' + name] = value;
		}
	} catch (...) {
		throw string("file does not exist");
	}

}

long int const ConfigFile::ValueI(string const& section, string const& entry) const {
	long int tmpI = 0;

	map<string, string>::const_iterator ci = content_.find(section + '/'
			+ entry);

	if (ci == content_.end())
		throw string("entry does not exist");
	else
		tmpI = atoi(ci->second.c_str());

	return tmpI;
}

double const ConfigFile::ValueD(string const& section, string const& entry) const {

	double tmpD = 0;
	map<string, string>::const_iterator ci = content_.find(section + '/'
			+ entry);

	if (ci == content_.end())
		throw string("entry does not exist");
	else
		tmpD = atof(ci->second.c_str());

	return tmpD;
}

string const& ConfigFile::ValueS(string const& section, string const& entry) const {
	map<string, string>::const_iterator ci = content_.find(section + '/'
			+ entry);
	if (ci == content_.end())
		throw string("entry does not exist");
	return ci->second;
}
