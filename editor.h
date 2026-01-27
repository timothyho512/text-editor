#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "buffer.h"

struct Cursor {
    int row;
    int col;
};

struct ViewportSize {
    int height;
    int width;
    int max_visible_lines;
    int max_visible_width;
};

enum ControlKeys {
    CTRL_F = 6,
    CTRL_N = 14,
    CTRL_P = 16,
    CTRL_R = 18,
    CTRL_U = 21,
    CTRL_W = 23,
    CTRL_X = 24,
    ESC = 27
};

class Editor {
	private:
    static const int LINE_NUMBER_WIDTH = 5;
    Buffer buffer;
    Cursor cursor;
    bool running;
    int desire_col;
    int scroll_offset_y;
    int scroll_offset_x;
    
    bool search_mode;
    std::string search_term;
    int current_match_row;
    int current_match_col;


    void init_ncurses();
    void render();
    void handle_input(int ch);
    void handle_search_input(int ch);

    void move_cursor_up();
    void move_cursor_down();
    void move_cursor_left();
    void move_cursor_right();
    void adjust_vertical_scroll(int max_visible_lines);
    void adjust_horizontal_scroll(int max_visible_width);
    void update_cursor_column_with_desired();
    void adjust_scroll_to_cursor();

    void enter_search_mode();
    void exit_search_mode();
    void search_next();
    void search_previous();
    void jump_cursor(const int& r, const int& c);
    void jump_cursor_to_match();

    ViewportSize get_view_port_size();

	public:
    Editor(std::string filename);
    ~Editor();

    void run();
};



#endif
