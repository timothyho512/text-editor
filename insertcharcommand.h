#ifndef INSERTCHARCOMMAND_H
#define INSERTCHARCOMMAND_H

#include "command.h"

class InsertCharCommand : public Command {
private:
    int row;
    int col;
    char character;

public:
    InsertCharCommand(int r, int c, char ch);
    void execute(Buffer &buffer, int& r, int& c) override;

    void undo(Buffer &buffer, int& r, int& c) override;
};

#endif