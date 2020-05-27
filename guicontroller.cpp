#include "guicontroller.h"
#include "sFileSystem.h"
#include "gwindow.h"
#include "gbutton.h"
#include "gtextfield.h"
#include "gevent.h"
#include "glabel.h"
#include "gchooser.h"
#include "spthread.h"
#include <string>
#include "QMenu"
#include "gobjects.h"
#include "gslider.h"
#include "math.h"
#include <iostream>
#include "gtextarea.h"
#include "gtable.h"



using namespace std;


GUIcontroller::GUIcontroller(string user)
{
    runnable = true;
    current_user = user;
    gw = new GWindow(getScreenWidth(),getScreenHeight());

    _widget();

    initThread();

    init_login();
    initMain();
    run(gw);



}
GUIcontroller::~GUIcontroller() {
    runnable = false;
    gw->close();
}

void GUIcontroller::run(GWindow * gw) {
    string comd;
    while (true) {
       GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
       comd = e.getActionCommand();
       if (comd.length() > 0) break;
    }

    switch (command.get(comd))
    {
    case 0:{
        string user_name = input_name->getText();
        string user_password =  input_word->getText();
        if (!login.get(user_name).compare(user_password)) {
            current_user = user_name;
            pick_user->addItem(user_name);
            pick_user->setSelectedItem(user_name);
            wrinfo = set_reporter(reporter_wrlock, thread->init_lock(user_name), wrinfo);
        }
        break;
    }

    case 1:{
        string user = pick_user->getSelectedItem();
        wrinfo = set_reporter(reporter_wrlock, thread->wrlock(user), wrinfo);
        break;
    }

    case 2:{
        string user = pick_user->getSelectedItem();
        wrinfo = set_reporter(reporter_wrlock, thread->rdlock(user), wrinfo);
        break;
    }
    case 3:{
        string user = pick_user->getSelectedItem();
        wrinfo = set_reporter(reporter_wrlock, thread->unlock(user), wrinfo);
        break;
    }
    case 4:{
        dealFileSystem();
        break;
    }
    case 5:{
        initFileSystem();
        break;
    }
    case 6:{
        initMemory();
        break;
    }
    case 7:{
        //calculator
        break;
    }
    case 8:{
        //calendar
        break;
    }

    default:
        cout << "No matches" << endl;
    }


    gw->pause(10);
}

