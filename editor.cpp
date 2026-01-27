#include "editor.h"
#include <ncurses.h>
#include <cctype>
#include <algorithm>

using namespace std;

Editor::Editor(string filename) : buffer(filename), running(true), desire_col(0), scroll_offset_y(0), scroll_offset_x(0), current_match_row(0), current_match_col(0) {
	cursor.row = 0;
	cursor.col = 0;
	init_ncurses();
}

Editor::~Editor() {
	endwin();
}

void Editor::init_ncurses() {
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	set_escdelay(25); // was default (~1000ms); make ESC exit immediate
}

void Editor::render() {
	int height, width;
	getmaxyx(stdscr, height, width);

	// reserved for status bar, visible height
	int max_visible_lines = height - 1;
	int max_visible_width = width - LINE_NUMBER_WIDTH;
	clear();
	for (int i = scroll_offset_y; i < buffer.line_count() && i < scroll_offset_y + max_visible_lines; i++) {
		// i + 1 is the line number
		int line_number = i + 1;
		int screen_row = i - scroll_offset_y;
		string line = buffer.get_line(i);
		string visible_portion;
		if (line.length() > scroll_offset_x) {
			visible_portion = line.substr(scroll_offset_x, max_visible_width);
		} else {
			visible_portion = "";  // Line is shorter than scroll offset, show nothing
		}
		mvprintw(screen_row, 0, "%d", line_number);
		mvprintw(screen_row, LINE_NUMBER_WIDTH, visible_portion.c_str());
	}

	// draw status bar
	string left_status = buffer.get_filename();
	string middle_status = "";
	if (buffer.get_isModified()) {
		left_status += " [+]";
	}
	if (search_mode) {
		middle_status = "search: " + search_term;
	}
	if (!search_mode) middle_status = "";
	string right_status = to_string(cursor.row + 1) + ":" + to_string(cursor.col + 1);
    right_status += " | " + to_string(buffer.line_count()) + " lines";

	int spaces_needed = width - right_status.length() - left_status.length() - middle_status.length();
	int half_spaces_needed = spaces_needed / 2;
	if (half_spaces_needed <= 5) half_spaces_needed = 5;

	string status = left_status + string(half_spaces_needed, ' ') + middle_status + string(half_spaces_needed, ' ') + right_status;

	mvprintw(height - 1, 0, status.c_str());

	int cursor_screen_row = cursor.row - scroll_offset_y;
	move(cursor_screen_row, cursor.col - scroll_offset_x + LINE_NUMBER_WIDTH);
	refresh();
}

void Editor::jump_cursor(const int& r, const int& c) {
	cursor.row = r;
	cursor.col = c;
	desire_col = cursor.col;
}

void Editor::jump_cursor_to_match() {
	cursor.row = current_match_row;
	cursor.col = current_match_col;
	desire_col = cursor.col;
}

void Editor::enter_search_mode() {
	search_mode = true;
}

void Editor::exit_search_mode() {
	search_mode = false;
}

void Editor::search_next() {
	if (buffer.find_next(search_term, current_match_row, current_match_col)) {
		jump_cursor_to_match();
	}

}

void Editor::search_previous() {
	if (buffer.find_prev(search_term, current_match_row, current_match_col)) {
		jump_cursor_to_match();
	}

}
void Editor::adjust_vertical_scroll(int max_visible_lines) {
    // Scroll up if cursor is above viewport
    if (cursor.row < scroll_offset_y) {
        scroll_offset_y = cursor.row;
    }
    // Scroll down if cursor is below viewport
    if (cursor.row >= scroll_offset_y + max_visible_lines) {
        scroll_offset_y = cursor.row - max_visible_lines + 1;
    }
}

void Editor::adjust_horizontal_scroll(int max_visible_width) {
    // Scroll left if cursor is left of viewport
    if (cursor.col < scroll_offset_x) {
        scroll_offset_x = cursor.col;
    }
    // Scroll right if cursor is right of viewport
    if (cursor.col >= scroll_offset_x + max_visible_width) {
        scroll_offset_x = cursor.col - max_visible_width + 1;
    }
}

void Editor::update_cursor_column_with_desired() {
    if (buffer.islengthless(cursor.row, desire_col)) {
        cursor.col = buffer.line_length(cursor.row);
    } else {
        cursor.col = desire_col;
    }
}

void Editor::move_cursor_up() {
    if (cursor.row > 0) {
        cursor.row--;
        update_cursor_column_with_desired();
    } else {
        cursor.col = 0;
        desire_col = 0;
    }
}

