#ifndef SPLITLINECOMMAND_H
#define SPLITLINECOMMAND_H

#include "command.h"
#include <string>

class SplitLineCommand : public Command {
private:
    int row;
    int col;
    std::string before;
    std::string after;

public:
    SplitLineCommand(int r, int c);
    void execute(Buffer &buffer, int& r, int& c) override;

    void undo(Buffer &buffer, int& r, int& c) override;
};

#endif