void GUIcontroller::dealFileSystem() {
    string operant = input_oper->getSelectedItem();
    if (!operant.compare("touch")){
        try {
            if (thread->has_wr(current_user)) {
                string filename = input_file->getText();
                GContainer * temp = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL,2,3);
                temp->setX(X/4);
                temp->setY(Y/10);
                temp->setWidth(X/3);
                temp->setHeight(Y/2);
                temp->setBackground(100);
                GTextArea * content_area = new GTextArea();
                content_area->setEditable(true);
                content_area->setWidth(1.63*X/5);
                content_area->setRows(20);
                GButton * enter = new GButton("Enter");
                enter->setActionCommand("enter");
                GButton * exit = new GButton("Exit");
                exit->setActionCommand("exit");
                GButton * revoke = new GButton("Revoke");
                revoke->setActionCommand("revoke");
                temp->addToGrid(content_area,0,0);
                temp->addToGrid(enter,1,0);
                temp->addToGrid(revoke,1,1);
                temp->addToGrid(exit,1,2);


                string complete;
                int revoke_time = 0;
                int enter_time = 0;
                string content;

                while(true) {
                    GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
                    complete = e.getActionCommand();

                    if (complete.compare("enter")==0 & enter_time == 0) {
                        revoke_time = 0;
                        content = content_area->getText();
                        fs->touch(filename,content,7);
                        enter_time += 1;
                        ls->setText(fs->ls());
                        file_info->setText(fs->file_info(filename));
                    }
                    if (complete.compare("enter")==0 & enter_time > 0){
                        fs->changeContent(filename,content);
                        file_info->setText(fs->file_info(filename));
                    }
                    if (complete.compare("revoke")==0 & content.compare(content_area->getText()) != 0) {
                        content_area->setText(content);
                    }
                    if (complete.compare("revoke")==0 & content.compare(content_area->getText()) == 0 & revoke_time == 0) {
                        revoke_time += 1;
                        fs->revoke(filename);
                        content_area->setText(fs->cat(filename));
                        file_info->setText(fs->file_info(filename));
                    }
                    if (!complete.compare("exit")){
                        file_info->clearText();
                        temp->clear();
                        temp->setVisible(false);
                        temp = nullptr;
                        break;
                    }
                }

            } else {
                set_reporter(reporter_wrlock, "You don't have right to write.", wrinfo);
            }

        } catch (ErrorException e) {
            cout << e.getMessage() << endl;
        }
    } else if (!operant.compare("cd")){
        string dir_name = input_file->getText();
        fs->cd(dir_name);
        pwd->setText(fs->pwd());
        ls->setText(fs->ls());
    } else if (!operant.compare("mkdir")){
        try {
            if (thread->has_wr(current_user)) {
                string dir_name = input_file->getText();
                fs->mkdir(dir_name);
                ls->setText(fs->ls());
            }
        } catch (ErrorException e) {
            e.getMessage();
        }
    } else if (!operant.compare("open")){
        try {
            string filename = input_file->getText();
            GContainer * temp = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL,2,3);
            temp->setX(X/4);
            temp->setY(Y/10);
            temp->setWidth(X/3);
            temp->setHeight(Y/2);
            temp->setBackground(100);
            GTextArea * content_area = new GTextArea();
            content_area->setEditable(true);
            content_area->setWidth(1.63*X/5);
            content_area->setRows(20);
            GButton * enter = new GButton("Enter");
            enter->setActionCommand("enter");
            GButton * exit = new GButton("Exit");
            exit->setActionCommand("exit");
            GButton * revoke = new GButton("Revoke");
            revoke->setActionCommand("revoke");
            temp->addToGrid(content_area,0,0);
            temp->addToGrid(enter,1,0);
            temp->addToGrid(revoke,1,1);
            temp->addToGrid(exit,1,2);

            if (thread->has_wr(current_user)) {

                string complete;
                int revoke_time = 0;
                int enter_time = 0;
                string content = fs->cat(filename);

                content_area->setText(content);

                while(true) {
                    GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
                    complete = e.getActionCommand();

                    if (complete.compare("enter")==0){
                        enter_time += 1;
                        fs->changeContent(filename,content);
                        file_info->setText(fs->file_info(filename));
                    }
                    if (complete.compare("revoke")==0 & content.compare(content_area->getText()) != 0) {
                        content_area->setText(content);
                    }
                    if (complete.compare("revoke")==0 & content.compare(content_area->getText()) == 0 & revoke_time == 0 && enter_time > 0) {
                        revoke_time += 1;
                        fs->revoke(filename);
                        content_area->setText(fs->cat(filename));
                        file_info->setText(fs->file_info(filename));
                    }
                    if (!complete.compare("exit")){
                        file_info->clearText();
                        temp->setVisible(false);
                        break;
                    }
                }

            } else if (thread->has_rd(current_user)){
                string complete;
                string content = fs->cat(filename);

                content_area->setText(content);
                content_area->setEditable(false);
                while(true) {
                    GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
                    complete = e.getActionCommand();

                    if (!complete.compare("exit")){
                        file_info->clearText();
                        temp->setVisible(false);
                        break;
                    }
                }

            }

        } catch(ErrorException e) {
                e.getMessage();
        }
    } else if(!operant.compare("remove")) {
        try {
            string filename = input_file->getText();
            if (filename.find(".") != filename.npos){
                fs->rm(filename);
                ls->setText(fs->ls());

            } else {
                fs->rm(filename, "-r");
                ls->setText(fs->ls());
            }

        } catch (ErrorException e) {
            e.getMessage();
        }
    } else if (!operant.compare("copy")) {
        try {

            string filename = input_file->getText();
            input_file->setText("Please input your target directory here. ");
            string complete;

            while (true) {
                oper->setText("copy!");
                GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
                complete = e.getActionCommand();
                if (!complete.compare("copy!")){
                    string target = input_file->getText();
                    if (filename.rfind(".")!=filename.npos){
                        fs->cp(filename, target, "-p");
                        fs->cd(target);
                        pwd->setText(fs->pwd());
                        ls->setText(fs->ls());
                        oper->setText("run");
                        break;
                    } else {
                        fs->cp(filename, target, "-r");
                        fs->cd(target);
                        pwd->setText(fs->pwd());
                        ls->setText(fs->ls());
                        oper->setText("run");
                        break;
                    }

                }

            }

        } catch (ErrorException e) {
            e.getMessage();
        }
    } else if (!operant.compare("move")) {
        try {
            string filename = input_file->getText();
            input_file->setText("Please input your target directory here. ");
            string complete;

            while (true) {
                oper->setText("move!");
                GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
                complete = e.getActionCommand();
                if (!complete.compare("move!")){
                    string target = input_file->getText();
                    if (filename.rfind(".") != filename.npos){
                        fs->mv(filename,target,"-p");
                        fs->cd(target);
                        pwd->setText(fs->pwd());
                        ls->setText(fs->ls());
                        oper->setText("run");
                        break;
                    } else {
                        fs->mv(filename,target,"-r");
                        fs->cd(target);
                        pwd->setText(fs->pwd());
                        ls->setText(fs->ls());
                        oper->setText("run");
                        break;
                    }

                }
            }
        } catch (ErrorException e) {
            e.getMessage();
        }
    } else if (!operant.compare("find")) {
        try {
            string filename = input_file->getText();
            ls->setText(fs->find(filename));
        } catch (ErrorException e) {
            e.getMessage();
        }
    }
}

