#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <ncurses.h>
#include <filesystem>
#include <fstream>

#include "quest.hpp"

void readQuests(std::vector<Quest> *quests)
{
    int iter = 0;
    std::map<std::string, int> nameList;
    std::vector<std::vector<std::string> > references;
    for(const auto & entry : std::filesystem::directory_iterator("./quests"))
    {
        std::ifstream file(entry.path());
        std::string line;
        Quest buffer;
        buffer.position = iter;

        std::getline(file,line);
        buffer.name = line;
        nameList[line] = iter;
        std::getline(file,line);

        while(std::getline(file,line))
        {
            if(line == "")
                break;
            buffer.description.push_back(line);
        }

        std::vector<std::string> refBuffer;
        while(std::getline(file,line))
        {
            refBuffer.push_back(line);
        }
        references.push_back(refBuffer);

        quests->push_back(buffer);
        iter++;
    }

    iter = 0;
    for(auto item = quests->begin();item != quests->end();item++)
    {
        for(auto ref : references[iter])
        {
            item->opens.push_back(&(*quests)[nameList[ref]]);
        }
        iter++;
    }
}

std::vector<int> sortByLayers(std::vector<Quest> *quests,int depth)
{
    std::vector<std::vector<int> > layers;
    {
        std::vector<int> buffer;
        layers.insert(layers.begin(),depth+1,buffer);
    }
    for(auto item : *quests)
    {
        layers[item.layer].push_back(item.position);
    }
    std::vector<int> ret;
    for(auto item : layers)
    {
        ret.insert(ret.end(),item.begin(),item.end());
    }
    return ret;
}

int main()
{
    initscr();
    start_color();
    init_color(COLOR_BLACK,0,0,0);
    init_pair(1,COLOR_WHITE,COLOR_BLACK);
    init_pair(2,COLOR_BLACK,COLOR_WHITE);
    init_pair(3,COLOR_RED,COLOR_BLACK);
    init_pair(4,COLOR_BLACK,COLOR_RED);
    init_pair(5,COLOR_GREEN,COLOR_BLACK);
    init_pair(6,COLOR_BLACK,COLOR_GREEN);
    attron(COLOR_PAIR(1));
    curs_set(0);
    noecho();

    int divide = COLS/4;

    for(int i = 0;i < LINES;i++)
    {
        mvaddch(i,divide,'|');
    }

    std::vector<Quest> quests;
    readQuests(&quests);
    for(auto item : quests) {item.applyActivations();}
    int depth = quests[0].recalculateLayers(50);
    std::vector<int> sortReference = sortByLayers(&quests,depth);
    WINDOW *desc;
    desc = newwin(LINES,COLS-divide-1,0,divide+1);

    int selected = 0;
    int selectedQuest = -1;

    while(true)
    {
        wclear(desc);
        {
            for(int i = 0;i < sortReference.size();i++)
            {
                int color = 1;
                if(quests[sortReference[i]].activations > 0)
                    color = 3;
                else if(quests[sortReference[i]].activations == 0)
                    color = 5;
                if(selected == i)
                    color++;

                if(color != 1) {
                attroff(COLOR_PAIR(1));
                attron(COLOR_PAIR(color));}

                mvprintw(i,0,quests[sortReference[i]].name.c_str());
                printw(" ");
                printw(std::to_string(quests[sortReference[i]].layer).c_str());

                if(color != 1) {
                attroff(COLOR_PAIR(color));
                attron(COLOR_PAIR(1));}
            }
            int i = 0;
            for(auto line = quests[sortReference[selected]].description.begin();line != quests[sortReference[selected]].description.end();line++,i++)
            {
                mvwprintw(desc,i,0,line->c_str());
            }
            if(quests[sortReference[selected]].opens.size() > 0)
            {
                i++;
                mvwprintw(desc,i,0,"Opens quests:");
                i+=2;
                int shift = i;
                for(auto reference = quests[sortReference[selected]].opens.begin();reference != quests[sortReference[selected]].opens.end();reference++,i++)
                {
                    if(selectedQuest == i - shift)
                    {
                        wattroff(desc,COLOR_PAIR(1));
                        wattron(desc,COLOR_PAIR(2));
                    }
                    mvwprintw(desc,i,0,(*reference)->name.c_str());
                    if(selectedQuest == i - shift)
                    {
                        wattroff(desc,COLOR_PAIR(2));
                        wattron(desc,COLOR_PAIR(1));
                    }
                }
            }
        }
        refresh();
        wrefresh(desc);
        int c = getch();
        if(selectedQuest == -1)
        {
            if(c == KEY_UP || c == 65) {
                selected--;
                if(selected < 0) {selected = quests.size()-1;}}
            else if(c == KEY_DOWN || c == 66) {
                selected++;
                if(selected >= quests.size()) {selected = 0;}}
            else if(c == 10){
                if(quests[sortReference[selected]].opens.size() > 0) {selectedQuest = 0;}}
            else if(c == KEY_BACKSPACE || c == 127) {
                quests[sortReference[selected]].activate();}
        }
        else
        {
            if(c == KEY_UP || c == 65) {
                selectedQuest--;
                if(selectedQuest < 0) {selectedQuest = quests[sortReference[selected]].opens.size()-1;}}
            else if(c == KEY_DOWN || c == 66) {
                selectedQuest++;
                if(selectedQuest >= quests[sortReference[selected]].opens.size()) {selectedQuest = 0;}}
            else if(c == 10) {
                selected = sortReference[quests[sortReference[selected]].opens[selectedQuest]->position];
                selectedQuest = -1;}
        }
    }

    endwin();
    return 0;
}
