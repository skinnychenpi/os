#include "sFileSystem.h"
#include <string>
#include <map>
#include <iostream>
#include <iomanip>
#include <error.h>
#include "set.h"
#include "stack.h"
#include "vector.h"
#include "BPlus_tree.h"


/*
 * Constructor: sFileSystem
 * Usage: sFileSystem system;
 * ---------------------------------
 * Initialize a new empty sFileSystem.
 */
sFileSystem::sFileSystem()
{
}


/*
 * Constructor: sFileSystem
 * Usage: sFileSystem system = sFileSystem(user);
 * ----------------------------------------------
 * Initialize a new empty sFileSystem, set a user.
 */
sFileSystem::sFileSystem(string user){
    if (checkuser(user)) current_user = user; else error ("No such user.");
    root = new sPath("root", true);
    allpath.add(root);
    current_path = root;
    path_amount = 1;
}


/*
 * Method: checkuser
 * Usage: checkuser(user);
 * ----------------------
 * Check if the user is valid.
 */
bool sFileSystem::checkuser(string user) {
    return alluser.find(user) != alluser.end();
}

/*
 * Method: touch
 * Usage: touch(filename, content, mod);
 * -----------------------------------------------------
 * Create a file. Assign filename, content, mode, from the operation path.
 */
int sFileSystem::touch(string filename, string content, int mod){
    return touch(filename, content, mod, current_path);
};

/*
 * Method: touch
 * Usage: touch(filename, content, mod, operation_path);
 * -----------------------------------------------------
 * Helper function to create a file. Assign filename, content, mode, from the operation path.
 */
int sFileSystem::touch(string filename, string content,int mod, sPath * operation_path) {
    string key = hashfunc(filename, false);
    if (tree.search(key)) {
        Set<sPath*> target= tree.select(key,EQ).front();
        target.add(operation_path);
        tree.insert(key, target);
        sFile* nfile = new sFile(current_user, mod, filename, content, pwd(operation_path));
        operation_path -> addFile(nfile);
    } else {
        Set<sPath*> target;
        target.add(operation_path);
        tree.insert(key, target);
        sFile* nfile = new sFile(current_user, mod, filename, content, pwd(operation_path));
        operation_path -> addFile(nfile);
    }
    return 2;
}

int sFileSystem::file_info(string filename){
    if (current_path -> has_file(filename)) {
        sFile* thisfile = current_path->get_file(filename);
        string info = thisfile->get_info();
        cout << info << endl;
        return thisfile->size;
    } else {
        error("No such file to delect.");
    }
}

/*
 * Method: mkdir
 * Usage: mkdir(pathname, operation_path);
 * -----------------------------------------------------
 * Create a path. Assign pathname from the operation path.
 */
int sFileSystem::mkdir(string pathname, sPath * operating_path) {
    if (operating_path -> is_subset(pathname)) error("The path already exists.");

    string key = hashfunc(pathname, true);
    string absolute_address = pathname.append("/").append(operating_path -> get_absolute());
    Set<sPath*> target;
    if (!tree.search(key)) {
        target.insert(operating_path);
        tree.insert(key, target);
        operating_path -> addPath(absolute_address);
        path_amount += 1;
        allpath.add(new sPath(absolute_address, operating_path));
    } else {
        Set<sPath*> target = tree.select(key,EQ).front();
        target.add(operating_path);
        tree.insert(key, target);
        operating_path -> addPath(absolute_address);
        path_amount += 1;
        allpath.add(new sPath(absolute_address, operating_path));
    }
    return 2;
}

/*
 * Method: mkdir
 * Usage: mkdir(pathname);
 * -----------------------------------------------------
 * Create a path at the current directory.
 */
int sFileSystem::mkdir(string pathname){
    return mkdir(pathname, current_path);
}

/*
 * Method: rm
 * Usage: rm(goalfile);
 * ----------------------------------
 * A helper function to remove a file.
 */
int sFileSystem::rm(string goalfile) {
    rmFile(goalfile, current_path);
    return 2;
}

/*
 * Method: rmFile
 * Usage: rmFile(goalfile, operationPath);
 * ----------------------------------
 * A helper function to remove a file.
 */
void sFileSystem::rmFile(string goalfile, sPath* operationPath) {
    if (operationPath -> has_file(goalfile)) {
        operationPath -> removeFile(goalfile);
        Set<sPath*> target;
        string key = hashfunc(goalfile,false);
        if(tree.search(key)) {
            target = tree.select(key, EQ).front();
            target.remove(operationPath);
            tree.insert(key,target);
        }
    } else {
        error("No such file to delect.");
    }

}


/*
 * Method: rm
 * Usage: rm(goalfile, operationPath);
 * ----------------------------------
 * A helper function to remove a file.
 */