string GUIcontroller::set_reporter(GTextArea * reporter, string toadd, string info){
    if (rows > 6) {
        string information = toadd;
        rows = 1;
        reporter->setText(information);
        return information;
    } else {
        rows += 1;
        info.append(toadd);
        reporter->setText(info);
        return info;
    }
}
void GUIcontroller::_widget(){
    X = gw->getWidth();
    Y = gw->getHeight();
    gw->setResizable(true);
    GImage * back_ground = new GImage("background.png");
    back_ground->sendToBack();
    gw->add(back_ground);
    gw->setTitle("Simulated Operating System");


}

void GUIcontroller::initThread() {
    command.add("wrlock", 1);
    command.add("rdlock", 2);
    command.add("unlock", 3);


    //w r lock
    GContainer * thread_lay1 = new GContainer;
    thread_lay1->setBounds(1.3*X/200,Y*0.919,0.98*X/5,1*Y/18);
    pick_user = new GChooser();
    pick_user->setBounds(X/144,Y,1.3*X/20,Y/20);
    pick_user->addItem(current_user);

    write = new GButton("write");
    write->setActionCommand("wrlock");
    read = new GButton("read");
    read ->setActionCommand("rdlock");
    unlock = new GButton("unlock");
    unlock->setActionCommand("unlock");


    thread_lay1->add(pick_user);
    thread_lay1->add(write);
    thread_lay1->add(read);
    thread_lay1->add(unlock);


    //r-w-reporter
    GContainer * thread_lay2 = new GContainer();
    reporter_wrlock = new GTextArea();
    thread_lay2->setBounds(0.208*X,Y/5*3.99,X*0.287,1.07*Y/6);
    reporter_wrlock->setRows(10);
    reporter_wrlock->setEditable(false);
    reporter_wrlock->setWidth(X*0.284);
    wrinfo = "Initialize the reader-writer lock reporter.\n";
    reporter_wrlock->setText(wrinfo);
    rows = 1;
    thread_lay2->add(reporter_wrlock);


    //thread pool reporter
    GContainer * thread_lay3 = new GContainer();
    thread_lay3->setBounds(X*0.502,Y/5*3.99,X*0.287,1.07*Y/6);
    reporter_pool = new GTextArea();
    reporter_pool->setRows(10);
    reporter_pool->setEditable(false);
    reporter_pool->setWidth(X*0.284);
    poolinfo = "Initialize the thread pool reporter.\n";
    reporter_pool->setText(poolinfo);
    thread_lay3->add(reporter_pool);




    thread = new spthread(current_user, true, false);
    wrinfo.append("Initialize a thread for ");
    wrinfo.append(current_user);
    wrinfo.append("\n");
    reporter_wrlock->setText(wrinfo);

}


