#ifndef V3D_COMMAND_RING_H_
#define V3D_COMMAND_RING_H_

/*
 * CommandRing.h
 *
 *  Created on: Dec 24, 2010
 *      Author: cmbruns
 */

class CommandRing
{
public:
    CommandRing(int ringSize)
        : commands(ringSize),
          newestCommandIndex(-1),
          oldestCommandIndex(-1),
          currentCommandIndex(-1)
    {}

    void increment(int& val) { // circular increment
        val = (val+1) % commands.size();
    }

    void decrement(int& val) { // circular decrement
        val = (val+commands.size()-1) % commands.size();
    }

    bool append(const QString& command) {
        if (command.length() == 0)
            return false; // Don't store empty commands
        if (newestCommandIndex == -1) {
            // This is the first command ever
            newestCommandIndex = 0;
            oldestCommandIndex = 0;
            currentCommandIndex = -1;
            commands[newestCommandIndex] = command;
            return true;
        }
        QString previousCommand = commands[newestCommandIndex];
        if (previousCommand == command)
            return false; // Don't store repeated commands

        increment(newestCommandIndex);
        if (oldestCommandIndex == newestCommandIndex)
            increment(oldestCommandIndex);
        currentCommandIndex = newestCommandIndex;
        commands[newestCommandIndex] = command;
        return true;
    }

    QString getNextCommand()
    {
        if (newestCommandIndex == -1) // no commands yet
            return "";
        if (currentCommandIndex == -1) // one past newest
            return "";
        if (currentCommandIndex == newestCommandIndex) {
            currentCommandIndex = -1;
            return "";
        }
        increment(currentCommandIndex);
        return commands[currentCommandIndex];
    }

    QString getPreviousCommand()
    {
        if (newestCommandIndex == -1) // no commands yet
            return "";
        if (currentCommandIndex == -1) { // one past newest
            currentCommandIndex = newestCommandIndex;
            return commands[currentCommandIndex];
        }
        // Sorry, you cannot get the very oldest command.
        if (currentCommandIndex == oldestCommandIndex)
            return "";
        decrement(currentCommandIndex);
        return commands[currentCommandIndex];
    }

private:
    std::vector<QString> commands;
    int newestCommandIndex;
    int oldestCommandIndex;
    int currentCommandIndex;
};

#endif // V3D_COMMAND_RING_H_
