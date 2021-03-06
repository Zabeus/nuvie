/*
Copyright (C) 2002 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <vector>
#include <set>

class XMLTree;
class ConfigNode;

#define NUVIE_CONF_READONLY true
#define NUVIE_CONF_READWRITE false

/*
Configuration class.

The configuration values are stored in a tree. (or a forest, technically)
All values are stored as strings, but access functions for ints and bools
 are provided
You should only store values in leaf nodes. (This isn't enforced everywhere,
 but contents of non-leaf nodes can disappear without warning)

You can load multiple config files, which can be read-only.
Each config file contains a single tree.
Values in files loaded last override values in files loaded earlier.
Values are written to the last-loaded writable config file with the right root.
 Because of this it's important to make sure the last-loaded config file with
 a given root is writable. (The idea is that you can load a system-wide config
 file first, and a user's config file after that.)

*/

class Configuration
{
 public:
	Configuration();
	~Configuration();

	// read config file. Multiple files may be read. Order is important.
	bool readConfigFile(std::string fname, std::string root,
						bool readonly=false);
	// write all (writable) config files
	void write();
	// clear everything
	void clear();

	std::string filename() { return config_filename; }

	// get value
	void value(std::string key, std::string &ret, const char *defaultvalue="");
	void value(std::string key, int &ret, int defaultvalue=0);
	void value(std::string key, bool &ret, bool defaultvalue=false);

  void pathFromValue(std::string key, std::string file, std::string &full_path);

	// set value
	bool set(std::string key, std::string value);
	bool set(std::string key, const char* value);
	bool set(std::string key, int value);
	bool set(std::string key, bool value);

	// get node ref. (delete it afterwards)
	ConfigNode* getNode(std::string key);

	// list all subkeys of a key. (no guaranteed order in result)
	std::set<std::string> listKeys(std::string key, bool longformat = false);

	typedef std::pair<std::string, std::string> KeyType;
	typedef std::vector<KeyType> KeyTypeList;

	void getSubkeys(KeyTypeList &ktl, std::string basekey);

 private:

	std::vector<XMLTree*> trees;
	std::string config_filename;
};

#endif
