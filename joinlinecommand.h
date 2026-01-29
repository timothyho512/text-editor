#ifndef JOINLINECOMMAND_H
#define JOINLINECOMMAND_H

#include "command.h"
#include <string>

class JoinLineCommand : public Command {
private:
    int row;
    std::string before;
    std::string original;

public:
    JoinLineCommand(int r);
    void execute(Buffer &buffer, int& r, int& c) override;

    void undo(Buffer &buffer, int& r, int& c) override;
};

#endif