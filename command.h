#ifndef COMMAND_H
#define COMMAND_H

class Buffer; // forward declaration

class Command {
public:
    virtual void execute(Buffer &buffer, int& r, int& c) = 0;
    virtual void undo(Buffer &buffer, int& r, int& c) = 0;
    virtual ~Command() {}
};

#endif