#include "buffer.h"
#include "insertcharcommand.h"
#include "deletecharcommand.h"
#include "splitlinecommand.h"
#include "joinlinecommand.h"
#include <fstream>
#include <algorithm>
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

void Buffer::execute_and_add_to_history(Command* cmd) {
	for (auto c : redo_stack) delete c;
	redo_stack.clear();

	int r, d;
	cmd->execute(*this, r, d);
	undo_stack.push_back(cmd);
	is_modified = true;
}

void Buffer::insert_char(int row, int col, char c) {
	execute_and_add_to_history(new InsertCharCommand(row, col, c));
}

void Buffer::delete_char(int row, int col) {
	if (col > 0) {
		execute_and_add_to_history(new DeleteCharCommand(row, col));
	}
}

void Buffer::split_line(int row, int col) {
	execute_and_add_to_history(new SplitLineCommand(row, col));
}

void Buffer::join_lines(int row) {
	if (row > 0) {
		execute_and_add_to_history(new JoinLineCommand(row));
	}
}

bool Buffer::islengthless(int row, int l) const {
	int len = lines[row].length();
	if (l > len) return true;
	return false;
}

bool Buffer::find_next(const string& term, int& r, int& c) {
	for (size_t i = static_cast<size_t>(r); i < lines.size(); i++) {
		const string& line = lines[i];

		size_t start = i == static_cast<size_t>(r) ? static_cast<size_t>(c + 1) : 0;

		size_t pos = line.find(term, start);
		if (pos != string::npos) {
			r = static_cast<int>(i);
            c = static_cast<int>(pos);
			return true;
		}
	}
	return false;
}

bool Buffer::find_prev(const string& term, int& r, int& c) {
	if (term.empty()) return false;
    if (r < 0) return false;

	int best_r = -1;
	int best_c = -1;
	for (size_t i = 0; i <= static_cast<size_t>(r) && i < lines.size(); i++) {
		
		string line = lines[i];
		size_t limit = line.size();
        if (i == static_cast<size_t>(r)) {
            if (c <= 0) limit = 0;
            else limit = std::min(static_cast<size_t>(c), line.size());
        }
		if (limit >= term.size()) {
			size_t pos = line.rfind(term, limit);
			if (pos != string::npos && pos + term.size() <= limit) {
				best_r = static_cast<int>(i);
				best_c = static_cast<int>(pos);
			}
		}
	}
	if (best_r == -1) return false;
	r = best_r;
	c = best_c;
	return true;
}

void Buffer::undo(int& row, int& col) {
	if (!undo_stack.empty()) {
		Command* i = undo_stack.back();
		undo_stack.pop_back();
		i->undo(*this, row, col);
		redo_stack.push_back(i);
	}
}

void Buffer::redo(int& row, int& col) {
	if (!redo_stack.empty()) {
		Command* i = redo_stack.back();
		redo_stack.pop_back();
		i->execute(*this, row, col);
		undo_stack.push_back(i);
	}
}


int Buffer::line_count() const { return lines.size();  }
string Buffer::get_line(int row) const { return lines[row];  }
int Buffer::line_length(int row) const { return lines[row].length();  }
string Buffer::get_filename() const { return filename; }
bool Buffer::get_isModified() const {return is_modified; }

