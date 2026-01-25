#include "buffer.h"
#include <fstream>
#include <iostream>

using namespace std;

Buffer::Buffer(string fname) : filename(fname), is_modified(false) {
	load_from_file();
}

void Buffer::load_from_file() {
	ifstream in_stream(filename);

	if (!in_stream.is_open()) {
        // File doesn't exist - start with empty buffer
        lines.push_back("");
        is_modified = true;  // Mark as modified so user knows to save
        return;
    }

	char character;
	in_stream.get(character);

	while (!in_stream.eof()) {
		string li = "";
		while (character != '\n') {
			li += character;
			in_stream.get(character);
		}
		lines.push_back(li);
		in_stream.get(character);
	}
	in_stream.close();

	// If file was empty, add one empty line
    if (lines.empty()) {
        lines.push_back("");
    }
}

void Buffer::save_to_file() {
	ofstream out_stream(filename);
	for (const string& line : lines) {
		// need handle invalid filename error here
		out_stream << line << endl;
	}
	out_stream.close();
	is_modified = false;
}

void Buffer::insert_char(int row, int col, char c) {
	lines[row].insert(col, 1, c);
	is_modified = true;
}

void Buffer::delete_char(int row, int col) {
	if (col > 0) {
		lines[row].erase(col - 1, 1);
	}
	is_modified = true;
}

void Buffer::split_line(int row, int col) {
	string before = lines[row].substr(0, col);
	string after = lines[row].substr(col);
	lines[row] = before;
	lines.insert(lines.begin() + row + 1, after);
	is_modified = true;
}

void Buffer::join_lines(int row) {
	if (row > 0) {
		lines[row - 1] += lines[row];
		lines.erase(lines.begin() + row);
	}
	is_modified = true;
}

bool Buffer::islengthless(int row, int l) const {
	int len = lines[row].length();
	if (l > len) return true;
	return false;
}


int Buffer::line_count() const { return lines.size();  }
string Buffer::get_line(int row) const { return lines[row];  }
int Buffer::line_length(int row) const { return lines[row].length();  }
string Buffer::get_filename() const { return filename; }
bool Buffer::get_isModified() const {return is_modified; }

