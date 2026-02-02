#include "editor.h"
#include <ncurses.h>
#include <cctype>
#include <algorithm>

using namespace std;

Editor::Editor(string filename) : buffer(filename), running(true), desire_col(0), scroll_offset_y(0), scroll_offset_x(0), current_match_row(-1), current_match_col(-1) {
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

bool Editor::is_selected(const int& r, const int& c) {
	if (cursor.row == initial_selection_row) {
		if (r == cursor.row) {
			if (cursor.col > initial_selection_col) {
				if (c <= cursor.col && c >= initial_selection_col) return true;
			}
			if (cursor.col < initial_selection_col) {
				if (c >= cursor.col && c <= initial_selection_col) return true;
			}
		}
	}
	if (cursor.row > initial_selection_row) {
		if (r >  initial_selection_row && r < cursor.row) {
			return true;
		}
		if (r == cursor.row) {
			if (c <= cursor.col) return true;
		}
		if (r == initial_selection_row) {
			if (c >= initial_selection_col) return true;
		}
	}

	if (cursor.row < initial_selection_row) {
		if (r < initial_selection_row && r > cursor.row) {
			return true;
		}
		if (r == cursor.row) {
			if (c >= cursor.col) return true;
		}
		if (r == initial_selection_row) {
			if (c <= initial_selection_col) return true;
		}
	}
	// if (cursor.row > initial_selection_row) {

	// }
	// if ((r <= cursor.row &&  c <= cursor.col) && (r >= initial_selection_row && c >= initial_selection_col)) {
	// 	return true;
	// }
	// if ((r <= initial_selection_row &&  c <= initial_selection_col) && (r >= cursor.row && c >= cursor.col)) {
	// 	return true;
	// }
	return false;
}

void Editor::render() {
	ViewportSize vp = get_view_port_size();
	clear();
	if (visual_mode) {
		for (int i = scroll_offset_y; i < buffer.line_count() && i < scroll_offset_y + vp.max_visible_lines; i++) {
			int line_number = i + 1;
			int screen_row = i - scroll_offset_y;
			string line = buffer.get_line(i);
			string visible_portion;
			if (line.length() > scroll_offset_x) {
				visible_portion = line.substr(scroll_offset_x, vp.max_visible_width);
			} else {
				visible_portion = "";  // Line is shorter than scroll offset, show nothing
			}
			if (visible_portion != "") {
				for (int c = scroll_offset_x; c < visible_portion.length(); c++) {
					char character = visible_portion[c];
					int screen_col = c - scroll_offset_x;
					if (is_selected(i, c)) {
						attron(A_REVERSE);
					}
					mvprintw(screen_row, LINE_NUMBER_WIDTH + screen_col, "%c", character);
					if (is_selected(i, c)) {
						attroff(A_REVERSE);
					}
				}
			} else {
				mvprintw(screen_row, LINE_NUMBER_WIDTH, visible_portion.c_str());
			}
			mvprintw(screen_row, 0, "%d", line_number);
		}
	} else {
		for (int i = scroll_offset_y; i < buffer.line_count() && i < scroll_offset_y + vp.max_visible_lines; i++) {
			// i + 1 is the line number shown on the left padding
			int line_number = i + 1;
			int screen_row = i - scroll_offset_y;
			string line = buffer.get_line(i);
			string visible_portion;
			if (line.length() > scroll_offset_x) {
				visible_portion = line.substr(scroll_offset_x, vp.max_visible_width);
			} else {
				visible_portion = "";  // Line is shorter than scroll offset, show nothing
			}
			mvprintw(screen_row, 0, "%d", line_number);
			mvprintw(screen_row, LINE_NUMBER_WIDTH, visible_portion.c_str());
		}
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
	if (replace_mode) {
		if (replace_navigation_mode) {
			middle_status = "Replace? (y)es (n)ext (p)previous (a)ll (q)uit";
		} else {
			middle_status = "search: " + search_term;
			middle_status = middle_status + "   " + "replace with: " + replace_term;
		}
	}
	
	if (!search_mode && !replace_mode) middle_status = "";
	string right_status = to_string(cursor.row + 1) + ":" + to_string(cursor.col + 1);
    right_status += " | " + to_string(buffer.line_count()) + " lines";

	int spaces_needed = vp.width - right_status.length() - left_status.length() - middle_status.length();
	int half_spaces_needed = spaces_needed / 2;
	if (half_spaces_needed <= 5) half_spaces_needed = 5;

	string status = left_status + string(half_spaces_needed, ' ') + middle_status + string(half_spaces_needed, ' ') + right_status;

	mvprintw(vp.height - 1, 0, status.c_str());

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

bool Editor::search_next() {
	if (search_term.empty()) {
		current_match_row = -1;
		current_match_col = -1;
		return false;
	}

	if (buffer.find_next(search_term, current_match_row, current_match_col)) {
		jump_cursor_to_match();
		return true;
	}

	// if no last matched, restart from the top of the file
	int wrap_r = 0;
	int wrap_c = 0;
	if (buffer.find_next(search_term, wrap_r, wrap_c)) {
		current_match_row = wrap_r;
		current_match_col = wrap_c;
		jump_cursor_to_match();
		return true;
	}

	current_match_row = -1;
	current_match_col = -1;
	return false;
}

bool Editor::search_previous() {
	if (search_term.empty()) {
		current_match_row = -1;
		current_match_col = -1;
		return false;
	}

	if (buffer.find_prev(search_term, current_match_row, current_match_col)) {
		jump_cursor_to_match();
		return true;
	}
	int wrap_r = buffer.line_count();
	int wrap_c = buffer.line_length(wrap_r);
	if (buffer.find_prev(search_term, wrap_r, wrap_c)) {
		current_match_row = wrap_r;
		current_match_col = wrap_c;
		jump_cursor_to_match();
		return true;
	}

	// if no last matched, find from the bottom of the file
	
	current_match_row = -1;
	current_match_col = -1;
	return false;

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
	ViewportSize vp = get_view_port_size();
	adjust_vertical_scroll(vp.max_visible_lines);
    adjust_horizontal_scroll(vp.max_visible_width);
}

void Editor::handle_input(int ch) {
	if (search_mode) {
		handle_search_input(ch);
		return;
	}
	if (visual_mode) {
		handle_visual_input(ch);
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
	else if (ch == CTRL_B) {
		enter_visual_mode();
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
	if (replace_mode) {
		handle_replace_input(ch);
		return;
	}
	// Exit Search
	if (ch == ESC) {
		exit_search_mode();
	}
	// replace mode
	else if (ch == CTRL_L) {
		if (current_match_row != -1 && current_match_col != -1) {
			enter_replace_mode();
		}
	}
	// Search next (Ctrl N)
	else if (ch == CTRL_N) {
		if (search_next()) adjust_scroll_to_cursor();
	}
	// Search previous (CTRL_P)
	else if (ch == CTRL_P) {
		if (search_previous()) {
			adjust_scroll_to_cursor();
		}
	}
	else if (isprint(ch)) {
		current_match_row = -1;
		current_match_col = -1;
		search_term += ch;
	}
	else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
		if (!search_term.empty()) {
			current_match_row = -1;
			current_match_col = -1;
			search_term.pop_back();
		}
	}
	else if (ch == KEY_ENTER || ch == '\n' || ch == '\r') {
		if (search_next()) adjust_scroll_to_cursor();
	}
}

void Editor::handle_replace_input(int ch) {
	if (replace_navigation_mode) {
		handle_navigation_input(ch);
		return;
	}
	// Exit replace
	if (ch == ESC) {
		exit_replace_mode();
	}
	else if (isprint(ch)) {
		replace_term += ch;
	}
	else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
		replace_term.pop_back();
	}
	else if (ch == KEY_ENTER || ch == '\n' || ch == '\r') {
		// user navigation
		// handle navigation input
		enter_replace_navigation_mode();
	}
}

void Editor::enter_replace_mode() {
	replace_mode = true;
}

void Editor::exit_replace_mode() {
	replace_mode = false;
}
void Editor::enter_replace_navigation_mode() {
	replace_navigation_mode = true;
}

void Editor::exit_replace_navigation_mode() {
	replace_navigation_mode = false;
}

void Editor::enter_visual_mode() {
	visual_mode = true;
	initial_selection_row = cursor.row;
	initial_selection_col = cursor.col;
}

void Editor::exit_visual_mode() {
	visual_mode = false;
}

void Editor::handle_navigation_input(int ch) {
	// four options, (y)es (n)ext (p)previous (a)ll (q)uit"
	if (ch == 'q') {
		exit_replace_navigation_mode();
	}
	else if (ch == 'y') {
		// replace
		buffer.replace(search_term, replace_term, current_match_row, current_match_col);
		if (search_next()) {
			adjust_scroll_to_cursor();
		} else {
			replace_navigation_mode = false;
			replace_mode = false;
		}
	}
	else if (ch == 'n') {
		// go to next term that matches
		if (search_next()) adjust_scroll_to_cursor();
	}
	else if (ch == 'p') {
		search_previous();
		adjust_scroll_to_cursor();
	}
	else if (ch == 'a') {
		//replace all matched
		buffer.replace_all(search_term, replace_term);
		replace_navigation_mode = false;
		replace_mode = false;
	}
}

void Editor::handle_visual_input(int ch) {
	// Exit Search
	if (ch == ESC) {
		exit_visual_mode();
	}
	// Arrow Keys
	else if (ch == KEY_UP) {
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
}

void Editor::run() {
	while (running) {
		render();
		int ch = getch();
		handle_input(ch);
	}
}

ViewportSize Editor::get_view_port_size() {
	ViewportSize vp;
    getmaxyx(stdscr, vp.height, vp.width);
    vp.max_visible_lines = vp.height - 1;  // Reserve for status bar
    vp.max_visible_width = vp.width - LINE_NUMBER_WIDTH;
    return vp;
}
