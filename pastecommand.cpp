#include "pastecommand.h"
#include "buffer.h"

using namespace std;

PasteCommand::PasteCommand(int r, int c, const vector<string>& textBuffer) : row(r), col(c), textBuffer(textBuffer), before("") {}

void PasteCommand::execute(Buffer &buffer, int& r, int& c) {
    before = buffer.lines[row];
    int size = static_cast<int>(textBuffer.size());
	if (size == 1) {
		buffer.lines[row].insert(col, textBuffer[0]);
        c = col + textBuffer[0].length();
		return;
	}
	string after = buffer.lines[row].substr(col, buffer.lines[row].length()-col);
	for (int i = row; i < row + size; i++) {
		if (i == row) {
			buffer.lines[row].erase(col, buffer.lines[row].length()-col);
			buffer.lines[row].insert(col, textBuffer[i-row]);
		}
		else if (i > row && i < row + size - 1) {
			buffer.lines.insert(buffer.lines.begin() + i, textBuffer[i-row]);
		}
		else if (i == row + size - 1) {
			buffer.lines.insert(buffer.lines.begin()+i, textBuffer[i-row]+after);
            col = textBuffer[i-row].length();
		}
	}
    r = row + size - 1;
    c = textBuffer[size - 1].length();
}

void PasteCommand::undo(Buffer &buffer, int& r, int& c) {
    buffer.lines[row] = before;
    buffer.lines.erase(buffer.lines.begin() + row + 1, buffer.lines.begin() + row + textBuffer.size());
    r = row;
    c = col;

}