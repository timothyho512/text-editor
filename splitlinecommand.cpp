#include "splitlinecommand.h"
#include "buffer.h"

using namespace std;

SplitLineCommand::SplitLineCommand(int r, int c) : row(r), col(c), before(""), after("") {}

void SplitLineCommand::execute(Buffer &buffer, int& r, int& c) {
    before = buffer.lines[row].substr(0, col);
	after = buffer.lines[row].substr(col);
	buffer.lines[row] = before;
	buffer.lines.insert(buffer.lines.begin() + row + 1, after);
    r = row+1;
    c = 0;
}

void SplitLineCommand::undo(Buffer &buffer, int& r, int& c) {
    buffer.lines[row] = before + after;
    buffer.lines.erase(buffer.lines.begin() + row + 1);
    r = row;
    c = col;
}