void GUIcontroller::initFileSystem() {
    command.add("run", 4);

    GContainer * con_pwd = new GContainer();
    pwd = new GLabel("/root");
    QFont font;
    font.setBold(true);
    font.setPointSize(20);
    pwd->setFont(font);
    con_pwd->setBounds(X/200,0.5*Y/30,X/5,1.89/36*Y);
    con_pwd->add(pwd);

    GContainer * con_ls = new GContainer();
    ls = new GTextArea();
    ls->setRows(15);
    ls->setWidth(X/5);
    con_ls->setX(0);
    con_ls->setY(1.9*Y/30);
    con_ls->setWidth(X/5);
    con_ls->setHeight(1.4*Y/4);
    con_ls->add(ls);

    GContainer * con_info = new GContainer();
    file_info = new GTextArea();
    file_info->setRows(13);
    file_info->setWidth(X/5);
    //con_info->setBackground();
    con_info->setX(0);
    con_info->setY(0.375*Y);
    con_info->setWidth(X/5);
    con_info->setHeight(1.4*Y/4);
    con_info->add(file_info);


    GContainer * con_oper = new GContainer();
    input_file = new GTextField();
    input_file->setEditable(true);
    input_file->setWidth(10/X);
    input_file->setBackground("Blue");

    input_oper = new GChooser;
    input_oper->addItem("touch");
    input_oper->addItem("mkdir");
    input_oper->addItem("open");
    input_oper->addItem("remove");
    input_oper->addItem("copy");
    input_oper->addItem("move");
    input_oper->addItem("cd");
    input_oper->addItem("find");
    input_oper->addItem("chmod");

    oper = new GButton("run");

    input_file = new GTextField;
    input_file->setWidth(X/10);

    con_oper->setX(0.003*X);
    con_oper->setY(0.7*Y);
    con_oper->setWidth(X/5);
    con_oper->setHeight(1.4*Y/20);

    con_oper->add(input_file);
    con_oper->add(input_oper);
    con_oper->add(oper);



    fs = new sFileSystem(current_user);



}

void GUIcontroller::initMain(){
    command.add("openfs",5);
    command.add("openmemory",6);
    command.add("opencalculator",7);
    command.add("opencalendar",8);

    table = new GImage("main.png");
    table->setWidth(3*X/5);
    table->setHeight(23*Y/30);
    gw->add(table,X/5,0);
    GContainer * con_main = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL);
    con_main->setX(3.4*X/5);
    con_main->setY(Y/30);
    con_main->setWidth(X/10);
    con_main->setHeight(Y/2);
    con_main->setSpacing(Y/60);
    fileb = new GButton("","file.png");
    fileb->setActionCommand("openfs");
    memoryb = new GButton("","memory.ico");
    memoryb->setActionCommand("openmemory");
    calculatorb = new GButton("","calculator.png");
    calculatorb->setActionCommand("opencalculator");
    calendarb = new GButton("","calendar.png");
    calendarb->setActionCommand("opencalendar");
    con_main->add(calendarb);
    con_main->add(calculatorb);
    con_main->add(fileb);
    con_main->add(memoryb);


}

void GUIcontroller::init_login() {
    command.add("confirm_user", 0);
    login.add("Jacy","abc");
    login.add("Eric","abc");
    login.add("Blaine","abc");
    login.add("Yuheng","abc");

    //log in
    GContainer * thread_lay0 = new GContainer;
    thread_lay0->setX(1.3*X/200);
    thread_lay0->setY(Y/5*4.03);
    thread_lay0->setWidth(0.98*X/5);
    thread_lay0->setHeight(Y/18);
    GLabel *name = new GLabel("name:");
    input_name = new GTextField();
    GLabel *password = new GLabel("password:");
    input_word = new GTextField();


    thread_lay0->add(name);
    thread_lay0->add(input_name);
    thread_lay0->add(password);
    thread_lay0->add(input_word);

    GContainer * thread_lay01 = new GContainer;
    thread_lay01->setX(1.3*X/200);
    thread_lay01->setY(Y/5*4.2);
    thread_lay01->setWidth(0.98*X/5);
    thread_lay01->setHeight(1.4*Y/18);
    confirm_user = new GButton("Confirm");
    confirm_user->setActionCommand("confirm_user");
    thread_lay01->add(confirm_user);

    thread_lay0->add(name);
    thread_lay0->add(input_name);
    thread_lay0->add(password);
    thread_lay0->add(input_word);


}

