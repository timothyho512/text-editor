#ifndef PASTECOMMAND_H
#define PASTECOMMAND_H

#include "command.h"
#include <string>
#include <vector>

class PasteCommand : public Command {
private:
    int row;
    int col;
    std::vector<std::string> textBuffer;
    std::string before;
    // std::string after;

public:
    PasteCommand(int r, int c, const std::vector<std::string>& textBuffer);
    void execute(Buffer &buffer, int& r, int& c) override;

    void undo(Buffer &buffer, int& r, int& c) override;
};

#endif