int sFileSystem::rm(string goal, string operants){
    if (!operants.compare("-r")) {
        if (current_path -> get_name().compare(goal) == 0) {
            error("Could not delect your current path.");
        } else if (current_path -> is_subset(goal)) {
            rmDir(goal, current_path);
        }
    } else {
        rmFile(goal, current_path);
    }
    return 2;
}

/*
 * Method: rmDir
 * Usage: rmDir(goal, operationPath);
 * ----------------------------------
 * A helper function to remove a path.
 */
void sFileSystem::rmDir(string goal, sPath* operationPath) {
    if (!goal.compare("*")) {
        for (string files : current_path->get_files()) rmFile(files, current_path);
        for (string subsets : current_path->get_subsets()) rmDir(subsets, current_path);

    } else if (operationPath -> is_subset(goal)) {
        operationPath -> removePath(goal);
        string absolute_goal = goal.append("/").append(operationPath->get_absolute());
        int location = get_location(absolute_goal);
        sPath * path_to_remove = allpath[location];
        for (string files : path_to_remove->get_files()) rmFile(files, path_to_remove);
        for (string subsets : path_to_remove->get_subsets_absolute()) rmDir(subsets,path_to_remove);

        Set<sPath*> target;
        target = tree.select(hashfunc(goal,true), EQ).front();
        try {
            if (target.contains(allpath[location])) target.remove(allpath[location]);
        } catch (ErrorException) {
            cout << "Failed to rmDir." << endl;
        }
        tree.insert(hashfunc(goal,true), target);
        allpath[location] -> ~sPath();
        allpath.remove(location);
    } else {
        error("Could not find the target directory.");
    }

}

/*
 * Method: cat
 * Usage: cat(filename);
 * ----------------------------
 * Show the content of the file.
 */
int sFileSystem::cat(string filename) {
    return current_path -> read_file(filename);
}

/*
 * Method: cpDir
 * Usage: cpDir(name, currentPath, targetPath);
 * --------------------------------------------
 * A helper function to copy a path.
 */
void sFileSystem::cpDir(string name, sPath *currentPath, sPath *targetPath) {
    string absolute_name = name.append("/").append(targetPath -> get_absolute());
    mkdir(absolute_name, targetPath);
    string origin_absolute = name.append("/").append(currentPath -> get_absolute());
    sPath * origin_path = allpath[get_location(origin_absolute)];
    for (string file : origin_path -> get_files()) cpFile(file, origin_path, allpath[get_location(absolute_name)]);
    for (string path : origin_path -> get_subsets()) cpDir(path, origin_path, allpath[get_location(absolute_name)]);
}

/*
 * Method: cpFile
 * Usage: cpFile(name, currentPath, targetPath);
 * --------------------------------------------
 * A helper function to copy a file.
 */
void sFileSystem::cpFile(string name, sPath *currentPath, sPath *targetPath) {
    sFile* origin_file = currentPath -> get_file(name);
    touch(name, origin_file -> get_content(), origin_file -> get_mod(current_user), targetPath);

}


/*
 * Method: cp
 * Usage: cp(from, to, operants);
 * --------------------------------------------
 * Copy a file or directory to another location.
 */
int sFileSystem::cp(string from, string to, string operants) {
    if (!operants.compare("-r")) {
        if (current_path -> is_subset(from) & (get_location(to) >= 0)){
            cpDir(from, current_path, allpath[get_location(to)]);
        }

    } else if (!operants.compare("-p")) {
        bool a = current_path -> has_file(from);
        bool b = (get_location(to) >= 0);
        if (current_path -> has_file(from) & (get_location(to) >= 0)) {
            cpFile(from, current_path, allpath[get_location(to)]);
        }
    } else {
        error("Wrong operants.");
    }

    return 2;
}

/*
 * Method: get_location
 * Usage: get_location(pathname);
 * ------------------------------
 * Return the path location in a pathgory.
 */
int sFileSystem::get_location(string pathname) {
    int location = -1;
    for (int i = 0; i < allpath.size(); i++) {
        if ((allpath[i] -> get_absolute().compare(pathname)) == 0) {
            location = i;
            break;
        }
    }
    return location;
}

/*
 * Method: mv
 * Usage: mv(from, to, operants);
 * ------------------------------
 * Move a file or path to another place.
 */
int sFileSystem::mv(string from, string to, string operants){
    cp(from, to, operants);
    rm(from, operants);
    return 2;
}


/*
 * Method: pwd
 * Usage: pwd();
 * ------------------------------
 * Get current directory absolute address.
 */
int sFileSystem::pwd() {
    cout << pwd(current_path) << endl;
    return 2;
}



/*
 * Method: pwd
 * Usage: pwd(thislevel);
 * ------------------------------
 * Get target directory absolute address.
 */
string sFileSystem::pwd(sPath * thislevel){
    Stack<string> parents_book;
    thislevel -> get_pwd(thislevel, parents_book);
    string output = "/";
    while (!parents_book.isEmpty()) {
        output.append(parents_book.pop());
        output.append("/");
    }
    return output;
}


