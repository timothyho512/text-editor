#ifndef DELETECHARCOMMAND_H
#define DELETECHARCOMMAND_H

#include "command.h"

class DeleteCharCommand : public Command {
private:
    int row;
    int col;
    char deleted_char;

public:
    DeleteCharCommand(int r, int c);
    void execute(Buffer &buffer, int& r, int& c) override;

    void undo(Buffer &buffer, int& r, int& c) override;
};

#endif