#include <iostream>
#include <stdio.h>
#include "os_file.h"

using namespace std;

int main ()
{   
    file_manager_t fm;
    setup_file_manager(&fm);

    // == == COMMON TEST 1
    cout << fm.create(1000);                       // expected: 1, passed
    cout << fm.create_file("test_file", 1000 + 1); // expected: 0, passed
    cout << fm.destroy();                          // expected: 1, passed
    cout << fm.destroy();                          // expected: 0, passed
    cout << fm.create(1000);                       // expected: 1, passed
    cout << fm.create(1000);                       // expected: 0, passed
    cout << fm.destroy();                          // expected: 1, passed
    // == == == == == == == == == == == == == == == ==

    // ==== COMMON TEST 2
    cout << fm.create(1000);                   // expected: 1, passed
    cout << fm.create_file("file1.txt", 1000);     // expected: 1, passed
    cout << fm.create_file("file2.txt", 1);        // expected: 0, passed
    cout << fm.remove("file1.txt", 0);             // expected: 1, passed
    cout << fm.remove("file1.txt", 0);             // expected: 0, passed
    cout << fm.create_file("file2.txt", 1000 / 2); // expected: 1, passed
    cout << fm.create_file("file2.txt", 1000 / 2); // expected: 0, passed
    cout << fm.create_file("file3.txt", 1000 / 2); // expected: 1, passed
    cout << fm.create_dir("test_dir");             // expected: 1, passed
    cout << fm.remove("file2.txt", 0);             // expected: 1, passed
    cout << fm.destroy();                          // expected: 1, passed
    // == == == == == == == == == == == == == == == ==

    // ==== COMMON TEST 3
    cout << fm.create(1000);                     // expected: 1, passed
    cout << fm.create_dir("dir1");                   // expected: 1, passed
    cout << fm.create_dir("dir1/dir11");             // expected: 1, passed
    cout << fm.change_dir("dir1");                   // expected: 1, passed
    cout << fm.create_dir("../dir2");                // expected: 1, passed
    cout << fm.create_dir("../dir2/dir3");           // expected: 1, passed
    cout << fm.remove("/dir2/dir3", 0);              // expected: 1, passed
    cout << fm.create_dir("/dir3/dir31");            // expected: 0, passed
    cout << fm.create_dir("../dir3/dir31");          // expected: 0, passed
    cout << fm.create_dir("../dir2");                // expected: 0, passed
    cout << fm.create_file("/dir2/file1", 1);        // expected: 1, passed
    cout << fm.create_dir("/dir2/dir21");            // expected: 1, passed
    cout << fm.create_dir("/dir2/file1");            // expected: 0, passed
    cout << fm.create_dir("../dir2/file1");          // expected: 0, passed
    cout << fm.create_dir("../dir2/file1/dir");      // expected: 0, passed
    cout << fm.create_dir("../dir2/dir22");          // expected: 1, passed
    cout << fm.create_dir("..");                     // expected: 0, passed
    cout << fm.create_dir("../dir2/.");              // expected: 0, passed
    cout << fm.change_dir("dir2");                   // expected: 0, passed
    cout << fm.change_dir("dir11");                  // expected: 1, passed
    cout << fm.remove("../../dir2/file1", 0);        // expected: 1, passed
    cout << fm.create_dir("../../dir2/file1");       // expected: 1, passed
    cout << fm.remove("../../dir2/file1", 0);        // expected: 1, passed
    cout << fm.create_dir("../../dir2/file1");       // expected: 1, passed
    cout << fm.remove("../../dir2/file1", 0);        // expected: 1, passed
    cout << fm.create_file("../../dir2/file1", 1);   // expected: 1, passed
    cout << fm.change_dir(".");                      // expected: 1, passed
    cout << fm.change_dir("/dir1/dir11");            // expected: 1, passed
    cout << fm.change_dir("/dir1/dir11/dir3");       // expected: 0, passed
    cout << fm.change_dir("/dir1");                  // expected: 1, passed
    cout << fm.change_dir("./dir11");                // expected: 1, passed
    cout << fm.change_dir("..");                     // expected: 1, passed
    cout << fm.create_file("ddir2/file1", 1000 - 1); // expected: 0, passed
    cout << fm.create_file("./dir11/file.txt", 1);   // expected: 1, passed
    cout << fm.remove("dir11", 0);                   // expected: 0, passed
    cout << fm.remove("./dir11", 0);                 // expected: 0, passed
    cout << fm.remove("./dir11", 1);                 // expected: 1, passed
    char* cur_dir = "";
    cout << fm.get_cur_dir(cur_dir);
    cout << (strcmp(cur_dir, "/dir1") == 0 || strcmp(cur_dir, "/dir1/") == 0); // expected: 1, passed
    cout << fm.destroy();                                                      // expected: 1, passed
    return 0;
}
