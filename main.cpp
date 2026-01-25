#include <iostream>
#include "editor.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc <= 1) {
	        cout << "Usage: " << argv[0] << " <filename>" << endl;
	        return 0;
	    }

    Editor editor(argv[1]);
    editor.run();

    return 0;
}