void GUIcontroller::initMemory() {
    int Linespace = Y/25;
    int Colspace = X/10;

    // Top right Block of memory GUI
    GContainer * memory_lay_top0 = new GContainer;
    memory_lay_top0->setX(4*X/5);
    memory_lay_top0->setY(Y/200);
    memory_lay_top0->setWidth(X/5);
    memory_lay_top0->setHeight(Y/18);
    GLabel *memory_title1 = new GLabel("Memory Summary Table");
    memory_title1->setColor("Black");
    memory_lay_top0->add(memory_title1);

    GContainer * memory_lay_top1 = new GContainer;
    memory_lay_top1->setX(4*X/5);
    memory_lay_top1->setY(Y/200 + Linespace);
    memory_lay_top1->setWidth(Colspace);
    memory_lay_top1->setHeight(Linespace);
    GLabel *memory_title2 = new GLabel("Physical memory:");
    memory_title2->setColor("Black");
    memory_lay_top1->add(memory_title2);

    GContainer * memory_lay_top2 = new GContainer;
    memory_lay_top2->setX(4*X/5 + Colspace);
    memory_lay_top2->setY(Y/200 + Linespace);
    memory_lay_top2->setWidth(Colspace);
    memory_lay_top2->setHeight(Linespace);
    GLabel *memory_content1 = new GLabel("4.00GB");
    memory_content1->setColor("Black");
    memory_lay_top2->add(memory_content1);

    GContainer * memory_lay_top3 = new GContainer;
    memory_lay_top3->setX(4*X/5);
    memory_lay_top3->setY(Y/200 + Linespace*2);
    memory_lay_top3->setWidth(Colspace);
    memory_lay_top3->setHeight(Linespace);
    GLabel *memory_title3 = new GLabel("Memory used:");
    memory_title3->setColor("Black");
    memory_lay_top3->add(memory_title3);

    GContainer * memory_lay_top4 = new GContainer;
    memory_lay_top4->setX(4*X/5 + Colspace);
    memory_lay_top4->setY(Y/200 + Linespace*2);
    memory_lay_top4->setWidth(Colspace);
    memory_lay_top4->setHeight(Linespace);
    GLabel *memory_content2 = new GLabel("2.00GB/4.00GB");
    memory_content2->setColor("Black");
    memory_lay_top4->add(memory_content2);

    GContainer * memory_lay_top5 = new GContainer;
    memory_lay_top5->setX(4*X/5);
    memory_lay_top5->setY(Y/200 + Linespace*3);
    memory_lay_top5->setWidth(Colspace*1.1);
    memory_lay_top5->setHeight(Linespace);
    GLabel *memory_title4 = new GLabel("Virtual memory invoked:");
    memory_title4->setColor("Black");
    memory_lay_top5->add(memory_title4);

    GContainer * memory_lay_top6 = new GContainer;
    memory_lay_top6->setX(4*X/5 + Colspace);
    memory_lay_top6->setY(Y/200 + Linespace*3);
    memory_lay_top6->setWidth(Colspace);
    memory_lay_top6->setHeight(Linespace);
    GLabel *memory_content3 = new GLabel("No");
    memory_content3->setColor("Black");
    memory_lay_top6->add(memory_content3);

    // Top Middle Block of memory GUI
    GContainer * memory_lay_mid0 = new GContainer;
    memory_lay_mid0->setX(4*X/5);
    memory_lay_mid0->setY(Y/200*40);
    memory_lay_mid0->setWidth(X/5);
    memory_lay_mid0->setHeight(Y/18);
    GLabel *memory_title5 = new GLabel("Memory Process Table");
    memory_title5->setColor("Black");
    memory_lay_mid0->add(memory_title5);


    GContainer * memory_table = new GContainer;

    GTable *MT = new GTable(16,4);
    MT->set(1,1,"HAHAHA");
    MT->setY(Y*23/90);
//    MT->setColumnHeaderStyle(COLUMN_HEADER_NONE);
    memory_table->setX(4*X/5);
    memory_table->setY(Y*23/90);
    memory_table->setWidth(X/5);
    memory_table->setHeight(Linespace*12.5);
    memory_table->add(MT);

//    GContainer * header = new GContainer;
//    header->setX(4*X/5);
//    header->setY(Y*23/90);
//    header->setWidth(X/5);
//    header->setHeight(Linespace*12.5);
//    header->setColor("Black");


}

void GUIcontroller::update() {
    while (true){
        run(gw);
    }
}

bool GUIcontroller::isStop() {
    return !runnable;
}
