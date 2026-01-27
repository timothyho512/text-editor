#include "insertcharcommand.h"
#include "buffer.h"

using namespace std;

InsertCharCommand::InsertCharCommand(int r, int c, char ch) : row(r), col(c), character(ch) {}

void InsertCharCommand::execute(Buffer &buffer, int& r, int& c) {
    buffer.lines[row].insert(col, 1, character);
    r = row;
    c = col+1;

}

void InsertCharCommand::undo(Buffer &buffer, int& r, int& c) {
    // Remove the character we inserted
    buffer.lines[row].erase(col, 1);
    r = row;
    c = col;
}