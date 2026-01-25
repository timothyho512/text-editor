#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

class Buffer {
	private:
    std::vector<std::string> lines;
    std::string filename;
    bool is_modified;

	public:
    Buffer(std::string fname);

    void load_from_file();
    void save_to_file();

    void insert_char(int row, int col, char c);
    void delete_char(int row, int col);
    void split_line(int row, int col);
    void join_lines(int row);

    bool islengthless(int row, int l) const;

    int line_count() const;
    std::string get_line(int row) const;
    int line_length(int row) const;

    std::string get_filename() const;
    bool get_isModified() const;
};

#endif