void Editor::move_cursor_down() {
    if (cursor.row < buffer.line_count() - 1) {
        cursor.row++;
        update_cursor_column_with_desired();
    } else {
        cursor.col = buffer.line_length(cursor.row);
        desire_col = cursor.col;
    }
}

void Editor::move_cursor_left() {
    if (cursor.col > 0) {
        cursor.col--;
        desire_col = cursor.col;
    } else if (cursor.row > 0) {
        // Wrap to end of previous line
        cursor.row--;
        cursor.col = buffer.line_length(cursor.row);
        desire_col = cursor.col;
    }
}

void Editor::move_cursor_right() {
    if (cursor.col < buffer.line_length(cursor.row)) {
        cursor.col++;
        desire_col = cursor.col;
    } else if (cursor.row < buffer.line_count() - 1) {
        // Wrap to start of next line
        cursor.row++;
        cursor.col = 0;
        desire_col = 0;
    }
}

void Editor::adjust_scroll_to_cursor() {
	int height, width;
	getmaxyx(stdscr, height, width);
	int max_visible_lines = height - 1;
	int max_visible_width = width - LINE_NUMBER_WIDTH;
	adjust_vertical_scroll(max_visible_lines);
    adjust_horizontal_scroll(max_visible_width);
}

void Editor::handle_input(int ch) {
	if (search_mode) {
		handle_search_input(ch);
		return;
	}
	// Arrow Keys
	if (ch == KEY_UP) {
		move_cursor_up();
		adjust_scroll_to_cursor();
	}

	else if (ch == KEY_DOWN) {
		move_cursor_down();
        adjust_scroll_to_cursor();
	}
	else if (ch == KEY_LEFT) {
		move_cursor_left();
        adjust_scroll_to_cursor();
		
	}
	else if (ch == KEY_RIGHT) {
		move_cursor_right();
        adjust_scroll_to_cursor();
	}
	// Search
	else if (ch == CTRL_F) {
		enter_search_mode();
	}
	// Regular character insertion
	else if (isprint(ch)) {
		buffer.insert_char(cursor.row, cursor.col, ch);
		cursor.col++;
		desire_col = cursor.col;
	}

	// Backspace
	else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
		if (cursor.col > 0) {
			buffer.delete_char(cursor.row, cursor.col);
			cursor.col--;
			desire_col = cursor.col;
		}
		else if (cursor.row > 0) {
			int prev_len = buffer.line_length(cursor.row - 1);
			buffer.join_lines(cursor.row);
			cursor.row--;
			cursor.col = prev_len;
			desire_col = cursor.col;
		}
	}

	// Enter
	else if (ch == KEY_ENTER || ch == '\n' || ch == '\r') {
		buffer.split_line(cursor.row, cursor.col);
		cursor.row++;
		cursor.col = 0;
		desire_col = cursor.col;
		adjust_scroll_to_cursor();
	}

	// undo (Ctrl+U)
	else if (ch == CTRL_U) {
		int row = -1;
		int col = -1;
		buffer.undo(row, col);
		if (row != -1 && col != -1) {
			jump_cursor(row, col);
			adjust_scroll_to_cursor();
		}
	}

	// redo (Ctrl+R)
	else if (ch == CTRL_R) {
		int row = -1;
		int col = -1;
		buffer.redo(row, col);
		if (row != -1 && col != -1) {
			jump_cursor(row, col);
			adjust_scroll_to_cursor();
		}
	}

	// Save (Ctrl+W)
	else if (ch == CTRL_W) {
		buffer.save_to_file();
	}

	// Quit (Ctrl+X)
	else if (ch == CTRL_X) {
		running = false;
	}
}

void Editor::handle_search_input(int ch) {
	// Exit Search
	if (ch == ESC) {
		exit_search_mode();
	}
	// Search next (Ctrl N)
	else if (ch == CTRL_N) {
		search_next();
		adjust_scroll_to_cursor();
	}
	// Search previous (CTRL_P)
	else if (ch == CTRL_P) {
		search_previous();
		adjust_scroll_to_cursor();
	}
	else if (isprint(ch)) {
		search_term += ch;
	}
	else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
		if (!search_term.empty()) {
			search_term.pop_back();
		}
	}
	else if (ch == KEY_ENTER || ch == '\n' || ch == '\r') {
		search_next();
		adjust_scroll_to_cursor();
	}
}

void Editor::run() {
	while (running) {
		render();
		int ch = getch();
		handle_input(ch);
	}
}
