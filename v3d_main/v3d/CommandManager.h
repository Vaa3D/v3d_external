#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <vector>
#include <string>

class CommandManager
{
public:
    CommandManager(std::vector<char*> *argList);
    bool execute();
    std::string getUsageString();

private:
    std::vector<char*> *argList;

};

#endif // COMMANDMANAGER_H