/*
 * Method: cd
 * Usage: cd(goalpath, operating_path);
 * -------------------------------------
 * A helper function to change current path.
 */
int sFileSystem::cd(string goalpath, sPath* operating_path){
    if (operating_path->is_root()) {
        string absolute_address = goalpath.append("/").append(operating_path->get_absolute());
        int i = get_location(absolute_address);
        if (i >= 0) {
            current_path = allpath[i];
            return 2;
        } else {
            error("No such exist path.");
        }
    } else if (!goalpath.compare(operating_path->get_parent()->get_name())) {
        current_path = operating_path->get_parent();
        return 2;
    } else {
        string absolute_address = goalpath.append("/").append(operating_path->get_absolute());
        int i = get_location(absolute_address);
        if (i >= 0) {
            current_path = allpath[i];
            return 2;
        } else {
            error("No such exist path.");
        }
    }
}

/*
 * Method: cd
 * Usage: cd(goalpath);
 * -------------------------------------
 * Change current path to target path.
 */
int sFileSystem::cd(string goalpath) {
    return cd(goalpath, current_path);
}


/*
 * Method: ls
 * Usage: ls();
 * -------------------------------------
 * List all files and paths in the current path.
 */
int sFileSystem::ls() {
    Set<string> subsets = current_path->get_subsets();
    cout << "subfolders:" << endl;
    cout << "------------" << endl;
    for (string p : subsets) {
        cout << p << endl;
    }
    cout << "------------" << endl;
    Set<string> files = current_path->get_files();
    cout << "files:"<< endl;
    cout << "------------" << endl;
    for (string f : files) {
        cout << f << endl;
    }
    cout << "------------" << endl;
    return 2;
}


/*
 * Method: chmod
 * Usage: chmod();
 * ----------------------------
 * Change the mode for the file.
 */
int sFileSystem::chmod(string file, int mod) {
    current_path -> chmod(current_user, file, mod);
    return 2;
}


/*
 * Method: find
 * Usage: find(file);
 * -----------------------------------
 * Find the file's absolute address. There maybe some duplicated name files.
 * Also duplicated name paths together.
 */
int sFileSystem::find(string file) {
    Set<sPath*> target;
    if (!tree.search(hashfunc(file, false))){
        cout << "No such file."<< endl;
    } else {
        target = tree.select(hashfunc(file, false),EQ).front();
        try {
            cout << "Try to find the file with name "<< file << " by using B+ tree search." << endl;
            for (sPath * path : target) {
                cout << pwd(path) << endl;
            }
        } catch (ErrorException) {
            cout << "Failed in find"<<endl;
        }
    }
    return 2;
}




/*
 * Method: revoke
 * Usage: revoke(file);
 * -----------------------------------
 * Revoke the file to the previous version.
 */
int sFileSystem::revoke(string file) {
    if (current_path->has_file(file)) {
        sFile * thisfile = current_path->get_file(file);
        thisfile->revoke();
        return 2;
    } else {
        error("No such file exists.");
    }
}


/*
 * Method: hashfunc
 * Usage: hashfunc(filename, is_path);
 * -----------------------------------
 * Hash function to convert file or path into a 16-digits hashcode.
 */
string sFileSystem::hashfunc(string filename, bool is_path){
    if (!is_path){
        int length = filename.length();
        const char *y = filename.c_str();
        string code = to_string(length);
        for (int i = 0; i < length; i++) {
            code.append(to_string((int)y[i]));
        }
        code.append("0000000000000000");
        return code.substr(0,16);
    } else {
        int length = filename.length();
        const char *y = filename.c_str();
        string code = "9";
        code.append(to_string(length));
        for (int i = 0; i < length; i++) {
            code.append(to_string((int)y[i]));
        }
        code.append("000000000000000");
        return code.substr(0,16);
    }
}


set<string> sFileSystem::alluser = {"Jacy", "Yanzhang", "Xiaojie", "Yuhao", "Yuheng"};

void main(){
    sFileSystem system = sFileSystem("Jacy");
    cout << "************************" << endl;
    system.pwd();
    system.touch("foo.txt","foo",7);
    system.touch("foo2.txt","Hello world",4);
    system.touch("hello.txt","NoNoNo.",7);
    system.mkdir("dev1");
    system.mkdir("dev2");
    cout << "************************" << endl;
    //system.ls();
    system.cd("dev1");
    system.touch("foo.txt","aaaaaaaa",7);
    cout << "************************" << endl;
    //system.ls();
    //system.cd("root");
    cout << "************************" << endl;
    //system.ls();
    //system.rm("foo.txt");
    cout << "************************" << endl;
    //system.ls();
    //system.file_info("foo2.txt");
    system.find("foo.txt");
    system.mv("foo.txt","dev2/root","-p");
    cout << "************************" << endl;
    system.cd("root");
    system.cd("dev2");
    system.ls();
    system.find("foo.txt");
    cout << "************************" << endl;


}
