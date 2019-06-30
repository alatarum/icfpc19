#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include <getopt.h>

#include "main.h"
#include "cMap.h"
#include "cWorker.h"
#include "cStrategy.h"

using namespace std;

void usage(char *name)
{
    printf("Solver for ICFPC 2019. See source code for usage information.\n\n");
}


std::vector<std::string> split(std::string strToSplit, char delimeter)
{
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter))
    {
       splittedStrings.push_back(item);
    }
    return splittedStrings;
}

vector<struct coords> parse_coords(string str)
{
    std::stringstream ss1(str);
    vector<struct coords> coords_list;

    std::string item;
    while (std::getline(ss1, item, '('))
    {
        if(!std::getline(ss1, item, ')'))
        {
            cout << "parse_coords: can't parse coordinates: " << item << " in " << str << endl;
            exit(-1);
        }
        std::stringstream ss2(item);
        if(!std::getline(ss2, item, ','))
        {
            cout << "parse_coords: can't parse coordinates: " << item << " in " << str << endl;
            exit(-1);
        }
        int coord_x = stoi(item);
        if(!std::getline(ss2, item, ','))
        {
            cout << "parse_coords: can't parse coordinates: " << item << " in " << str << endl;
            exit(-1);
        }
        int coord_y = stoi(item);
        coords_list.push_back(coords(coord_x, coord_y));
    }
    return coords_list;
}
vector<vector<struct coords>> parse_coords_obstacles(string str)
{
    std::stringstream ss(str);
    vector<vector<struct coords>> obstacles;

    std::string item;
    while (std::getline(ss, item, ';'))
    {
        obstacles.push_back(parse_coords(item));
    }
    return obstacles;
}

vector<struct coords> parse_coords_boost(string str)
{
    std::stringstream ss(str);
    vector<struct coords> coords_list;

    std::string item;
    while (std::getline(ss, item, ';'))
    {
        boosters_e booster = BOOST_NONE;
        switch(item[0])
        {
            case 'B': booster = BOOST_EXT_MANIP; break;
            case 'F': booster = BOOST_FAST_WHEELS; break;
            case 'L': booster = BOOST_DRILL; break;
            case 'X': booster = BOOST_X; break;
            case 'R': booster = BOOST_RESET; break;
            case 'C': booster = BOOST_CLONE; break;
            default:
                cout << "parse_coords: can't parse booster: " <<  item << " in " << str << endl;
                exit(-1);
        }
        std::stringstream ss1(item);
        if(!std::getline(ss1, item, '('))
        {
            cout << "parse_coords: can't parse coordinates: " << item << " in " << str << endl;
            exit(-1);
        }
        if(!std::getline(ss1, item, ')'))
        {
            cout << "parse_coords: can't parse coordinates: " << item << " in " << str << endl;
            exit(-1);
        }
        std::stringstream ss2(item);
        if(!std::getline(ss2, item, ','))
        {
            cout << "parse_coords: can't parse coordinates: " << item << " in " << str << endl;
            exit(-1);
        }
        int coord_x = stoi(item);
        if(!std::getline(ss2, item, ','))
        {
            cout << "parse_coords: can't parse coordinates: " << item << " in " << str << endl;
            exit(-1);
        }
        int coord_y = stoi(item);
        coords_list.push_back(coords(coord_x, coord_y, booster));
    }
    return coords_list;
}

enum modes_e
{
    MODE_DEFAULT,
    MODE_PARSE_ONLY
};

int main(int argc, char *argv[])
{
    int opt;
    int verbose = 0;
    char *problem_file = NULL;
    char *solution_file = NULL;
    modes_e mode = MODE_DEFAULT;
    int max_steps = 1000;
    while ((opt = getopt(argc, argv, "hvi:o:n:p")) != -1)
    {
        switch (opt) {
        case 'i':
            problem_file = optarg;
            break;
        case 'o':
            solution_file = optarg;
            break;
        case 'n':
            max_steps = atoi(optarg);
            break;
        case 'p':
            mode = MODE_PARSE_ONLY;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
            usage(argv[0]);
            return 0;
        default:
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }


    string line;
    ifstream problem_f (problem_file);
    if (problem_f.is_open())
    {
        getline (problem_f,line);
        problem_f.close();
    } else {
        cout << "Unable to open file" << endl;
        return -1;
    }

    std::vector<std::string> problem = split(line, '#');
    vector<struct coords> map_border_coords = parse_coords(problem[PROB_MAP_BORDER]);
    vector<struct coords> init_location_tmp = parse_coords(problem[PROB_INIT_LOCATION]);
    vector<vector<struct coords>> obstacles_list;
    if(problem.size() > 2)
        obstacles_list = parse_coords_obstacles(problem[PROB_OBSTACLES_LIST]);
    vector<struct coords> boosters_coords;
    if(problem.size() > 3)
    {
        boosters_coords = parse_coords_boost(problem[PROB_BOOSTERS_LIST]);
    }

    if(init_location_tmp.size() != 1)
    {
        cout << "Exactly 1 point for init location expected, while parsed " << init_location_tmp.size() << endl;
        exit(-1);
    }
    struct coords init_location = init_location_tmp[0];

    if(verbose)
    {
        cout << "Init location: " << init_location.tostr() << endl;
        cout << "Map: " << endl;
        for (auto coord : map_border_coords)
        {
            coord.print();
        }
        int counter = 0;
        for (auto obstacle : obstacles_list)
        {
            counter++;
            cout << "Obstacle " << counter << ": " << endl;
            for (auto coord : obstacle)
            {
                coord.print();
            }
        }
        cout << "Boosters: " << endl;
        for (auto coord : boosters_coords)
        {
            coord.print();
        }
    }
    if(mode == MODE_PARSE_ONLY)
    {
        return 0;
    }

    auto mine_map = new cMap(map_border_coords, obstacles_list);
    mine_map->place_boosters(boosters_coords);
    if(verbose)
        mine_map->draw(init_location, vector<struct coords> (), init_location);

    auto worker = new cWorker(init_location);
    auto strategy = new cStrategy(mine_map, worker);
    int steps = 0;
    while(strategy->step(verbose))
    {
        steps++;

        if(steps > max_steps)
        {
            cout << "Can't solve this in " << max_steps << " steps." << endl;
            return -1;
        }
        if(verbose)
        {
            cout << "step "  << steps<< endl;
        }
    }

    string log = worker->dump_log();

    cout << "Solution: " << log << endl;
    cout << "Number of steps: " << steps << endl;
    ofstream solution_f(solution_file);
    if (solution_f.is_open())
    {
        solution_f << log << endl;
        solution_f.close();
    } else {
        cout << "Unable to open file" << endl;
        return -1;
    }

    return 0;
}
