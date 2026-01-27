#include "deletecharcommand.h"
#include "buffer.h"

using namespace std;

DeleteCharCommand::DeleteCharCommand(int r, int c) : row(r), col(c), deleted_char('\n') {}

void DeleteCharCommand::execute(Buffer &buffer, int& r, int& c) {
    deleted_char = buffer.get_line(row)[col-1];
    buffer.lines[row].erase(col - 1, 1);
    r = row;
    c = col-1;
}

void DeleteCharCommand::undo(Buffer &buffer, int& r, int& c) {
    buffer.lines[row].insert(col - 1, 1, deleted_char);
    r = row;
    c = col;
}