#include "joinlinecommand.h"
#include "buffer.h"

using namespace std;

JoinLineCommand::JoinLineCommand(int r) : row(r), before(""), original("") {}

void JoinLineCommand::execute(Buffer &buffer, int& r, int& c) {
    r = row -1;
    original = buffer.lines[row];
    before = buffer.lines[row-1];
    c = before.length();
    buffer.lines[row - 1] += original;
    buffer.lines.erase(buffer.lines.begin() + row);

}

void JoinLineCommand::undo(Buffer &buffer, int& r, int& c) {
    buffer.lines[row-1] = before;
    buffer.lines.insert(buffer.lines.begin() + row, original);
    r = row;
    c = 0;
}