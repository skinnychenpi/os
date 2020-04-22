#include "sFileSystem.h"
#include <string>
#include <map>
#include <iostream>
#include <iomanip>
#include <error.h>
#include "set.h"
#include "stack.h"


sFileSystem::sFileSystem()
{

}
sFileSystem::sFileSystem(string user){
    if (checkuser(user)) current_user = user; else error ("No such user.");
    allpath = new sPath * [STORAGE];
    root = new sPath("root", true);
    allpath[0] = root;
    current_path = root;
    path_amount = 1;

}

bool sFileSystem::checkuser(string user) {
    return (alluser.find(user) != alluser.end()) ? true: false;
}

int sFileSystem::touch(string filename, string content){
    sFile nfile = sFile(current_user, 7, filename, content);
    current_path->addFile(nfile);
    return 2;
}

int sFileSystem::mkdir(string pathname) {
    if (current_path->is_subset(pathname)) error("The path already exists.");
    current_path->addPath(pathname);
    path_amount += 1;
    allpath[path_amount - 1] = new sPath(pathname, current_path);
    return 2;
}
int sFileSystem::rm(string goalfile){
    current_path->removeFile(goalfile);
    return 2;
}

int sFileSystem::rm(string goalpath, string operants) {
    if (operants.compare("-r") == 0) {
        if (current_path->get_name().compare(goalpath) == 0) error("Could not delect your current path.");
        if (current_path->is_subset(goalpath)) {
            current_path->removePath(goalpath);
            int location = 1; //我还没想好
            allpath[location] -> ~sPath();
            allpath[location] = nullptr;
        } else {
            Set<string> parent_book;
            current_path->get_all_parent(current_path->get_parent(), parent_book);
            if (parent_book.contains(goalpath)) error("Could not delete super path.");
            //写删除那个分支的所有文件夹
        }
    }
    return 2;
}
int sFileSystem::cat(string filename) {
    return current_path->read_file(filename);
}

int sFileSystem::cp(string from, string to, string operants) {
    //太难写了，一会儿再写
    return 0;
}

int sFileSystem::mv(string from, string to, string operants){
    //先cp，后delete
    return 0;
}

int sFileSystem::pwd(){
    Stack<string> parents_book;
    current_path->get_pwd(current_path, parents_book);
    string output;
    for (string i; !parents_book.isEmpty(); i = parents_book.pop()) {
        output.append(i);
        output.append("/");
    }
    cout << output << endl;
    return 2;
}



set<string> sFileSystem::alluser = {"Jacy","Yanzhang"};