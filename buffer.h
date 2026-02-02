#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

class Command; // forward declaration

class Buffer {
    friend class InsertCharCommand;
    friend class DeleteCharCommand;
    friend class SplitLineCommand;
    friend class JoinLineCommand;

	private:
    std::vector<std::string> lines;
    std::string filename;
    bool is_modified;

    std::vector<Command*> undo_stack;
    std::vector<Command*> redo_stack; // redo is reverse an undo action

	public:
    Buffer(std::string fname);

    void load_from_file();
    void save_to_file();

    void execute_and_add_to_history(Command* cmd);
    void insert_char(int row, int col, char c);
    void delete_char(int row, int col);
    void split_line(int row, int col);
    void join_lines(int row);

    bool find_next(const std::string& term, int& r, int& c);
    bool find_prev(const std::string& term, int& r, int& c);

    void replace(const std::string& search_term, const std::string& replace_term, int& r, int& c);
    void replace_all(const std::string& search_term, const std::string& replace_term);

    void undo(int& row, int& col);
    void redo(int& row, int& col);

    void paste_textBuffer(std::vector<std::string> textBuffer, const int& r, const int& c);

    bool islengthless(int row, int l) const;

    int line_count() const;
    std::string get_line(int row) const;
    int line_length(int row) const;

    std::string get_filename() const;
    bool get_isModified() const;
};

#endif
